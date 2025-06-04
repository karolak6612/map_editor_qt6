#ifndef ENHANCEDMAPRENDERER_H
#define ENHANCEDMAPRENDERER_H

#include <QObject>
#include <QPainter>
#include <QRect>
#include <QPoint>
#include <QPixmap>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QElapsedTimer>

// Forward declarations
class Map;
class Tile;
class Item;
class MapView;
class GameSprite;
class LODManager;
class EnhancedLightingSystem;
class TransparencyManager;
class EnhancedDrawingOptions;

/**
 * @brief Enhanced Map Renderer for Task 83
 * 
 * Complete implementation of advanced map rendering for Qt6 map editor:
 * - Integration of LOD, lighting, and transparency systems
 * - High-performance rendering with optimization
 * - 1:1 compatibility with wxwidgets MapDrawer
 * - Advanced visual effects and composition
 * - Multi-threaded rendering support
 * - Comprehensive performance monitoring
 */

/**
 * @brief Rendering statistics
 */
struct RenderingStatistics {
    // Basic statistics
    int totalTilesRendered = 0;
    int totalItemsRendered = 0;
    int totalSpritesRendered = 0;
    
    // Performance statistics
    double totalRenderTime = 0.0;
    double averageRenderTime = 0.0;
    double lastFrameTime = 0.0;
    double currentFPS = 0.0;
    
    // LOD statistics
    int itemsSkippedByLOD = 0;
    int spritesSimplified = 0;
    double lodOptimizationTime = 0.0;
    
    // Lighting statistics
    int lightsProcessed = 0;
    int lightsRendered = 0;
    double lightingCalculationTime = 0.0;
    double lightingRenderTime = 0.0;
    
    // Transparency statistics
    int transparentItemsRendered = 0;
    int opaqueItemsRendered = 0;
    double transparencyCalculationTime = 0.0;
    double transparencyRenderTime = 0.0;
    
    // Memory statistics
    int cacheHits = 0;
    int cacheMisses = 0;
    double cacheHitRatio = 0.0;
    
    void reset() {
        totalTilesRendered = 0;
        totalItemsRendered = 0;
        totalSpritesRendered = 0;
        totalRenderTime = 0.0;
        averageRenderTime = 0.0;
        lastFrameTime = 0.0;
        currentFPS = 0.0;
        itemsSkippedByLOD = 0;
        spritesSimplified = 0;
        lodOptimizationTime = 0.0;
        lightsProcessed = 0;
        lightsRendered = 0;
        lightingCalculationTime = 0.0;
        lightingRenderTime = 0.0;
        transparentItemsRendered = 0;
        opaqueItemsRendered = 0;
        transparencyCalculationTime = 0.0;
        transparencyRenderTime = 0.0;
        cacheHits = 0;
        cacheMisses = 0;
        cacheHitRatio = 0.0;
    }
};

/**
 * @brief Rendering configuration
 */
struct RenderingConfig {
    // Performance settings
    bool enableMultithreading = false;
    bool enableAsyncRendering = false;
    int maxRenderThreads = 4;
    bool enableRenderCaching = true;
    bool enableBatchRendering = true;
    
    // Quality settings
    bool enableAntialiasing = true;
    bool enableSmoothing = true;
    bool enableHighQualityRendering = false;
    QPainter::RenderHints renderHints = QPainter::Antialiasing | QPainter::SmoothPixmapTransform;
    
    // Optimization settings
    bool enableFrustumCulling = true;
    bool enableOcclusionCulling = false;
    bool enableDirtyRectOptimization = true;
    double cullingMargin = 1.0;
    
    // Debug settings
    bool enableDebugOverlay = false;
    bool showRenderingStats = false;
    bool showBoundingBoxes = false;
    bool showTileGrid = false;
    
    RenderingConfig() = default;
};

/**
 * @brief Main Enhanced Map Renderer class
 */
class EnhancedMapRenderer : public QObject
{
    Q_OBJECT

public:
    explicit EnhancedMapRenderer(QObject* parent = nullptr);
    ~EnhancedMapRenderer() override;

