#include "HouseExitBrush.h"
#include "House.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "ItemManager.h"
#include <QDebug>
#include <QUndoCommand>
#include <QDomElement>

// HouseExitBrush implementation
HouseExitBrush::HouseExitBrush(QObject* parent)
    : Brush(parent),
      name_("House Exit Brush"),
      currentHouse_(nullptr),
      currentHouseId_(0),
      autoSetAsEntry_(true),
      markWithItem_(true),
      markerItemId_(1387) { // Default exit marker item
}

HouseExitBrush::~HouseExitBrush() {
    // Don't delete currentHouse_ as we don't own it
}

// Brush interface implementation
Brush::Type HouseExitBrush::type() const {
    return Type::HouseExit;
}

QString HouseExitBrush::name() const {
    return name_;
}

void HouseExitBrush::setName(const QString& newName) {
    if (name_ != newName) {
        name_ = newName;
        emit brushChanged();
    }
}

int HouseExitBrush::getLookID() const {
    // Return the marker item ID for UI display
    return markerItemId_;
}

// Type identification
bool HouseExitBrush::isHouseExit() const {
    return true;
}

HouseExitBrush* HouseExitBrush::asHouseExit() {
    return this;
}

const HouseExitBrush* HouseExitBrush::asHouseExit() const {
    return this;
}

// Core brush actions
bool HouseExitBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(drawingContext);
    
    if (!map || !currentHouse_) {
        return false;
    }
    
    return isValidExitPosition(map, tilePos);
}

QUndoCommand* HouseExitBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    
    if (!canDraw(map, tilePos)) {
        return nullptr;
    }
    
    return new HouseExitBrushCommand(map, tilePos, currentHouse_, true, parentCommand);
}

QUndoCommand* HouseExitBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    
    if (!map) {
        return nullptr;
    }
    
    return new HouseExitBrushCommand(map, tilePos, nullptr, false, parentCommand);
}

// House exit specific properties
House* HouseExitBrush::getHouse() const {
    return currentHouse_;
}

void HouseExitBrush::setHouse(House* house) {
    if (currentHouse_ != house) {
        currentHouse_ = house;
        currentHouseId_ = house ? house->getId() : 0;
        emit houseChanged(house);
        emit brushChanged();
    }
}

quint32 HouseExitBrush::getHouseId() const {
    return currentHouseId_;
}

void HouseExitBrush::setHouseId(quint32 houseId) {
    if (currentHouseId_ != houseId) {
        currentHouseId_ = houseId;
        // Note: currentHouse_ pointer may become invalid, should be resolved when needed
        currentHouse_ = nullptr;
        emit brushChanged();
    }
}

// Exit configuration
bool HouseExitBrush::getAutoSetAsEntry() const {
    return autoSetAsEntry_;
}

void HouseExitBrush::setAutoSetAsEntry(bool autoSet) {
    if (autoSetAsEntry_ != autoSet) {
        autoSetAsEntry_ = autoSet;
        emit exitConfigChanged();
        emit brushChanged();
    }
}

bool HouseExitBrush::getMarkWithItem() const {
    return markWithItem_;
}

void HouseExitBrush::setMarkWithItem(bool mark) {
    if (markWithItem_ != mark) {
        markWithItem_ = mark;
        emit exitConfigChanged();
        emit brushChanged();
    }
}

quint16 HouseExitBrush::getMarkerItemId() const {
    return markerItemId_;
}

void HouseExitBrush::setMarkerItemId(quint16 itemId) {
    if (markerItemId_ != itemId) {
        markerItemId_ = itemId;
        emit exitConfigChanged();
        emit brushChanged();
    }
}

// Drawing options
bool HouseExitBrush::canDrag() const {
    return false; // House exit brush doesn't support dragging
}

bool HouseExitBrush::canSmear() const {
    return false; // House exit brush doesn't support smearing
}

bool HouseExitBrush::oneSizeFitsAll() const {
    return true; // House exit brush doesn't have size variations
}

// Brush loading
bool HouseExitBrush::load(const QDomElement& element, QStringList& warnings) {
    if (!Brush::load(element, warnings)) {
        return false;
    }
    
    // Load house exit specific properties
    name_ = element.attribute("name", "House Exit Brush");
    currentHouseId_ = element.attribute("house_id", "0").toUInt();
    autoSetAsEntry_ = element.attribute("auto_set_as_entry", "true").toLower() == "true";
    markWithItem_ = element.attribute("mark_with_item", "true").toLower() == "true";
    markerItemId_ = element.attribute("marker_item_id", "1387").toUShort();
    
    return true;
}

// Helper methods
void HouseExitBrush::setHouseExit(Map* map, Tile* tile, House* house) const {
    if (!tile || !house) return;
    
    MapPos exitPos = tile->mapPos();
    
    // Set as house exit
    house->setExitPosition(exitPos);
    
    // Also set as entry if configured
    if (autoSetAsEntry_) {
        house->setEntryPosition(exitPos);
    }
    
    // Add exit marker item if configured
    if (markWithItem_) {
        addExitMarker(tile);
    }
    
    qDebug() << "Set house exit for house" << house->getId() << "at position" << exitPos.x << exitPos.y << exitPos.z;
}

void HouseExitBrush::removeHouseExit(Map* map, Tile* tile) const {
    Q_UNUSED(map);
    
    if (!tile) return;
    
    MapPos tilePos = tile->mapPos();
    
    // Find houses that have this position as exit and remove it
    // Note: This would require access to all houses in the map
    // For now, we just remove the marker item
    
    // Remove exit marker item
    removeExitMarker(tile);
    
    qDebug() << "Removed house exit at position" << tilePos.x << tilePos.y << tilePos.z;
}

