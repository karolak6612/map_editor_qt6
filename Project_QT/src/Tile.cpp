#include "Tile.h"
#include "TileRenderer.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
#include "TableBrush.h"
#include "CarpetBrush.h"
#include "DrawingOptions.h"
#include <QPainter>
#include <QColor>
#include <QDebug>
#include <algorithm>
#include <QMutableVectorIterator>

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
}

// Destructor
Tile::~Tile() {
    delete ground_;
    ground_ = nullptr; 
    qDeleteAll(items_);
    items_.clear();
    delete creature_;
    creature_ = nullptr;
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
        // Insert item in correct Z-order position based on Tibia's layering rules
        insertItemInZOrder(item);

        if (item->parent() != this) {
           item->setParent(this);
        }
        if (item && item->isTable()) {
            setStateFlag(TileStateFlag::HasTable, true);
        }
        if (item && item->isCarpet()) {
            setStateFlag(TileStateFlag::HasCarpet, true);
        }
        setModified(true);
        emit tileChanged(x_, y_, z_);
        // visualChanged is handled by setStateFlag if the flag causes a visual change
    }
}

void Tile::insertItemInZOrder(Item* item) {
    if (!item) return;

    // Get item properties for Z-ordering
    const ItemProperties& props = ItemManager::getInstance().getItemProperties(item->getServerId());

    // Find correct insertion position based on Tibia's layering rules
    // Items are sorted by stack position (lower values drawn first)
    int insertPos = 0;
    int itemStackPos = item->getStackPos();

    for (int i = 0; i < items_.size(); ++i) {
        Item* existingItem = items_[i];
        if (existingItem && existingItem->getStackPos() > itemStackPos) {
            insertPos = i;
            break;
        }
        insertPos = i + 1;
    }

    items_.insert(insertPos, item);
}

bool Tile::removeItem(Item* item) {
    if (!item) {
        return false;
    }
    if (ground_ == item) {
        delete ground_;
        ground_ = nullptr;
        setModified(true);
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_); // Ground change is always visual
        return true;
    }

    int index = items_.indexOf(item);
    if (index != -1) {
        Item* removedItem = items_.takeAt(index);
        bool wasTable = removedItem && removedItem->isTable();
        bool wasCarpet = removedItem && removedItem->isCarpet();
        delete removedItem;

        if (wasTable) {
            if (!getTable()) {
                setStateFlag(TileStateFlag::HasTable, false);
            }
        }
        if (wasCarpet) {
            if (!getCarpet()) {
                setStateFlag(TileStateFlag::HasCarpet, false);
            }
        }
        setModified(true);
        emit tileChanged(x_, y_, z_);
        // visualChanged is handled by setStateFlag if flags changed
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

    if (item && item->isTable()) {
        if (!getTable()) {
            setStateFlag(TileStateFlag::HasTable, false);
        }
    }
    if (item && item->isCarpet()) {
        if (!getCarpet()) {
            setStateFlag(TileStateFlag::HasCarpet, false);
        }
    }
    setModified(true);
    emit tileChanged(x_, y_, z_);
    // visualChanged is handled by setStateFlag if flags changed
    return item;
}

void Tile::setGround(Item* groundItem) {
    if (ground_ == groundItem) return;
    delete ground_;
    ground_ = groundItem;
    if (ground_ && ground_->parent() != this) {
        ground_->setParent(this);
    }
    setModified(true);
    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_); // Ground change is always visual
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
    setModified(true);
    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_); // Creature change is visual
}

Spawn* Tile::spawn() const {
    return spawn_;
}

void Tile::setSpawn(Spawn* newSpawn) {
    if (spawn_ == newSpawn) return;
    spawn_ = newSpawn;
    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_); // Spawn visibility might change
}

// Task 55: Enhanced creature management implementations
void Tile::addCreature(Creature* creature) {
    if (!creature) return;

    // Add to list if not already present
    if (!creatures_.contains(creature)) {
        creatures_.append(creature);
        creature->setParent(this);
        setModified(true);
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
    }
}

