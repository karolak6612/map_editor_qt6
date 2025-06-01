#include "PasteCommand.h"
#include "Tile.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
#include <QDebug>

// --- PasteCommand Implementation ---

PasteCommand::PasteCommand(Map* map, const ClipboardData& clipboardData, 
                           const MapPos& targetPosition, PasteMode mode,
                           QUndoCommand* parent)
    : QUndoCommand(parent), map_(map), clipboardData_(clipboardData), 
      targetPosition_(targetPosition), pasteMode_(mode), executed_(false) {
    
    setText(QString("Paste %1 tiles at (%2, %3, %4)")
            .arg(clipboardData_.getTileCount())
            .arg(targetPosition.x).arg(targetPosition.y).arg(targetPosition.z));
}

PasteCommand::~PasteCommand() {
    // Clean up stored tiles if command was never executed or was undone
    if (!executed_) {
        for (const OriginalTileData& data : originalTiles_) {
            if (data.originalTile && !data.originalTile->parent()) {
                delete data.originalTile;
            }
        }
    }
}

void PasteCommand::undo() {
    if (!executed_) {
        qWarning() << "PasteCommand::undo - Command was never executed";
        return;
    }
    
    restoreOriginalTiles();
    executed_ = false;
    qDebug() << "Paste operation undone at" << targetPosition_.x << targetPosition_.y << targetPosition_.z;
}

void PasteCommand::redo() {
    if (!executed_) {
        storeOriginalTiles();
    }
    
    applyPaste();
    executed_ = true;
    qDebug() << "Paste operation applied at" << targetPosition_.x << targetPosition_.y << targetPosition_.z;
}

QList<MapPos> PasteCommand::getModifiedPositions() const {
    return modifiedPositions_;
}

void PasteCommand::storeOriginalTiles() {
    originalTiles_.clear();
    modifiedPositions_.clear();
    
    const QList<ClipboardTileData>& clipboardTiles = clipboardData_.getTilesData();
    
    for (const ClipboardTileData& clipboardTile : clipboardTiles) {
        MapPos targetPos(targetPosition_.x + clipboardTile.relativeX,
                        targetPosition_.y + clipboardTile.relativeY,
                        targetPosition_.z + clipboardTile.relativeZ);
        
        OriginalTileData originalData;
        originalData.position = targetPos;
        originalData.originalTile = map_->getTile(targetPos.x, targetPos.y, targetPos.z);
        originalData.tileExisted = (originalData.originalTile != nullptr);
        
        // Create a deep copy of the original tile if it exists
        if (originalData.originalTile) {
            // For now, we'll store the tile pointer and rely on proper cleanup
            // In a full implementation, we'd create a deep copy
        }
        
        originalTiles_.append(originalData);
        modifiedPositions_.append(targetPos);
    }
}

void PasteCommand::restoreOriginalTiles() {
    for (const OriginalTileData& data : originalTiles_) {
        if (data.tileExisted && data.originalTile) {
            // Restore the original tile
            map_->setTile(data.position.x, data.position.y, data.position.z, data.originalTile);
        } else {
            // Remove the tile that was created during paste
            map_->removeTile(data.position.x, data.position.y, data.position.z);
        }
    }
}

void PasteCommand::applyPaste() {
    const QList<ClipboardTileData>& clipboardTiles = clipboardData_.getTilesData();
    
    for (const ClipboardTileData& clipboardTile : clipboardTiles) {
        MapPos targetPos(targetPosition_.x + clipboardTile.relativeX,
                        targetPosition_.y + clipboardTile.relativeY,
                        targetPosition_.z + clipboardTile.relativeZ);
        
        Tile* existingTile = map_->getTile(targetPos.x, targetPos.y, targetPos.z);
        
        if (pasteMode_ == REPLACE_MODE || !existingTile) {
            // Replace mode or no existing tile - create new tile
            Tile* newTile = createTileFromClipboardData(clipboardTile, targetPos);
            if (newTile) {
                map_->setTile(targetPos.x, targetPos.y, targetPos.z, newTile);
            }
        } else if (pasteMode_ == MERGE_MODE && existingTile) {
            // Merge mode - merge with existing tile
            mergeTileWithClipboardData(existingTile, clipboardTile);
        }
    }
}

