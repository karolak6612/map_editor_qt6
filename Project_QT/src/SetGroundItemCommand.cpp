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
          oldGroundItemId_(0), // Initialized
          hadOldGround_(false) { // Initialized

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

    map_->requestBorderUpdate(tilePos_); // Request border update
}

void SetGroundItemCommand::redo() {
    if (!map_) {
        qWarning() << "SetGroundItemCommand::redo(): Map pointer is null.";
        return;
    }

    Tile* tile = map_->getOrCreateTile(tilePos_); // Ensure tile exists to get its current state
    if (!tile) {
        qWarning() << "SetGroundItemCommand::redo(): Could not get or create tile at" << tilePos_;
        // Depending on Map::setGround/removeGround, this might not be fatal if they also create tiles.
        // However, capturing old state requires the tile.
        // If tile cannot be obtained, we cannot safely capture old state.
        // For now, proceed, Map methods will have to be robust.
    }

    // Capture current state for undo, only if it's the first time redo is called
    // (or if the command is re-pushed after an undo sequence).
    // QUndoStack usually doesn't re-use command objects in a way that firstRedo_ is strictly needed
    // after an undo if the command remains on stack. But if command is recreated, it's good.
    // More robust: capture state if old state is not yet known (e.g. oldGroundItemId_ hasn't been set meaningfully)
    // For simplicity, the provided logic used a firstRedo_ flag, but it's often better to capture
    // state *before* the first change. Here, we capture it *during* the first redo.

    // In QUndoCommand, redo() is called when the command is first pushed, and then for every subsequent redo.
    // So, we need to get the state of the tile *before* this command makes its change.
    // This should happen *before* newGroundItemId_ is applied.

    // If old state hasn't been captured yet (e.g., oldGroundItemId_ is still its initial default,
    // or we use a specific flag like firstApplication_).
    // The current text uses a firstRedo_ flag in SetTileColorCommand. Let's adapt that.
    // For SetGroundItemCommand, this is slightly different as a tile might not exist.
    // The state capture should ideally be done before the first redo() if possible,
    // or if tile is guaranteed to exist, here.

    if (tile) { // Only capture if tile exists
        Item* currentGround = tile->getGround();
        if (currentGround) {
            if (!hadOldGround_ && oldGroundItemId_ == 0) { // Capture only if not already set by a previous redo on this instance
                 oldGroundItemId_ = currentGround->getID();
            }
            hadOldGround_ = true; // Mark that there was something (or we've stored its ID)
        } else {
             if (!hadOldGround_ && oldGroundItemId_ == 0) {
                // No item, so old ID is 0
             }
            hadOldGround_ = false; // Mark that there was no ground item
            oldGroundItemId_ = 0; // Ensure it's 0 if no item
        }
    } else {
        // Cannot determine old state if tile doesn't exist. Assume no ground.
        hadOldGround_ = false;
        oldGroundItemId_ = 0;
    }
    // The logic for capturing old state on first redo is tricky if the command can be undone and redone multiple times.
    // A more common pattern is for the command constructor to receive the *old* state.
    // However, following the pattern from SetTileColorCommand:
    // (This part is problematic if redo is called multiple times after undos)
    // A better way for SetTileColorCommand's `firstRedo_` was:
    // if (firstRedo_) { oldColor_ = tile->getDebugColor(); firstRedo_ = false; }
    // This means oldColor_ is captured *once*. Let's assume this command instance is fresh or firstRedo_ is reset if reused.
    // For SetGroundItemCommand, the initial state (oldGroundItemId_, hadOldGround_) should be captured
    // when the command is *created*, not on first redo.
    // Let's adjust: Assume constructor receives enough info, or we query map *before* first change.
    // The current structure has redo() making the change and *then* potentially capturing.
    // This should be: capture old state, then make change.
    // The provided SetTileColorCommand captures oldColor_ in redo() *before* setting new.

    // Let's refine redo() for SetGroundItemCommand:
    // 1. Get current ground item on tile (this is what will become "old" if we change it)
    // This is the state *before* this redo action.
    Tile* tileForOldState = map_->getOrCreateTile(tilePos_); // Re-fetch or use 'tile' if valid
    if (tileForOldState) {
        Item* groundBeforeChange = tileForOldState->getGround();
        if (groundBeforeChange) {
            oldGroundItemId_ = groundBeforeChange->getID();
            hadOldGround_ = true;
        } else {
            oldGroundItemId_ = 0;
            hadOldGround_ = false;
        }
    } else { // Should not happen if getOrCreateTile works
        oldGroundItemId_ = 0;
        hadOldGround_ = false;
        qWarning() << "SetGroundItemCommand::redo(): Could not get tile to save old state for" << tilePos_;
    }


    // 2. Apply the new state
    if (newGroundItemId_ != 0) {
        map_->setGround(tilePos_, newGroundItemId_);
        qDebug() << "SetGroundItemCommand: Redone - Set ground ID" << newGroundItemId_ << "at" << tilePos_;
    } else {
        map_->removeGround(tilePos_);
        qDebug() << "SetGroundItemCommand: Redone - Removed ground at" << tilePos_;
    }

    map_->requestBorderUpdate(tilePos_);
}
