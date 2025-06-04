#include "PlaceCreatureCommand.h"
#include "Map.h"
#include "Tile.h"
#include "Creature.h"
#include "CreatureManager.h"
#include <QDebug>

PlaceCreatureCommand::PlaceCreatureCommand(Map* map,
                                         const QPointF& tilePos,
                                         quint16 creatureId,
                                         const QString& creatureName,
                                         QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , tilePos_(tilePos)
    , creatureId_(creatureId)
    , creatureName_(creatureName)
    , creatureInstance_(nullptr)
    , tilePreviouslyExisted_(false)
{
    if (creatureId_ != 0) {
        setText(QObject::tr("Place Creature (ID: %1) at (%2,%3)")
                    .arg(creatureId_)
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    } else {
        setText(QObject::tr("Place Creature at (%1,%2)")
                    .arg(tilePos_.x())
                    .arg(tilePos_.y()));
    }
}

PlaceCreatureCommand::~PlaceCreatureCommand()
{
    // If we still own the creature instance (command was never executed or was undone),
    // we need to clean it up
    if (creatureInstance_ && !creatureInstance_->parent()) {
        delete creatureInstance_;
        creatureInstance_ = nullptr;
    }
}

void PlaceCreatureCommand::undo()
{
    if (!map_) {
        qWarning() << "PlaceCreatureCommand::undo(): Map pointer is null.";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    
    if (creatureInstance_ && tile) {
        // Remove creature from tile
        bool removed = tile->removeCreature(creatureInstance_);
        if (removed) {
            qDebug() << "PlaceCreatureCommand: Undone - Removed creature ID" << creatureId_ << "from" << tilePos_;
            // The creature instance is now owned by this command again
        } else {
            qWarning() << "PlaceCreatureCommand::undo(): Failed to remove creature instance" << creatureId_
                       << "from tile at" << tilePos_ << ". Creature might have been removed by other means.";
        }
    } else if (creatureInstance_ && !tile) {
        qWarning() << "PlaceCreatureCommand::undo(): Tile at" << tilePos_ << "not found, but a creature instance was recorded.";
    }

    // If tile was created by this command and is now empty, remove it
    if (!tilePreviouslyExisted_ && tile && tile->isEmpty()) {
        map_->removeTile(tilePos_);
        qDebug() << "PlaceCreatureCommand: Undone - Removed now-empty tile at" << tilePos_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
}

void PlaceCreatureCommand::redo()
{
    if (!map_ || creatureId_ == 0) {
        qWarning() << "PlaceCreatureCommand::redo(): Map is null or creatureId_ is 0 (no action).";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        tile = map_->getOrCreateTile(tilePos_);
        tilePreviouslyExisted_ = false; // Tile was created now
        if (!tile) {
            qWarning() << "PlaceCreatureCommand::redo(): Could not get or create tile at" << tilePos_;
            return;
        }
    } else {
        tilePreviouslyExisted_ = true; // Tile already existed
    }

    // Create creature instance if not already created
    if (!creatureInstance_) {
        CreatureManager* creatureManager = CreatureManager::getInstancePtr();
        if (creatureManager) {
            creatureInstance_ = creatureManager->createCreature(creatureId_);
            if (creatureInstance_ && !creatureName_.isEmpty()) {
                creatureInstance_->setName(creatureName_);
            }
        }
        
        if (!creatureInstance_) {
            // Fallback: create basic creature
            creatureInstance_ = new Creature(creatureId_);
            if (!creatureName_.isEmpty()) {
                creatureInstance_->setName(creatureName_);
            }
        }
    }

    if (creatureInstance_) {
        // Set creature position
        int x = qFloor(tilePos_.x());
        int y = qFloor(tilePos_.y());
        int z = qFloor(tilePos_.z());
        creatureInstance_->setPosition(MapPos(x, y, z));
        
        // Add creature to tile
        tile->addCreature(creatureInstance_);
        qDebug() << "PlaceCreatureCommand: Redone - Placed creature ID" << creatureId_ << "at" << tilePos_;
    } else {
        qWarning() << "PlaceCreatureCommand::redo(): Could not create creature for ID" << creatureId_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
}