Tile* PasteCommand::createTileFromClipboardData(const ClipboardTileData& clipboardTile, const MapPos& targetPos) {
    Tile* tile = new Tile(targetPos);
    
    // Set ground item if present
    if (clipboardTile.hasGround) {
        // Create ground item from clipboard data
        // This would need proper Item creation from the stored data
        qDebug() << "Creating ground item from clipboard data (stub)";
    }
    
    // Add items
    for (const ClipboardItemData& itemData : clipboardTile.items) {
        // Create item from clipboard data
        // This would need proper Item creation from the stored data
        qDebug() << "Creating item from clipboard data (stub)";
    }
    
    // Add creatures
    for (const ClipboardCreatureData& creatureData : clipboardTile.creatures) {
        // Create creature from clipboard data
        // This would need proper Creature creation from the stored data
        qDebug() << "Creating creature from clipboard data (stub)";
    }
    
    // Set spawn if present
    if (clipboardTile.hasSpawn) {
        // Create spawn from clipboard data
        // This would need proper Spawn creation from the stored data
        qDebug() << "Creating spawn from clipboard data (stub)";
    }
    
    return tile;
}

void PasteCommand::mergeTileWithClipboardData(Tile* existingTile, const ClipboardTileData& clipboardTile) {
    // Merge clipboard data with existing tile
    // This would involve adding items, creatures, etc. to the existing tile
    qDebug() << "Merging clipboard data with existing tile (stub)";
}

// --- CutCommand Implementation ---

CutCommand::CutCommand(Map* map, const QSet<MapPos>& selection, ClipboardData& clipboardData,
                       QUndoCommand* parent)
    : QUndoCommand(parent), map_(map), selection_(selection), 
      clipboardData_(&clipboardData), executed_(false) {
    
    setText(QString("Cut %1 tiles").arg(selection.size()));
}

CutCommand::~CutCommand() {
    // Clean up stored tiles if command was never executed or was undone
    if (!executed_) {
        for (Tile* tile : originalTiles_) {
            if (tile && !tile->parent()) {
                delete tile;
            }
        }
    }
}

void CutCommand::undo() {
    if (!executed_) {
        qWarning() << "CutCommand::undo - Command was never executed";
        return;
    }
    
    restoreOriginalTiles();
    executed_ = false;
    qDebug() << "Cut operation undone";
}

void CutCommand::redo() {
    if (!executed_) {
        storeOriginalTiles();
    }
    
    applyCut();
    executed_ = true;
    qDebug() << "Cut operation applied";
}

void CutCommand::storeOriginalTiles() {
    originalTiles_.clear();
    originalPositions_.clear();
    
    for (const MapPos& pos : selection_) {
        Tile* tile = map_->getTile(pos.x, pos.y, pos.z);
        if (tile) {
            originalTiles_.append(tile);
            originalPositions_.append(pos);
        }
    }
}

void CutCommand::restoreOriginalTiles() {
    for (int i = 0; i < originalTiles_.size(); ++i) {
        if (i < originalPositions_.size()) {
            const MapPos& pos = originalPositions_[i];
            Tile* tile = originalTiles_[i];
            map_->setTile(pos.x, pos.y, pos.z, tile);
        }
    }
}

void CutCommand::applyCut() {
    // First populate clipboard data
    clipboardData_->populateFromSelectionCut(selection_, *map_);
    
    // Then remove the tiles
    for (const MapPos& pos : selection_) {
        map_->removeTile(pos.x, pos.y, pos.z);
    }
}
