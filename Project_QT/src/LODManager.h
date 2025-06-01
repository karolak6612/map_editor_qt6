#ifndef LODMANAGER_H
#define LODMANAGER_H

#include <QObject>
#include <QVariantMap>
#include <QStringList>
#include <QRect>
#include <QPoint>
#include <QHash>
#include <QTimer>
#include <QMutex>

// Forward declarations
class Item;
class ItemType;
class GameSprite;
class DrawingOptions;
class MapView;

/**
 * @brief Enhanced Level of Detail (LOD) Manager for Task 83
 * 
 * Complete implementation of LOD system for Qt6 map editor:
 * - Zoom-based LOD levels with configurable thresholds
 * - Item-specific LOD behavior based on ItemType properties
 * - Sprite simplification and detail reduction
 * - Performance optimization for large maps
 * - Integration with DrawingOptions and MapView
 * - 1:1 compatibility with wxwidgets LOD system
 */

/**
 * @brief LOD level enumeration (1:1 wxwidgets compatibility)
 */
enum class LODLevel {
    FULL_DETAIL = 0,    // Zoom 1.0-3.0: Full detail rendering
    MEDIUM_DETAIL = 1,  // Zoom 3.0-7.0: Reduced detail rendering
    GROUND_ONLY = 2,    // Zoom 7.0+: Ground tiles only
    MINIMAL = 3         // Zoom 10.0+: Minimal rendering
};

/**
 * @brief LOD configuration structure
 */
struct LODConfiguration {
    // Zoom thresholds for LOD levels
    double fullDetailMaxZoom = 3.0;
    double mediumDetailMaxZoom = 7.0;
    double groundOnlyMaxZoom = 10.0;
    
    // Item rendering limits per LOD level
    int maxItemsFullDetail = -1;        // No limit
    int maxItemsMediumDetail = 100;     // Limit to 100 items per tile
    int maxItemsGroundOnly = 1;         // Ground only
    int maxItemsMinimal = 0;            // No items
    
    // Sprite detail levels
    bool useSimplifiedSprites = true;
    bool skipAnimations = true;
    bool skipEffects = true;
    bool skipTransparency = false;
    
    // Performance settings
    bool enableCaching = true;
    bool enableBatching = true;
    bool enableCulling = true;
    
    // Item type specific settings
    QHash<QString, LODLevel> itemTypeLODOverrides;
    QStringList alwaysRenderTypes;      // Always render regardless of LOD
    QStringList skipInMediumDetail;     // Skip in medium detail
    QStringList skipInGroundOnly;       // Skip in ground only
    
    LODConfiguration() = default;
};

/**
 * @brief LOD rendering statistics
 */
struct LODStatistics {
    int totalTilesProcessed = 0;
    int totalItemsProcessed = 0;
    int itemsSkippedByLOD = 0;
    int spritesSimplified = 0;
    int animationsSkipped = 0;
    double averageRenderTime = 0.0;
    double lodOverhead = 0.0;
    
    void reset() {
        totalTilesProcessed = 0;
        totalItemsProcessed = 0;
        itemsSkippedByLOD = 0;
        spritesSimplified = 0;
        animationsSkipped = 0;
        averageRenderTime = 0.0;
        lodOverhead = 0.0;
    }
};

/**
 * @brief Main LOD Manager class
 */
class LODManager : public QObject
{
    Q_OBJECT

public:
    explicit LODManager(QObject* parent = nullptr);
    ~LODManager() override;

    // LOD level management
    LODLevel getCurrentLODLevel() const { return currentLevel_; }
    LODLevel getLevelForZoom(double zoom) const;
    void updateLODLevel(double zoom);
    void setLODLevel(LODLevel level);
    
    // Configuration management
    void setConfiguration(const LODConfiguration& config);
    LODConfiguration getConfiguration() const { return config_; }
    void resetToDefaults();
    void loadConfiguration(const QString& filePath);
    void saveConfiguration(const QString& filePath) const;
    
    // Rendering decisions
    bool shouldRenderItem(const Item* item, LODLevel level = LODLevel::FULL_DETAIL) const;
    bool shouldRenderItemType(const ItemType* itemType, LODLevel level = LODLevel::FULL_DETAIL) const;
    bool shouldUseSimplifiedSprite(const Item* item, LODLevel level = LODLevel::FULL_DETAIL) const;
    bool shouldSkipAnimation(const Item* item, LODLevel level = LODLevel::FULL_DETAIL) const;
    bool shouldSkipEffects(const Item* item, LODLevel level = LODLevel::FULL_DETAIL) const;
    
    // Item filtering
    QList<Item*> filterItemsByLOD(const QList<Item*>& items, LODLevel level = LODLevel::FULL_DETAIL) const;
    int getMaxItemsForLOD(LODLevel level) const;
    QStringList getSkippedTypesForLOD(LODLevel level) const;
    
    // Sprite simplification
    GameSprite* getSimplifiedSprite(const GameSprite* originalSprite, LODLevel level) const;
    QRect getSimplifiedSpriteRect(const QRect& originalRect, LODLevel level) const;
    double getSimplificationFactor(LODLevel level) const;
    
    // Performance optimization
    void enableCaching(bool enabled) { config_.enableCaching = enabled; }
    void enableBatching(bool enabled) { config_.enableBatching = enabled; }
    void enableCulling(bool enabled) { config_.enableCulling = enabled; }
    bool isCachingEnabled() const { return config_.enableCaching; }
    bool isBatchingEnabled() const { return config_.enableBatching; }
    bool isCullingEnabled() const { return config_.enableCulling; }
    
