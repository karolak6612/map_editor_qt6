#include "HouseBrush.h"
#include "House.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include <QDebug>
#include <QUndoCommand>
#include <QDomElement>

// HouseBrush implementation
HouseBrush::HouseBrush(QObject* parent)
    : Brush(parent),
      name_("House Brush"),
      currentHouse_(nullptr),
      currentHouseId_(0),
      removeItems_(true),
      autoAssignDoorId_(true) {
}

HouseBrush::~HouseBrush() {
    // Don't delete currentHouse_ as we don't own it
}

// Brush interface implementation
Brush::Type HouseBrush::type() const {
    return Type::House;
}

QString HouseBrush::name() const {
    return name_;
}

void HouseBrush::setName(const QString& newName) {
    if (name_ != newName) {
        name_ = newName;
        emit brushChanged();
    }
}

int HouseBrush::getLookID() const {
    // Return a representative house item ID for UI display
    return 1209; // Door ID as representative
}

// Type identification
bool HouseBrush::isHouse() const {
    return true;
}

HouseBrush* HouseBrush::asHouse() {
    return this;
}

const HouseBrush* HouseBrush::asHouse() const {
    return this;
}

// Core brush actions
bool HouseBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(drawingContext);
    
    if (!map || !currentHouse_) {
        return false;
    }
    
    // Check if position is valid
    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());
    
    if (!map->isValidCoordinate(x, y, 0)) {
        return false;
    }
    
    return true;
}

QUndoCommand* HouseBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    
    if (!canDraw(map, tilePos)) {
        return nullptr;
    }
    
    return new HouseBrushCommand(map, tilePos, currentHouse_, true, parentCommand);
}

QUndoCommand* HouseBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    
    if (!map) {
        return nullptr;
    }
    
    return new HouseBrushCommand(map, tilePos, nullptr, false, parentCommand);
}

// House-specific properties
House* HouseBrush::getHouse() const {
    return currentHouse_;
}

void HouseBrush::setHouse(House* house) {
    if (currentHouse_ != house) {
        currentHouse_ = house;
        currentHouseId_ = house ? house->getId() : 0;
        emit houseChanged(house);
        emit brushChanged();
    }
}

quint32 HouseBrush::getHouseId() const {
    return currentHouseId_;
}

void HouseBrush::setHouseId(quint32 houseId) {
    if (currentHouseId_ != houseId) {
        currentHouseId_ = houseId;
        // Note: currentHouse_ pointer may become invalid, should be resolved when needed
        currentHouse_ = nullptr;
        emit brushChanged();
    }
}

// Brush configuration
bool HouseBrush::getRemoveItems() const {
    return removeItems_;
}

void HouseBrush::setRemoveItems(bool remove) {
    if (removeItems_ != remove) {
        removeItems_ = remove;
        emit brushChanged();
    }
}

bool HouseBrush::getAutoAssignDoorId() const {
    return autoAssignDoorId_;
}

void HouseBrush::setAutoAssignDoorId(bool autoAssign) {
    if (autoAssignDoorId_ != autoAssign) {
        autoAssignDoorId_ = autoAssign;
        emit brushChanged();
    }
}

// Drawing options
bool HouseBrush::canDrag() const {
    return true; // House brush supports dragging
}

bool HouseBrush::canSmear() const {
    return true; // House brush supports smearing
}

bool HouseBrush::oneSizeFitsAll() const {
    return true; // House brush doesn't have size variations
}

// Brush loading
bool HouseBrush::load(const QDomElement& element, QStringList& warnings) {
    if (!Brush::load(element, warnings)) {
        return false;
    }
    
    // Load house-specific properties
    name_ = element.attribute("name", "House Brush");
    currentHouseId_ = element.attribute("house_id", "0").toUInt();
    removeItems_ = element.attribute("remove_items", "true").toLower() == "true";
    autoAssignDoorId_ = element.attribute("auto_assign_door_id", "true").toLower() == "true";
    
    return true;
}

