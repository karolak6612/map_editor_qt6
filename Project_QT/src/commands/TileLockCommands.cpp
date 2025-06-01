#include "TileLockCommands.h"
#include "Map.h"
#include "Tile.h"
#include "Selection.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
#include <QDateTime>
#include <QDebug>

// =============================================================================
// TileLockCommandBase Implementation
// =============================================================================

TileLockCommandBase::TileLockCommandBase(Map* map, QUndoCommand* parent)
    : QUndoCommand(parent), map_(map)
{
    Q_ASSERT(map_);
}

Tile* TileLockCommandBase::getTile(const MapPos& pos) const
{
    if (!map_ || !isValidPosition(pos)) {
        return nullptr;
    }
    return map_->getTile(pos.x, pos.y, pos.z);
}

bool TileLockCommandBase::isValidPosition(const MapPos& pos) const
{
    if (!map_) return false;
    return pos.x >= 0 && pos.y >= 0 && pos.z >= 0 && 
           pos.x < map_->getWidth() && pos.y < map_->getHeight() && pos.z < map_->getDepth();
}

void TileLockCommandBase::updateTileVisuals(const MapPos& pos)
{
    if (Tile* tile = getTile(pos)) {
        emit tile->visualChanged(pos.x, pos.y, pos.z);
    }
}

void TileLockCommandBase::updateTileVisuals(const QList<MapPos>& positions)
{
    for (const MapPos& pos : positions) {
        updateTileVisuals(pos);
    }
}

// =============================================================================
// LockTileCommand Implementation
// =============================================================================

LockTileCommand::LockTileCommand(Map* map, const MapPos& position, bool lock, QUndoCommand* parent)
    : TileLockCommandBase(map, parent)
    , position_(position)
    , lockState_(lock)
    , previousState_(false)
    , firstExecution_(true)
{
    setText(lockState_ ? QObject::tr("Lock Tile") : QObject::tr("Unlock Tile"));
    
    // Store the current state
    if (Tile* tile = getTile(position_)) {
        previousState_ = tile->isLocked();
    }
}

void LockTileCommand::undo()
{
    if (Tile* tile = getTile(position_)) {
        tile->setLocked(previousState_);
        updateTileVisuals(position_);
    }
}

void LockTileCommand::redo()
{
    if (firstExecution_) {
        firstExecution_ = false;
        // Store the current state before first execution
        if (Tile* tile = getTile(position_)) {
            previousState_ = tile->isLocked();
        }
    }
    
    if (Tile* tile = getTile(position_)) {
        tile->setLocked(lockState_);
        updateTileVisuals(position_);
    }
}

bool LockTileCommand::mergeWith(const QUndoCommand* other)
{
    const LockTileCommand* otherCommand = static_cast<const LockTileCommand*>(other);
    if (!otherCommand || otherCommand->position_ != position_) {
        return false;
    }
    
    // Merge if the final state is different from the original state
    if (otherCommand->lockState_ != previousState_) {
        lockState_ = otherCommand->lockState_;
        setText(lockState_ ? QObject::tr("Lock Tile") : QObject::tr("Unlock Tile"));
        return true;
    }
    
    // If we're back to the original state, this command can be discarded
    return true;
}

// =============================================================================
// LockTilesCommand Implementation
// =============================================================================

LockTilesCommand::LockTilesCommand(Map* map, const QList<MapPos>& positions, bool lock, QUndoCommand* parent)
    : TileLockCommandBase(map, parent)
    , positions_(positions)
    , lockState_(lock)
    , firstExecution_(true)
{
    setText(lockState_ ? QObject::tr("Lock %1 Tiles").arg(positions.size()) 
                      : QObject::tr("Unlock %1 Tiles").arg(positions.size()));
    
    storePreviousStates();
}

void LockTilesCommand::storePreviousStates()
{
    previousStates_.clear();
    previousStates_.reserve(positions_.size());
    
    for (const MapPos& pos : positions_) {
        if (Tile* tile = getTile(pos)) {
            previousStates_.append(tile->isLocked());
        } else {
            previousStates_.append(false);
        }
    }
}

