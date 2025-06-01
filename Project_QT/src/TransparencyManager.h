#ifndef TRANSPARENCYMANAGER_H
#define TRANSPARENCYMANAGER_H

#include <QObject>
#include <QPainter>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QHash>
#include <QList>
#include <QVariantMap>
#include <QTimer>
#include <QMutex>

// Forward declarations
class Item;
class Tile;
class Map;
class MapView;
class DrawingOptions;
class GameSprite;

/**
 * @brief Comprehensive Transparency Manager for Task 83
 * 
 * Complete implementation of transparency handling for Qt6 map editor:
 * - Floor-based transparency with configurable levels
 * - Item-specific transparency based on properties
 * - Dynamic transparency effects and animations
 * - Integration with QPainter alpha blending
 * - Performance optimization for transparent rendering
 * - 1:1 compatibility with wxwidgets transparency system
 */

/**
 * @brief Transparency mode enumeration
 */
enum class TransparencyMode {
    NONE = 0,           // No transparency
    FLOOR_BASED = 1,    // Transparency based on floor level
    ITEM_BASED = 2,     // Transparency based on item properties
    DISTANCE_BASED = 3, // Transparency based on distance from view
    CUSTOM = 4          // Custom transparency rules
};

/**
 * @brief Transparency configuration structure
 */
struct TransparencyConfig {
    // Global transparency settings
    bool enableTransparency = true;
    TransparencyMode mode = TransparencyMode::FLOOR_BASED;
    double globalTransparencyFactor = 1.0;
    
    // Floor-based transparency
    bool enableFloorTransparency = true;
    double floorTransparencyFactor = 0.7;      // Transparency per floor level
    double maxFloorTransparency = 0.9;         // Maximum transparency
    int transparentFloorRange = 3;             // Number of floors to make transparent
    bool fadeUpperFloors = true;               // Fade floors above current
    bool fadeLowerFloors = false;              // Fade floors below current
    
    // Item-based transparency
    bool enableItemTransparency = true;
    QHash<QString, double> itemTypeTransparency;    // Per-item-type transparency
    QStringList alwaysOpaqueTypes;                  // Items that are never transparent
    QStringList alwaysTransparentTypes;             // Items that are always transparent
    double defaultItemTransparency = 0.8;          // Default transparency for items
    
    // Distance-based transparency
    bool enableDistanceTransparency = false;
    double maxTransparencyDistance = 10.0;     // Distance at which items become fully transparent
    double transparencyFalloff = 2.0;          // Falloff factor for distance transparency
    
    // Animation settings
    bool enableTransparencyAnimation = true;
    double animationSpeed = 2.0;               // Animation speed factor
    double animationAmplitude = 0.2;           // Animation amplitude
    
    // Performance settings
    bool enableTransparencyCaching = true;
    bool enableTransparencyBatching = true;
    int maxTransparentItems = 1000;           // Maximum transparent items per frame
    double minTransparencyThreshold = 0.01;   // Minimum transparency to render
    
    TransparencyConfig() = default;
};

/**
 * @brief Transparency statistics
 */
struct TransparencyStatistics {
    int totalItemsProcessed = 0;
    int transparentItemsRendered = 0;
    int opaqueItemsRendered = 0;
    int itemsSkippedByTransparency = 0;
    double averageTransparencyCalculationTime = 0.0;
    double averageRenderTime = 0.0;
    int cacheHits = 0;
    int cacheMisses = 0;
    
    void reset() {
        totalItemsProcessed = 0;
        transparentItemsRendered = 0;
        opaqueItemsRendered = 0;
        itemsSkippedByTransparency = 0;
        averageTransparencyCalculationTime = 0.0;
        averageRenderTime = 0.0;
        cacheHits = 0;
        cacheMisses = 0;
    }
};

/**
 * @brief Main Transparency Manager class
 */
class TransparencyManager : public QObject
{
    Q_OBJECT

public:
    explicit TransparencyManager(QObject* parent = nullptr);
    ~TransparencyManager() override;

    // Configuration management
    void setConfiguration(const TransparencyConfig& config);
    TransparencyConfig getConfiguration() const { return config_; }
    void resetToDefaults();
    void loadConfiguration(const QString& filePath);
    void saveConfiguration(const QString& filePath) const;
    
    // Transparency mode management
    void setTransparencyMode(TransparencyMode mode);
    TransparencyMode getTransparencyMode() const { return config_.mode; }
    void setGlobalTransparencyFactor(double factor);
    double getGlobalTransparencyFactor() const { return config_.globalTransparencyFactor; }
    
