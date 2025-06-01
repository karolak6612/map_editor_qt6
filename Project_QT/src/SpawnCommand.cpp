#include "SpawnCommand.h"
#include "Spawn.h"
#include "Tile.h"
#include <QDebug>

// PlaceSpawnCommand implementation
PlaceSpawnCommand::PlaceSpawnCommand(Map* map, const MapPos& position, Spawn* spawn, QUndoCommand* parent)
    : QUndoCommand(parent), map_(map), position_(position), spawn_(spawn), 
      previousSpawn_(nullptr), spawnWasCreated_(false) {
    
    setText(QString("Place spawn at (%1, %2, %3)").arg(position.x).arg(position.y).arg(position.z));
    
    // Check if there's already a spawn at this position
    Tile* tile = map_->getTile(position_.x, position_.y, position_.z);
    if (tile && tile->spawn()) {
        previousSpawn_ = tile->spawn();
    }
}

PlaceSpawnCommand::~PlaceSpawnCommand() {
    // Clean up spawn if it was created by this command and not placed
    if (spawnWasCreated_ && spawn_ && !spawn_->parent()) {
        delete spawn_;
    }
}

void PlaceSpawnCommand::undo() {
    Tile* tile = map_->getTile(position_.x, position_.y, position_.z);
    if (!tile) {
        qWarning() << "PlaceSpawnCommand::undo - Tile not found at position" << position_.x << position_.y << position_.z;
        return;
    }
    
    // Remove current spawn and restore previous one
    tile->setSpawn(previousSpawn_);
    
    // Remove from map's spawn list
    map_->removeSpawn(spawn_);
    
    qDebug() << "Spawn placement undone at" << position_.x << position_.y << position_.z;
}

void PlaceSpawnCommand::redo() {
    Tile* tile = map_->getTile(position_.x, position_.y, position_.z);
    if (!tile) {
        // Create tile if it doesn't exist
        tile = map_->createTile(position_.x, position_.y, position_.z);
        if (!tile) {
            qWarning() << "PlaceSpawnCommand::redo - Failed to create tile at position" << position_.x << position_.y << position_.z;
            return;
        }
    }
    
    // Set spawn position if not already set
    if (spawn_->position() != position_) {
        spawn_->setPosition(position_);
    }
    
    // Place spawn on tile
    tile->setSpawn(spawn_);
    
    // Add to map's spawn list
    map_->addSpawn(spawn_);
    
    qDebug() << "Spawn placed at" << position_.x << position_.y << position_.z;
}

// RemoveSpawnCommand implementation
RemoveSpawnCommand::RemoveSpawnCommand(Map* map, const MapPos& position, QUndoCommand* parent)
    : QUndoCommand(parent), map_(map), position_(position), removedSpawn_(nullptr) {
    
    setText(QString("Remove spawn at (%1, %2, %3)").arg(position.x).arg(position.y).arg(position.z));
    
    // Store the spawn that will be removed
    Tile* tile = map_->getTile(position_.x, position_.y, position_.z);
    if (tile) {
        removedSpawn_ = tile->spawn();
    }
}

RemoveSpawnCommand::~RemoveSpawnCommand() {
    // Clean up removed spawn if it's not restored
    if (removedSpawn_ && !removedSpawn_->parent()) {
        delete removedSpawn_;
    }
}

void RemoveSpawnCommand::undo() {
    if (!removedSpawn_) {
        qWarning() << "RemoveSpawnCommand::undo - No spawn to restore";
        return;
    }
    
    Tile* tile = map_->getTile(position_.x, position_.y, position_.z);
    if (!tile) {
        // Create tile if it doesn't exist
        tile = map_->createTile(position_.x, position_.y, position_.z);
        if (!tile) {
            qWarning() << "RemoveSpawnCommand::undo - Failed to create tile at position" << position_.x << position_.y << position_.z;
            return;
        }
    }
    
    // Restore spawn
    tile->setSpawn(removedSpawn_);
    map_->addSpawn(removedSpawn_);
    
    qDebug() << "Spawn removal undone at" << position_.x << position_.y << position_.z;
}

void RemoveSpawnCommand::redo() {
    Tile* tile = map_->getTile(position_.x, position_.y, position_.z);
    if (!tile || !tile->spawn()) {
        qWarning() << "RemoveSpawnCommand::redo - No spawn to remove at position" << position_.x << position_.y << position_.z;
        return;
    }
    
    // Remove spawn
    removedSpawn_ = tile->spawn();
    tile->setSpawn(nullptr);
    map_->removeSpawn(removedSpawn_);
    
    qDebug() << "Spawn removed from" << position_.x << position_.y << position_.z;
}

// EditSpawnCommand implementation
EditSpawnCommand::EditSpawnCommand(Spawn* spawn, const QStringList& newCreatureNames, 
                                   int newRadius, int newInterval, int newMaxCreatures,
                                   QUndoCommand* parent)
    : QUndoCommand(parent), spawn_(spawn),
      newCreatureNames_(newCreatureNames), newRadius_(newRadius), 
      newInterval_(newInterval), newMaxCreatures_(newMaxCreatures) {
    
    setText("Edit spawn properties");
    
    // Store old values
    oldCreatureNames_ = spawn_->creatureNames();
    oldRadius_ = spawn_->radius();
    oldInterval_ = spawn_->interval();
    oldMaxCreatures_ = spawn_->maxCreatures();
}

void EditSpawnCommand::undo() {
    spawn_->setCreatureNames(oldCreatureNames_);
    spawn_->setRadius(oldRadius_);
    spawn_->setInterval(oldInterval_);
    spawn_->setMaxCreatures(oldMaxCreatures_);
    
    qDebug() << "Spawn edit undone";
}

void EditSpawnCommand::redo() {
    spawn_->setCreatureNames(newCreatureNames_);
    spawn_->setRadius(newRadius_);
    spawn_->setInterval(newInterval_);
    spawn_->setMaxCreatures(newMaxCreatures_);
    
    qDebug() << "Spawn edited";
}
