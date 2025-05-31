#include "Tile.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
#include "TableBrush.h" // Required for TableBrush::doTables
#include "DrawingOptions.h"
#include <QPainter>
#include <QColor>
#include <QDebug>
#include <algorithm> // For std::sort for zoneIds, and potentially std::find if needed
#include <QMutableVectorIterator> // For cleanTables

// Constructor
Tile::Tile(int x, int y, int z, QObject *parent) 
    : QObject(parent), 
      x_(x), y_(y), z_(z),
      ground_(nullptr),
      creature_(nullptr),
      spawn_(nullptr),
      houseId_(0),
      mapFlags_(TileMapFlag::NoFlag),
      stateFlags_(TileStateFlag::NoState) {
    // items_ and zoneIds_ are default initialized (empty QVector)
}

// Destructor
Tile::~Tile() {
    delete ground_;
    ground_ = nullptr; 
    qDeleteAll(items_);
    items_.clear();
    delete creature_;
    creature_ = nullptr;
    // spawn_ is not owned by Tile
}

// Coordinate Getters
MapPos Tile::mapPos() const {
    return MapPos(x_, y_, z_);
}

// Item/Creature Management
void Tile::addItem(Item* item) {
    if (!item) {
        return;
    }
    
    if (item->isGroundTile()) { 
        setGround(item);
    } else {
        items_.append(item);
        if (item->parent() != this) {
           item->setParent(this); 
        }
        // Table-specific logic
        if (item && item->isTable()) {
            setStateFlag(TileStateFlag::HasTable, true);
        }
        // End table-specific logic
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
    }
}

bool Tile::removeItem(Item* item) {
    if (!item) {
        return false;
    }
    if (ground_ == item) {
        delete ground_;
        ground_ = nullptr;
        // Table-specific logic (if ground could be a table)
        // if (item && item->isTable()) { // item here is already ground_
        //     if (!getTable()) { // Check if NO other table items remain (including items_)
        //         setStateFlag(TileStateFlag::HasTable, false);
        //     }
        // }
        // End table-specific logic
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
        return true;
    }

    int index = items_.indexOf(item);
    if (index != -1) {
        Item* removedItem = items_.takeAt(index);
        bool wasTable = removedItem && removedItem->isTable();
        delete removedItem;

        // Table-specific logic
        if (wasTable) {
            if (!getTable()) { // Check if NO other table items remain
                setStateFlag(TileStateFlag::HasTable, false);
            }
        }
        // End table-specific logic
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
        return true;
    }
    return false;
}

Item* Tile::removeItem(int index) {
    if (index < 0 || index >= items_.size()) {
        return nullptr;
    }
    Item* item = items_.takeAt(index);
    item->setParent(nullptr);

    // Table-specific logic
    if (item && item->isTable()) {
        if (!getTable()) { // Check if NO other table items remain
            setStateFlag(TileStateFlag::HasTable, false);
        }
    }
    // End table-specific logic

    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_);
    return item;
}

void Tile::setGround(Item* groundItem) {
    if (ground_ == groundItem) return;

    // Table-specific logic (if old ground was a table)
    // Item* oldGround = ground_; // Keep pointer to old ground for check after swapping

    delete ground_;
    ground_ = groundItem;
    if (ground_ && ground_->parent() != this) {
        ground_->setParent(this);
    }

    // Table-specific logic
    // bool oldGroundWasTable = oldGround && oldGround->isTable();
    // bool newGroundIsTable = ground_ && ground_->isTable();
    // if (oldGroundWasTable && !newGroundIsTable) { // If a table ground is replaced by non-table
    //     if (!getTable()) { // Check items_ as well
    //         setStateFlag(TileStateFlag::HasTable, false);
    //     }
    // } else if (newGroundIsTable) { // If new ground is a table
    //     setStateFlag(TileStateFlag::HasTable, true);
    // }
    // A general update() call at the end is safer if ground items can be tables
    // For now, assuming tables are primarily in items_ and update() will handle ground if it becomes a table.

    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_);
}

Item* Tile::getGround() const {
    return ground_;
}

const QVector<Item*>& Tile::items() const {
    return items_;
}

QVector<Item*>& Tile::items() {
    return items_;
}

Creature* Tile::creature() const {
    return creature_;
}

void Tile::setCreature(Creature* newCreature) {
    if (creature_ == newCreature) return;
    delete creature_;
    creature_ = newCreature;
    if (creature_ && creature_->parent() != this) {
        creature_->setParent(this);
    }
    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_);
}

Spawn* Tile::spawn() const {
    return spawn_;
}

