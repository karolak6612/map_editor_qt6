#ifndef TILELOCKCOMMANDS_H
#define TILELOCKCOMMANDS_H

#include <QUndoCommand>
#include <QList>
#include <QSet>
#include <QString>
#include "MapPos.h"

// Forward declarations
class Map;
class Tile;
class Selection;

/**
 * @brief Tile Locking Commands for Task 85
 * 
 * Complete implementation of undoable tile locking commands:
 * - Single tile lock/unlock operations
 * - Batch tile lock/unlock operations
 * - Selection-based lock/unlock operations
 * - Area-based lock/unlock operations
 * - Full undo/redo support with proper state tracking
 */

/**
 * @brief Base class for tile locking commands
 */
class TileLockCommandBase : public QUndoCommand
{
public:
    explicit TileLockCommandBase(Map* map, QUndoCommand* parent = nullptr);
    virtual ~TileLockCommandBase() = default;

protected:
    Map* map_;
    
    // Helper methods
    Tile* getTile(const MapPos& pos) const;
    bool isValidPosition(const MapPos& pos) const;
    void updateTileVisuals(const MapPos& pos);
    void updateTileVisuals(const QList<MapPos>& positions);
};

/**
 * @brief Command for locking/unlocking a single tile
 */
class LockTileCommand : public TileLockCommandBase
{
public:
    explicit LockTileCommand(Map* map, const MapPos& position, bool lock, QUndoCommand* parent = nullptr);
    ~LockTileCommand() override = default;

    void undo() override;
    void redo() override;
    int id() const override { return 1001; } // Unique command ID
    bool mergeWith(const QUndoCommand* other) override;

private:
    MapPos position_;
    bool lockState_;
    bool previousState_;
    bool firstExecution_;
};

/**
 * @brief Command for locking/unlocking multiple tiles
 */
class LockTilesCommand : public TileLockCommandBase
{
public:
    explicit LockTilesCommand(Map* map, const QList<MapPos>& positions, bool lock, QUndoCommand* parent = nullptr);
    ~LockTilesCommand() override = default;

    void undo() override;
    void redo() override;
    int id() const override { return 1002; } // Unique command ID
    bool mergeWith(const QUndoCommand* other) override;

private:
    QList<MapPos> positions_;
    bool lockState_;
    QList<bool> previousStates_;
    bool firstExecution_;
    
    void storePreviousStates();
    void restorePreviousStates();
    void applyLockState();
};

/**
 * @brief Command for locking/unlocking tiles in a selection
 */
class LockSelectionCommand : public TileLockCommandBase
{
public:
    explicit LockSelectionCommand(Map* map, Selection* selection, bool lock, QUndoCommand* parent = nullptr);
    ~LockSelectionCommand() override = default;

    void undo() override;
    void redo() override;
    int id() const override { return 1003; } // Unique command ID

private:
    QList<MapPos> selectedPositions_;
    bool lockState_;
    QList<bool> previousStates_;
    bool firstExecution_;
    
    void extractSelectionPositions(Selection* selection);
    void storePreviousStates();
    void restorePreviousStates();
    void applyLockState();
};

/**
 * @brief Command for locking/unlocking tiles in a rectangular area
 */
class LockAreaCommand : public TileLockCommandBase
{
public:
    explicit LockAreaCommand(Map* map, const MapPos& topLeft, const MapPos& bottomRight, bool lock, QUndoCommand* parent = nullptr);
    ~LockAreaCommand() override = default;

    void undo() override;
    void redo() override;
    int id() const override { return 1004; } // Unique command ID

private:
    MapPos topLeft_;
    MapPos bottomRight_;
    bool lockState_;
    QList<MapPos> affectedPositions_;
    QList<bool> previousStates_;
    bool firstExecution_;
    
    void calculateAffectedPositions();
    void storePreviousStates();
    void restorePreviousStates();
    void applyLockState();
};

/**
 * @brief Command for toggling lock state of tiles
 */
class ToggleTileLockCommand : public TileLockCommandBase
{
public:
    explicit ToggleTileLockCommand(Map* map, const QList<MapPos>& positions, QUndoCommand* parent = nullptr);
    ~ToggleTileLockCommand() override = default;

    void undo() override;
    void redo() override;
    int id() const override { return 1005; } // Unique command ID

private:
    QList<MapPos> positions_;
    QList<bool> originalStates_;
    QList<bool> newStates_;
    bool firstExecution_;
    
    void calculateNewStates();
    void storePreviousStates();
    void applyStates(const QList<bool>& states);
};

/**
 * @brief Command for locking/unlocking tiles by filter criteria
 */
class LockTilesByFilterCommand : public TileLockCommandBase
{
public:
    enum FilterType {
        ByItemId,           // Lock tiles containing specific item ID
        ByItemType,         // Lock tiles containing specific item type
        ByTileFlags,        // Lock tiles with specific flags
        ByHouseId,          // Lock tiles belonging to specific house
        ByZoneId,           // Lock tiles in specific zone
        ByCreatureType,     // Lock tiles with specific creature type
        BySpawnType,        // Lock tiles with specific spawn type
        IsEmpty,            // Lock empty tiles
        IsModified,         // Lock modified tiles
        IsBlocking,         // Lock blocking tiles
        HasGround,          // Lock tiles with ground
        HasItems,           // Lock tiles with items
        HasCreatures        // Lock tiles with creatures
    };