void Tile::removeCreature(Creature* creature) {
    if (!creature) return;

    if (creatures_.removeOne(creature)) {
        creature->setParent(nullptr);

        // Also remove from map if present
        auto it = creatureMap_.begin();
        while (it != creatureMap_.end()) {
            if (it.value() == creature) {
                it = creatureMap_.erase(it);
            } else {
                ++it;
            }
        }

        setModified(true);
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
    }
}

void Tile::addCreature(quint32 creatureId, Creature* creature) {
    if (!creature) return;

    // Add to map
    creatureMap_[creatureId] = creature;

    // Also add to list if not already present
    if (!creatures_.contains(creature)) {
        creatures_.append(creature);
        creature->setParent(this);
    }

    setModified(true);
    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_);
}

void Tile::removeCreature(quint32 creatureId) {
    auto it = creatureMap_.find(creatureId);
    if (it != creatureMap_.end()) {
        Creature* creature = it.value();
        creatureMap_.erase(it);

        // Remove from list as well
        if (creatures_.removeOne(creature)) {
            creature->setParent(nullptr);
        }

        setModified(true);
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
    }
}

Creature* Tile::getCreature(quint32 creatureId) const {
    return creatureMap_.value(creatureId, nullptr);
}

const QList<Creature*>& Tile::getCreatures() const {
    return creatures_;
}

const QMap<quint32, Creature*>& Tile::getCreatureMap() const {
    return creatureMap_;
}

bool Tile::hasCreatures() const {
    return !creatures_.isEmpty() || creature_ != nullptr;
}

void Tile::clearCreatures() {
    // Clear map
    for (auto it = creatureMap_.begin(); it != creatureMap_.end(); ++it) {
        if (it.value()) {
            it.value()->setParent(nullptr);
        }
    }
    creatureMap_.clear();

    // Clear list
    for (Creature* creature : creatures_) {
        if (creature) {
            creature->setParent(nullptr);
        }
    }
    creatures_.clear();

    // Clear single creature
    if (creature_) {
        creature_->setParent(nullptr);
        creature_ = nullptr;
    }

    setModified(true);
    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_);
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