void Tile::setSpawn(Spawn* newSpawn) {
    if (spawn_ == newSpawn) return;
    spawn_ = newSpawn;
    emit tileChanged(x_, y_, z_); 
}

QList<Item*> Tile::getWallItems() const {
    QList<Item*> wallItems;
    for (Item* item : items_) {
        if (item && item->isWall()) {
            wallItems.append(item);
        }
    }
    return wallItems;
}

void Tile::clearWalls() {
    bool changed = false;
    for (int i = items_.size() - 1; i >= 0; --i) {
        if (items_[i] && items_[i]->isWall()) {
            Item* removedItem = items_.takeAt(i);
            delete removedItem;
            changed = true;
        }
    }
    if (changed) {
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
    }
    qDebug() << "Tile::clearWalls called for" << mapPos();
}

void Tile::addWallItemById(quint16 wallItemId) {
    if (wallItemId == 0) {
        qWarning() << "Tile::addWallItemById: Attempted to add wall with ID 0 to" << mapPos();
        return;
    }
    Item* wallItem = new Item(wallItemId);
    if (wallItem) {
        if (!wallItem->isWall()) {
            qWarning() << "Tile::addWallItemById: Item ID" << wallItemId << "is not a wall type. Adding anyway to" << mapPos();
        }
        addItem(wallItem);
        qDebug() << "Tile::addWallItemById: Added wall ID" << wallItemId << "to" << mapPos();
    } else {
        qWarning() << "Tile::addWallItemById: Could not create item for ID" << wallItemId << "at" << mapPos();
    }
}

void Tile::removeGround() {
    if (ground_) {
        // Table-specific logic (if ground was a table)
        // bool groundWasTable = ground_ && ground_->isTable();
        delete ground_;
        ground_ = nullptr;
        // if (groundWasTable) {
        //     if (!getTable()) { // Check items_
        //         setStateFlag(TileStateFlag::HasTable, false);
        //     }
        // }
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
        qDebug() << "Tile::removeGround called for" << mapPos();
    }
}

void Tile::setGroundById(quint16 groundItemId) {
    if (groundItemId == 0) {
        removeGround();
        return;
    }
    Item* newGround = new Item(groundItemId);
    if (newGround) {
        if (!newGround->isGroundTile()) {
            qWarning() << "Tile::setGroundById: Item ID" << groundItemId << "is not a ground type for tile" << mapPos();
        }
        setGround(newGround);
        qDebug() << "Tile::setGroundById: Set ground ID" << groundItemId << "for" << mapPos();
    } else {
        qWarning() << "Tile::setGroundById: Could not create item for ID" << groundItemId << "for" << mapPos();
    }
}

int Tile::itemCount() const {
    return (ground_ ? 1 : 0) + items_.size();
}

int Tile::creatureCount() const {
    return (creature_ ? 1 : 0);
}

bool Tile::isEmpty() const {
    return !ground_ && items_.isEmpty() && !creature_;
}

Item* Tile::getTopLookItem() const {
    Item* topItem = nullptr;
    int currentTopOrder = -1;

    if (ground_ && ground_->getTopOrder() > currentTopOrder) {
        topItem = ground_;
        currentTopOrder = ground_->getTopOrder();
    }
    for (Item* item : items_) {
        if (item && item->getTopOrder() >= currentTopOrder) {
            topItem = item;
            currentTopOrder = item->getTopOrder();
        }
    }
    return topItem;
}

Item* Tile::getTopUseItem() const {
    for (int i = items_.size() - 1; i >= 0; --i) {
        if (items_[i] && items_[i]->isUseable()) return items_[i];
    }
    if (ground_ && ground_->isUseable()) return ground_;
    return nullptr; 
}

Item* Tile::getTopSelectableItem() const {
     for (int i = items_.size() - 1; i >= 0; --i) {
        if (items_[i] && items_[i]->isSelectable()) return items_[i];
    }
    if (ground_ && ground_->isSelectable()) return ground_;
    return nullptr;
}

void Tile::setMapFlag(TileMapFlag flag, bool on) {
    TileMapFlags oldFlags = mapFlags_;
    mapFlags_.setFlag(flag, on);
    if (oldFlags != mapFlags_) {
        emit tileChanged(x_, y_, z_);
    }
}

bool Tile::hasMapFlag(TileMapFlag flag) const {
    return mapFlags_.testFlag(flag);
}

Tile::TileMapFlags Tile::getMapFlags() const {
    return mapFlags_;
}

