#include "PlaceDecorationCommand.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
// #include "ItemFactory.h" // Assumed for Item creation
#include <QDebug>
#include <QObject> // For QObject::tr

// Assumed Map/Tile methods:
// Tile* Map::getOrCreateTile(const QPointF& pos);
// Tile* Map::getTile(const QPointF& pos);
// void Map::removeTile(const QPointF& pos);
// void Tile::addItem(Item* item); // Adds item to its item stack
// bool Tile::removeItem(Item* item); // Removes a specific item instance AND DELETES IT.
// bool Tile::isEmpty() const;
// Item* ItemFactory::createItem(quint16 itemId); // Or new Item(id)

PlaceDecorationCommand::PlaceDecorationCommand(Map* map,
                                             const QPointF& tilePos,
                                             quint16 decorationItemId,
                                             QUndoCommand* parent)
    : QUndoCommand(parent),
      map_(map),
      tilePos_(tilePos),
      decorationItemId_(decorationItemId),
      itemInstance_(nullptr),
      tilePreviouslyExisted_(false) {

    if (decorationItemId_ != 0) {
        setText(QObject::tr("Place Decoration (ID: %1) at (%2,%3)")
                    .arg(decorationItemId_)
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    } else {
        // This command is designed for placing. An ID of 0 means it will do nothing in redo.
        // A separate "RemoveDecorationCommand" or flag would be better for explicit removal.
        setText(QObject::tr("Place Decoration (No ID) at (%1,%2)")
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    }
}

PlaceDecorationCommand::~PlaceDecorationCommand() {
    // If itemInstance_ holds an item that is not on any tile (e.g., after undo, or if redo failed to add it),
    // it should be deleted to prevent memory leaks.
    // This happens if the command is destroyed while it "owns" the item.
    delete itemInstance_;
    itemInstance_ = nullptr;
}

void PlaceDecorationCommand::undo() {
    if (!map_) {
        qWarning() << "PlaceDecorationCommand::undo(): Map is null.";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);

    if (itemInstance_ && tile) { // If an item was placed by this command and tile exists
        bool removed = tile->removeItem(itemInstance_); // Tile::removeItem must find this specific instance and delete it.
        if (removed) {
            qDebug() << "PlaceDecorationCommand: Undone - Removed decoration ID" << itemInstance_->getID() << "from" << tilePos_;
            // Since Tile::removeItem is assumed to delete the item, we only nullify our pointer.
            itemInstance_ = nullptr;
        } else {
            qWarning() << "PlaceDecorationCommand::undo(): Failed to remove item instance" << itemInstance_->getID()
                       << "from tile at" << tilePos_ << ". Item might have been removed by other means.";
            // If tile->removeItem didn't find/delete it, but we have a pointer, it's a dangling pointer
            // or an item that shouldn't exist. To prevent double deletion if it was already deleted,
            // or to clean up if it's somehow still alive but not on tile, it's risky.
            // Best is if Tile::removeItem is robust. If it returns false, we assume item is gone.
            delete itemInstance_; // Delete it ourselves if tile didn't. This assumes removeItem doesn't delete if not found.
            itemInstance_ = nullptr;
        }
    } else if (itemInstance_ && !tile) {
        // Item was placed, but tile doesn't exist anymore? Should not happen if tile was created by redo.
        qWarning() << "PlaceDecorationCommand::undo(): Tile at" << tilePos_ << "not found, but an item instance was recorded. Leaking item.";
        // To prevent leak, delete it. This indicates an issue elsewhere or complex scenario.
        delete itemInstance_;
        itemInstance_ = nullptr;
    }


    if (!tilePreviouslyExisted_ && tile && tile->isEmpty()) { // If tile was created by redo AND is now empty
         map_->removeTile(tilePos_); // Assumes Map::removeTile exists and is safe
         qDebug() << "PlaceDecorationCommand: Undone - Removed now-empty tile at" << tilePos_;
    }

    // No border/wall update typically needed for simple decorations unless they affect it.
}

void PlaceDecorationCommand::redo() {
    if (!map_ || decorationItemId_ == 0) {
        qWarning() << "PlaceDecorationCommand::redo(): Map is null or decorationItemId_ is 0 (no action).";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        tile = map_->getOrCreateTile(tilePos_);
        tilePreviouslyExisted_ = false; // Tile was created now
        if (!tile) {
            qWarning() << "PlaceDecorationCommand::redo(): Could not get or create tile at" << tilePos_;
            return;
        }
    } else {
        tilePreviouslyExisted_ = true; // Tile already existed
    }

    // If itemInstance_ is not null, it means we are redoing after an undo.
    // Undo should have nulled itemInstance_ if it ensured the item was deleted.
    // If itemInstance_ is not null here, it implies undo didn't null it, meaning it expects
    // this redo to re-add the *exact same instance*. This is complex with Qt's ownership.
    // Safest: redo always creates a new item if itemInstance_ is null (first run, or after undo deleted).
    if (itemInstance_) {
        qWarning() << "PlaceDecorationCommand::redo(): itemInstance_ is not null. This may indicate an issue in undo logic or command reuse.";
        // If we are to re-add this exact instance, tile->addItem should not take ownership again
        // or should handle it. For simplicity, let's assume undo always results in itemInstance_ being null.
        // If not, we might delete it here before creating a new one to avoid leaks.
        delete itemInstance_;
        itemInstance_ = nullptr;
    }

    // Item* newItem = ItemFactory::createItem(decorationItemId_);
    Item* newItem = new Item(decorationItemId_); // Assumes Item(quint16 id) constructor exists
                                                 // and sets up type info for Item::isDecoration() if needed.

    if (newItem) {
        // Optional: Check if newItem is actually a decoration type if that's relevant.
        // if (!newItem->isDecoration()) { qWarning() << "Item ID" << decorationItemId_ << "is not a decoration type."; }

        tile->addItem(newItem); // Tile::addItem should handle parenting and emit signals.
        itemInstance_ = newItem; // Store the instance we added.
        qDebug() << "PlaceDecorationCommand: Redone - Placed decoration ID" << decorationItemId_ << "at" << tilePos_;
    } else {
        qWarning() << "PlaceDecorationCommand::redo(): Could not create decoration item for ID" << decorationItemId_;
    }
    // No border/wall update typically needed for simple decorations.
}
