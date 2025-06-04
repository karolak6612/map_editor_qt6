#include "RemoveCreatureCommand.h"
#include "Map.h"
#include "Tile.h"
#include "Creature.h"
#include <QDebug>

RemoveCreatureCommand::RemoveCreatureCommand(Map* map,
                                           const QPointF& tilePos,
                                           Creature* creature,
                                           QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , tilePos_(tilePos)
    , creatureInstance_(creature)
    , creatureId_(0)
    , creatureWasRemoved_(false)
    , tileBecameEmpty_(false)
{
    if (creatureInstance_) {
        creatureId_ = creatureInstance_->getId();
        creatureName_ = creatureInstance_->getName();
        setText(QObject::tr("Remove Creature (ID: %1) from (%2,%3)")
                    .arg(creatureId_)
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    } else {
        setText(QObject::tr("Remove Creature from (%1,%2)")
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    }
}

RemoveCreatureCommand::~RemoveCreatureCommand()
{
    // If we own the creature instance (it was removed and not restored),
    // we need to clean it up
    if (creatureInstance_ && creatureWasRemoved_) {
        delete creatureInstance_;
        creatureInstance_ = nullptr;
    }
}

void RemoveCreatureCommand::undo()
{
    if (!map_ || !creatureInstance_) {
        qWarning() << "RemoveCreatureCommand::undo(): Map or creature instance is null.";
        return;
    }

    // Get or create tile
    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        tile = map_->getOrCreateTile(tilePos_);
        if (!tile) {
            qWarning() << "RemoveCreatureCommand::undo(): Could not get or create tile at" << tilePos_;
            return;
        }
    }

    // Restore creature to tile
    tile->addCreature(creatureInstance_);
    creatureWasRemoved_ = false; // Creature is now back on the map
    
    qDebug() << "RemoveCreatureCommand: Undone - Restored creature ID" << creatureId_ << "to" << tilePos_;

    // Ensure map is marked as modified
    map_->setModified(true);
}

void RemoveCreatureCommand::redo()
{
    if (!map_) {
        qWarning() << "RemoveCreatureCommand::redo(): Map pointer is null.";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        qWarning() << "RemoveCreatureCommand::redo(): No tile found at" << tilePos_;
        return;
    }

    // If no specific creature was provided, find the first creature on the tile
    if (!creatureInstance_) {
        const QList<Creature*>& creatures = tile->getCreatures();
        if (!creatures.isEmpty()) {
            creatureInstance_ = creatures.first();
            creatureId_ = creatureInstance_->getId();
            creatureName_ = creatureInstance_->getName();
        } else {
            qWarning() << "RemoveCreatureCommand::redo(): No creatures found on tile at" << tilePos_;
            return;
        }
    }

    // Remove creature from tile
    bool removed = tile->removeCreature(creatureInstance_);
    if (removed) {
        creatureWasRemoved_ = true; // We now own the creature instance
        qDebug() << "RemoveCreatureCommand: Redone - Removed creature ID" << creatureId_ << "from" << tilePos_;
        
        // Check if tile became empty
        if (tile->isEmpty()) {
            tileBecameEmpty_ = true;
            // Note: We don't automatically remove empty tiles as other systems might depend on them
        }
    } else {
        qWarning() << "RemoveCreatureCommand::redo(): Failed to remove creature from tile at" << tilePos_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
}