void LockTilesCommand::restorePreviousStates()
{
    for (int i = 0; i < positions_.size() && i < previousStates_.size(); ++i) {
        if (Tile* tile = getTile(positions_[i])) {
            tile->setLocked(previousStates_[i]);
        }
    }
    updateTileVisuals(positions_);
}

void LockTilesCommand::applyLockState()
{
    for (const MapPos& pos : positions_) {
        if (Tile* tile = getTile(pos)) {
            tile->setLocked(lockState_);
        }
    }
    updateTileVisuals(positions_);
}

void LockTilesCommand::undo()
{
    restorePreviousStates();
}

void LockTilesCommand::redo()
{
    if (firstExecution_) {
        firstExecution_ = false;
        storePreviousStates();
    }
    
    applyLockState();
}

bool LockTilesCommand::mergeWith(const QUndoCommand* other)
{
    const LockTilesCommand* otherCommand = static_cast<const LockTilesCommand*>(other);
    if (!otherCommand || otherCommand->positions_ != positions_) {
        return false;
    }
    
    // Update the lock state to the latest command
    lockState_ = otherCommand->lockState_;
    setText(lockState_ ? QObject::tr("Lock %1 Tiles").arg(positions_.size()) 
                      : QObject::tr("Unlock %1 Tiles").arg(positions_.size()));
    return true;
}

// =============================================================================
// LockSelectionCommand Implementation
// =============================================================================

LockSelectionCommand::LockSelectionCommand(Map* map, Selection* selection, bool lock, QUndoCommand* parent)
    : TileLockCommandBase(map, parent)
    , lockState_(lock)
    , firstExecution_(true)
{
    extractSelectionPositions(selection);
    setText(lockState_ ? QObject::tr("Lock Selection (%1 tiles)").arg(selectedPositions_.size()) 
                      : QObject::tr("Unlock Selection (%1 tiles)").arg(selectedPositions_.size()));
    
    storePreviousStates();
}

void LockSelectionCommand::extractSelectionPositions(Selection* selection)
{
    selectedPositions_.clear();
    if (!selection) return;
    
    // Extract positions from selection
    // This would depend on the Selection class implementation
    // For now, we'll assume a method to get selected tile positions
    // selectedPositions_ = selection->getSelectedTilePositions();
}

void LockSelectionCommand::storePreviousStates()
{
    previousStates_.clear();
    previousStates_.reserve(selectedPositions_.size());
    
    for (const MapPos& pos : selectedPositions_) {
        if (Tile* tile = getTile(pos)) {
            previousStates_.append(tile->isLocked());
        } else {
            previousStates_.append(false);
        }
    }
}

void LockSelectionCommand::restorePreviousStates()
{
    for (int i = 0; i < selectedPositions_.size() && i < previousStates_.size(); ++i) {
        if (Tile* tile = getTile(selectedPositions_[i])) {
            tile->setLocked(previousStates_[i]);
        }
    }
    updateTileVisuals(selectedPositions_);
}

void LockSelectionCommand::applyLockState()
{
    for (const MapPos& pos : selectedPositions_) {
        if (Tile* tile = getTile(pos)) {
            tile->setLocked(lockState_);
        }
    }
    updateTileVisuals(selectedPositions_);
}

void LockSelectionCommand::undo()
{
    restorePreviousStates();
}

void LockSelectionCommand::redo()
{
    if (firstExecution_) {
        firstExecution_ = false;
        storePreviousStates();
    }
    
    applyLockState();
}

// =============================================================================
// LockAreaCommand Implementation
// =============================================================================

LockAreaCommand::LockAreaCommand(Map* map, const MapPos& topLeft, const MapPos& bottomRight, bool lock, QUndoCommand* parent)
    : TileLockCommandBase(map, parent)
    , topLeft_(topLeft)
    , bottomRight_(bottomRight)
    , lockState_(lock)
    , firstExecution_(true)
{
    calculateAffectedPositions();
    setText(lockState_ ? QObject::tr("Lock Area (%1 tiles)").arg(affectedPositions_.size()) 
                      : QObject::tr("Unlock Area (%1 tiles)").arg(affectedPositions_.size()));
    
    storePreviousStates();
}