    // Core rendering methods
    void renderMap(QPainter* painter, Map* map, const QRect& viewRect, int currentFloor, double zoom);
    void renderTile(QPainter* painter, Tile* tile, const QRect& tileRect, int currentFloor, double zoom);
    void renderItem(QPainter* painter, Item* item, const QRect& itemRect, int currentFloor, double zoom);
    void renderSprite(QPainter* painter, GameSprite* sprite, const QRect& spriteRect, double zoom);
    
    // Advanced rendering methods
    void renderMapWithEffects(QPainter* painter, Map* map, const QRect& viewRect, int currentFloor, double zoom);
    void renderBackground(QPainter* painter, const QRect& viewRect);
    void renderGrid(QPainter* painter, const QRect& viewRect, double zoom);
    void renderOverlays(QPainter* painter, const QRect& viewRect, int currentFloor);
    void renderDebugInfo(QPainter* painter, const QRect& viewRect);
    
    // System integration
    void setLODManager(LODManager* lodManager) { lodManager_ = lodManager; }
    void setLightingSystem(EnhancedLightingSystem* lightingSystem) { lightingSystem_ = lightingSystem; }
    void setTransparencyManager(TransparencyManager* transparencyManager) { transparencyManager_ = transparencyManager; }
    void setDrawingOptions(EnhancedDrawingOptions* drawingOptions) { drawingOptions_ = drawingOptions; }
    
    LODManager* getLODManager() const { return lodManager_; }
    EnhancedLightingSystem* getLightingSystem() const { return lightingSystem_; }
    TransparencyManager* getTransparencyManager() const { return transparencyManager_; }
    EnhancedDrawingOptions* getDrawingOptions() const { return drawingOptions_; }
    
    // Configuration management
    void setRenderingConfig(const RenderingConfig& config);
    RenderingConfig getRenderingConfig() const { return config_; }
    void resetToDefaults();
    
    // Performance optimization
    void enableMultithreading(bool enabled);
    void enableAsyncRendering(bool enabled);
    void enableRenderCaching(bool enabled);
    void enableBatchRendering(bool enabled);
    bool isMultithreadingEnabled() const { return config_.enableMultithreading; }
    bool isAsyncRenderingEnabled() const { return config_.enableAsyncRendering; }
    bool isRenderCachingEnabled() const { return config_.enableRenderCaching; }
    bool isBatchRenderingEnabled() const { return config_.enableBatchRendering; }
    
    // Caching management
    void clearRenderCache();
    void updateRenderCache(const QRect& area, int floor);
    bool isRenderCached(const QRect& area, int floor) const;
    QPixmap getCachedRender(const QRect& area, int floor) const;
    void cacheRender(const QRect& area, int floor, const QPixmap& pixmap);
    
    // Statistics and monitoring
    RenderingStatistics getStatistics() const { return statistics_; }
    void resetStatistics();
    void updateStatistics();
    double getCurrentFPS() const { return statistics_.currentFPS; }
    double getLastFrameTime() const { return statistics_.lastFrameTime; }
    
    // Quality and performance settings
    void setRenderQuality(QPainter::RenderHints hints);
    QPainter::RenderHints getRenderQuality() const { return config_.renderHints; }
    void setAntialiasing(bool enabled);
    void setSmoothing(bool enabled);
    void setHighQualityRendering(bool enabled);
    
    // Culling and optimization
    void enableFrustumCulling(bool enabled) { config_.enableFrustumCulling = enabled; }
    void enableOcclusionCulling(bool enabled) { config_.enableOcclusionCulling = enabled; }
    void enableDirtyRectOptimization(bool enabled) { config_.enableDirtyRectOptimization = enabled; }
    bool isFrustumCullingEnabled() const { return config_.enableFrustumCulling; }
    bool isOcclusionCullingEnabled() const { return config_.enableOcclusionCulling; }
    bool isDirtyRectOptimizationEnabled() const { return config_.enableDirtyRectOptimization; }
    
    // Debug and diagnostics
    void setDebugOverlayEnabled(bool enabled) { config_.enableDebugOverlay = enabled; }
    void setRenderingStatsEnabled(bool enabled) { config_.showRenderingStats = enabled; }
    void setBoundingBoxesEnabled(bool enabled) { config_.showBoundingBoxes = enabled; }
    bool isDebugOverlayEnabled() const { return config_.enableDebugOverlay; }
    bool isRenderingStatsEnabled() const { return config_.showRenderingStats; }
    bool isBoundingBoxesEnabled() const { return config_.showBoundingBoxes; }
    