void HouseExitBrush::addExitMarker(Tile* tile) const {
    if (!tile) return;
    
    // Check if marker item already exists
    const QVector<Item*>& items = tile->getItems();
    for (Item* item : items) {
        if (item && item->getServerId() == markerItemId_) {
            return; // Marker already exists
        }
    }
    
    // Create and add marker item
    ItemManager* itemManager = ItemManager::getInstancePtr();
    if (itemManager) {
        Item* marker = itemManager->createItem(markerItemId_);
        if (marker) {
            tile->addItem(marker);
            qDebug() << "Added house exit marker item" << markerItemId_ << "to tile";
        }
    }
}

void HouseExitBrush::removeExitMarker(Tile* tile) const {
    if (!tile) return;
    
    // Find and remove marker items
    QVector<Item*> itemsToRemove;
    const QVector<Item*>& items = tile->getItems();
    
    for (Item* item : items) {
        if (item && item->getServerId() == markerItemId_) {
            itemsToRemove.append(item);
        }
    }
    
    for (Item* item : itemsToRemove) {
        tile->removeItem(item);
        delete item;
        qDebug() << "Removed house exit marker item" << markerItemId_ << "from tile";
    }
}

bool HouseExitBrush::isValidExitPosition(Map* map, const QPointF& tilePos) const {
    if (!map) return false;
    
    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());
    
    // Check if position is valid
    if (!map->isValidCoordinate(x, y, 0)) {
        return false;
    }
    
    // Check if tile exists or can be created
    Tile* tile = map->getTile(x, y, 0);
    if (!tile) {
        // Position is valid for creating a new tile
        return true;
    }
    
    // Check if tile is not blocking (can be used as exit)
    return !tile->isBlocking();
}

// HouseExitBrushCommand implementation
HouseExitBrushCommand::HouseExitBrushCommand(Map* map, const QPointF& tilePos, House* house, bool isAdding, QUndoCommand* parent)
    : QUndoCommand(parent),
      map_(map),
      tilePos_(tilePos),
      house_(house),
      houseId_(house ? house->getId() : 0),
      isAdding_(isAdding),
      hadExit_(false),
      previousHouseId_(0),
      previousExitPos_(0, 0, 0),
      hadMarkerItem_(false),
      markerItemId_(1387) {
    
    if (isAdding && house) {
        setText(QString("Set house %1 exit").arg(house->getName()));
    } else {
        setText("Remove house exit");
    }
    
    storeCurrentState();
}

HouseExitBrushCommand::~HouseExitBrushCommand() {
    // Clean up if needed
}

void HouseExitBrushCommand::undo() {
    restorePreviousState();
}

void HouseExitBrushCommand::redo() {
    executeOperation(isAdding_);
}

void HouseExitBrushCommand::executeOperation(bool adding) {
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
        // Set house exit
        MapPos exitPos = tile->mapPos();
        house_->setExitPosition(exitPos);
        
        // Add marker item
        ItemManager* itemManager = ItemManager::getInstancePtr();
        if (itemManager) {
            Item* marker = itemManager->createItem(markerItemId_);
            if (marker) {
                tile->addItem(marker);
            }
        }
    } else {
        // Remove house exit
        if (house_) {
            house_->setExitPosition(MapPos(0, 0, 0));
        }
        
        // Remove marker items
        QVector<Item*> itemsToRemove;
        const QVector<Item*>& items = tile->getItems();
        
        for (Item* item : items) {
            if (item && item->getServerId() == markerItemId_) {
                itemsToRemove.append(item);
            }
        }
        
        for (Item* item : itemsToRemove) {
            tile->removeItem(item);
            delete item;
        }
    }
}

void HouseExitBrushCommand::storeCurrentState() {
    if (!map_) return;
    
    int x = static_cast<int>(tilePos_.x());
    int y = static_cast<int>(tilePos_.y());
    Tile* tile = map_->getTile(x, y, 0);
    
    if (tile) {
        // Check if tile has marker item
        const QVector<Item*>& items = tile->getItems();
        for (Item* item : items) {
            if (item && item->getServerId() == markerItemId_) {
                hadMarkerItem_ = true;
                break;
            }
        }
    }
    
    // Store house exit state
    if (house_) {
        previousExitPos_ = house_->getExitPosition();
        hadExit_ = (previousExitPos_.x != 0 || previousExitPos_.y != 0 || previousExitPos_.z != 0);
    }
}

void HouseExitBrushCommand::restorePreviousState() {
    if (!map_) return;
    
    int x = static_cast<int>(tilePos_.x());
    int y = static_cast<int>(tilePos_.y());
    Tile* tile = map_->getTile(x, y, 0);
    if (!tile) return;
    
    if (isAdding_) {
        // Restore previous exit position
        if (house_) {
            house_->setExitPosition(previousExitPos_);
        }
        
        // Remove marker item if it wasn't there before
        if (!hadMarkerItem_) {
            QVector<Item*> itemsToRemove;
            const QVector<Item*>& items = tile->getItems();
            
            for (Item* item : items) {
                if (item && item->getServerId() == markerItemId_) {
                    itemsToRemove.append(item);
                }
            }
            
            for (Item* item : itemsToRemove) {
                tile->removeItem(item);
                delete item;
            }
        }
    } else {
        // Restore exit
        if (house_ && hadExit_) {
            house_->setExitPosition(previousExitPos_);
        }
        
        // Restore marker item if it was there before
        if (hadMarkerItem_) {
            ItemManager* itemManager = ItemManager::getInstancePtr();
            if (itemManager) {
                Item* marker = itemManager->createItem(markerItemId_);
                if (marker) {
                    tile->addItem(marker);
                }
            }
        }
    }
}