void LockAreaCommand::calculateAffectedPositions()
{
    affectedPositions_.clear();
    
    int minX = qMin(topLeft_.x, bottomRight_.x);
    int maxX = qMax(topLeft_.x, bottomRight_.x);
    int minY = qMin(topLeft_.y, bottomRight_.y);
    int maxY = qMax(topLeft_.y, bottomRight_.y);
    int minZ = qMin(topLeft_.z, bottomRight_.z);
    int maxZ = qMax(topLeft_.z, bottomRight_.z);
    
    for (int z = minZ; z <= maxZ; ++z) {
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                MapPos pos(x, y, z);
                if (isValidPosition(pos)) {
                    affectedPositions_.append(pos);
                }
            }
        }
    }
}

void LockAreaCommand::storePreviousStates()
{
    previousStates_.clear();
    previousStates_.reserve(affectedPositions_.size());
    
    for (const MapPos& pos : affectedPositions_) {
        if (Tile* tile = getTile(pos)) {
            previousStates_.append(tile->isLocked());
        } else {
            previousStates_.append(false);
        }
    }
}

void LockAreaCommand::restorePreviousStates()
{
    for (int i = 0; i < affectedPositions_.size() && i < previousStates_.size(); ++i) {
        if (Tile* tile = getTile(affectedPositions_[i])) {
            tile->setLocked(previousStates_[i]);
        }
    }
    updateTileVisuals(affectedPositions_);
}

void LockAreaCommand::applyLockState()
{
    for (const MapPos& pos : affectedPositions_) {
        if (Tile* tile = getTile(pos)) {
            tile->setLocked(lockState_);
        }
    }
    updateTileVisuals(affectedPositions_);
}

void LockAreaCommand::undo()
{
    restorePreviousStates();
}

void LockAreaCommand::redo()
{
    if (firstExecution_) {
        firstExecution_ = false;
        storePreviousStates();
    }

    applyLockState();
}

// =============================================================================
// ToggleTileLockCommand Implementation
// =============================================================================

ToggleTileLockCommand::ToggleTileLockCommand(Map* map, const QList<MapPos>& positions, QUndoCommand* parent)
    : TileLockCommandBase(map, parent)
    , positions_(positions)
    , firstExecution_(true)
{
    setText(QObject::tr("Toggle Lock State (%1 tiles)").arg(positions.size()));

    storePreviousStates();
    calculateNewStates();
}

void ToggleTileLockCommand::storePreviousStates()
{
    originalStates_.clear();
    originalStates_.reserve(positions_.size());

    for (const MapPos& pos : positions_) {
        if (Tile* tile = getTile(pos)) {
            originalStates_.append(tile->isLocked());
        } else {
            originalStates_.append(false);
        }
    }
}

void ToggleTileLockCommand::calculateNewStates()
{
    newStates_.clear();
    newStates_.reserve(originalStates_.size());

    for (bool originalState : originalStates_) {
        newStates_.append(!originalState);
    }
}

void ToggleTileLockCommand::applyStates(const QList<bool>& states)
{
    for (int i = 0; i < positions_.size() && i < states.size(); ++i) {
        if (Tile* tile = getTile(positions_[i])) {
            tile->setLocked(states[i]);
        }
    }
    updateTileVisuals(positions_);
}

void ToggleTileLockCommand::undo()
{
    applyStates(originalStates_);
}

void ToggleTileLockCommand::redo()
{
    if (firstExecution_) {
        firstExecution_ = false;
        storePreviousStates();
        calculateNewStates();
    }

    applyStates(newStates_);
}

// =============================================================================
// LockTilesByFilterCommand Implementation
// =============================================================================

LockTilesByFilterCommand::LockTilesByFilterCommand(Map* map, FilterType filterType, const QVariant& filterValue, bool lock, QUndoCommand* parent)
    : TileLockCommandBase(map, parent)
    , filterType_(filterType)
    , filterValue_(filterValue)
    , lockState_(lock)
    , firstExecution_(true)
{
    findMatchingTiles();
    setText(lockState_ ? QObject::tr("Lock Tiles by Filter (%1 tiles)").arg(affectedPositions_.size())
                      : QObject::tr("Unlock Tiles by Filter (%1 tiles)").arg(affectedPositions_.size()));

    storePreviousStates();
}

