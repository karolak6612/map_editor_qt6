#include "commands/TileLockCommands.h"
#include "Map.h"
#include "Tile.h"
#include "Selection.h"
#include <QDebug>
#include <QDateTime>

// =============================================================================
// TileLockManager Implementation
// =============================================================================

TileLockManager::TileLockManager(Map* map, QObject* parent)
    : QObject(parent)
    , map_(map)
    , historyEnabled_(true)
{
    Q_ASSERT(map_);
    
    // Connect to map signals for automatic cleanup
    if (map_) {
        connect(map_, &Map::mapCleared, this, &TileLockManager::onMapCleared);
        // Connect to tile change signals if available
        // connect(map_, &Map::tileChanged, this, &TileLockManager::onTileChanged);
    }
    
    qDebug() << "TileLockManager: Initialized for map";
}

// Lock operations
void TileLockManager::lockTile(const MapPos& position, const QString& reason)
{
    if (!map_) return;
    
    Tile* tile = map_->getTile(position.x, position.y, position.z);
    if (!tile) return;
    
    bool wasLocked = tile->isLocked();
    if (!wasLocked) {
        tile->setLocked(true);
        recordLockStateChange(position, wasLocked, true, reason);
        emitLockStateChanged(position, true, reason);
        emit tileLocked(position, reason);
    }
}

void TileLockManager::unlockTile(const MapPos& position, const QString& reason)
{
    if (!map_) return;
    
    Tile* tile = map_->getTile(position.x, position.y, position.z);
    if (!tile) return;
    
    bool wasLocked = tile->isLocked();
    if (wasLocked) {
        tile->setLocked(false);
        recordLockStateChange(position, wasLocked, false, reason);
        emitLockStateChanged(position, false, reason);
        emit tileUnlocked(position, reason);
    }
}

void TileLockManager::lockTiles(const QList<MapPos>& positions, const QString& reason)
{
    if (!map_ || positions.isEmpty()) return;
    
    QList<MapPos> actuallyLocked;
    
    for (const MapPos& pos : positions) {
        Tile* tile = map_->getTile(pos.x, pos.y, pos.z);
        if (tile && !tile->isLocked()) {
            tile->setLocked(true);
            recordLockStateChange(pos, false, true, reason);
            actuallyLocked.append(pos);
        }
    }
    
    if (!actuallyLocked.isEmpty()) {
        emit tilesLocked(actuallyLocked, reason);
        emit statisticsChanged();
    }
}

void TileLockManager::unlockTiles(const QList<MapPos>& positions, const QString& reason)
{
    if (!map_ || positions.isEmpty()) return;
    
    QList<MapPos> actuallyUnlocked;
    
    for (const MapPos& pos : positions) {
        Tile* tile = map_->getTile(pos.x, pos.y, pos.z);
        if (tile && tile->isLocked()) {
            tile->setLocked(false);
            recordLockStateChange(pos, true, false, reason);
            actuallyUnlocked.append(pos);
        }
    }
    
    if (!actuallyUnlocked.isEmpty()) {
        emit tilesUnlocked(actuallyUnlocked, reason);
        emit statisticsChanged();
    }
}

void TileLockManager::toggleTileLock(const MapPos& position, const QString& reason)
{
    if (!map_) return;
    
    Tile* tile = map_->getTile(position.x, position.y, position.z);
    if (!tile) return;
    
    bool wasLocked = tile->isLocked();
    tile->setLocked(!wasLocked);
    recordLockStateChange(position, wasLocked, !wasLocked, reason);
    emitLockStateChanged(position, !wasLocked, reason);
    
    if (wasLocked) {
        emit tileUnlocked(position, reason);
    } else {
        emit tileLocked(position, reason);
    }
}

// Query operations
bool TileLockManager::isTileLocked(const MapPos& position) const
{
    if (!map_) return false;
    
    Tile* tile = map_->getTile(position.x, position.y, position.z);
    return tile ? tile->isLocked() : false;
}

QList<MapPos> TileLockManager::getLockedTiles() const
{
    return TileLockUtils::getLockedTiles(map_);
}

int TileLockManager::getLockedTileCount() const
{
    return TileLockUtils::getLockedTileCount(map_);
}

// Batch operations
void TileLockManager::lockAll(const QString& reason)
{
    if (!map_) return;
    
    QList<MapPos> allPositions;
    
    for (int z = 0; z < map_->getDepth(); ++z) {
        for (int y = 0; y < map_->getHeight(); ++y) {
            for (int x = 0; x < map_->getWidth(); ++x) {
                if (map_->getTile(x, y, z)) {
                    allPositions.append(MapPos(x, y, z));
                }
            }
        }
    }
    
    lockTiles(allPositions, reason.isEmpty() ? tr("Lock All") : reason);
}

void TileLockManager::unlockAll(const QString& reason)
{
    if (!map_) return;
    
    QList<MapPos> lockedPositions = getLockedTiles();
    unlockTiles(lockedPositions, reason.isEmpty() ? tr("Unlock All") : reason);
}

void TileLockManager::lockSelection(Selection* selection, const QString& reason)
{
    if (!selection) return;
    
    QSet<MapPos> selectedTiles = selection->getSelectedTiles();
    QList<MapPos> positions;
    positions.reserve(selectedTiles.size());
    
    for (const MapPos& pos : selectedTiles) {
        positions.append(pos);
    }
    
    lockTiles(positions, reason.isEmpty() ? tr("Lock Selection") : reason);
}