// Helper methods
void HouseBrush::updateTileHouseProperties(Map* map, Tile* tile, bool adding) const {
    if (!tile) return;
    
    if (adding && currentHouse_) {
        // Set house ID on tile
        tile->setHouseId(currentHouseId_);
        
        // Set PZ flag for house tiles
        tile->setMapFlag(TileMapFlag::ProtectionZone, true);
        
        // Add tile to house
        currentHouse_->addTile(tile);
        
        // Remove loose items if configured
        if (removeItems_) {
            removeLooseItems(tile);
        }
        
        // Assign door IDs if configured
        if (autoAssignDoorId_) {
            assignDoorIds(map, tile);
        }
    } else {
        // Remove house ID from tile
        quint32 oldHouseId = tile->getHouseId();
        tile->setHouseId(0);
        
        // Remove PZ flag
        tile->setMapFlag(TileMapFlag::ProtectionZone, false);
        
        // Remove tile from house if it was part of one
        if (oldHouseId > 0) {
            // Find house and remove tile
            const QList<House*>& houses = map->getHouses();
            for (House* house : houses) {
                if (house && house->getId() == oldHouseId) {
                    house->removeTile(tile);
                    break;
                }
            }
        }
        
        // Remove door IDs
        removeDoorIds(tile);
    }
}

void HouseBrush::removeLooseItems(Tile* tile) const {
    if (!tile) return;
    
    // Remove items that are moveable (loose items)
    QVector<Item*> itemsToRemove;
    const QVector<Item*>& items = tile->getItems();
    
    for (Item* item : items) {
        if (item && item->isMoveable()) {
            itemsToRemove.append(item);
        }
    }
    
    for (Item* item : itemsToRemove) {
        tile->removeItem(item);
        delete item;
    }
}

void HouseBrush::assignDoorIds(Map* map, Tile* tile) const {
    Q_UNUSED(map);
    
    if (!tile || !currentHouse_) return;
    
    // Check if tile has door items
    const QVector<Item*>& items = tile->getItems();
    for (Item* item : items) {
        if (item && item->isDoor()) {
            // Get empty door ID from house
            quint8 doorId = currentHouse_->getEmptyDoorId();
            if (doorId > 0) {
                item->setDoorId(doorId);
                currentHouse_->setDoorId(tile->mapPos(), doorId);
            }
        }
    }
}

void HouseBrush::removeDoorIds(Tile* tile) const {
    if (!tile) return;
    
    // Remove door IDs from door items
    const QVector<Item*>& items = tile->getItems();
    for (Item* item : items) {
        if (item && item->isDoor()) {
            item->setDoorId(0);
        }
    }
}

// HouseBrushCommand implementation
HouseBrushCommand::HouseBrushCommand(Map* map, const QPointF& tilePos, House* house, bool isAdding, QUndoCommand* parent)
    : QUndoCommand(parent),
      map_(map),
      tilePos_(tilePos),
      house_(house),
      houseId_(house ? house->getId() : 0),
      isAdding_(isAdding),
      hadHouse_(false),
      previousHouseId_(0) {
    
    if (isAdding && house) {
        setText(QString("Add house %1 to tile").arg(house->getName()));
    } else {
        setText("Remove house from tile");
    }
    
    // Store current state for undo
    if (map_) {
        int x = static_cast<int>(tilePos_.x());
        int y = static_cast<int>(tilePos_.y());
        Tile* tile = map_->getTile(x, y, 0);
        if (tile) {
            hadHouse_ = tile->isHouseTile();
            previousHouseId_ = tile->getHouseId();
            
            if (isAdding_) {
                storeRemovedItems(tile);
            }
        }
    }
}

HouseBrushCommand::~HouseBrushCommand() {
    // Clean up stored items if needed
}

void HouseBrushCommand::undo() {
    if (!map_) return;
    
    int x = static_cast<int>(tilePos_.x());
    int y = static_cast<int>(tilePos_.y());
    Tile* tile = map_->getTile(x, y, 0);
    if (!tile) return;
    
    if (isAdding_) {
        // Restore previous house state
        tile->setHouseId(previousHouseId_);
        tile->setMapFlag(TileMapFlag::ProtectionZone, hadHouse_);
        
        // Restore removed items
        restoreRemovedItems(tile);
        
        // Remove tile from house
        if (house_) {
            house_->removeTile(tile);
        }
    } else {
        // Restore house
        tile->setHouseId(previousHouseId_);
        tile->setMapFlag(TileMapFlag::ProtectionZone, hadHouse_);
        
        // Add tile back to house
        if (previousHouseId_ > 0) {
            const QList<House*>& houses = map_->getHouses();
            for (House* house : houses) {
                if (house && house->getId() == previousHouseId_) {
                    house->addTile(tile);
                    break;
                }
            }
        }
    }
}

