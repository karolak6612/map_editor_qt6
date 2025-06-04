#include "PlaceItemCommand.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include <QDebug>

PlaceItemCommand::PlaceItemCommand(Map* map,
                                  const QPointF& tilePos,
                                  quint16 itemId,
                                  quint8 count,
                                  QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , tilePos_(tilePos)
    , itemId_(itemId)
    , count_(count)
    , itemInstance_(nullptr)
    , tilePreviouslyExisted_(false)
{
    if (itemId_ != 0) {
        setText(QObject::tr("Place Item (ID: %1) at (%2,%3)")
                    .arg(itemId_)
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    } else {
        setText(QObject::tr("Place Item at (%1,%2)")
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    }
}

PlaceItemCommand::~PlaceItemCommand()
{
    // If we still own the item instance, clean it up
    if (itemInstance_ && !itemInstance_->parent()) {
        delete itemInstance_;
        itemInstance_ = nullptr;
    }
}

void PlaceItemCommand::undo()
{
    if (!map_) {
        qWarning() << "PlaceItemCommand::undo(): Map pointer is null.";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    
    if (itemInstance_ && tile) {
        // Remove item from tile
        bool removed = tile->removeItem(itemInstance_);
        if (removed) {
            qDebug() << "PlaceItemCommand: Undone - Removed item ID" << itemId_ << "from" << tilePos_;
            // The item instance is now owned by this command again
        } else {
            qWarning() << "PlaceItemCommand::undo(): Failed to remove item instance" << itemId_
                       << "from tile at" << tilePos_ << ". Item might have been removed by other means.";
        }
    }

    // If tile was created by this command and is now empty, remove it
    if (!tilePreviouslyExisted_ && tile && tile->isEmpty()) {
        map_->removeTile(tilePos_);
        qDebug() << "PlaceItemCommand: Undone - Removed now-empty tile at" << tilePos_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
}

void PlaceItemCommand::redo()
{
    if (!map_ || itemId_ == 0) {
        qWarning() << "PlaceItemCommand::redo(): Map is null or itemId_ is 0 (no action).";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        tile = map_->getOrCreateTile(tilePos_);
        tilePreviouslyExisted_ = false; // Tile was created now
        if (!tile) {
            qWarning() << "PlaceItemCommand::redo(): Could not get or create tile at" << tilePos_;
            return;
        }
    } else {
        tilePreviouslyExisted_ = true; // Tile already existed
    }

    // Create item instance if not already created
    if (!itemInstance_) {
        itemInstance_ = new Item(itemId_);
        if (itemInstance_ && count_ > 1) {
            itemInstance_->setCount(count_);
        }
    }

    if (itemInstance_) {
        // Add item to tile
        tile->addItem(itemInstance_);
        qDebug() << "PlaceItemCommand: Redone - Placed item ID" << itemId_ << "at" << tilePos_;
    } else {
        qWarning() << "PlaceItemCommand::redo(): Could not create item for ID" << itemId_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
}

// RemoveItemCommand implementation
RemoveItemCommand::RemoveItemCommand(Map* map,
                                    const QPointF& tilePos,
                                    quint16 itemId,
                                    Item* specificItem,
                                    QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , tilePos_(tilePos)
    , itemId_(itemId)
    , itemInstance_(specificItem)
    , itemCount_(1)
    , itemWasRemoved_(false)
    , itemStackPosition_(0)
{
    if (itemInstance_) {
        itemId_ = itemInstance_->getID();
        itemCount_ = itemInstance_->getCount();
        setText(QObject::tr("Remove Item (ID: %1) from (%2,%3)")
                    .arg(itemId_)
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    } else if (itemId_ != 0) {
        setText(QObject::tr("Remove Item (ID: %1) from (%2,%3)")
                    .arg(itemId_)
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    } else {
        setText(QObject::tr("Remove Item from (%1,%2)")
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    }
}

RemoveItemCommand::~RemoveItemCommand()
{
    // If we own the item instance (it was removed and not restored), clean it up
    if (itemInstance_ && itemWasRemoved_) {
        delete itemInstance_;
        itemInstance_ = nullptr;
    }
}

void RemoveItemCommand::undo()
{
    if (!map_ || !itemInstance_) {
        qWarning() << "RemoveItemCommand::undo(): Map or item instance is null.";
        return;
    }

    // Get or create tile
    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        tile = map_->getOrCreateTile(tilePos_);
        if (!tile) {
            qWarning() << "RemoveItemCommand::undo(): Could not get or create tile at" << tilePos_;
            return;
        }
    }

    // Restore item to tile (ideally at the same stack position)
    tile->addItem(itemInstance_);
    itemWasRemoved_ = false; // Item is now back on the map
    
    qDebug() << "RemoveItemCommand: Undone - Restored item ID" << itemId_ << "to" << tilePos_;

    // Ensure map is marked as modified
    map_->setModified(true);
}

void RemoveItemCommand::redo()
{
    if (!map_) {
        qWarning() << "RemoveItemCommand::redo(): Map pointer is null.";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        qWarning() << "RemoveItemCommand::redo(): No tile found at" << tilePos_;
        return;
    }

    // If no specific item was provided, find an item to remove
    if (!itemInstance_) {
        const QVector<Item*>& items = tile->getItems();
        for (int i = 0; i < items.size(); ++i) {
            Item* item = items[i];
            if (item && (itemId_ == 0 || item->getID() == itemId_)) {
                itemInstance_ = item;
                itemId_ = item->getID();
                itemCount_ = item->getCount();
                itemStackPosition_ = i;
                break;
            }
        }
        
        if (!itemInstance_) {
            qWarning() << "RemoveItemCommand::redo(): No suitable items found on tile at" << tilePos_;
            return;
        }
    }

    // Remove item from tile
    bool removed = tile->removeItem(itemInstance_);
    if (removed) {
        itemWasRemoved_ = true; // We now own the item instance
        qDebug() << "RemoveItemCommand: Redone - Removed item ID" << itemId_ << "from" << tilePos_;
    } else {
        qWarning() << "RemoveItemCommand::redo(): Failed to remove item from tile at" << tilePos_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
}
