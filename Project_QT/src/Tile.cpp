#include "Tile.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
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