void LockTilesByFilterCommand::findMatchingTiles()
{
    affectedPositions_.clear();
    if (!map_) return;

    // Iterate through all tiles in the map
    for (int z = 0; z < map_->getDepth(); ++z) {
        for (int y = 0; y < map_->getHeight(); ++y) {
            for (int x = 0; x < map_->getWidth(); ++x) {
                MapPos pos(x, y, z);
                if (Tile* tile = getTile(pos)) {
                    if (tileMatchesFilter(tile)) {
                        affectedPositions_.append(pos);
                    }
                }
            }
        }
    }
}

bool LockTilesByFilterCommand::tileMatchesFilter(Tile* tile) const
{
    if (!tile) return false;

    switch (filterType_) {
        case ByItemId: {
            quint16 itemId = filterValue_.toUInt();
            for (Item* item : tile->items()) {
                if (item && item->getId() == itemId) {
                    return true;
                }
            }
            if (tile->getGround() && tile->getGround()->getId() == itemId) {
                return true;
            }
            return false;
        }

        case IsEmpty:
            return tile->isEmpty();

        case IsModified:
            return tile->isModified();

        case IsBlocking:
            return tile->isBlocking();

        case HasGround:
            return tile->getGround() != nullptr;

        case HasItems:
            return tile->itemCount() > 0;

        case HasCreatures:
            return tile->hasCreatures();

        case ByHouseId: {
            quint32 houseId = filterValue_.toUInt();
            return tile->getHouseId() == houseId;
        }

        case ByZoneId: {
            quint16 zoneId = filterValue_.toUInt();
            return tile->hasZoneId(zoneId);
        }

        default:
            return false;
    }
}

void LockTilesByFilterCommand::storePreviousStates()
{
    previousStates_.clear();
    previousStates_.reserve(affectedPositions_.size());

    for (const MapPos& pos : affectedPositions_) {
        if (Tile* tile = getTile(pos)) {
            previousStates_.append(tile->isLocked());
        } else {
            previousStates_.append(false);
        }
    }
}

void LockTilesByFilterCommand::restorePreviousStates()
{
    for (int i = 0; i < affectedPositions_.size() && i < previousStates_.size(); ++i) {
        if (Tile* tile = getTile(affectedPositions_[i])) {
            tile->setLocked(previousStates_[i]);
        }
    }
    updateTileVisuals(affectedPositions_);
}

void LockTilesByFilterCommand::applyLockState()
{
    for (const MapPos& pos : affectedPositions_) {
        if (Tile* tile = getTile(pos)) {
            tile->setLocked(lockState_);
        }
    }
    updateTileVisuals(affectedPositions_);
}

void LockTilesByFilterCommand::undo()
{
    restorePreviousStates();
}

void LockTilesByFilterCommand::redo()
{
    if (firstExecution_) {
        firstExecution_ = false;
        findMatchingTiles();
        storePreviousStates();
    }

    applyLockState();
}

// =============================================================================
// TileLockUtils Implementation
// =============================================================================

QList<MapPos> TileLockUtils::getLockedTiles(Map* map)
{
    QList<MapPos> lockedTiles;
    if (!map) return lockedTiles;

    for (int z = 0; z < map->getDepth(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                if (Tile* tile = map->getTile(x, y, z)) {
                    if (tile->isLocked()) {
                        lockedTiles.append(MapPos(x, y, z));
                    }
                }
            }
        }
    }

    return lockedTiles;
}

QList<MapPos> TileLockUtils::getUnlockedTiles(Map* map)
{
    QList<MapPos> unlockedTiles;
    if (!map) return unlockedTiles;

    for (int z = 0; z < map->getDepth(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                if (Tile* tile = map->getTile(x, y, z)) {
                    if (!tile->isLocked()) {
                        unlockedTiles.append(MapPos(x, y, z));
                    }
                }
            }
        }
    }

    return unlockedTiles;
}

