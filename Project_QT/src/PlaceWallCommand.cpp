#include "PlaceWallCommand.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
// #include "ItemFactory.h" // If item creation is via a factory
#include <QDebug>
#include <QObject> // For QObject::tr

// Assumptions for Map/Tile methods:
// Tile* Map::getTile(const QPointF& pos); (Used in undo)
// Tile* Map::getOrCreateTile(const QPointF& pos); (Used in redo)
// void Map::removeTile(const QPointF& pos); (Placeholder for undo if tile was created)
// QList<Item*> Tile::getWallItems() const; // Gets all items considered walls
// void Tile::clearWalls(); // Removes all wall items
// void Tile::addWallItemById(quint16 wallItemId); // Adds a wall item by ID
// void Map::requestWallUpdate(const QPointF& tilePos);
// Item* ItemFactory::createItem(quint16 itemId); // Alternative to addWallItemById

PlaceWallCommand::PlaceWallCommand(Map* map,
                                     const QPointF& tilePos,
                                     quint16 wallItemId,
                                     QUndoCommand* parent)
    : QUndoCommand(parent),
      map_(map),
      tilePos_(tilePos),
      newWallItemId_(wallItemId),
      tilePreviouslyExisted_(false) { // Initialized
    // oldWallStates_ is default initialized (empty list)
    if (newWallItemId_ != 0) {
        setText(QObject::tr("Place Wall (ID: %1) at (%2,%3)")
                    .arg(newWallItemId_)
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    } else {
        // This command is for placing walls. If newWallItemId_ is 0, it's essentially "place nothing",
        // which might be interpreted as clear. A separate "RemoveWallCommand" might be clearer.
        // For now, assume 0 means "clear walls of this type" or "place an 'empty' wall marker if such exists".
        // Let's adjust text if newWallItemId_ is 0 to reflect clearing.
        setText(QObject::tr("Clear Wall at (%1,%2)")
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    }
}

PlaceWallCommand::~PlaceWallCommand() {
    // If oldWallStates_ stored Item pointers, they'd need cleanup if not owned by tile after undo.
    // Storing IDs avoids this.
}

void PlaceWallCommand::undo() {
    if (!map_) {
        qWarning() << "PlaceWallCommand::undo(): Map pointer is null.";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);

    if (!tilePreviouslyExisted_ && tile) {
        // If the tile was created by this command's redo, attempt to remove it.
        // This assumes removeTile is safe and possibly checks if tile is empty.
        map_->removeTile(tilePos_);
        qDebug() << "PlaceWallCommand: Undone - Removed tile at" << tilePos_ << "that was created by this command.";
    } else if (tile) {
        tile->clearWalls(); // Remove whatever walls were placed by redo()
        for (const OldWallState& oldState : oldWallStates_) {
            if (oldState.itemId != 0) { // Don't try to add an "empty" item
                 tile->addWallItemById(oldState.itemId); // Assumes Tile::addWallItemById(quint16)
            }
        }
        qDebug() << "PlaceWallCommand: Undone - Restored" << oldWallStates_.count() << "old wall items (IDs) at" << tilePos_;
    } else if (tilePreviouslyExisted_) {
        // Tile was expected to exist but doesn't. This is an unexpected state.
        qWarning() << "PlaceWallCommand::undo(): Tile at" << tilePos_ << "not found, but it was expected to exist.";
    }
    // If !tilePreviouslyExisted_ and !tile, nothing to do.

    // Ensure map is marked as modified
    map_->setModified(true);

    map_->requestWallUpdate(tilePos_);
    map_->requestWallUpdate(tilePos_ + QPointF(0, -1)); // N
    map_->requestWallUpdate(tilePos_ + QPointF(0, 1));  // S
    map_->requestWallUpdate(tilePos_ + QPointF(-1, 0)); // W
    map_->requestWallUpdate(tilePos_ + QPointF(1, 0));  // E
}

void PlaceWallCommand::redo() {
    if (!map_) {
        qWarning() << "PlaceWallCommand::redo(): Map pointer is null.";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        tile = map_->getOrCreateTile(tilePos_);
        tilePreviouslyExisted_ = false; // Tile was created now
        if (!tile) {
            qWarning() << "PlaceWallCommand::redo(): Could not get or create tile at" << tilePos_;
            return;
        }
    } else {
        tilePreviouslyExisted_ = true; // Tile already existed
    }

    // Capture old wall state *before* clearing
    oldWallStates_.clear();
    QList<Item*> currentWalls = tile->getWallItems(); // Assumes Tile::getWallItems
    for (Item* oldWall : currentWalls) {
        if (oldWall) {
            oldWallStates_.append({oldWall->getID()});
        }
    }

    tile->clearWalls(); // Remove existing walls before adding the new one

    if (newWallItemId_ != 0) {
         tile->addWallItemById(newWallItemId_); // Assumes Tile::addWallItemById(quint16)
         qDebug() << "PlaceWallCommand: Redone - Placed wall ID" << newWallItemId_ << "at" << tilePos_;
    } else {
        // If newWallItemId_ is 0, clearWalls() already did the job.
        qDebug() << "PlaceWallCommand: Redone - Cleared walls at" << tilePos_ << "(newWallItemId was 0).";
    }

    // Ensure map is marked as modified
    map_->setModified(true);

    map_->requestWallUpdate(tilePos_);
    map_->requestWallUpdate(tilePos_ + QPointF(0, -1)); // N
    map_->requestWallUpdate(tilePos_ + QPointF(0, 1));  // S
    map_->requestWallUpdate(tilePos_ + QPointF(-1, 0)); // W
    map_->requestWallUpdate(tilePos_ + QPointF(1, 0));  // E
}