void Tile::setStateFlag(TileStateFlag flag, bool on) {
    TileStateFlags oldFlags = stateFlags_;
    stateFlags_.setFlag(flag, on);
     if (oldFlags != stateFlags_) {
        emit tileChanged(x_, y_, z_); 
        if (flag == TileStateFlag::Selected || flag == TileStateFlag::Modified || flag == TileStateFlag::Blocking || flag == TileStateFlag::HasTable) { // Added HasTable
             emit visualChanged(x_,y_,z_);
        }
    }
}

bool Tile::hasStateFlag(TileStateFlag flag) const {
    return stateFlags_.testFlag(flag);
}

Tile::TileStateFlags Tile::getStateFlags() const {
    return stateFlags_;
}

bool Tile::isBlocking() const {
    if (hasStateFlag(TileStateFlag::Blocking)) return true;
    if (ground_ && ground_->isBlocking()) return true;
    for (Item* item : items_) {
        if (item && item->isBlocking()) return true;
    }
    return false;
}

bool Tile::isPZ() const { return hasMapFlag(TileMapFlag::ProtectionZone); }
void Tile::setPZ(bool on) { setMapFlag(TileMapFlag::ProtectionZone, on); }

bool Tile::isNoPVP() const { return hasMapFlag(TileMapFlag::NoPVP); }
void Tile::setNoPVP(bool on) { setMapFlag(TileMapFlag::NoPVP, on); }

bool Tile::isNoLogout() const { return hasMapFlag(TileMapFlag::NoLogout); }
void Tile::setNoLogout(bool on) { setMapFlag(TileMapFlag::NoLogout, on); }

bool Tile::isPVPZone() const { return hasMapFlag(TileMapFlag::PVPZone); }
void Tile::setPVPZone(bool on) { setMapFlag(TileMapFlag::PVPZone, on); }

bool Tile::isModified() const { return hasStateFlag(TileStateFlag::Modified); }
void Tile::setModified(bool on) { setStateFlag(TileStateFlag::Modified, on); }

bool Tile::isSelected() const { return hasStateFlag(TileStateFlag::Selected); }
void Tile::setSelected(bool on) { setStateFlag(TileStateFlag::Selected, on); }

quint32 Tile::getHouseId() const {
    return houseId_;
}
void Tile::setHouseId(quint32 id) {
    if (houseId_ != id) {
        houseId_ = id;
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_,y_,z_);
    }
}
bool Tile::isHouseTile() const {
    return houseId_ != 0;
}

void Tile::addZoneId(quint16 zoneId) {
    if (!zoneIds_.contains(zoneId)) {
        zoneIds_.append(zoneId);
        std::sort(zoneIds_.begin(), zoneIds_.end());
        emit tileChanged(x_, y_, z_);
    }
}
bool Tile::removeZoneId(quint16 zoneId) {
    int removedCount = zoneIds_.removeAll(zoneId); 
    if (removedCount > 0) {
        emit tileChanged(x_, y_, z_);
        return true;
    }
    return false;
}
void Tile::clearZoneIds() {
    if (!zoneIds_.isEmpty()) {
        zoneIds_.clear();
        emit tileChanged(x_, y_, z_);
    }
}
const QVector<quint16>& Tile::getZoneIds() const {
    return zoneIds_;
}
bool Tile::hasZoneId(quint16 zoneId) const {
    return zoneIds_.contains(zoneId);
}
    
void Tile::update() {
    qDebug() << "Tile::update() called for tile at" << x_ << "," << y_ << "," << z_;
    
    bool newBlockingState = false;
    if (ground_ && ground_->isBlocking()) newBlockingState = true;
    if (!newBlockingState) {
        for (Item* item : items_) {
            if (item && item->isBlocking()) {
                newBlockingState = true;
                break;
            }
        }
    }
    setStateFlag(TileStateFlag::Blocking, newBlockingState);

    // Update HasTable state
    bool currentHasTable = false;
    for (Item* item : items_) {
        if (item && item->isTable()) {
            currentHasTable = true;
            break;
        }
    }
    // If ground items can be tables, include this:
    // if (!currentHasTable && ground_ && ground_->isTable()) {
    //    currentHasTable = true;
    // }
    setStateFlag(TileStateFlag::HasTable, currentHasTable);

    emit visualChanged(x_, y_, z_);
}

// Table specific method implementations
bool Tile::hasTable() const {
    return hasStateFlag(TileStateFlag::HasTable);
}

Item* Tile::getTable() const {
    for (Item* item : items_) {
        if (item && item->isTable()) {
            return item;
        }
    }
    // if (ground_ && ground_->isTable()) { // If ground can be a table
    //     return ground_;
    // }
    return nullptr;
}