int TileLockUtils::getLockedTileCount(Map* map)
{
    int count = 0;
    if (!map) return count;

    for (int z = 0; z < map->getDepth(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                if (Tile* tile = map->getTile(x, y, z)) {
                    if (tile->isLocked()) {
                        count++;
                    }
                }
            }
        }
    }

    return count;
}

int TileLockUtils::getUnlockedTileCount(Map* map)
{
    int count = 0;
    if (!map) return count;

    for (int z = 0; z < map->getDepth(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                if (Tile* tile = map->getTile(x, y, z)) {
                    if (!tile->isLocked()) {
                        count++;
                    }
                }
            }
        }
    }

    return count;
}

QList<MapPos> TileLockUtils::getTilesInArea(Map* map, const MapPos& topLeft, const MapPos& bottomRight)
{
    QList<MapPos> tiles;
    if (!map) return tiles;

    int minX = qMin(topLeft.x, bottomRight.x);
    int maxX = qMax(topLeft.x, bottomRight.x);
    int minY = qMin(topLeft.y, bottomRight.y);
    int maxY = qMax(topLeft.y, bottomRight.y);
    int minZ = qMin(topLeft.z, bottomRight.z);
    int maxZ = qMax(topLeft.z, bottomRight.z);

    for (int z = minZ; z <= maxZ; ++z) {
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                if (map->getTile(x, y, z)) {
                    tiles.append(MapPos(x, y, z));
                }
            }
        }
    }

    return tiles;
}

QList<MapPos> TileLockUtils::getLockedTilesInArea(Map* map, const MapPos& topLeft, const MapPos& bottomRight)
{
    QList<MapPos> lockedTiles;
    if (!map) return lockedTiles;

    QList<MapPos> allTiles = getTilesInArea(map, topLeft, bottomRight);
    for (const MapPos& pos : allTiles) {
        if (Tile* tile = map->getTile(pos.x, pos.y, pos.z)) {
            if (tile->isLocked()) {
                lockedTiles.append(pos);
            }
        }
    }

    return lockedTiles;
}

QList<MapPos> TileLockUtils::getUnlockedTilesInArea(Map* map, const MapPos& topLeft, const MapPos& bottomRight)
{
    QList<MapPos> unlockedTiles;
    if (!map) return unlockedTiles;

    QList<MapPos> allTiles = getTilesInArea(map, topLeft, bottomRight);
    for (const MapPos& pos : allTiles) {
        if (Tile* tile = map->getTile(pos.x, pos.y, pos.z)) {
            if (!tile->isLocked()) {
                unlockedTiles.append(pos);
            }
        }
    }

    return unlockedTiles;
}

bool TileLockUtils::canLockTile(Tile* tile)
{
    return tile != nullptr && !tile->isLocked();
}

bool TileLockUtils::canUnlockTile(Tile* tile)
{
    return tile != nullptr && tile->isLocked();
}

bool TileLockUtils::isValidLockOperation(Map* map, const QList<MapPos>& positions, bool lock)
{
    if (!map || positions.isEmpty()) return false;

    for (const MapPos& pos : positions) {
        if (Tile* tile = map->getTile(pos.x, pos.y, pos.z)) {
            if (lock && !canLockTile(tile)) {
                return false;
            }
            if (!lock && !canUnlockTile(tile)) {
                return false;
            }
        }
    }

    return true;
}

QString TileLockUtils::getLockStatistics(Map* map)
{
    if (!map) return QString("No map available");

    int totalTiles = 0;
    int lockedTiles = 0;

    for (int z = 0; z < map->getDepth(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                if (Tile* tile = map->getTile(x, y, z)) {
                    totalTiles++;
                    if (tile->isLocked()) {
                        lockedTiles++;
                    }
                }
            }
        }
    }

    int unlockedTiles = totalTiles - lockedTiles;
    double lockPercentage = totalTiles > 0 ? (double(lockedTiles) / totalTiles) * 100.0 : 0.0;

    return QString("Tile Lock Statistics:\n"
                  "Total tiles: %1\n"
                  "Locked tiles: %2 (%.1f%%)\n"
                  "Unlocked tiles: %3 (%.1f%%)")
           .arg(totalTiles)
           .arg(lockedTiles).arg(lockPercentage)
           .arg(unlockedTiles).arg(100.0 - lockPercentage);
}

