#include "Tile.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
#include "DrawingOptions.h" // Already included via Tile.h but good for clarity
#include <QPainter>
#include <QColor>
#include <QDebug>
#include <algorithm> // For std::sort for zoneIds, and potentially std::find if needed

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
    qDeleteAll(items_); // Deletes all QObject* (or derived) in the QVector
    items_.clear();
    delete creature_;
    creature_ = nullptr;
    // spawn_ is not owned by Tile, so it's not deleted here.
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
        setGround(item); // setGround handles ownership and signals
    } else {
        items_.append(item);
        if (item->parent() != this) { // Manage ownership if item is not already parented to this tile
           item->setParent(this); 
        }
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
    }
}

bool Tile::removeItem(Item* item) {
    if (!item) {
        return false;
    }
    if (ground_ == item) {
        delete ground_; // This will also set ground_->setParent(nullptr) implicitly if it was a QObject
        ground_ = nullptr;
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
        return true;
    }
    // For items in items_ vector
    int index = items_.indexOf(item);
    if (index != -1) {
        Item* removedItem = items_.takeAt(index); // Removes from vector
        delete removedItem; // Tile owns items in items_ vector
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
        return true;
    }
    return false;
}

Item* Tile::removeItem(int index) { // Note: Qt uses int for QVector indices, not int_fast8_t
    if (index < 0 || index >= items_.size()) {
        return nullptr;
    }
    Item* item = items_.takeAt(index); // Removes and returns, ownership transferred to caller
    item->setParent(nullptr); // Clear parent if it was this tile
    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_);
    return item;
}

void Tile::setGround(Item* groundItem) {
    if (ground_ == groundItem) return;
    delete ground_; // Delete previous ground item
    ground_ = groundItem;
    if (ground_ && ground_->parent() != this) {
        ground_->setParent(this); // Tile takes ownership
    }
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
    delete creature_; // Delete previous creature
    creature_ = newCreature;
    if (creature_ && creature_->parent() != this) {
        creature_->setParent(this); // Tile takes ownership
    }
    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_);
}

Spawn* Tile::spawn() const {
    return spawn_;
}

void Tile::setSpawn(Spawn* newSpawn) {
    if (spawn_ == newSpawn) return;
    // Tile does NOT own spawn_
    spawn_ = newSpawn;
    emit tileChanged(x_, y_, z_); 
    // Visual change for spawn might depend on editor settings (e.g., show spawns overlay)
    // Consider if visualChanged is appropriate or if a specific spawnChanged signal is better.
    // For now, assuming general tileChanged covers it.
}

// --- New methods for command interactions ---

QList<Item*> Tile::getWallItems() const {
    QList<Item*> wallItems;
    // Ground is typically not a wall, but some specific ground items might be.
    // Iterate through items_ list to find items that are walls.
    for (Item* item : items_) {
        if (item && item->isWall()) { // Assumes Item::isWall() exists
            wallItems.append(item);
        }
    }
    // If your game logic allows ground items to also function as walls (e.g. cave walls):
    // if (ground_ && ground_->isWall()) {
    //     wallItems.append(ground_);
    // }
    return wallItems;
}

void Tile::clearWalls() {
    bool changed = false;
    // Iterate backwards for safe removal from QVector
    for (int i = items_.size() - 1; i >= 0; --i) {
        if (items_[i] && items_[i]->isWall()) { // Assumes Item::isWall()
            Item* removedItem = items_.takeAt(i);
            delete removedItem; // Tile owns items in items_
            changed = true;
        }
    }
    // If ground can be a wall:
    // if (ground_ && ground_->isWall()) {
    //     delete ground_;
    //     ground_ = nullptr;
    //     changed = true;
    // }
    if (changed) {
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_); // Wall changes definitely affect visuals
    }
    qDebug() << "Tile::clearWalls called for" << mapPos();
}

void Tile::addWallItemById(quint16 wallItemId) {
    if (wallItemId == 0) { // 0 might mean "no item" or "invalid item"
        qWarning() << "Tile::addWallItemById: Attempted to add wall with ID 0 to" << mapPos();
        return;
    }
    // Item creation logic:
    // Option 1: Direct creation (requires Item to have appropriate constructor and type info)
    Item* wallItem = new Item(wallItemId); // Assumes Item(quint16 id) constructor sets up the item
                                          // and that Item::isWall() can be determined from this.
    // Option 2: ItemFactory (safer if item properties are complex)
    // Item* wallItem = ItemFactory::getInstance()->createItem(wallItemId);

    if (wallItem) {
        // Optional: Check if the created item is actually a wall type
        if (!wallItem->isWall()) {
            qWarning() << "Tile::addWallItemById: Item ID" << wallItemId << "is not a wall type. Adding anyway to" << mapPos();
            // If strict, delete wallItem and return.
            // delete wallItem;
            // return;
        }
        addItem(wallItem); // addItem handles parenting, signals, and adding to items_ or ground_
                           // If addItem distinguishes ground/non-ground, ensure wallItem is not ground type
                           // or that addItem correctly places it in items_ if it's a wall.
                           // Current addItem puts ground types as ground_. Walls are usually not ground.
                           // So this should add to items_ if wallItem->isGroundTile() is false.
        qDebug() << "Tile::addWallItemById: Added wall ID" << wallItemId << "to" << mapPos();
    } else {
        qWarning() << "Tile::addWallItemById: Could not create item for ID" << wallItemId << "at" << mapPos();
    }
}