    // Statistics and monitoring
    LODStatistics getStatistics() const { return statistics_; }
    void resetStatistics();
    void updateStatistics(int tilesProcessed, int itemsProcessed, int itemsSkipped, double renderTime);
    
    // Integration with DrawingOptions
    void applyToDrawingOptions(DrawingOptions& options) const;
    void updateFromDrawingOptions(const DrawingOptions& options);
    
    // MapView integration
    void setMapView(MapView* mapView) { mapView_ = mapView; }
    MapView* getMapView() const { return mapView_; }
    void updateFromMapView();
    
    // Item type overrides
    void setItemTypeLODOverride(const QString& typeName, LODLevel level);
    void removeItemTypeLODOverride(const QString& typeName);
    LODLevel getItemTypeLODOverride(const QString& typeName) const;
    bool hasItemTypeLODOverride(const QString& typeName) const;
    
    // Always render types
    void addAlwaysRenderType(const QString& typeName);
    void removeAlwaysRenderType(const QString& typeName);
    bool isAlwaysRenderType(const QString& typeName) const;
    QStringList getAlwaysRenderTypes() const { return config_.alwaysRenderTypes; }
    
    // Utility methods
    QString getLODLevelName(LODLevel level) const;
    QStringList getAvailableLODLevels() const;
    bool isValidLODLevel(LODLevel level) const;
    
    // Debugging and diagnostics
    QString getDiagnosticInfo() const;
    QVariantMap getDebugInfo() const;
    void dumpConfiguration() const;

signals:
    void lodLevelChanged(LODLevel newLevel, LODLevel oldLevel);
    void configurationChanged();
    void statisticsUpdated(const LODStatistics& stats);
    void renderingOptimized(int itemsSkipped, double timeSaved);

public slots:
    void onZoomChanged(double newZoom);
    void onMapViewChanged();
    void onDrawingOptionsChanged();
    void updateStatisticsTimer();

private slots:
    void onStatisticsTimer();

private:
    // Core LOD logic
    LODLevel calculateLODLevel(double zoom) const;
    bool shouldSkipItemByType(const Item* item, LODLevel level) const;
    bool shouldSkipItemByProperties(const Item* item, LODLevel level) const;
    bool isItemTypeInSkipList(const QString& typeName, LODLevel level) const;
    
    // Sprite simplification implementation
    GameSprite* createSimplifiedSprite(const GameSprite* originalSprite, LODLevel level) const;
    void cacheSimplifiedSprite(const GameSprite* original, GameSprite* simplified, LODLevel level) const;
    GameSprite* getCachedSimplifiedSprite(const GameSprite* original, LODLevel level) const;
    
    // Performance optimization
    void optimizeRenderingForLOD(LODLevel level);
    void updateRenderingHints(LODLevel level);
    void clearCaches();
    
    // Configuration validation
    bool validateConfiguration(const LODConfiguration& config) const;
    void sanitizeConfiguration(LODConfiguration& config) const;
    
    // Statistics tracking
    void trackRenderingOperation(int itemsProcessed, int itemsSkipped, double renderTime);
    void updateAverageRenderTime(double newTime);
    
    // Error handling
    void handleLODError(const QString& error, const QString& context = QString()) const;

private:
    LODLevel currentLevel_;
    LODConfiguration config_;
    LODStatistics statistics_;
    MapView* mapView_;
    
    // Caching
    mutable QHash<const GameSprite*, QHash<LODLevel, GameSprite*>> simplifiedSpriteCache_;
    mutable QHash<QString, bool> itemTypeRenderCache_;
    mutable QHash<LODLevel, QStringList> skipListCache_;
    
    // Performance monitoring
    QTimer* statisticsTimer_;
    mutable QMutex cacheMutex_;
    
    // Constants
    static const double DEFAULT_FULL_DETAIL_MAX_ZOOM;
    static const double DEFAULT_MEDIUM_DETAIL_MAX_ZOOM;
    static const double DEFAULT_GROUND_ONLY_MAX_ZOOM;
    static const int STATISTICS_UPDATE_INTERVAL;
    static const int MAX_CACHE_SIZE;
};

/**
 * @brief LOD-aware rendering helper class
 */
class LODRenderer : public QObject
{
    Q_OBJECT

public:
    explicit LODRenderer(LODManager* lodManager, QObject* parent = nullptr);
    ~LODRenderer() override;

    // LOD-aware rendering methods
    void renderTileWithLOD(QPainter* painter, const QRect& tileRect, const QList<Item*>& items, const DrawingOptions& options);
    void renderItemWithLOD(QPainter* painter, const QRect& itemRect, Item* item, LODLevel level, const DrawingOptions& options);
    void renderSpriteWithLOD(QPainter* painter, const QRect& spriteRect, GameSprite* sprite, LODLevel level, const DrawingOptions& options);
    
    // Batch rendering for performance
    void beginBatchRendering(LODLevel level);
    void addToBatch(const QRect& rect, Item* item);
    void renderBatch(QPainter* painter, const DrawingOptions& options);
    void endBatchRendering();
    
    // Statistics
    int getItemsRendered() const { return itemsRendered_; }
    int getItemsSkipped() const { return itemsSkipped_; }
    double getLastRenderTime() const { return lastRenderTime_; }

private:
    LODManager* lodManager_;
    
    // Batch rendering
    struct BatchItem {
        QRect rect;
        Item* item;
        LODLevel level;
    };
    
    QList<BatchItem> batchItems_;
    bool batchingActive_;
    
    // Statistics
    int itemsRendered_;
    int itemsSkipped_;
    double lastRenderTime_;
};

#endif // LODMANAGER_H