bool Tile::hasWall() const {
    for (Item* item : items_) {
        if (item && item->isWall()) {
            return true;
        }
    }
    return false;
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
        setModified(true);
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
        delete ground_;
        ground_ = nullptr;
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

// --- Advanced Item Access Methods ---

Item* Tile::getTopItem() const {
    // Return the topmost item based on Z-ordering (highest stack position)
    Item* topItem = ground_;
    int highestStackPos = ground_ ? ground_->getStackPos() : -1;

    for (Item* item : items_) {
        if (item && item->getStackPos() > highestStackPos) {
            topItem = item;
            highestStackPos = item->getStackPos();
        }
    }

    return topItem;
}

Item* Tile::getItemAt(int index) const {
    // Index 0 is ground, index 1+ are items in the vector
    if (index == 0) {
        return ground_;
    }
    int itemIndex = index - 1;
    if (itemIndex >= 0 && itemIndex < items_.size()) {
        return items_[itemIndex];
    }
    return nullptr;
}

int Tile::getIndexOf(Item* item) const {
    if (!item) return -1;

    if (ground_ == item) {
        return 0; // Ground is always index 0
    }

    int itemIndex = items_.indexOf(item);
    if (itemIndex != -1) {
        return itemIndex + 1; // Items start at index 1
    }

    return -1; // Item not found
}

int Tile::itemCount() const {
    return (ground_ ? 1 : 0) + items_.size();
}

int Tile::creatureCount() const {
    return (creature_ ? 1 : 0) + creatures_.size();
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
        setModified(true);
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
       if (flag == TileStateFlag::Modified && on) {
           Map* map = qobject_cast<Map*>(parent());
           if (map) {
               map->setModified(true);
           }
       }
        emit tileChanged(x_, y_, z_); 
        if (flag == TileStateFlag::Selected ||
            flag == TileStateFlag::Modified ||
            flag == TileStateFlag::Blocking ||
            flag == TileStateFlag::HasTable ||
            flag == TileStateFlag::HasCarpet ||
            flag == TileStateFlag::OptionalBorder) { // Added OptionalBorder here
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

// Task 85: Tile locking mechanism
bool Tile::isLocked() const { return hasStateFlag(TileStateFlag::Locked); }
void Tile::setLocked(bool on) {
    setStateFlag(TileStateFlag::Locked, on);
    emit tileChanged(x_, y_, z_);
}

void Tile::lock() { setLocked(true); }
void Tile::unlock() { setLocked(false); }

// --- Advanced Selection Methods ---

void Tile::select() {
    setSelected(true);
}

void Tile::deselect() {
    setSelected(false);
    // Also deselect all items on the tile
    if (ground_) {
        ground_->deselect();
    }
    for (Item* item : items_) {
        if (item) {
            item->deselect();
        }
    }
}

void Tile::selectGround() {
    if (ground_) {
        ground_->select();
    }
}

void Tile::deselectGround() {
    if (ground_) {
        ground_->deselect();
    }
}

QVector<Item*> Tile::popSelectedItems(bool ignoreTileSelected) {
    QVector<Item*> selectedItems;

    // If tile is selected and we're not ignoring it, return all items
    if (isSelected() && !ignoreTileSelected) {
        if (ground_) {
            selectedItems.append(ground_);
        }
        selectedItems.append(items_);

        // Clear the tile
        ground_ = nullptr;
        items_.clear();
        setModified(true);
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
        return selectedItems;
    }

    // Otherwise, only pop individually selected items
    if (ground_ && ground_->isSelected()) {
        selectedItems.append(ground_);
        ground_ = nullptr;
    }

    for (int i = items_.size() - 1; i >= 0; --i) {
        if (items_[i] && items_[i]->isSelected()) {
            selectedItems.append(items_.takeAt(i));
        }
    }

    if (!selectedItems.isEmpty()) {
        setModified(true);
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
    }

    return selectedItems;
}

QVector<Item*> Tile::getSelectedItems(bool unzoomed) const {
    Q_UNUSED(unzoomed) // For future use with zoom functionality

    QVector<Item*> selectedItems;

    // If tile is selected, return all items
    if (isSelected()) {
        if (ground_) {
            selectedItems.append(ground_);
        }
        selectedItems.append(items_);
        return selectedItems;
    }

    // Otherwise, only return individually selected items
    if (ground_ && ground_->isSelected()) {
        selectedItems.append(ground_);
    }

    for (Item* item : items_) {
        if (item && item->isSelected()) {
            selectedItems.append(item);
        }
    }

    return selectedItems;
}

Item* Tile::getTopSelectedItem() const {
    // Check items from top to bottom (reverse order)
    for (int i = items_.size() - 1; i >= 0; --i) {
        if (items_[i] && items_[i]->isSelected()) {
            return items_[i];
        }
    }

    // Check ground last
    if (ground_ && ground_->isSelected()) {
        return ground_;
    }

    return nullptr;
}

bool Tile::hasUniqueItem() const {
    return hasStateFlag(TileStateFlag::Unique);
}

quint32 Tile::getHouseId() const {
    return houseId_;
}
void Tile::setHouseId(quint32 id) {
    if (houseId_ != id) {
        houseId_ = id;
        setModified(true);
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_,y_,z_);
    }
}
bool Tile::isHouseTile() const {
    return houseId_ != 0;
}

// Task 66: House door ID implementation
quint8 Tile::getHouseDoorId() const {
    return houseDoorId_;
}

void Tile::setHouseDoorId(quint8 doorId) {
    if (houseDoorId_ != doorId) {
        houseDoorId_ = doorId;
        setModified(true);
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
    }
}

void Tile::addZoneId(quint16 zoneId) {
    if (!zoneIds_.contains(zoneId)) {
        zoneIds_.append(zoneId);
        std::sort(zoneIds_.begin(), zoneIds_.end());
        setModified(true);
        emit tileChanged(x_, y_, z_);
    }
}
bool Tile::removeZoneId(quint16 zoneId) {
    int removedCount = zoneIds_.removeAll(zoneId); 
    if (removedCount > 0) {
        setModified(true);
        emit tileChanged(x_, y_, z_);
        return true;
    }
    return false;
}
void Tile::clearZoneIds() {
    if (!zoneIds_.isEmpty()) {
        zoneIds_.clear();
        setModified(true);
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

    bool currentHasTable = false;
    for (Item* item : items_) {
        if (item && item->isTable()) {
            currentHasTable = true;
            break;
        }
    }
    setStateFlag(TileStateFlag::HasTable, currentHasTable);

    bool currentHasCarpet = false;
    for (Item* item : items_) {
        if (item && item->isCarpet()) {
            currentHasCarpet = true;
            break;
        }
    }
    setStateFlag(TileStateFlag::HasCarpet, currentHasCarpet);

    // Note: TileStateFlag::OptionalBorder is typically set explicitly by user actions,
    // not derived during update(). So, no specific update logic for it here.

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
    if (changed) {
        if (!getTable()) {
            setStateFlag(TileStateFlag::HasTable, false);
        } else {
             setStateFlag(TileStateFlag::HasTable, true);
        }
        if (map_param) {
             map_param->markModified();
             emit tileChanged(x_, y_, z_);
             // visualChanged is handled by setStateFlag
        } else {
             emit tileChanged(x_, y_, z_);
             // visualChanged is handled by setStateFlag
        }
    }
}
void Tile::tableize(Map* map_param) {
    if (!map_param) return;
    TableBrush::doTables(map_param, this);
    bool stillHasTable = (getTable() != nullptr);
    setStateFlag(TileStateFlag::HasTable, stillHasTable);
    // visualChanged is handled by setStateFlag
}

// Carpet specific method implementations
bool Tile::hasCarpet() const {
    return hasStateFlag(TileStateFlag::HasCarpet);
}
Item* Tile::getCarpet() const {
    for (Item* item : items_) {
        if (item && item->isCarpet()) {
            return item;
        }
    }
    return nullptr;
}
void Tile::cleanCarpets(Map* map_param, bool dontDelete) {
    bool changed = false;
    QMutableVectorIterator<Item*> it(items_);
    while (it.hasNext()) {
        Item* item = it.next();
        if (item && item->isCarpet()) {
            if (dontDelete) {
                it.remove();
            } else {
                it.remove();
                delete item;
            }
            changed = true;
        }
    }
    if (changed) {
        if (!getCarpet()) {
            setStateFlag(TileStateFlag::HasCarpet, false);
        } else {
             setStateFlag(TileStateFlag::HasCarpet, true);
        }
        if (map_param) {
             map_param->markModified();
             emit tileChanged(x_, y_, z_);
             // visualChanged is handled by setStateFlag
        } else {
             emit tileChanged(x_, y_, z_);
             // visualChanged is handled by setStateFlag
        }
    }
}
void Tile::carpetize(Map* map_param) {
    if (!map_param) return;
    CarpetBrush::doCarpets(map_param, this);
    bool stillHasCarpet = (getCarpet() != nullptr);
    setStateFlag(TileStateFlag::HasCarpet, stillHasCarpet);
    // visualChanged is handled by setStateFlag
}

// Optional Border specific methods
bool Tile::hasSetOptionalBorder() const {
    return hasStateFlag(TileStateFlag::OptionalBorder);
}

void Tile::setOptionalBorder(bool on) {
    // setStateFlag will emit tileChanged and visualChanged if the flag value actually changes.
    // The Map or BorderSystem should observe these signals to trigger border recalculation.
    setStateFlag(TileStateFlag::OptionalBorder, on);
    // Optional: Add a specific log if the flag was indeed changed to help trace border logic.
    // if (hasStateFlag(TileStateFlag::OptionalBorder) == on) { // This check is implicitly handled by setStateFlag's oldFlags check
    //     qDebug() << "Tile::setOptionalBorder flag set to" << on << "for" << mapPos() << ". Border recalculation should be triggered by observing signals.";
    // }
}

// --- Border Management Methods ---

bool Tile::hasBorders() const {
    // Check if the first items are borders (borders are typically placed at the beginning)
    return !items_.isEmpty() && items_[0]->isBorder();
}

void Tile::cleanBorders(bool dontDelete) {
    bool changed = false;

    // Remove all border items
    for (int i = items_.size() - 1; i >= 0; --i) {
        if (items_[i] && items_[i]->isBorder()) {
            Item* borderItem = items_.takeAt(i);
            if (!dontDelete) {
                delete borderItem;
            }
            changed = true;
        }
    }

    if (changed) {
        setModified(true);
        emit tileChanged(x_, y_, z_);
        emit visualChanged(x_, y_, z_);
    }
}

void Tile::addBorderItem(Item* item) {
    if (!item || !item->isBorder()) {
        qWarning() << "Tile::addBorderItem: Item is not a border item";
        return;
    }

    // Use proper Z-ordering for border items (they should be at bottom)
    insertItemInZOrder(item);
    if (item->parent() != this) {
        item->setParent(this);
    }

    setModified(true);
    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_);
}

void Tile::borderize(Map* map) {
    if (!map) {
        qWarning() << "Tile::borderize: Map parameter is null";
        return;
    }

    // This would typically call a GroundBrush::doBorders equivalent
    // For now, we'll implement a placeholder that can be extended
    qDebug() << "Tile::borderize called for" << mapPos() << "- implement GroundBrush integration";

    // TODO: Implement actual border logic when GroundBrush is available
    // GroundBrush::doBorders(map, this);
}

Map* Tile::getMap() const {
    // Placeholder: returning nullptr. Proper implementation depends on project structure.
    // qWarning() << "Tile::getMap() placeholder called. Actual implementation needed if Map context is required directly by Tile methods like setOptionalBorder for borderize calls.";
    return qobject_cast<Map*>(parent()); // A common approach if Map is the parent
}

// --- Memory and Utility Methods ---

quint32 Tile::memsize() const {
    quint32 size = sizeof(Tile);

    // Add size of ground item
    if (ground_) {
        size += ground_->memsize();
    }

    // Add size of all items
    for (const Item* item : items_) {
        if (item) {
            size += item->memsize();
        }
    }

    // Add size of creature
    if (creature_) {
        size += creature_->memsize();
    }

    // Add size of spawn
    if (spawn_) {
        size += spawn_->memsize();
    }

    // Add size of zone IDs vector
    size += zoneIds_.size() * sizeof(quint16);

    return size;
}

Tile* Tile::deepCopy(Map* map) const {
    if (!map) {
        qWarning() << "Tile::deepCopy: Map parameter is null";
        return nullptr;
    }

    // Create new tile with same coordinates
    Tile* newTile = new Tile(x_, y_, z_, map);

    // Copy ground item
    if (ground_) {
        newTile->ground_ = ground_->deepCopy();
        if (newTile->ground_) {
            newTile->ground_->setParent(newTile);
        }
    }

    // Copy all items
    for (const Item* item : items_) {
        if (item) {
            Item* newItem = item->deepCopy();
            if (newItem) {
                newItem->setParent(newTile);
                newTile->items_.append(newItem);
            }
        }
    }

    // Copy creature
    if (creature_) {
        newTile->creature_ = creature_->deepCopy();
        if (newTile->creature_) {
            newTile->creature_->setParent(newTile);
        }
    }

    // Copy spawn (spawns are typically not deep copied, just referenced)
    newTile->spawn_ = spawn_;

    // Copy all properties
    newTile->houseId_ = houseId_;
    newTile->houseDoorId_ = houseDoorId_; // Task 66: Copy house door ID
    newTile->mapFlags_ = mapFlags_;
    newTile->stateFlags_ = stateFlags_;
    newTile->zoneIds_ = zoneIds_;
    newTile->minimapColor_ = minimapColor_;

    return newTile;
}

void Tile::merge(Tile* other) {
    if (!other) {
        return;
    }

    // Merge ground (other's ground takes precedence if it exists)
    if (other->ground_) {
        delete ground_;
        ground_ = other->ground_;
        other->ground_ = nullptr;
        if (ground_) {
            ground_->setParent(this);
        }
    }

    // Merge items
    for (Item* item : other->items_) {
        if (item) {
            item->setParent(this);
            items_.append(item);
        }
    }
    other->items_.clear();

    // Merge creature (other's creature takes precedence)
    if (other->creature_) {
        delete creature_;
        creature_ = other->creature_;
        other->creature_ = nullptr;
        if (creature_) {
            creature_->setParent(this);
        }
    }

    // Merge spawn (other's spawn takes precedence)
    if (other->spawn_) {
        spawn_ = other->spawn_;
        other->spawn_ = nullptr;
    }

    // Merge flags (OR operation)
    mapFlags_ |= other->mapFlags_;
    stateFlags_ |= other->stateFlags_;

    // Merge house ID (other takes precedence if non-zero)
    if (other->houseId_ != 0) {
        houseId_ = other->houseId_;
    }

    // Task 66: Merge house door ID (other takes precedence if non-zero)
    if (other->houseDoorId_ != 0) {
        houseDoorId_ = other->houseDoorId_;
    }

    // Merge zone IDs
    for (quint16 zoneId : other->zoneIds_) {
        if (!zoneIds_.contains(zoneId)) {
            zoneIds_.append(zoneId);
        }
    }
    std::sort(zoneIds_.begin(), zoneIds_.end());

    setModified(true);
    emit tileChanged(x_, y_, z_);
    emit visualChanged(x_, y_, z_);
}

int Tile::size() const {
    return (ground_ ? 1 : 0) + items_.size();
}

// --- Property System ---

bool Tile::hasProperty(int property) const {
    // Check ground item properties
    if (ground_ && ground_->hasProperty(property)) {
        return true;
    }

    // Check all item properties
    for (const Item* item : items_) {
        if (item && item->hasProperty(property)) {
            return true;
        }
    }

    return false;
}

// --- Minimap Support ---

quint8 Tile::getMiniMapColor() const {
    if (minimapColor_ != 0xFF) {
        return minimapColor_; // Return cached color if set
    }

    // Calculate minimap color based on ground item
    if (ground_) {
        return ground_->getMiniMapColor();
    }

    return 0xFF; // Invalid/default color
}

void Tile::setMiniMapColor(quint8 color) {
    if (minimapColor_ != color) {
        minimapColor_ = color;
        setModified(true);
        // Minimap color change doesn't require visual update
        emit tileChanged(x_, y_, z_);
    }
}

void Tile::draw(QPainter* painter, const QRectF& targetScreenRect, const DrawingOptions& options) const {
    // Delegate to TileRenderer for all drawing operations
    TileRenderer::draw(this, painter, targetScreenRect, options);
}

QStringList Tile::getItemZOrderDebugInfo() const {
    QStringList debugInfo;

    if (ground_) {
        debugInfo << QString("Ground: ID=%1, StackPos=%2, TopOrder=%3")
                     .arg(ground_->getServerId())
                     .arg(ground_->getStackPos())
                     .arg(ground_->getTopOrder());
    }

    for (int i = 0; i < items_.size(); ++i) {
        Item* item = items_[i];
        if (item) {
            const ItemProperties& props = ItemManager::getInstance().getItemProperties(item->getServerId());
            debugInfo << QString("Item[%1]: ID=%2, StackPos=%3, TopOrder=%4, AlwaysOnBottom=%5, AlwaysOnTop=%6")
                         .arg(i)
                         .arg(item->getServerId())
                         .arg(item->getStackPos())
                         .arg(item->getTopOrder())
                         .arg(props.alwaysOnBottom ? "true" : "false")
                         .arg(item->isAlwaysOnTop() ? "true" : "false");
        }
    }

    return debugInfo;
}