QVariantMap TileLockUtils::getLockStatisticsMap(Map* map)
{
    QVariantMap stats;
    if (!map) return stats;

    int totalTiles = 0;
    int lockedTiles = 0;

    for (int z = 0; z < map->getDepth(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                if (Tile* tile = map->getTile(x, y, z)) {
                    totalTiles++;
                    if (tile->isLocked()) {
                        lockedTiles++;
                    }
                }
            }
        }
    }

    int unlockedTiles = totalTiles - lockedTiles;
    double lockPercentage = totalTiles > 0 ? (double(lockedTiles) / totalTiles) * 100.0 : 0.0;

    stats["totalTiles"] = totalTiles;
    stats["lockedTiles"] = lockedTiles;
    stats["unlockedTiles"] = unlockedTiles;
    stats["lockPercentage"] = lockPercentage;
    stats["unlockPercentage"] = 100.0 - lockPercentage;

    return stats;
}

// =============================================================================
// TileLockManager Implementation
// =============================================================================

TileLockManager::TileLockManager(Map* map, QObject* parent)
    : QObject(parent), map_(map), historyEnabled_(true)
{
    Q_ASSERT(map_);

    // Connect to map signals for tracking changes
    if (map_) {
        connect(map_, &Map::tileChanged, this, &TileLockManager::onTileChanged);
        connect(map_, &Map::mapCleared, this, &TileLockManager::onMapCleared);
    }
}

void TileLockManager::lockTile(const MapPos& position, const QString& reason)
{
    if (Tile* tile = map_->getTile(position.x, position.y, position.z)) {
        bool wasLocked = tile->isLocked();
        if (!wasLocked) {
            tile->setLocked(true);
            recordLockStateChange(position, wasLocked, true, reason);
            emitLockStateChanged(position, true, reason);
        }
    }
}

void TileLockManager::unlockTile(const MapPos& position, const QString& reason)
{
    if (Tile* tile = map_->getTile(position.x, position.y, position.z)) {
        bool wasLocked = tile->isLocked();
        if (wasLocked) {
            tile->setLocked(false);
            recordLockStateChange(position, wasLocked, false, reason);
            emitLockStateChanged(position, false, reason);
        }
    }
}

void TileLockManager::lockTiles(const QList<MapPos>& positions, const QString& reason)
{
    QList<MapPos> changedPositions;

    for (const MapPos& pos : positions) {
        if (Tile* tile = map_->getTile(pos.x, pos.y, pos.z)) {
            bool wasLocked = tile->isLocked();
            if (!wasLocked) {
                tile->setLocked(true);
                recordLockStateChange(pos, wasLocked, true, reason);
                changedPositions.append(pos);
            }
        }
    }

    if (!changedPositions.isEmpty()) {
        emit tilesLocked(changedPositions, reason);
        emit statisticsChanged();
    }
}

void TileLockManager::unlockTiles(const QList<MapPos>& positions, const QString& reason)
{
    QList<MapPos> changedPositions;

    for (const MapPos& pos : positions) {
        if (Tile* tile = map_->getTile(pos.x, pos.y, pos.z)) {
            bool wasLocked = tile->isLocked();
            if (wasLocked) {
                tile->setLocked(false);
                recordLockStateChange(pos, wasLocked, false, reason);
                changedPositions.append(pos);
            }
        }
    }

    if (!changedPositions.isEmpty()) {
        emit tilesUnlocked(changedPositions, reason);
        emit statisticsChanged();
    }
}

void TileLockManager::toggleTileLock(const MapPos& position, const QString& reason)
{
    if (Tile* tile = map_->getTile(position.x, position.y, position.z)) {
        bool wasLocked = tile->isLocked();
        tile->setLocked(!wasLocked);
        recordLockStateChange(position, wasLocked, !wasLocked, reason);
        emitLockStateChanged(position, !wasLocked, reason);
    }
}

bool TileLockManager::isTileLocked(const MapPos& position) const
{
    if (Tile* tile = map_->getTile(position.x, position.y, position.z)) {
        return tile->isLocked();
    }
    return false;
}