void Tile::cleanTables(Map* map_param, bool dontDelete) {
    bool changed = false;
    QMutableVectorIterator<Item*> it(items_);
    while (it.hasNext()) {
        Item* item = it.next();
        if (item && item->isTable()) {
            if (dontDelete) {
                it.remove();
            } else {
                it.remove();
                delete item;
            }
            changed = true;
        }
    }

    // If ground can be a table:
    // if (ground_ && ground_->isTable()) {
    //     if (dontDelete) {
    //         // ground_ = nullptr; // Detach without deleting if caller handles it
    //     } else {
    //         delete ground_;
    //         ground_ = nullptr;
    //     }
    //     changed = true;
    // }

    if (changed) {
        // Re-evaluate HasTable flag based on remaining items
        if (!getTable()) {
            setStateFlag(TileStateFlag::HasTable, false);
        } else {
             setStateFlag(TileStateFlag::HasTable, true); // Should be redundant if getTable() is now null
        }

        if (map_param) {
             map_param->markModified();
             emit tileChanged(x_, y_, z_);
             emit visualChanged(x_, y_, z_);
        } else { // Still emit local signals if map context is not available
             emit tileChanged(x_, y_, z_);
             emit visualChanged(x_, y_, z_);
        }
    }
}

void Tile::tableize(Map* map_param) {
    if (!map_param) {
        // qDebug() << "Tile::tableize called with null map for tile at" << mapPos();
        return;
    }
    TableBrush::doTables(map_param, this);

    bool stillHasTable = (getTable() != nullptr);
    setStateFlag(TileStateFlag::HasTable, stillHasTable);
    
    emit visualChanged(x_, y_, z_);
}

// Placeholder for Tile::getMap() - actual implementation depends on how Tile knows its Map
// For example, if Map is always the parent QObject:
// Map* Tile::getMap() const { return qobject_cast<Map*>(parent()); }
// Or if it's passed around/stored differently.
// This is not strictly needed for the current changes as map context is passed to tableize/cleanTables.

void Tile::draw(QPainter* painter, const QRectF& targetScreenRect, const DrawingOptions& options) const {
    if (!painter) return;

    if (options.highlightSelectedTile && isSelected()) {
        painter->save();
        QColor selectionColor = Qt::yellow;
        selectionColor.setAlpha(80);
        painter->fillRect(targetScreenRect, selectionColor);
        QPen pen(Qt::yellow, 1);
        pen.setStyle(Qt::DotLine);
        painter->setPen(pen);
        painter->drawRect(targetScreenRect);
        painter->restore();
    }

    if (options.showGround && ground_) {
        DrawingOptions groundOptions = options;
        ground_->draw(painter, targetScreenRect, groundOptions);
    } else if (options.showGround) {
        painter->save();
        painter->fillRect(targetScreenRect, QColor(50, 50, 50, 100));
        painter->restore();
    }

    if (options.showItems) {
        for (Item* item : items_) {
            if (item) {
                DrawingOptions itemOptions = options;
                item->draw(painter, targetScreenRect, itemOptions);
            }
        }
    }

    if (options.showCreatures && creature_) {
        creature_->draw(painter, targetScreenRect, options);
    }

    if (options.showSpawns && spawn_) {
        painter->save();
        painter->setBrush(QColor(128, 0, 128, 100));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(targetScreenRect.topLeft() + QPointF(2,2), 4, 4); 
        painter->restore();
    }
    
    if (options.showTileFlags) {
        QString flagsText;
        if (hasMapFlag(TileMapFlag::ProtectionZone)) flagsText += "PZ ";
        if (hasMapFlag(TileMapFlag::NoPVP)) flagsText += "NoPvP ";
        if (hasMapFlag(TileMapFlag::PVPZone)) flagsText += "PvP ";
        
        if (!flagsText.isEmpty()) {
            painter->save();
            painter->setPen(Qt::white);
            QFont font = painter->font();
            font.setPointSize(font.pointSize() - 2 > 0 ? font.pointSize() - 2 : 6);
            painter->setFont(font);
            painter->drawText(targetScreenRect, Qt::AlignBottom | Qt::AlignHCenter, flagsText.trimmed());
            painter->restore();
        }
    }

    if (options.drawDebugInfo) {
        painter->save();
        QFont font = painter->font();
        font.setPointSize(7);
        painter->setFont(font);
        painter->setPen(Qt::cyan);
        QString coordText = QString("%1,%2,%3").arg(x_).arg(y_).arg(z_);
        painter->drawText(targetScreenRect.adjusted(2,2,0,0), Qt::AlignTop | Qt::AlignLeft | Qt::TextDontClip, coordText);
        painter->restore();
    }
}