    explicit LockTilesByFilterCommand(Map* map, FilterType filterType, const QVariant& filterValue, bool lock, QUndoCommand* parent = nullptr);
    ~LockTilesByFilterCommand() override = default;

    void undo() override;
    void redo() override;
    int id() const override { return 1006; } // Unique command ID

private:
    FilterType filterType_;
    QVariant filterValue_;
    bool lockState_;
    QList<MapPos> affectedPositions_;
    QList<bool> previousStates_;
    bool firstExecution_;
    
    void findMatchingTiles();
    bool tileMatchesFilter(Tile* tile) const;
    void storePreviousStates();
    void restorePreviousStates();
    void applyLockState();
};

/**
 * @brief Utility class for tile locking operations
 */
class TileLockUtils
{
public:
    // Static utility methods
    static QList<MapPos> getLockedTiles(Map* map);
    static QList<MapPos> getUnlockedTiles(Map* map);
    static int getLockedTileCount(Map* map);
    static int getUnlockedTileCount(Map* map);
    
    // Area operations
    static QList<MapPos> getTilesInArea(Map* map, const MapPos& topLeft, const MapPos& bottomRight);
    static QList<MapPos> getLockedTilesInArea(Map* map, const MapPos& topLeft, const MapPos& bottomRight);
    static QList<MapPos> getUnlockedTilesInArea(Map* map, const MapPos& topLeft, const MapPos& bottomRight);
    
    // Selection operations
    static QList<MapPos> getTilesInSelection(Selection* selection);
    static QList<MapPos> getLockedTilesInSelection(Selection* selection);
    static QList<MapPos> getUnlockedTilesInSelection(Selection* selection);
    
    // Filter operations
    static QList<MapPos> findTilesByFilter(Map* map, LockTilesByFilterCommand::FilterType filterType, const QVariant& filterValue);
    
    // Validation
    static bool canLockTile(Tile* tile);
    static bool canUnlockTile(Tile* tile);
    static bool isValidLockOperation(Map* map, const QList<MapPos>& positions, bool lock);
    
    // Statistics
    static QString getLockStatistics(Map* map);
    static QVariantMap getLockStatisticsMap(Map* map);
    
private:
    TileLockUtils() = default; // Static class
};

/**
 * @brief Tile lock state change event
 */
struct TileLockStateChangeEvent
{
    MapPos position;
    bool wasLocked;
    bool isLocked;
    qint64 timestamp;
    QString reason;
    
    TileLockStateChangeEvent() = default;
    TileLockStateChangeEvent(const MapPos& pos, bool was, bool is, const QString& r = QString())
        : position(pos), wasLocked(was), isLocked(is), timestamp(QDateTime::currentMSecsSinceEpoch()), reason(r) {}
};

/**
 * @brief Tile lock manager for tracking and managing lock states
 */
class TileLockManager : public QObject
{
    Q_OBJECT

public:
    explicit TileLockManager(Map* map, QObject* parent = nullptr);
    ~TileLockManager() override = default;

    // Lock operations
    void lockTile(const MapPos& position, const QString& reason = QString());
    void unlockTile(const MapPos& position, const QString& reason = QString());
    void lockTiles(const QList<MapPos>& positions, const QString& reason = QString());
    void unlockTiles(const QList<MapPos>& positions, const QString& reason = QString());
    void toggleTileLock(const MapPos& position, const QString& reason = QString());
    
    // Query operations
    bool isTileLocked(const MapPos& position) const;
    QList<MapPos> getLockedTiles() const;
    int getLockedTileCount() const;
    
    // Batch operations
    void lockAll(const QString& reason = QString());
    void unlockAll(const QString& reason = QString());
    void lockSelection(Selection* selection, const QString& reason = QString());
    void unlockSelection(Selection* selection, const QString& reason = QString());
    void lockArea(const MapPos& topLeft, const MapPos& bottomRight, const QString& reason = QString());
    void unlockArea(const MapPos& topLeft, const MapPos& bottomRight, const QString& reason = QString());
    
    // Filter operations
    void lockByFilter(LockTilesByFilterCommand::FilterType filterType, const QVariant& filterValue, const QString& reason = QString());
    void unlockByFilter(LockTilesByFilterCommand::FilterType filterType, const QVariant& filterValue, const QString& reason = QString());
    
    // History and tracking
    QList<TileLockStateChangeEvent> getLockHistory() const;
    void clearLockHistory();
    void setHistoryEnabled(bool enabled);
    bool isHistoryEnabled() const;
    
    // Statistics
    QString getStatistics() const;
    QVariantMap getStatisticsMap() const;

signals:
    void tileLocked(const MapPos& position, const QString& reason);
    void tileUnlocked(const MapPos& position, const QString& reason);
    void tilesLocked(const QList<MapPos>& positions, const QString& reason);
    void tilesUnlocked(const QList<MapPos>& positions, const QString& reason);
    void lockStateChanged(const MapPos& position, bool isLocked, const QString& reason);
    void statisticsChanged();

public slots:
    void onTileChanged(int x, int y, int z);
    void onMapCleared();

private:
    Map* map_;
    QList<TileLockStateChangeEvent> lockHistory_;
    bool historyEnabled_;
    
    void recordLockStateChange(const MapPos& position, bool wasLocked, bool isLocked, const QString& reason);
    void emitLockStateChanged(const MapPos& position, bool isLocked, const QString& reason);
};

#endif // TILELOCKCOMMANDS_H
