#ifndef MINIMAPINTEGRATION_H
#define MINIMAPINTEGRATION_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QVariantMap>
#include <QStringList>
#include <QElapsedTimer>

#include "MapPos.h"

// Forward declarations
class Map;
class MapView;
class MinimapWindow;
class MinimapRenderer;
class Tile;
class Selection;

/**
 * @brief Task 90: Minimap Integration Manager
 * 
 * Complete integration system for minimap synchronization:
 * - MapView viewport synchronization (panning, zooming, floor changes)
 * - Map content synchronization (tile changes, additions, deletions)
 * - Selection synchronization (show selection area on minimap)
 * - Navigation integration (click-to-center, waypoint navigation)
 * - Performance optimization (intelligent update batching)
 * - Event filtering and throttling for smooth operation
 * - 1:1 compatibility with wxwidgets minimap integration
 */

/**
 * @brief Minimap Sync Event for batched updates
 */
struct MinimapSyncEvent {
    enum Type {
        VIEWPORT_CHANGED,
        FLOOR_CHANGED,
        TILE_CHANGED,
        AREA_CHANGED,
        SELECTION_CHANGED,
        MAP_LOADED,
        MAP_CLOSED
    };
    
    Type type;
    QVariantMap data;
    qint64 timestamp;
    int priority;
    
    MinimapSyncEvent(Type t = VIEWPORT_CHANGED, const QVariantMap& d = QVariantMap(), int p = 0)
        : type(t), data(d), priority(p) {
        timestamp = QDateTime::currentMSecsSinceEpoch();
    }
    
    bool operator<(const MinimapSyncEvent& other) const {
        if (priority != other.priority) return priority < other.priority;
        return timestamp < other.timestamp;
    }
};

/**
 * @brief Main Minimap Integration Manager
 */
class MinimapIntegrationManager : public QObject
{
    Q_OBJECT

public:
    explicit MinimapIntegrationManager(QObject* parent = nullptr);
    ~MinimapIntegrationManager() override;

    // Component registration
    void setMap(Map* map);
    void setMapView(MapView* mapView);
    void setMinimapWindow(MinimapWindow* minimapWindow);
    void setMinimapRenderer(MinimapRenderer* renderer);

    // Integration control
    void enableIntegration(bool enabled);
    bool isIntegrationEnabled() const { return integrationEnabled_; }
    
    void enableViewportSync(bool enabled);
    bool isViewportSyncEnabled() const { return viewportSyncEnabled_; }
    
    void enableContentSync(bool enabled);
    bool isContentSyncEnabled() const { return contentSyncEnabled_; }
    
    void enableSelectionSync(bool enabled);
    bool isSelectionSyncEnabled() const { return selectionSyncEnabled_; }

    // Update control
    void setUpdateInterval(int milliseconds);
    int getUpdateInterval() const { return updateInterval_; }
    
    void setBatchSize(int maxEvents);
    int getBatchSize() const { return batchSize_; }
    
    void setThrottleDelay(int milliseconds);
    int getThrottleDelay() const { return throttleDelay_; }

    // Manual synchronization
    void syncViewport();
    void syncFloor();
    void syncSelection();
    void syncAll();
    void forceUpdate();

    // Statistics and monitoring
    QVariantMap getStatistics() const;
    void resetStatistics();
    bool isSyncInProgress() const;
    int getPendingEventCount() const;

public slots:
    // MapView event handlers
    void onMapViewViewportChanged(const QRect& viewport);
    void onMapViewFloorChanged(int floor);
    void onMapViewZoomChanged(double zoom);
    void onMapViewCenterChanged(const QPoint& center);
    void onMapViewSelectionChanged();
    
    // Map event handlers
    void onMapLoaded();
    void onMapClosed();
    void onMapTileChanged(int x, int y, int z);
    void onMapAreaChanged(const QRect& area, int floor);
    void onMapSizeChanged(int width, int height);
    
    // Minimap event handlers
    void onMinimapPositionClicked(const MapPos& position);
    void onMinimapViewportChanged(const QRect& viewport);
    void onMinimapZoomChanged(double zoom);
    
    // Update processing
    void processUpdates();
    void processBatch();

signals:
    void syncStarted();
    void syncCompleted();
    void syncError(const QString& error);
    void statisticsUpdated(const QVariantMap& stats);

private slots:
    void onUpdateTimer();
    void onThrottleTimer();

private:
    // Event processing
    void queueEvent(const MinimapSyncEvent& event);
    void processEvent(const MinimapSyncEvent& event);
    void processBatchedEvents(const QList<MinimapSyncEvent>& events);
    
    // Specific sync operations
    void syncViewportInternal();
    void syncFloorInternal();
    void syncTileChange(int x, int y, int z);
    void syncAreaChange(const QRect& area, int floor);
    void syncSelectionInternal();
    void syncMapLoad();
    void syncMapClose();
    
    // Optimization
    void optimizeEventQueue();
    void mergeEvents(QList<MinimapSyncEvent>& events);
    bool canMergeEvents(const MinimapSyncEvent& a, const MinimapSyncEvent& b) const;
    MinimapSyncEvent mergeEvents(const MinimapSyncEvent& a, const MinimapSyncEvent& b) const;
    
    // Throttling and batching
    bool shouldThrottle(const MinimapSyncEvent& event) const;
    void startThrottleTimer();
    void stopThrottleTimer();
    
    // Statistics tracking
    void updateStatistics();
    void trackEvent(const MinimapSyncEvent& event);
    void trackSyncTime(qint64 duration);

private:
    // Core components
    Map* map_;
    MapView* mapView_;
    MinimapWindow* minimapWindow_;
    MinimapRenderer* renderer_;
    
