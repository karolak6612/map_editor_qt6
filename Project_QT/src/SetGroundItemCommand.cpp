#include "SetGroundItemCommand.h"
#include "Map.h"   // Include actual Map header
#include "Tile.h"  // Include actual Tile header for Tile operations
#include "Item.h"  // For creating/handling Item objects
// #include "ItemFactory.h" // Assuming an ItemFactory for creating items from IDs
#include <QDebug>
#include <QObject> // Required for QObject::tr

// Assumptions for Map methods (stubs might be needed in Map.h/cpp if not present):
// Tile* Map::getOrCreateTile(const QPointF& pos);
// Item* Tile::getGround() const; // Returns current ground item or nullptr
// void Tile::setGround(Item* groundItem); // Sets new ground, handles deletion of old if any
// void Tile::removeGround(); // Removes ground, handles deletion
// void Map::requestBorderUpdate(const QPointF& tilePos); // Placeholder for border system
// Item* ItemFactory::createItem(quint16 itemId); // Placeholder

// Map interaction refinement:
// void Map::setGround(const QPointF& pos, quint16 groundItemId); // Map handles item creation from ID
// void Map::removeGround(const QPointF& pos);


SetGroundItemCommand::SetGroundItemCommand(Map* map,
                                             const QPointF& tilePos,
                                             quint16 newGroundItemId,
                                             QUndoCommand* parent)
        : QUndoCommand(parent),
          map_(map),
          tilePos_(tilePos),
          newGroundItemId_(newGroundItemId),
          oldGroundItemId_(0), // Will be captured in first redo
          hadOldGround_(false), // Will be captured in first redo
          firstRedo_(true) { // Task 016: Flag to capture state only once

    // Generate command text
    if (newGroundItemId_ != 0) {
        setText(QObject::tr("Place Ground (ID: %1) at (%2,%3)")
                    .arg(newGroundItemId_)
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    } else {
        setText(QObject::tr("Remove Ground at (%1,%2)")
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    }
}

SetGroundItemCommand::~SetGroundItemCommand() {
    // If oldGroundItem_ was storing an actual Item* pointer not owned by a tile anymore after undo,
    // it might need deletion here, but storing ID is safer and current design.
}

void SetGroundItemCommand::undo() {
    if (!map_) {
        qWarning() << "SetGroundItemCommand::undo(): Map pointer is null.";
        return;
    }

    // Tile* tile = map_->getOrCreateTile(tilePos_); // Tile existence is assumed by Map methods
    // if (!tile) {
    //     qWarning() << "SetGroundItemCommand::undo(): Could not get or create tile at" << tilePos_;
    //     return;
    // }

    if (hadOldGround_) {
        // Restore the previously existing ground item (or explicit absence if oldGroundItemId_ is 0)
        map_->setGround(tilePos_, oldGroundItemId_); // Assumes setGround(pos, 0) is equivalent to removeGround(pos) or handles it.
        qDebug() << "SetGroundItemCommand: Undone - Restored/Set ground ID" << oldGroundItemId_ << "at" << tilePos_;
    } else {
        // There was no ground item recorded before this command's first redo.
        // This means the original state was "no ground item".
        map_->removeGround(tilePos_);
        qDebug() << "SetGroundItemCommand: Undone - Cleared ground at" << tilePos_ << "(no prior ground was recorded).";
    }

    // Ensure map is marked as modified
    map_->setModified(true);
    map_->requestBorderUpdate(tilePos_); // Request border update
}

void SetGroundItemCommand::redo() {
    if (!map_) {
        qWarning() << "SetGroundItemCommand::redo(): Map pointer is null.";
        return;
    }

    // Task 016: Capture old state only on first redo to avoid overwriting original state
    if (firstRedo_) {
        Tile* tile = map_->getOrCreateTile(tilePos_);
        if (tile) {
            Item* currentGround = tile->getGround();
            if (currentGround) {
                oldGroundItemId_ = currentGround->getID();
                hadOldGround_ = true;
                qDebug() << "SetGroundItemCommand: Captured old ground ID" << oldGroundItemId_ << "at" << tilePos_;
            } else {
                oldGroundItemId_ = 0;
                hadOldGround_ = false;
                qDebug() << "SetGroundItemCommand: No ground item to capture at" << tilePos_;
            }
        } else {
            // Cannot determine old state if tile doesn't exist. Assume no ground.
            oldGroundItemId_ = 0;
            hadOldGround_ = false;
            qWarning() << "SetGroundItemCommand::redo(): Could not get tile to capture old state for" << tilePos_;
        }
        firstRedo_ = false; // Mark that we've captured the state
    }

    // Apply the new state
    if (newGroundItemId_ != 0) {
        map_->setGround(tilePos_, newGroundItemId_);
        qDebug() << "SetGroundItemCommand: Redone - Set ground ID" << newGroundItemId_ << "at" << tilePos_;
    } else {
        map_->removeGround(tilePos_);
        qDebug() << "SetGroundItemCommand: Redone - Removed ground at" << tilePos_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
    map_->requestBorderUpdate(tilePos_);
}
