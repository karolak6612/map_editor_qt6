#ifndef HOUSETOWNINTEGRATIONMANAGER_H
#define HOUSETOWNINTEGRATIONMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QVariantMap>
#include <QStringList>
#include <QElapsedTimer>
#include <QQueue>
#include <QUndoStack>
#include <QUndoCommand>

#include "MapPos.h"
#include "House.h"
#include "Town.h"

// Forward declarations
class Map;
class MapView;
class MapScene;
class Minimap;
class Tile;
class HouseEditorDialog;
class TownEditorDialog;
class MainWindow;

/**
 * @brief Task 94: Houses and Towns Integration Manager for complete system integration
 * 
 * Complete integration system for Houses and Towns with full life-cycle management:
 * - Map-level data management with Houses and Towns ownership
 * - Complete UI interaction with dialogs and panels for all editing operations
 * - Robust synchronization with Map data model and MapView/Minimap visuals
 * - Correct persistence through OTBM attributes and dedicated XML files
 * - Tile property interaction with house exits and stairs
 * - Undo/redo support for all house/town operations
 * - Minimap layers integration for house/town visualization
 * - 1:1 compatibility with wxwidgets house/town system
 */

/**
 * @brief House/Town Integration Event for batched updates
 */
struct HouseTownIntegrationEvent {
    enum Type {
        HOUSE_ADDED,
        HOUSE_REMOVED,
        HOUSE_MODIFIED,
        HOUSE_TILE_ADDED,
        HOUSE_TILE_REMOVED,
        HOUSE_DOOR_CHANGED,
        TOWN_ADDED,
        TOWN_REMOVED,
        TOWN_MODIFIED,
        TOWN_HOUSE_LINKED,
        TOWN_HOUSE_UNLINKED,
        TILE_HOUSE_FLAG_CHANGED,
        TILE_TOWN_FLAG_CHANGED,
        MAP_VISUAL_UPDATE_REQUIRED,
        MINIMAP_UPDATE_REQUIRED,
        PERSISTENCE_UPDATE_REQUIRED
    };
    
    Type type;
    House* house;
    Town* town;
    Tile* tile;
    MapPos position;
    QVariantMap data;
    qint64 timestamp;
    int priority;
    
    HouseTownIntegrationEvent(Type t = HOUSE_MODIFIED, House* h = nullptr, 
                             Town* tn = nullptr, Tile* tl = nullptr, int p = 0)
        : type(t), house(h), town(tn), tile(tl), priority(p) {
        timestamp = QDateTime::currentMSecsSinceEpoch();
    }
    
    bool operator<(const HouseTownIntegrationEvent& other) const {
        if (priority != other.priority) return priority < other.priority;
        return timestamp < other.timestamp;
    }
};

/**
 * @brief Main Houses and Towns Integration Manager
 */
class HouseTownIntegrationManager : public QObject
{
    Q_OBJECT

public:
    explicit HouseTownIntegrationManager(QObject* parent = nullptr);
    ~HouseTownIntegrationManager() override;

    // Component registration
    void setMap(Map* map);
    void setMapView(MapView* mapView);
    void setMapScene(MapScene* scene);
    void setMinimap(Minimap* minimap);
    void setMainWindow(MainWindow* mainWindow);
    void setUndoStack(QUndoStack* undoStack);

    // Integration control
    void enableIntegration(bool enabled);
    bool isIntegrationEnabled() const { return integrationEnabled_; }
    
    void enableMapSync(bool enabled);
    bool isMapSyncEnabled() const { return mapSyncEnabled_; }
    
    void enableVisualSync(bool enabled);
    bool isVisualSyncEnabled() const { return visualSyncEnabled_; }
    
    void enablePersistenceSync(bool enabled);
    bool isPersistenceSyncEnabled() const { return persistenceSyncEnabled_; }

    // Update control
    void setUpdateInterval(int milliseconds);
    int getUpdateInterval() const { return updateInterval_; }
    
    void setBatchSize(int maxEvents);
    int getBatchSize() const { return batchSize_; }
    
    void setThrottleDelay(int milliseconds);
    int getThrottleDelay() const { return throttleDelay_; }

    // Manual synchronization
    void syncMapData();
    void syncVisualDisplay();
    void syncMinimap();
    void syncPersistence();
    void syncAll();
    void forceUpdate();

    // House operations
    void handleHouseCreation(House* house);
    void handleHouseModification(House* house);
    void handleHouseDeletion(House* house);
    void handleHouseTileChange(House* house, const MapPos& position, bool added);
    void handleHouseDoorChange(House* house, const MapPos& position);

    // Town operations
    void handleTownCreation(Town* town);
    void handleTownModification(Town* town);
    void handleTownDeletion(Town* town);
    void handleTownHouseLink(Town* town, House* house);
    void handleTownHouseUnlink(Town* town, House* house);

    // Tile integration
    void updateTileForHouse(Tile* tile, House* house, bool hasHouse);
    void updateTileForTown(Tile* tile, Town* town, bool hasTown);
    void updateTileHouseFlags(Tile* tile, bool hasHouse);
    void updateTileTownFlags(Tile* tile, bool hasTown);
    void notifyTileChanged(const MapPos& position);

    // UI integration
    void openHouseEditor(House* house = nullptr);
    void openTownEditor(Town* town = nullptr);
    void showHouseOnMap(House* house);
    void showTownOnMap(Town* town);
    void highlightHouseArea(House* house, bool highlight = true);
    void highlightTownArea(Town* town, bool highlight = true);

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
    
