#include "PlaceDoorCommand.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "DoorItem.h"
#include <QDebug>

PlaceDoorCommand::PlaceDoorCommand(Map* map,
                                 const QPointF& tilePos,
                                 quint16 doorItemId,
                                 DoorType doorType,
                                 quint8 doorId,
                                 QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , tilePos_(tilePos)
    , doorItemId_(doorItemId)
    , doorType_(doorType)
    , doorId_(doorId)
    , doorInstance_(nullptr)
    , tilePreviouslyExisted_(false)
    , previousDoorItemId_(0)
    , hadPreviousDoor_(false)
{
    if (doorItemId_ != 0) {
        setText(QObject::tr("Place Door (ID: %1) at (%2,%3)")
                    .arg(doorItemId_)
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    } else {
        setText(QObject::tr("Place Door at (%1,%2)")
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    }
}

PlaceDoorCommand::~PlaceDoorCommand()
{
    // If we still own the door instance, clean it up
    if (doorInstance_ && !doorInstance_->parent()) {
        delete doorInstance_;
        doorInstance_ = nullptr;
    }
}

void PlaceDoorCommand::undo()
{
    if (!map_) {
        qWarning() << "PlaceDoorCommand::undo(): Map pointer is null.";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    
    if (doorInstance_ && tile) {
        // Remove door from tile
        bool removed = tile->removeItem(doorInstance_);
        if (removed) {
            qDebug() << "PlaceDoorCommand: Undone - Removed door ID" << doorItemId_ << "from" << tilePos_;
        } else {
            qWarning() << "PlaceDoorCommand::undo(): Failed to remove door instance from tile at" << tilePos_;
        }
    }

    // Restore previous door if there was one
    if (hadPreviousDoor_ && previousDoorItemId_ != 0 && tile) {
        Item* restoredDoor = new DoorItem(previousDoorItemId_);
        if (restoredDoor) {
            tile->addItem(restoredDoor);
            qDebug() << "PlaceDoorCommand: Undone - Restored previous door ID" << previousDoorItemId_ << "at" << tilePos_;
        }
    }

    // If tile was created by this command and is now empty, remove it
    if (!tilePreviouslyExisted_ && tile && tile->isEmpty()) {
        map_->removeTile(tilePos_);
        qDebug() << "PlaceDoorCommand: Undone - Removed now-empty tile at" << tilePos_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
}

void PlaceDoorCommand::redo()
{
    if (!map_ || doorItemId_ == 0) {
        qWarning() << "PlaceDoorCommand::redo(): Map is null or doorItemId_ is 0 (no action).";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        tile = map_->getOrCreateTile(tilePos_);
        tilePreviouslyExisted_ = false; // Tile was created now
        if (!tile) {
            qWarning() << "PlaceDoorCommand::redo(): Could not get or create tile at" << tilePos_;
            return;
        }
    } else {
        tilePreviouslyExisted_ = true; // Tile already existed
    }

    // Check for existing door and store it
    const QVector<Item*>& items = tile->getItems();
    for (Item* item : items) {
        if (item && item->isDoor()) {
            previousDoorItemId_ = item->getID();
            hadPreviousDoor_ = true;
            // Remove existing door
            tile->removeItem(item);
            delete item;
            break;
        }
    }

    // Create door instance if not already created
    if (!doorInstance_) {
        doorInstance_ = new DoorItem(doorItemId_);
        if (doorInstance_ && doorId_ > 0) {
            doorInstance_->setDoorId(doorId_);
        }
    }

    if (doorInstance_) {
        // Add door to tile
        tile->addItem(doorInstance_);
        qDebug() << "PlaceDoorCommand: Redone - Placed door ID" << doorItemId_ << "at" << tilePos_;
    } else {
        qWarning() << "PlaceDoorCommand::redo(): Could not create door for ID" << doorItemId_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
}

// RemoveDoorCommand implementation
RemoveDoorCommand::RemoveDoorCommand(Map* map,
                                   const QPointF& tilePos,
                                   Item* door,
                                   QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , tilePos_(tilePos)
    , doorInstance_(door)
    , doorItemId_(0)
    , doorId_(0)
    , doorWasRemoved_(false)
{
    if (doorInstance_) {
        doorItemId_ = doorInstance_->getID();
        if (doorInstance_->isDoor()) {
            doorId_ = doorInstance_->getDoorId();
        }
        setText(QObject::tr("Remove Door (ID: %1) from (%2,%3)")
                    .arg(doorItemId_)
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    } else {
        setText(QObject::tr("Remove Door from (%1,%2)")
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    }
}

RemoveDoorCommand::~RemoveDoorCommand()
{
    // If we own the door instance (it was removed and not restored), clean it up
    if (doorInstance_ && doorWasRemoved_) {
        delete doorInstance_;
        doorInstance_ = nullptr;
    }
}

void RemoveDoorCommand::undo()
{
    if (!map_ || !doorInstance_) {
        qWarning() << "RemoveDoorCommand::undo(): Map or door instance is null.";
        return;
    }

    // Get or create tile
    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        tile = map_->getOrCreateTile(tilePos_);
        if (!tile) {
            qWarning() << "RemoveDoorCommand::undo(): Could not get or create tile at" << tilePos_;
            return;
        }
    }

    // Restore door to tile
    tile->addItem(doorInstance_);
    doorWasRemoved_ = false; // Door is now back on the map
    
    qDebug() << "RemoveDoorCommand: Undone - Restored door ID" << doorItemId_ << "to" << tilePos_;

    // Ensure map is marked as modified
    map_->setModified(true);
}

void RemoveDoorCommand::redo()
{
    if (!map_) {
        qWarning() << "RemoveDoorCommand::redo(): Map pointer is null.";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        qWarning() << "RemoveDoorCommand::redo(): No tile found at" << tilePos_;
        return;
    }

    // If no specific door was provided, find the first door on the tile
    if (!doorInstance_) {
        const QVector<Item*>& items = tile->getItems();
        for (Item* item : items) {
            if (item && item->isDoor()) {
                doorInstance_ = item;
                doorItemId_ = item->getID();
                if (item->isDoor()) {
                    doorId_ = item->getDoorId();
                }
                break;
            }
        }
        
        if (!doorInstance_) {
            qWarning() << "RemoveDoorCommand::redo(): No doors found on tile at" << tilePos_;
            return;
        }
    }

    // Remove door from tile
    bool removed = tile->removeItem(doorInstance_);
    if (removed) {
        doorWasRemoved_ = true; // We now own the door instance
        qDebug() << "RemoveDoorCommand: Redone - Removed door ID" << doorItemId_ << "from" << tilePos_;
    } else {
        qWarning() << "RemoveDoorCommand::redo(): Failed to remove door from tile at" << tilePos_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
}