void HouseBrushCommand::redo() {
    executeOperation(isAdding_);
}

void HouseBrushCommand::executeOperation(bool adding) {
    if (!map_) return;
    
    int x = static_cast<int>(tilePos_.x());
    int y = static_cast<int>(tilePos_.y());
    Tile* tile = map_->getTile(x, y, 0);
    if (!tile) {
        // Create tile if it doesn't exist
        tile = map_->createTile(x, y, 0);
        if (!tile) return;
    }
    
    if (adding && house_) {
        // Apply house to tile
        tile->setHouseId(houseId_);
        tile->setMapFlag(TileMapFlag::ProtectionZone, true);
        house_->addTile(tile);
        
        // Remove loose items if they were stored
        if (!removedItems_.isEmpty()) {
            QVector<Item*> itemsToRemove;
            const QVector<Item*>& items = tile->getItems();
            
            for (Item* item : items) {
                if (item && item->isMoveable()) {
                    itemsToRemove.append(item);
                }
            }
            
            for (Item* item : itemsToRemove) {
                tile->removeItem(item);
                delete item;
            }
        }
    } else {
        // Remove house from tile
        tile->setHouseId(0);
        tile->setMapFlag(TileMapFlag::ProtectionZone, false);
        
        // Remove tile from any house
        const QList<House*>& houses = map_->getHouses();
        for (House* house : houses) {
            if (house && house->containsTile(tile)) {
                house->removeTile(tile);
                break;
            }
        }
    }
}

void HouseBrushCommand::storeRemovedItems(Tile* tile) {
    if (!tile) return;
    
    removedItems_.clear();
    const QVector<Item*>& items = tile->getItems();
    
    for (Item* item : items) {
        if (item && item->isMoveable()) {
            // Store item data for restoration
            QPair<quint16, QVariant> itemData;
            itemData.first = item->getServerId();
            // Store additional item properties if needed
            removedItems_.append(itemData);
        }
    }
}

void HouseBrushCommand::restoreRemovedItems(Tile* tile) {
    if (!tile || removedItems_.isEmpty()) return;
    
    // This is a simplified restoration - in a full implementation,
    // you would need to store complete item state and recreate items
    // For now, we just note that items were removed
    qDebug() << "HouseBrushCommand: Would restore" << removedItems_.size() << "items";
}

// Task 73: Enhanced house operations implementation
void HouseBrush::applyToArea(Map* map, const QRect& area, int floor) {
    if (!map || !currentHouse_) return;

    int z = (floor >= 0) ? floor : 0;

    for (int x = area.left(); x <= area.right(); ++x) {
        for (int y = area.top(); y <= area.bottom(); ++y) {
            QPointF pos(x, y);
            if (canDraw(map, pos)) {
                Tile* tile = map->getTile(x, y, z);
                if (!tile) {
                    tile = map->createTile(x, y, z);
                }
                if (tile) {
                    updateTileHouseProperties(map, tile, true);
                    emit tileAdded(pos);
                }
            }
        }
    }

    emit houseChanged(currentHouse_);
}

void HouseBrush::removeFromArea(Map* map, const QRect& area, int floor) {
    if (!map) return;

    int z = (floor >= 0) ? floor : 0;

    for (int x = area.left(); x <= area.right(); ++x) {
        for (int y = area.top(); y <= area.bottom(); ++y) {
            Tile* tile = map->getTile(x, y, z);
            if (tile && tile->isHouseTile()) {
                QPointF pos(x, y);
                updateTileHouseProperties(map, tile, false);
                emit tileRemoved(pos);
            }
        }
    }
}

void HouseBrush::applyToSelection(Map* map, const QList<QPointF>& positions) {
    if (!map || !currentHouse_) return;

    for (const QPointF& pos : positions) {
        if (canDraw(map, pos)) {
            int x = static_cast<int>(pos.x());
            int y = static_cast<int>(pos.y());
            Tile* tile = map->getTile(x, y, 0);
            if (!tile) {
                tile = map->createTile(x, y, 0);
            }
            if (tile) {
                updateTileHouseProperties(map, tile, true);
                emit tileAdded(pos);
            }
        }
    }

    emit houseChanged(currentHouse_);
}

