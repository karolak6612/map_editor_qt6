#ifndef SPAWNINTEGRATIONMANAGER_H
#define SPAWNINTEGRATIONMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QVariantMap>
#include <QStringList>
#include <QElapsedTimer>
#include <QQueue>

#include "MapPos.h"
#include "rendering/SpawnVisualizationManager.h"

// Forward declarations
class Map;
class MapView;
class MapScene;
class Spawn;
class Tile;
class SpawnBrush;
class SpawnItem;
class TileItem;

/**
 * @brief Task 92: Spawn Integration Manager for complete map display integration
 * 
 * Complete integration system for spawn visualization with map display:
 * - MapView and MapScene integration for spawn rendering
 * - SpawnBrush integration for spawn placement and removal
 * - Tile flag management and borderization logic
 * - Real-time spawn updates and attribute reflection
 * - Performance optimization for large spawn counts
 * - Event batching and throttling for smooth operation
 * - 1:1 compatibility with wxwidgets spawn display integration
 */

/**
 * @brief Spawn Integration Event for batched updates
 */
struct SpawnIntegrationEvent {
    enum Type {
        SPAWN_ADDED,
        SPAWN_REMOVED,
        SPAWN_MODIFIED,
        SPAWN_ATTRIBUTE_CHANGED,
        SPAWN_SELECTED,
        SPAWN_DESELECTED,
        TILE_FLAG_CHANGED,
        BORDER_UPDATE_REQUIRED,
        VISUALIZATION_REFRESH_REQUESTED
    };
    
    Type type;
    Spawn* spawn;
    Tile* tile;
    QString attribute;
    QVariantMap data;
    qint64 timestamp;
    int priority;
    
    SpawnIntegrationEvent(Type t = SPAWN_MODIFIED, Spawn* s = nullptr, 
                         Tile* tl = nullptr, int p = 0)
        : type(t), spawn(s), tile(tl), priority(p) {
        timestamp = QDateTime::currentMSecsSinceEpoch();
    }
    
    bool operator<(const SpawnIntegrationEvent& other) const {
        if (priority != other.priority) return priority < other.priority;
        return timestamp < other.timestamp;
    }
};

/**
 * @brief Main Spawn Integration Manager
 */
class SpawnIntegrationManager : public QObject
{
    Q_OBJECT

public:
    explicit SpawnIntegrationManager(QObject* parent = nullptr);
    ~SpawnIntegrationManager() override;

    // Component registration
    void setMap(Map* map);
    void setMapView(MapView* mapView);
    void setMapScene(MapScene* scene);
    void setSpawnVisualizationManager(SpawnVisualizationManager* manager);
    void setSpawnBrush(SpawnBrush* brush);

    // Integration control
    void enableIntegration(bool enabled);
    bool isIntegrationEnabled() const { return integrationEnabled_; }
    
    void enableVisualizationSync(bool enabled);
    bool isVisualizationSyncEnabled() const { return visualizationSyncEnabled_; }
    
    void enableTileFlagSync(bool enabled);
    bool isTileFlagSyncEnabled() const { return tileFlagSyncEnabled_; }
    
    void enableBorderSync(bool enabled);
    bool isBorderSyncEnabled() const { return borderSyncEnabled_; }

    // Update control
    void setUpdateInterval(int milliseconds);
    int getUpdateInterval() const { return updateInterval_; }
    
    void setBatchSize(int maxEvents);
    int getBatchSize() const { return batchSize_; }
    
    void setThrottleDelay(int milliseconds);
    int getThrottleDelay() const { return throttleDelay_; }

    // Manual synchronization
    void syncSpawnVisualization();
    void syncTileFlags();
    void syncBorderization();
    void syncAll();
    void forceUpdate();

    // Spawn operations
    void handleSpawnPlacement(Spawn* spawn, Tile* tile);
    void handleSpawnRemoval(Spawn* spawn, Tile* tile);
    void handleSpawnModification(Spawn* spawn);
    void handleSpawnAttributeChange(Spawn* spawn, const QString& attribute);
    void handleSpawnSelection(Spawn* spawn, bool selected);