    // Floor-based transparency
    void setFloorTransparencyEnabled(bool enabled);
    bool isFloorTransparencyEnabled() const { return config_.enableFloorTransparency; }
    void setFloorTransparencyFactor(double factor);
    double getFloorTransparencyFactor() const { return config_.floorTransparencyFactor; }
    void setTransparentFloorRange(int range);
    int getTransparentFloorRange() const { return config_.transparentFloorRange; }
    void setFadeUpperFloors(bool fade);
    void setFadeLowerFloors(bool fade);
    
    // Item-based transparency
    void setItemTransparency(const QString& itemType, double transparency);
    double getItemTransparency(const QString& itemType) const;
    void removeItemTransparency(const QString& itemType);
    void addAlwaysOpaqueType(const QString& itemType);
    void removeAlwaysOpaqueType(const QString& itemType);
    void addAlwaysTransparentType(const QString& itemType);
    void removeAlwaysTransparentType(const QString& itemType);
    bool isAlwaysOpaqueType(const QString& itemType) const;
    bool isAlwaysTransparentType(const QString& itemType) const;
    
    // Transparency calculation
    double calculateTransparency(const Item* item, const QPoint& position, int floor, int currentFloor) const;
    double calculateFloorTransparency(int itemFloor, int currentFloor) const;
    double calculateItemTransparency(const Item* item) const;
    double calculateDistanceTransparency(const QPoint& itemPosition, const QPoint& viewCenter) const;
    double calculateAnimatedTransparency(double baseTransparency, double time) const;
    
    // Rendering methods
    void renderWithTransparency(QPainter* painter, const QRect& rect, const Item* item, const QPoint& position, int floor, int currentFloor);
    void renderSpriteWithTransparency(QPainter* painter, const QRect& rect, GameSprite* sprite, double transparency);
    void renderTileWithTransparency(QPainter* painter, const QRect& rect, const Tile* tile, int currentFloor);
    void applyTransparencyToPixmap(QPixmap& pixmap, double transparency);
    void applyTransparencyToColor(QColor& color, double transparency);
    
    // Batch rendering for performance
    void beginTransparencyBatch();
    void addToTransparencyBatch(const QRect& rect, const Item* item, const QPoint& position, int floor, int currentFloor);
    void renderTransparencyBatch(QPainter* painter);
    void endTransparencyBatch();
    
    // Performance optimization
    void enableTransparencyCaching(bool enabled) { config_.enableTransparencyCaching = enabled; }
    void enableTransparencyBatching(bool enabled) { config_.enableTransparencyBatching = enabled; }
    bool isTransparencyCachingEnabled() const { return config_.enableTransparencyCaching; }
    bool isTransparencyBatchingEnabled() const { return config_.enableTransparencyBatching; }
    
    // Caching management
    void clearTransparencyCache();
    void updateTransparencyCache(const QRect& area, int floor);
    bool isTransparencyCached(const Item* item, const QPoint& position, int floor, int currentFloor) const;
    double getCachedTransparency(const Item* item, const QPoint& position, int floor, int currentFloor) const;
    void cacheTransparency(const Item* item, const QPoint& position, int floor, int currentFloor, double transparency);
    
    // Animation and dynamics
    void updateTransparencyAnimations(double deltaTime);
    void setTransparencyAnimationEnabled(bool enabled) { config_.enableTransparencyAnimation = enabled; }
    bool isTransparencyAnimationEnabled() const { return config_.enableTransparencyAnimation; }
    void setAnimationSpeed(double speed) { config_.animationSpeed = speed; }
    double getAnimationSpeed() const { return config_.animationSpeed; }
    
    // Statistics and monitoring
    TransparencyStatistics getStatistics() const { return statistics_; }
    void resetStatistics();
    void updateStatistics(int itemsProcessed, int transparentItems, int opaqueItems, double calculationTime, double renderTime);
    
    // Integration with MapView and DrawingOptions
    void setMapView(MapView* mapView) { mapView_ = mapView; }
    MapView* getMapView() const { return mapView_; }
    void updateFromDrawingOptions(const DrawingOptions& options);
    void applyToDrawingOptions(DrawingOptions& options) const;
    
    // Utility methods
    bool shouldRenderTransparent(const Item* item, const QPoint& position, int floor, int currentFloor) const;
    bool isTransparencySignificant(double transparency) const;
    QString getTransparencyDiagnosticInfo() const;
    QVariantMap getTransparencyDebugInfo() const;
    