    // House event handlers
    void onHouseAdded(House* house);
    void onHouseRemoved(quint32 houseId);
    void onHouseModified(House* house);
    void onHouseTileAdded(const MapPos& position);
    void onHouseTileRemoved(const MapPos& position);
    void onHouseDoorChanged(House* house, const MapPos& position);
    
    // Town event handlers
    void onTownAdded(Town* town);
    void onTownRemoved(quint32 townId);
    void onTownModified(Town* town);
    void onTownHouseLinked(Town* town, House* house);
    void onTownHouseUnlinked(Town* town, House* house);
    
    // UI event handlers
    void onHouseEditorRequested(House* house);
    void onTownEditorRequested(Town* town);
    void onHouseVisualizationRequested(House* house);
    void onTownVisualizationRequested(Town* town);
    
    // Update processing
    void processUpdates();
    void processBatch();

signals:
    void syncStarted();
    void syncCompleted();
    void syncError(const QString& error);
    void houseIntegrationUpdated(House* house);
    void townIntegrationUpdated(Town* town);
    void tileFlagsUpdated(Tile* tile);
    void visualDisplayUpdated();
    void minimapUpdated();
    void persistenceUpdated();
    void statisticsUpdated(const QVariantMap& stats);

private slots:
    void onUpdateTimer();
    void onThrottleTimer();

private:
    // Event processing
    void queueEvent(const HouseTownIntegrationEvent& event);
    void processEvent(const HouseTownIntegrationEvent& event);
    void processBatchedEvents(const QList<HouseTownIntegrationEvent>& events);
    
    // Specific sync operations
    void syncHouseAddition(House* house);
    void syncHouseRemoval(House* house);
    void syncHouseModification(House* house);
    void syncHouseTileChange(House* house, const MapPos& position, bool added);
    void syncHouseDoorChange(House* house, const MapPos& position);
    void syncTownAddition(Town* town);
    void syncTownRemoval(Town* town);
    void syncTownModification(Town* town);
    void syncTownHouseLink(Town* town, House* house);
    void syncTileFlagChange(Tile* tile, bool hasHouse, bool hasTown);
    void syncMapVisualUpdate();
    void syncMinimapUpdate();
    void syncPersistenceUpdate();
    
    // Map integration
    void updateMapForHouse(House* house, bool added);
    void updateMapForTown(Town* town, bool added);
    void updateMapTileFlags(const MapPos& position);
    void refreshMapArea(const QRect& area, int floor);
    
    // Visual integration
    void updateVisualForHouse(House* house, bool added);
    void updateVisualForTown(Town* town, bool added);
    void updateHouseBorders(House* house, bool visible);
    void updateTownMarkers(Town* town, bool visible);
    void refreshVisualArea(const QRect& area, int floor);
    
    // Minimap integration
    void updateMinimapForHouse(House* house, bool added);
    void updateMinimapForTown(Town* town, bool added);
    void updateMinimapLayers();
    void refreshMinimapArea(const QRect& area, int floor);
    
    // Persistence integration
    void updatePersistenceForHouse(House* house, bool added);
    void updatePersistenceForTown(Town* town, bool added);
    void markPersistenceDirty();
    void schedulePersistenceUpdate();
    
    // Tile flag management
    void setTileHouseFlag(Tile* tile, bool hasHouse);
    void setTileTownFlag(Tile* tile, bool hasTown);
    bool getTileHouseFlag(Tile* tile) const;
    bool getTileTownFlag(Tile* tile) const;
    void updateAllTileFlags();
    void clearAllTileFlags();
    
    // UI management
    void createHouseEditor();
    void createTownEditor();
    void updateUIForHouse(House* house);
    void updateUIForTown(Town* town);
    void connectUISignals();
    void disconnectUISignals();
    
    // Optimization
    void optimizeEventQueue();
    void mergeEvents(QList<HouseTownIntegrationEvent>& events);
    bool canMergeEvents(const HouseTownIntegrationEvent& a, const HouseTownIntegrationEvent& b) const;
    HouseTownIntegrationEvent mergeEvents(const HouseTownIntegrationEvent& a, const HouseTownIntegrationEvent& b) const;
    
    // Throttling and batching
    bool shouldThrottle(const HouseTownIntegrationEvent& event) const;
    void startThrottleTimer();
    void stopThrottleTimer();
    
    // Statistics tracking
    void updateStatistics();
    void trackEvent(const HouseTownIntegrationEvent& event);
    void trackSyncTime(qint64 duration);

private:
    // Core components
    Map* map_;
    MapView* mapView_;
    MapScene* scene_;
    Minimap* minimap_;
    MainWindow* mainWindow_;
    QUndoStack* undoStack_;
    
    // UI components
    HouseEditorDialog* houseEditor_;
    TownEditorDialog* townEditor_;
    
    // Integration settings
    bool integrationEnabled_;
    bool mapSyncEnabled_;
    bool visualSyncEnabled_;
    bool persistenceSyncEnabled_;
    
    // Update control
    int updateInterval_;
    int batchSize_;
    int throttleDelay_;
    
    // Event processing
    QMutex eventMutex_;
    QQueue<HouseTownIntegrationEvent> eventQueue_;
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
    QMap<HouseTownIntegrationEvent::Type, qint64> lastEventTime_;
    
    // Constants
    static const int DEFAULT_UPDATE_INTERVAL = 100; // milliseconds
    static const int DEFAULT_BATCH_SIZE = 10;
    static const int DEFAULT_THROTTLE_DELAY = 50; // milliseconds
    static const int MAX_QUEUE_SIZE = 1000;
};

#endif // HOUSETOWNINTEGRATIONMANAGER_H