void HouseBrush::removeFromSelection(Map* map, const QList<QPointF>& positions) {
    if (!map) return;

    for (const QPointF& pos : positions) {
        int x = static_cast<int>(pos.x());
        int y = static_cast<int>(pos.y());
        Tile* tile = map->getTile(x, y, 0);
        if (tile && tile->isHouseTile()) {
            updateTileHouseProperties(map, tile, false);
            emit tileRemoved(pos);
        }
    }
}

// Task 73: Cleanup operations
void HouseBrush::cleanupHouseTiles(Map* map) {
    if (!map || !currentHouse_) return;

    QList<QPointF> houseTiles = getHouseTiles(map);

    for (const QPointF& pos : houseTiles) {
        int x = static_cast<int>(pos.x());
        int y = static_cast<int>(pos.y());
        Tile* tile = map->getTile(x, y, 0);
        if (tile) {
            // Remove loose items
            if (removeItems_) {
                removeLooseItems(tile);
            }

            // Ensure PZ flag is set
            tile->setMapFlag(TileMapFlag::ProtectionZone, true);

            // Assign door IDs if needed
            if (autoAssignDoorId_) {
                assignDoorIds(map, tile);
            }
        }
    }

    emit cleanupCompleted();
}

void HouseBrush::validateHouseTiles(Map* map) {
    if (!map || !currentHouse_) return;

    bool isValid = true;
    QStringList errors = getValidationErrors(map);

    if (!errors.isEmpty()) {
        isValid = false;
        qDebug() << "House validation errors:" << errors;
    }

    emit houseValidated(isValid);
}

void HouseBrush::updateHouseReferences(Map* map) {
    if (!map || !currentHouse_) return;

    QList<QPointF> houseTiles = getHouseTiles(map);

    for (const QPointF& pos : houseTiles) {
        int x = static_cast<int>(pos.x());
        int y = static_cast<int>(pos.y());
        Tile* tile = map->getTile(x, y, 0);
        if (tile && tile->getHouseId() != currentHouseId_) {
            tile->setHouseId(currentHouseId_);
        }
    }
}

// Task 73: Door management
void HouseBrush::assignDoorIDs(Map* map, const QList<QPointF>& positions) {
    if (!map || !currentHouse_) return;

    for (const QPointF& pos : positions) {
        int x = static_cast<int>(pos.x());
        int y = static_cast<int>(pos.y());
        Tile* tile = map->getTile(x, y, 0);
        if (tile) {
            const QVector<Item*>& items = tile->getItems();
            for (Item* item : items) {
                if (item && item->isDoor() && item->getDoorId() == 0) {
                    quint8 doorId = getNextAvailableDoorID(map);
                    if (doorId > 0) {
                        item->setDoorId(doorId);
                        currentHouse_->setDoorId(tile->mapPos(), doorId);
                        emit doorAssigned(pos, doorId);
                    }
                }
            }
        }
    }
}

void HouseBrush::clearDoorIDs(Map* map, const QList<QPointF>& positions) {
    if (!map || !currentHouse_) return;

    for (const QPointF& pos : positions) {
        int x = static_cast<int>(pos.x());
        int y = static_cast<int>(pos.y());
        Tile* tile = map->getTile(x, y, 0);
        if (tile) {
            const QVector<Item*>& items = tile->getItems();
            for (Item* item : items) {
                if (item && item->isDoor()) {
                    item->setDoorId(0);
                }
            }
        }
    }
}

quint8 HouseBrush::getNextAvailableDoorID(Map* map) const {
    if (!map || !currentHouse_) return 0;

    // Start from 1 and find first unused door ID
    for (quint8 doorId = 1; doorId < 255; ++doorId) {
        bool isUsed = false;

        // Check if door ID is used in current house
        QList<QPointF> houseTiles = getHouseTiles(map);
        for (const QPointF& pos : houseTiles) {
            int x = static_cast<int>(pos.x());
            int y = static_cast<int>(pos.y());
            Tile* tile = map->getTile(x, y, 0);
            if (tile) {
                const QVector<Item*>& items = tile->getItems();
                for (Item* item : items) {
                    if (item && item->isDoor() && item->getDoorId() == doorId) {
                        isUsed = true;
                        break;
                    }
                }
                if (isUsed) break;
            }
        }

        if (!isUsed) {
            return doorId;
        }
    }

    return 0; // No available door ID found
}

// Task 73: House utilities
QList<QPointF> HouseBrush::getHouseTiles(Map* map) const {
    QList<QPointF> tiles;
    if (!map || !currentHouse_) return tiles;

    const QList<MapPos>& housePositions = currentHouse_->getTilePositions();
    for (const MapPos& pos : housePositions) {
        tiles.append(QPointF(pos.x, pos.y));
    }

    return tiles;
}