    // Utility methods
    QString getRenderingDiagnosticInfo() const;
    QVariantMap getRenderingDebugInfo() const;
    void dumpRenderingConfiguration() const;

signals:
    void renderingStarted();
    void renderingCompleted(double renderTime);
    void renderingError(const QString& error);
    void statisticsUpdated(const RenderingStatistics& stats);
    void performanceWarning(const QString& warning);

public slots:
    void onDrawingOptionsChanged();
    void onLODSettingsChanged();
    void onLightingSettingsChanged();
    void onTransparencySettingsChanged();
    void updateRenderingSystem();

private slots:
    void onStatisticsTimer();

private:
    // Core rendering implementation
    void renderTileLayer(QPainter* painter, Map* map, const QRect& viewRect, int floor, double zoom);
    void renderItemLayer(QPainter* painter, Map* map, const QRect& viewRect, int currentFloor, double zoom);
    void renderLightingLayer(QPainter* painter, Map* map, const QRect& viewRect, int currentFloor, double zoom);
    void renderTransparencyLayer(QPainter* painter, Map* map, const QRect& viewRect, int currentFloor, double zoom);
    
    // Optimization implementation
    QList<Tile*> cullTiles(Map* map, const QRect& viewRect, int floor) const;
    QList<Item*> cullItems(const QList<Item*>& items, const QRect& viewRect, double zoom) const;
    bool isTileVisible(Tile* tile, const QRect& viewRect, int floor) const;
    bool isItemVisible(Item* item, const QRect& itemRect, const QRect& viewRect) const;
    
    // Batch rendering implementation
    void beginBatchRendering();
    void addToBatch(const QRect& rect, Item* item, int floor);
    void renderBatch(QPainter* painter, double zoom);
    void endBatchRendering();
    
    // Cache management implementation
    QString getRenderCacheKey(const QRect& area, int floor) const;
    void invalidateRenderCache(const QRect& area, int floor);
    void cleanupRenderCache();
    
    // Performance monitoring
    void startPerformanceTimer();
    void stopPerformanceTimer();
    void updateFPSCounter();
    void trackRenderingOperation(const QString& operation, double time);
    
    // Error handling
    void handleRenderingError(const QString& error, const QString& context = QString());

private:
    // Rendering systems
    LODManager* lodManager_;
    EnhancedLightingSystem* lightingSystem_;
    TransparencyManager* transparencyManager_;
    EnhancedDrawingOptions* drawingOptions_;
    
    // Configuration and state
    RenderingConfig config_;
    RenderingStatistics statistics_;
    
    // Performance monitoring
    QElapsedTimer performanceTimer_;
    QTimer* statisticsTimer_;
    QList<double> frameTimeHistory_;
    double lastFPSUpdate_;
    
    // Caching
    mutable QHash<QString, QPixmap> renderCache_;
    mutable QMutex cacheMutex_;
    
    // Batch rendering
    struct BatchItem {
        QRect rect;
        Item* item;
        int floor;
    };
    
    QList<BatchItem> batchItems_;
    bool batchingActive_;
    
    // Threading (for future implementation)
    QThread* renderThread_;
    
    // Constants
    static const int STATISTICS_UPDATE_INTERVAL;
    static const int MAX_CACHE_SIZE;
    static const int FPS_HISTORY_SIZE;
    static const double CULLING_MARGIN;
};

/**
 * @brief Async rendering worker for multi-threaded rendering
 */
class AsyncRenderWorker : public QObject
{
    Q_OBJECT

public:
    explicit AsyncRenderWorker(EnhancedMapRenderer* renderer, QObject* parent = nullptr);
    ~AsyncRenderWorker() override;

public slots:
    void renderMapAsync(Map* map, const QRect& viewRect, int currentFloor, double zoom);

signals:
    void renderingCompleted(const QPixmap& result, double renderTime);
    void renderingError(const QString& error);

private:
    EnhancedMapRenderer* renderer_;
};

#endif // ENHANCEDMAPRENDERER_H