    // Tile integration
    void updateTileForSpawn(Tile* tile, Spawn* spawn, bool hasSpawn);
    void updateTileFlags(Tile* tile, bool hasSpawn);
    void updateTileBorderization(Tile* tile);
    void notifyAdjacentTiles(Tile* tile);

    // Statistics and monitoring
    QVariantMap getStatistics() const;
    void resetStatistics();
    bool isSyncInProgress() const;
    int getPendingEventCount() const;

public slots:
    // Map event handlers
    void onMapLoaded();
    void onMapClosed();
    void onMapModified();
    void onTileChanged(int x, int y, int z);
    void onAreaChanged(const QRect& area, int floor);
    
    // Spawn event handlers
    void onSpawnAdded(Spawn* spawn);
    void onSpawnRemoved(Spawn* spawn);
    void onSpawnModified(Spawn* spawn);
    void onSpawnAttributeChanged(Spawn* spawn, const QString& attribute);
    void onSpawnSelected(Spawn* spawn);
    void onSpawnDeselected(Spawn* spawn);
    
    // SpawnBrush event handlers
    void onSpawnBrushApplied(const MapPos& position, Spawn* spawn);
    void onSpawnBrushRemoved(const MapPos& position);
    void onSpawnBrushModified(const MapPos& position, Spawn* spawn);
    
    // Visualization event handlers
    void onSpawnItemCreated(SpawnItem* item);
    void onSpawnItemRemoved(SpawnItem* item);
    void onSpawnItemUpdated(SpawnItem* item);
    void onVisualizationRefreshed();
    
    // Update processing
    void processUpdates();
    void processBatch();

signals:
    void syncStarted();
    void syncCompleted();
    void syncError(const QString& error);
    void spawnIntegrationUpdated(Spawn* spawn);
    void tileFlagsUpdated(Tile* tile);
    void borderizationUpdated(Tile* tile);
    void statisticsUpdated(const QVariantMap& stats);

private slots:
    void onUpdateTimer();
    void onThrottleTimer();

private:
    // Event processing
    void queueEvent(const SpawnIntegrationEvent& event);
    void processEvent(const SpawnIntegrationEvent& event);
    void processBatchedEvents(const QList<SpawnIntegrationEvent>& events);
    
    // Specific sync operations
    void syncSpawnAddition(Spawn* spawn);
    void syncSpawnRemoval(Spawn* spawn);
    void syncSpawnModification(Spawn* spawn);
    void syncSpawnAttributeChange(Spawn* spawn, const QString& attribute);
    void syncSpawnSelection(Spawn* spawn, bool selected);
    void syncTileFlagChange(Tile* tile, bool hasSpawn);
    void syncBorderUpdate(Tile* tile);
    void syncVisualizationRefresh();
    
    // Map display integration
    void updateMapDisplayForSpawn(Spawn* spawn, bool added);
    void updateMapDisplayForTile(Tile* tile);
    void refreshMapDisplayArea(const QRect& area, int floor);
    
    // SpawnBrush integration
    void integrateWithSpawnBrush(SpawnBrush* brush);
    void handleBrushSpawnPlacement(const MapPos& position, Spawn* spawn);
    void handleBrushSpawnRemoval(const MapPos& position);
    
    // Tile flag management
    void setTileSpawnFlag(Tile* tile, bool hasSpawn);
    bool getTileSpawnFlag(Tile* tile) const;
    void updateTileSpawnFlags(Map* map);
    void clearTileSpawnFlags(Map* map);
    
    // Borderization integration
    void triggerBorderizationUpdate(Tile* tile);
    void updateBorderizationForSpawnChange(Tile* tile, bool hasSpawn);
    void notifyBorderSystemSpawnChange(Tile* tile);
    
    // Optimization
    void optimizeEventQueue();
    void mergeEvents(QList<SpawnIntegrationEvent>& events);
    bool canMergeEvents(const SpawnIntegrationEvent& a, const SpawnIntegrationEvent& b) const;
    SpawnIntegrationEvent mergeEvents(const SpawnIntegrationEvent& a, const SpawnIntegrationEvent& b) const;
    