    // Integration settings
    bool integrationEnabled_;
    bool viewportSyncEnabled_;
    bool contentSyncEnabled_;
    bool selectionSyncEnabled_;
    
    // Update control
    int updateInterval_;
    int batchSize_;
    int throttleDelay_;
    
    // Event processing
    QMutex eventMutex_;
    QList<MinimapSyncEvent> eventQueue_;
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
    QMap<MinimapSyncEvent::Type, qint64> lastEventTime_;
    
    // Constants
    static const int DEFAULT_UPDATE_INTERVAL = 100; // milliseconds
    static const int DEFAULT_BATCH_SIZE = 10;
    static const int DEFAULT_THROTTLE_DELAY = 50; // milliseconds
    static const int MAX_QUEUE_SIZE = 1000;
};

/**
 * @brief Minimap Navigation Manager for click navigation and waypoints
 */
class MinimapNavigationManager : public QObject
{
    Q_OBJECT

public:
    explicit MinimapNavigationManager(QObject* parent = nullptr);
    ~MinimapNavigationManager() override = default;

    // Component setup
    void setMapView(MapView* mapView);
    void setMinimapWindow(MinimapWindow* minimapWindow);

    // Navigation settings
    void setClickToCenter(bool enabled);
    bool isClickToCenter() const { return clickToCenter_; }
    
    void setSmoothNavigation(bool enabled);
    bool isSmoothNavigation() const { return smoothNavigation_; }
    
    void setNavigationSpeed(double speed);
    double getNavigationSpeed() const { return navigationSpeed_; }

    // Navigation methods
    void navigateToPosition(const MapPos& position);
    void navigateToTile(int x, int y, int z);
    void navigateToArea(const QRect& area, int floor);
    void centerOnSelection();

public slots:
    void onMinimapClicked(const MapPos& position);
    void onWaypointSelected(int waypointIndex);
    void onNavigationRequested(const MapPos& target);

signals:
    void navigationStarted(const MapPos& target);
    void navigationCompleted(const MapPos& position);
    void navigationCancelled();

private slots:
    void onNavigationTimer();

private:
    void startSmoothNavigation(const MapPos& target);
    void stopNavigation();
    MapPos interpolatePosition(const MapPos& start, const MapPos& end, double progress) const;

private:
    MapView* mapView_;
    MinimapWindow* minimapWindow_;
    
    // Navigation settings
    bool clickToCenter_;
    bool smoothNavigation_;
    double navigationSpeed_;
    
    // Smooth navigation state
    QTimer* navigationTimer_;
    MapPos navigationStart_;
    MapPos navigationTarget_;
    QElapsedTimer navigationTime_;
    int navigationDuration_;
    bool navigationInProgress_;
    
    static const int NAVIGATION_INTERVAL = 16; // ~60 FPS
    static const int DEFAULT_NAVIGATION_DURATION = 500; // milliseconds
};

/**
 * @brief Minimap Selection Synchronizer for selection visualization
 */
class MinimapSelectionSynchronizer : public QObject
{
    Q_OBJECT

public:
    explicit MinimapSelectionSynchronizer(QObject* parent = nullptr);
    ~MinimapSelectionSynchronizer() override = default;

    // Component setup
    void setMapView(MapView* mapView);
    void setMinimapWindow(MinimapWindow* minimapWindow);

    // Selection visualization
    void setShowSelection(bool show);
    bool isShowSelection() const { return showSelection_; }
    
    void setSelectionColor(const QColor& color);
    QColor getSelectionColor() const { return selectionColor_; }
    
    void setSelectionOpacity(double opacity);
    double getSelectionOpacity() const { return selectionOpacity_; }

public slots:
    void onSelectionChanged();
    void onSelectionCleared();
    void updateSelectionVisualization();

signals:
    void selectionVisualizationUpdated();

private:
    void syncSelectionToMinimap();
    QRect getSelectionBounds() const;

private:
    MapView* mapView_;
    MinimapWindow* minimapWindow_;
    
    bool showSelection_;
    QColor selectionColor_;
    double selectionOpacity_;
    
    QRect lastSelectionBounds_;
    int lastSelectionFloor_;
};

/**
 * @brief Minimap Performance Monitor for optimization
 */
class MinimapPerformanceMonitor : public QObject
{
    Q_OBJECT

public:
    explicit MinimapPerformanceMonitor(QObject* parent = nullptr);
    ~MinimapPerformanceMonitor() override = default;

    // Monitoring control
    void startMonitoring();
    void stopMonitoring();
    void resetStatistics();

    // Performance metrics
    QVariantMap getPerformanceMetrics() const;
    double getAverageFrameTime() const;
    double getFrameRate() const;
    int getMemoryUsage() const;
    int getCacheHitRatio() const;

    // Optimization recommendations
    QStringList getOptimizationRecommendations() const;
    void applyOptimizations();

signals:
    void performanceUpdated(const QVariantMap& metrics);
    void optimizationRecommended(const QStringList& recommendations);

private slots:
    void updateMetrics();

private:
    void collectMetrics();
    void analyzePerformance();

private:
    QTimer* monitorTimer_;
    bool monitoring_;
    
    // Performance data
    QElapsedTimer frameTimer_;
    QList<qint64> frameTimes_;
    int frameCount_;
    qint64 totalFrameTime_;
    
    // Memory tracking
    int peakMemoryUsage_;
    int currentMemoryUsage_;
    
    // Cache statistics
    int cacheHits_;
    int cacheMisses_;
    
    static const int MONITOR_INTERVAL = 1000; // milliseconds
    static const int MAX_FRAME_SAMPLES = 100;
};

#endif // MINIMAPINTEGRATION_H