QList<MapPos> TileLockManager::getLockedTiles() const
{
    return TileLockUtils::getLockedTiles(map_);
}

int TileLockManager::getLockedTileCount() const
{
    return TileLockUtils::getLockedTileCount(map_);
}

void TileLockManager::lockAll(const QString& reason)
{
    QList<MapPos> changedPositions;

    for (int z = 0; z < map_->getDepth(); ++z) {
        for (int y = 0; y < map_->getHeight(); ++y) {
            for (int x = 0; x < map_->getWidth(); ++x) {
                if (Tile* tile = map_->getTile(x, y, z)) {
                    bool wasLocked = tile->isLocked();
                    if (!wasLocked) {
                        tile->setLocked(true);
                        MapPos pos(x, y, z);
                        recordLockStateChange(pos, wasLocked, true, reason);
                        changedPositions.append(pos);
                    }
                }
            }
        }
    }

    if (!changedPositions.isEmpty()) {
        emit tilesLocked(changedPositions, reason);
        emit statisticsChanged();
    }
}

void TileLockManager::unlockAll(const QString& reason)
{
    QList<MapPos> changedPositions;

    for (int z = 0; z < map_->getDepth(); ++z) {
        for (int y = 0; y < map_->getHeight(); ++y) {
            for (int x = 0; x < map_->getWidth(); ++x) {
                if (Tile* tile = map_->getTile(x, y, z)) {
                    bool wasLocked = tile->isLocked();
                    if (wasLocked) {
                        tile->setLocked(false);
                        MapPos pos(x, y, z);
                        recordLockStateChange(pos, wasLocked, false, reason);
                        changedPositions.append(pos);
                    }
                }
            }
        }
    }

    if (!changedPositions.isEmpty()) {
        emit tilesUnlocked(changedPositions, reason);
        emit statisticsChanged();
    }
}

void TileLockManager::lockArea(const MapPos& topLeft, const MapPos& bottomRight, const QString& reason)
{
    QList<MapPos> positions = TileLockUtils::getTilesInArea(map_, topLeft, bottomRight);
    lockTiles(positions, reason);
}

void TileLockManager::unlockArea(const MapPos& topLeft, const MapPos& bottomRight, const QString& reason)
{
    QList<MapPos> positions = TileLockUtils::getTilesInArea(map_, topLeft, bottomRight);
    unlockTiles(positions, reason);
}

QString TileLockManager::getStatistics() const
{
    return TileLockUtils::getLockStatistics(map_);
}

QVariantMap TileLockManager::getStatisticsMap() const
{
    return TileLockUtils::getLockStatisticsMap(map_);
}

QList<TileLockStateChangeEvent> TileLockManager::getLockHistory() const
{
    return lockHistory_;
}

void TileLockManager::clearLockHistory()
{
    lockHistory_.clear();
}

void TileLockManager::setHistoryEnabled(bool enabled)
{
    historyEnabled_ = enabled;
    if (!enabled) {
        lockHistory_.clear();
    }
}

bool TileLockManager::isHistoryEnabled() const
{
    return historyEnabled_;
}

void TileLockManager::recordLockStateChange(const MapPos& position, bool wasLocked, bool isLocked, const QString& reason)
{
    if (historyEnabled_) {
        lockHistory_.append(TileLockStateChangeEvent(position, wasLocked, isLocked, reason));

        // Limit history size to prevent memory issues
        const int maxHistorySize = 10000;
        if (lockHistory_.size() > maxHistorySize) {
            lockHistory_.removeFirst();
        }
    }
}

void TileLockManager::emitLockStateChanged(const MapPos& position, bool isLocked, const QString& reason)
{
    if (isLocked) {
        emit tileLocked(position, reason);
    } else {
        emit tileUnlocked(position, reason);
    }
    emit lockStateChanged(position, isLocked, reason);
    emit statisticsChanged();
}

void TileLockManager::onTileChanged(int x, int y, int z)
{
    // Handle tile changes if needed
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(z)
}

void TileLockManager::onMapCleared()
{
    clearLockHistory();
    emit statisticsChanged();
}

#include "TileLockCommands.moc"