void Tile::removeGround() {
    if (ground_) {
        delete ground_;
        ground_ = nullptr;
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
        qDebug() << "Tile::removeGround called for" << mapPos();
    }
}

void Tile::setGroundById(quint16 groundItemId) {
    if (groundItemId == 0) { // ID 0 means remove ground
        removeGround();
        return;
    }
    // Item creation logic:
    // Option 1: Direct creation
    Item* newGround = new Item(groundItemId); // Assumes Item(quint16 id) constructor
    // Option 2: ItemFactory
    // Item* newGround = ItemFactory::getInstance()->createItem(groundItemId);

    if (newGround) {
        // Optional: Check if it's a valid ground type
        if (!newGround->isGroundTile()) { // Assuming Item::isGroundTile()
            qWarning() << "Tile::setGroundById: Item ID" << groundItemId << "is not a ground type for tile" << mapPos();
            // delete newGround; // Avoid setting non-ground as ground
            // return; // Or set it anyway if this check is not strict
        }
        setGround(newGround); // setGround(Item*) handles deletion of old, parenting, signals
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
    // More complex logic needed based on item properties (topOrder, etc.)
    // Simple version: last non-ground item, else ground.
    // Top order: ground (0), items on ground (1), creatures/top items (2), items always on top (3)
    Item* topItem = nullptr;
    int currentTopOrder = -1;

    if (ground_ && ground_->getTopOrder() > currentTopOrder) {
        topItem = ground_;
        currentTopOrder = ground_->getTopOrder();
    }
    for (Item* item : items_) {
        if (item && item->getTopOrder() >= currentTopOrder) { // >= to prefer items over ground if same order
            topItem = item;
            currentTopOrder = item->getTopOrder();
        }
    }
    // Creature is conceptually on top of items for looking
    // if (creature_ && ??? creature_top_order > currentTopOrder) { ... }
    return topItem;
}

Item* Tile::getTopUseItem() const {
    // Iterate backwards through items_ to find first useable item
    for (int i = items_.size() - 1; i >= 0; --i) {
        if (items_[i] && items_[i]->isUseable()) return items_[i];
    }
    if (ground_ && ground_->isUseable()) return ground_;
    return nullptr; 
}

Item* Tile::getTopSelectableItem() const {
    // Iterate backwards through items_ to find first selectable item
     for (int i = items_.size() - 1; i >= 0; --i) {
        if (items_[i] && items_[i]->isSelectable()) return items_[i];
    }
    if (ground_ && ground_->isSelectable()) return ground_;
    return nullptr;
}


// Flag/Property Management
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
        if (flag == TileStateFlag::Selected || flag == TileStateFlag::Modified || flag == TileStateFlag::Blocking) {
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
    if (hasStateFlag(TileStateFlag::Blocking)) return true; // Explicitly set as blocking
    if (ground_ && ground_->isBlocking()) return true;
    for (Item* item : items_) {
        if (item && item->isBlocking()) return true;
    }
    // if (creature_) return true; // Creatures usually block
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

// House ID
quint32 Tile::getHouseId() const {
    return houseId_;
}
void Tile::setHouseId(quint32 id) {
    if (houseId_ != id) {
        houseId_ = id;
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_,y_,z_); // House state often has visual representation
    }
}
bool Tile::isHouseTile() const {
    return houseId_ != 0;
}

// Zone IDs
void Tile::addZoneId(quint16 zoneId) {
    if (!zoneIds_.contains(zoneId)) {
        zoneIds_.append(zoneId);
        // std::sort(zoneIds_.begin(), zoneIds_.end()); // QVector doesn't have .begin()/.end() for std::sort directly
        // Use qSort if sorting is desired
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
    // Placeholder for recalculating derived states:
    // - TILESTATE_BLOCKING based on items and their properties.
    // - Minimap color.
    // - TILESTATE_HAS_TABLE, TILESTATE_HAS_CARPET, TILESTATE_UNIQUE etc.
    
    // Example: Update blocking state (simplified)
    bool newBlockingState = false;
    if (ground_ && ground_->isBlocking()) newBlockingState = true;
    if (!newBlockingState) { // only check items if ground doesn't block
        for (Item* item : items_) {
            if (item && item->isBlocking()) {
                newBlockingState = true;
                break;
            }
        }
    }
    // if (creature_) newBlockingState = true; // If creatures always block
    setStateFlag(TileStateFlag::Blocking, newBlockingState);
    
    emit visualChanged(x_, y_, z_); // Always emit visual changed after an update
}

void Tile::draw(QPainter* painter, const QRectF& targetScreenRect, const DrawingOptions& options) const {
    if (!painter) return;

    // 0. Optional: Highlight selected tile (as per DrawingOptions)
    if (options.highlightSelectedTile && isSelected()) {
        painter->save();
        // Use a distinct color for selection, e.g., yellow or a bright semi-transparent overlay
        QColor selectionColor = Qt::yellow;
        selectionColor.setAlpha(80); // Semi-transparent overlay
        painter->fillRect(targetScreenRect, selectionColor);

        QPen pen(Qt::yellow, 1); // Thin border for selected tile
        pen.setStyle(Qt::DotLine);
        painter->setPen(pen);
        painter->drawRect(targetScreenRect);
        painter->restore();
    }

    // 1. Draw Ground
    if (options.showGround && ground_) {
        // Make a copy of options, in case ground_->draw modifies it (though it shouldn't for const ref)
        DrawingOptions groundOptions = options;
        // Potentially adjust options specifically for ground if needed, e.g. groundOptions.someFlag = false;
        ground_->draw(painter, targetScreenRect, groundOptions);
    } else if (options.showGround) {
        // Draw a base placeholder for the tile itself if ground is null but should be shown
        painter->save();
        painter->fillRect(targetScreenRect, QColor(50, 50, 50, 100)); // Dark semi-transparent gray
        painter->restore();
    }

    // 2. Draw Items (items_ should be in correct visual order: bottom-most first)
    if (options.showItems) {
        for (Item* item : items_) {
            if (item) {
                // TODO: Future checks based on options.showInvisibleItems and item properties
                DrawingOptions itemOptions = options;
                // Example: Adjust targetRect for item if items are not full tile size
                item->draw(painter, targetScreenRect, itemOptions); // Assuming items draw relative to tile's full rect for now
            }
        }
    }

    // 3. Draw Creature
    if (options.showCreatures && creature_) {
        // TODO: Future checks based on options.showInvisibleItems (if creatures can be invisible)
        creature_->draw(painter, targetScreenRect, options);
    } // else if (options.showCreatures && options.drawDebugInfo) {
      // Optionally draw a placeholder for where a creature *would* be if debug info is on for creatures
    //}

    // 4. Draw Spawn Indicator (placeholder)
    if (options.showSpawns && spawn_) {
        painter->save();
        painter->setBrush(QColor(128, 0, 128, 100)); // Semi-transparent purple
        painter->setPen(Qt::NoPen);
        // Draw a small circle or symbol for spawn in a corner or center
        // Example: small circle at the top-left corner
        painter->drawEllipse(targetScreenRect.topLeft() + QPointF(2,2), 4, 4); 
        // Or centered: painter->drawEllipse(targetScreenRect.center(), targetScreenRect.width() / 5, targetScreenRect.height() / 5);
        painter->restore();
        // qDebug() << "Tile::draw() Spawn indicator for tile at" << x_ << y_ << z_;
    }
    
    // 5. Draw Tile Flags (e.g., PZ text)
    if (options.showTileFlags) {
        QString flagsText;
        if (hasMapFlag(TileMapFlag::ProtectionZone)) {
            flagsText += "PZ ";
        }
        if (hasMapFlag(TileMapFlag::NoPVP)) {
            flagsText += "NoPvP ";
        }
        if (hasMapFlag(TileMapFlag::PVPZone)) {
            flagsText += "PvP ";
        }
        // Add more flags as needed, e.g., NoLogout, ZoneBrush (if it has text)
        
        if (!flagsText.isEmpty()) {
            painter->save();
            painter->setPen(Qt::white); // Text color for flags
            // Adjust font size if needed
            QFont font = painter->font();
            font.setPointSize(font.pointSize() - 2 > 0 ? font.pointSize() - 2 : 6); // Smaller font
            painter->setFont(font);
            // Draw text at bottom of tile, centered or left-aligned
            painter->drawText(targetScreenRect, Qt::AlignBottom | Qt::AlignHCenter, flagsText.trimmed());
            painter->restore();
            // qDebug() << "Tile::draw() Flags:" << flagsText.trimmed() << "for tile at" << x_ << y_ << z_;
        }
    }

    // 6. Draw Debug Info (Coordinates)
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
    
    // Future: Draw effects (if options.showEffects)
}