    // Throttling and batching
    bool shouldThrottle(const SpawnIntegrationEvent& event) const;
    void startThrottleTimer();
    void stopThrottleTimer();
    
    // Statistics tracking
    void updateStatistics();
    void trackEvent(const SpawnIntegrationEvent& event);
    void trackSyncTime(qint64 duration);

private:
    // Core components
    Map* map_;
    MapView* mapView_;
    MapScene* scene_;
    SpawnVisualizationManager* visualizationManager_;
    SpawnBrush* spawnBrush_;
    
    // Integration settings
    bool integrationEnabled_;
    bool visualizationSyncEnabled_;
    bool tileFlagSyncEnabled_;
    bool borderSyncEnabled_;
    
    // Update control
    int updateInterval_;
    int batchSize_;
    int throttleDelay_;
    
    // Event processing
    QMutex eventMutex_;
    QQueue<SpawnIntegrationEvent> eventQueue_;
    QTimer* updateTimer_;
    QTimer* throttleTimer_;
    bool syncInProgress_;
    
    // Performance tracking
    mutable QMutex statsMutex_;
    QVariantMap statistics_;
    QElapsedTimer syncTimer_;
    int eventsProcessed_;
    int batchesProcessed_;
    qint64 totalSyncTime_;
    
    // Throttling state
    QElapsedTimer lastSyncTime_;
    QMap<SpawnIntegrationEvent::Type, qint64> lastEventTime_;
    
    // Constants
    static const int DEFAULT_UPDATE_INTERVAL = 100; // milliseconds
    static const int DEFAULT_BATCH_SIZE = 10;
    static const int DEFAULT_THROTTLE_DELAY = 50; // milliseconds
    static const int MAX_QUEUE_SIZE = 1000;
};

/**
 * @brief Spawn Tile Flag Synchronizer for tile state management
 */
class SpawnTileFlagSynchronizer : public QObject
{
    Q_OBJECT

public:
    explicit SpawnTileFlagSynchronizer(QObject* parent = nullptr);
    ~SpawnTileFlagSynchronizer() override = default;

    // Component setup
    void setMap(Map* map);
    void setSpawnIntegrationManager(SpawnIntegrationManager* manager);

    // Flag synchronization
    void syncTileFlags(Tile* tile, bool hasSpawn);
    void syncAllTileFlags();
    void clearAllTileFlags();

public slots:
    void onSpawnAdded(Spawn* spawn);
    void onSpawnRemoved(Spawn* spawn);
    void onTileChanged(Tile* tile);

signals:
    void tileFlagsSynced(Tile* tile);
    void allTileFlagsSynced();

private:
    void updateTileFlagsInternal(Tile* tile, bool hasSpawn);

private:
    Map* map_;
    SpawnIntegrationManager* integrationManager_;
};

/**
 * @brief Spawn Border Synchronizer for borderization logic
 */
class SpawnBorderSynchronizer : public QObject
{
    Q_OBJECT

public:
    explicit SpawnBorderSynchronizer(QObject* parent = nullptr);
    ~SpawnBorderSynchronizer() override = default;

    // Component setup
    void setMap(Map* map);
    void setSpawnIntegrationManager(SpawnIntegrationManager* manager);

    // Border synchronization
    void syncBorderization(Tile* tile);
    void syncAreaBorderization(const QRect& area, int floor);
    void notifyBorderSystem(Tile* tile, bool hasSpawn);

public slots:
    void onSpawnChanged(Spawn* spawn);
    void onTileSpawnFlagChanged(Tile* tile, bool hasSpawn);

signals:
    void borderizationSynced(Tile* tile);
    void borderSystemNotified(Tile* tile);

private:
    void updateBorderizationInternal(Tile* tile);
    void notifyAdjacentTilesInternal(Tile* tile);

private:
    Map* map_;
    SpawnIntegrationManager* integrationManager_;
};

#endif // SPAWNINTEGRATIONMANAGER_H