int HouseBrush::getHouseTileCount(Map* map) const {
    if (!map || !currentHouse_) return 0;
    return currentHouse_->getTileCount();
}

QRectF HouseBrush::getHouseBounds(Map* map) const {
    if (!map || !currentHouse_) return QRectF();

    QRect bounds = currentHouse_->getBoundingRect();
    return QRectF(bounds.x(), bounds.y(), bounds.width(), bounds.height());
}

bool HouseBrush::isHouseConnected(Map* map) const {
    if (!map || !currentHouse_) return false;
    return currentHouse_->isConnected(map);
}

// Task 73: Validation
bool HouseBrush::isValidHouseTile(Map* map, const QPointF& tilePos) const {
    if (!map) return false;

    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());

    // Check if coordinates are valid
    if (!map->isValidCoordinate(x, y, 0)) {
        return false;
    }

    Tile* tile = map->getTile(x, y, 0);
    if (!tile) return true; // Can create new tile

    // Check if tile has blocking items
    return !hasBlockingItems(map, tilePos);
}

bool HouseBrush::hasBlockingItems(Map* map, const QPointF& tilePos) const {
    if (!map) return false;

    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());
    Tile* tile = map->getTile(x, y, 0);
    if (!tile) return false;

    const QVector<Item*>& items = tile->getItems();
    for (Item* item : items) {
        if (item && item->isBlocking()) {
            return true;
        }
    }

    return false;
}

QStringList HouseBrush::getValidationErrors(Map* map) const {
    QStringList errors;
    if (!map || !currentHouse_) {
        errors << "No map or house available for validation";
        return errors;
    }

    // Check if house has tiles
    if (currentHouse_->getTileCount() == 0) {
        errors << "House has no tiles";
    }

    // Check if house is connected
    if (!currentHouse_->isConnected(map)) {
        errors << "House tiles are not connected";
    }

    // Check if house has valid exit
    if (!currentHouse_->hasValidExit(map)) {
        errors << "House has no valid exit position";
    }

    // Check individual tiles
    QList<QPointF> houseTiles = getHouseTiles(map);
    for (const QPointF& pos : houseTiles) {
        int x = static_cast<int>(pos.x());
        int y = static_cast<int>(pos.y());
        Tile* tile = map->getTile(x, y, 0);
        if (!tile) {
            errors << QString("Missing tile at [%1, %2]").arg(x).arg(y);
        } else if (tile->getHouseId() != currentHouseId_) {
            errors << QString("Tile at [%1, %2] has incorrect house ID").arg(x).arg(y);
        }
    }

    return errors;
}

// Task 73: HouseManager implementation
HouseManager::HouseManager(QObject* parent) : QObject(parent) {
}

HouseManager::~HouseManager() {
}

void HouseManager::cleanupHouse(Map* map, House* house) {
    if (!map || !house) return;

    house->removeLooseItems(map);
    house->setPZOnTiles(map, true);
    house->assignDoorIDs(map);
    house->updateTileHouseReferences(map);
}

void HouseManager::validateHouse(Map* map, House* house) {
    if (!map || !house) return;

    house->removeInvalidTiles(map);
    house->updateTileHouseReferences(map);
}

void HouseManager::removeInvalidHouses(Map* map) {
    if (!map) return;

    QList<House*> houses = map->getHouses();
    QList<House*> invalidHouses;

    for (House* house : houses) {
        if (house && (!house->isValid() || house->getTileCount() == 0)) {
            invalidHouses.append(house);
        }
    }

    for (House* house : invalidHouses) {
        map->removeHouse(house->getId());
    }
}

void HouseManager::updateAllHouseReferences(Map* map) {
    if (!map) return;

    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house) {
            house->updateTileHouseReferences(map);
        }
    }
}

void HouseManager::cleanupAllHouses(Map* map) {
    if (!map) return;

    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house) {
            cleanupHouse(map, house);
        }
    }
}

void HouseManager::validateAllHouses(Map* map) {
    if (!map) return;

    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house) {
            validateHouse(map, house);
        }
    }
}

void HouseManager::assignAllDoorIDs(Map* map) {
    if (!map) return;

    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house) {
            house->assignDoorIDs(map);
        }
    }
}