void TileLockManager::unlockSelection(Selection* selection, const QString& reason)
{
    if (!selection) return;
    
    QSet<MapPos> selectedTiles = selection->getSelectedTiles();
    QList<MapPos> positions;
    positions.reserve(selectedTiles.size());
    
    for (const MapPos& pos : selectedTiles) {
        positions.append(pos);
    }
    
    unlockTiles(positions, reason.isEmpty() ? tr("Unlock Selection") : reason);
}

void TileLockManager::lockArea(const MapPos& topLeft, const MapPos& bottomRight, const QString& reason)
{
    QList<MapPos> areaPositions = TileLockUtils::getTilesInArea(map_, topLeft, bottomRight);
    lockTiles(areaPositions, reason.isEmpty() ? tr("Lock Area") : reason);
}

void TileLockManager::unlockArea(const MapPos& topLeft, const MapPos& bottomRight, const QString& reason)
{
    QList<MapPos> areaPositions = TileLockUtils::getTilesInArea(map_, topLeft, bottomRight);
    unlockTiles(areaPositions, reason.isEmpty() ? tr("Unlock Area") : reason);
}

// Filter operations
void TileLockManager::lockByFilter(LockTilesByFilterCommand::FilterType filterType, const QVariant& filterValue, const QString& reason)
{
    QList<MapPos> matchingPositions = TileLockUtils::findTilesByFilter(map_, filterType, filterValue);
    lockTiles(matchingPositions, reason.isEmpty() ? tr("Lock by Filter") : reason);
}

void TileLockManager::unlockByFilter(LockTilesByFilterCommand::FilterType filterType, const QVariant& filterValue, const QString& reason)
{
    QList<MapPos> matchingPositions = TileLockUtils::findTilesByFilter(map_, filterType, filterValue);
    unlockTiles(matchingPositions, reason.isEmpty() ? tr("Unlock by Filter") : reason);
}

// History and tracking
QList<TileLockStateChangeEvent> TileLockManager::getLockHistory() const
{
    return lockHistory_;
}

void TileLockManager::clearLockHistory()
{
    lockHistory_.clear();
    qDebug() << "TileLockManager: Lock history cleared";
}

void TileLockManager::setHistoryEnabled(bool enabled)
{
    historyEnabled_ = enabled;
    qDebug() << "TileLockManager: History tracking" << (enabled ? "enabled" : "disabled");
}

bool TileLockManager::isHistoryEnabled() const
{
    return historyEnabled_;
}

// Statistics
QString TileLockManager::getStatistics() const
{
    QVariantMap stats = getStatisticsMap();
    
    return tr("Tile Lock Statistics:\n"
              "Total Tiles: %1\n"
              "Locked Tiles: %2\n"
              "Unlocked Tiles: %3\n"
              "Lock Percentage: %4%\n"
              "History Events: %5")
           .arg(stats["totalTiles"].toInt())
           .arg(stats["lockedTiles"].toInt())
           .arg(stats["unlockedTiles"].toInt())
           .arg(stats["lockPercentage"].toDouble(), 0, 'f', 1)
           .arg(stats["historyEvents"].toInt());
}

QVariantMap TileLockManager::getStatisticsMap() const
{
    QVariantMap stats;
    
    int totalTiles = 0;
    int lockedTiles = 0;
    
    if (map_) {
        for (int z = 0; z < map_->getDepth(); ++z) {
            for (int y = 0; y < map_->getHeight(); ++y) {
                for (int x = 0; x < map_->getWidth(); ++x) {
                    if (Tile* tile = map_->getTile(x, y, z)) {
                        totalTiles++;
                        if (tile->isLocked()) {
                            lockedTiles++;
                        }
                    }
                }
            }
        }
    }
    
    int unlockedTiles = totalTiles - lockedTiles;
    double lockPercentage = totalTiles > 0 ? (double(lockedTiles) / totalTiles * 100.0) : 0.0;
    
    stats["totalTiles"] = totalTiles;
    stats["lockedTiles"] = lockedTiles;
    stats["unlockedTiles"] = unlockedTiles;
    stats["lockPercentage"] = lockPercentage;
    stats["historyEvents"] = lockHistory_.size();
    
    return stats;
}

// Public slots
void TileLockManager::onTileChanged(int x, int y, int z)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(z);
    // Handle tile changes if needed
    emit statisticsChanged();
}

void TileLockManager::onMapCleared()
{
    clearLockHistory();
    emit statisticsChanged();
    qDebug() << "TileLockManager: Map cleared, history reset";
}

// Private helper methods
void TileLockManager::recordLockStateChange(const MapPos& position, bool wasLocked, bool isLocked, const QString& reason)
{
    if (!historyEnabled_) return;
    
    TileLockStateChangeEvent event(position, wasLocked, isLocked, reason);
    lockHistory_.append(event);
    
    // Limit history size to prevent memory issues
    const int maxHistorySize = 10000;
    if (lockHistory_.size() > maxHistorySize) {
        lockHistory_.removeFirst();
    }
}

void TileLockManager::emitLockStateChanged(const MapPos& position, bool isLocked, const QString& reason)
{
    emit lockStateChanged(position, isLocked, reason);
    emit statisticsChanged();
}