    // Advanced transparency effects
    void renderWithFadeEffect(QPainter* painter, const QRect& rect, const Item* item, double fadeAmount);
    void renderWithGhostEffect(QPainter* painter, const QRect& rect, const Item* item, double ghostAmount);
    void renderWithBlurEffect(QPainter* painter, const QRect& rect, const Item* item, double blurAmount);

signals:
    void transparencyConfigChanged();
    void transparencyModeChanged(TransparencyMode newMode, TransparencyMode oldMode);
    void transparencyStatisticsUpdated(const TransparencyStatistics& stats);
    void transparencyAnimationUpdated(double deltaTime);

public slots:
    void onMapChanged();
    void onDrawingOptionsChanged();
    void onCurrentFloorChanged(int newFloor);
    void onAnimationTimer();
    void updateTransparencySystem();

private slots:
    void onStatisticsTimer();

private:
    // Core transparency calculations
    double calculateBaseTransparency(const Item* item, const QPoint& position, int floor, int currentFloor) const;
    double applyTransparencyMode(double baseTransparency, const Item* item, const QPoint& position, int floor, int currentFloor) const;
    double clampTransparency(double transparency) const;
    bool shouldSkipTransparentItem(double transparency) const;
    
    // Rendering optimization
    void optimizeTransparencyRendering(const QRect& area, int floor);
    void batchTransparentItems(QPainter* painter, const QList<const Item*>& items, const QRect& area, int currentFloor);
    void cullInvisibleTransparentItems(const QRect& viewRect, int floor);
    
    // Cache management
    QString getTransparencyCacheKey(const Item* item, const QPoint& position, int floor, int currentFloor) const;
    void invalidateTransparencyCache(const QRect& area, int floor);
    void cleanupTransparencyCache();
    
    // Animation helpers
    double calculateAnimationPhase(double time) const;
    double applyAnimationToTransparency(double baseTransparency, double animationPhase) const;
    
    // Configuration validation
    bool validateConfiguration(const TransparencyConfig& config) const;
    void sanitizeConfiguration(TransparencyConfig& config) const;
    
    // Error handling
    void handleTransparencyError(const QString& error, const QString& context = QString()) const;

private:
    TransparencyConfig config_;
    TransparencyStatistics statistics_;
    MapView* mapView_;
    int currentFloor_;
    
    // Caching
    mutable QHash<QString, double> transparencyCache_;
    mutable QHash<QString, bool> itemTypeOpaqueCache_;
    
    // Batch rendering
    struct TransparencyBatchItem {
        QRect rect;
        const Item* item;
        QPoint position;
        int floor;
        int currentFloor;
        double transparency;
    };
    
    QList<TransparencyBatchItem> batchItems_;
    bool batchingActive_;
    
    // Animation
    QTimer* animationTimer_;
    double lastAnimationTime_;
    
    // Performance monitoring
    QTimer* statisticsTimer_;
    mutable QMutex cacheMutex_;
    
    // Constants
    static const double DEFAULT_FLOOR_TRANSPARENCY_FACTOR;
    static const double DEFAULT_ITEM_TRANSPARENCY;
    static const int ANIMATION_UPDATE_INTERVAL;
    static const int STATISTICS_UPDATE_INTERVAL;
    static const int MAX_CACHE_SIZE;
    static const double MIN_TRANSPARENCY_THRESHOLD;
};

/**
 * @brief Transparency effect renderer for advanced visual effects
 */
class TransparencyEffectRenderer : public QObject
{
    Q_OBJECT

public:
    explicit TransparencyEffectRenderer(TransparencyManager* transparencyManager, QObject* parent = nullptr);
    ~TransparencyEffectRenderer() override;

    // Effect rendering methods
    void renderFadeTransition(QPainter* painter, const QRect& rect, const Item* item, double fadeProgress);
    void renderGhostEffect(QPainter* painter, const QRect& rect, const Item* item, double ghostIntensity);
    void renderXRayEffect(QPainter* painter, const QRect& rect, const Item* item, double xrayIntensity);
    void renderWireframeEffect(QPainter* painter, const QRect& rect, const Item* item, double wireframeIntensity);
    
    // Composition mode effects
    void renderWithMultiply(QPainter* painter, const QRect& rect, const Item* item, double transparency);
    void renderWithScreen(QPainter* painter, const QRect& rect, const Item* item, double transparency);
    void renderWithOverlay(QPainter* painter, const QRect& rect, const Item* item, double transparency);

private:
    TransparencyManager* transparencyManager_;
    
    // Effect helpers
    void applyFadeGradient(QPainter* painter, const QRect& rect, double fadeAmount);
    void applyGhostFilter(QPainter* painter, const QRect& rect, double ghostAmount);
    void drawWireframe(QPainter* painter, const QRect& rect, const Item* item, double intensity);
};

#endif // TRANSPARENCYMANAGER_H