void HouseManager::clearAllDoorIDs(Map* map) {
    if (!map) return;

    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house) {
            house->clearDoorIDs(map);
        }
    }
}

QList<House*> HouseManager::getHousesInArea(Map* map, const QRect& area) {
    QList<House*> housesInArea;
    if (!map) return housesInArea;

    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house) {
            QRect houseBounds = house->getBoundingRect();
            if (houseBounds.intersects(area)) {
                housesInArea.append(house);
            }
        }
    }

    return housesInArea;
}

QList<House*> HouseManager::getInvalidHouses(Map* map) {
    QList<House*> invalidHouses;
    if (!map) return invalidHouses;

    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house && !house->isValid()) {
            invalidHouses.append(house);
        }
    }

    return invalidHouses;
}

QList<House*> HouseManager::getDisconnectedHouses(Map* map) {
    QList<House*> disconnectedHouses;
    if (!map) return disconnectedHouses;

    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house && !house->isConnected(map)) {
            disconnectedHouses.append(house);
        }
    }

    return disconnectedHouses;
}

House* HouseManager::getHouseAt(Map* map, const QPointF& position) {
    if (!map) return nullptr;

    int x = static_cast<int>(position.x());
    int y = static_cast<int>(position.y());
    Tile* tile = map->getTile(x, y, 0);
    if (!tile || !tile->isHouseTile()) {
        return nullptr;
    }

    quint32 houseId = tile->getHouseId();
    return map->getHouse(houseId);
}

int HouseManager::getTotalHouseTiles(Map* map) {
    if (!map) return 0;

    int totalTiles = 0;
    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house) {
            totalTiles += house->getTileCount();
        }
    }

    return totalTiles;
}

int HouseManager::getTotalHouseDoors(Map* map) {
    if (!map) return 0;

    int totalDoors = 0;
    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house) {
            // Count doors in house tiles
            const QList<MapPos>& positions = house->getTilePositions();
            for (const MapPos& pos : positions) {
                Tile* tile = map->getTile(pos);
                if (tile) {
                    const QVector<Item*>& items = tile->getItems();
                    for (Item* item : items) {
                        if (item && item->isDoor()) {
                            totalDoors++;
                        }
                    }
                }
            }
        }
    }

    return totalDoors;
}

QMap<quint32, int> HouseManager::getHouseSizeStatistics(Map* map) {
    QMap<quint32, int> statistics;
    if (!map) return statistics;

    QList<House*> houses = map->getHouses();
    for (House* house : houses) {
        if (house) {
            statistics[house->getId()] = house->getTileCount();
        }
    }

    return statistics;
}

QStringList HouseManager::getHouseValidationReport(Map* map) {
    QStringList report;
    if (!map) {
        report << "No map available for validation";
        return report;
    }

    QList<House*> houses = map->getHouses();
    report << QString("=== House Validation Report ===");
    report << QString("Total houses: %1").arg(houses.size());

    int validHouses = 0;
    int invalidHouses = 0;
    int disconnectedHouses = 0;
    int totalTiles = 0;
    int totalDoors = 0;

    for (House* house : houses) {
        if (house) {
            bool isValid = house->isValid();
            bool isConnected = house->isConnected(map);

            if (isValid) validHouses++;
            else invalidHouses++;

            if (!isConnected) disconnectedHouses++;

            totalTiles += house->getTileCount();

            // Count doors
            const QList<MapPos>& positions = house->getTilePositions();
            for (const MapPos& pos : positions) {
                Tile* tile = map->getTile(pos);
                if (tile) {
                    const QVector<Item*>& items = tile->getItems();
                    for (Item* item : items) {
                        if (item && item->isDoor()) {
                            totalDoors++;
                        }
                    }
                }
            }

            if (!isValid || !isConnected) {
                report << QString("House %1 (%2): %3%4")
                          .arg(house->getId())
                          .arg(house->getName())
                          .arg(isValid ? "Valid" : "Invalid")
                          .arg(isConnected ? "" : ", Disconnected");
            }
        }
    }

    report << QString("Valid houses: %1").arg(validHouses);
    report << QString("Invalid houses: %1").arg(invalidHouses);
    report << QString("Disconnected houses: %1").arg(disconnectedHouses);
    report << QString("Total house tiles: %1").arg(totalTiles);
    report << QString("Total house doors: %1").arg(totalDoors);

    return report;
}

#include "HouseBrush.moc"
