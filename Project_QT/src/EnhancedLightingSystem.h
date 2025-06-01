#ifndef ENHANCEDLIGHTINGSYSTEM_H
#define ENHANCEDLIGHTINGSYSTEM_H

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
#include <QGraphicsEffect>

// Forward declarations
class Item;
class Tile;
class Map;
class MapView;
class DrawingOptions;
class GameSprite;

/**
 * @brief Enhanced Lighting System for Task 83
 * 
 * Complete implementation of lighting effects for Qt6 map editor:
 * - Item-emitted light sources with configurable properties
 * - Global lighting and shading effects
 * - Floor-based lighting attenuation
 * - Dynamic light calculation and rendering
 * - Integration with QPainter composition modes
 * - Performance optimization for large maps
 * - 1:1 compatibility with wxwidgets lighting system
 */

/**
 * @brief Light source structure (enhanced from wxwidgets SpriteLight)
 */
struct LightSource {
    QPoint position;
    int floor = 7;
    
    // Light properties
    quint8 intensity = 0;           // Light intensity (0-255)
    quint8 color = 215;             // Light color (8-bit color index)
    QColor rgbColor = Qt::white;    // RGB color representation
    
    // Advanced properties
    double radius = 1.0;            // Light radius in tiles
    double falloff = 1.0;           // Light falloff factor
    bool enabled = true;            // Whether light is active
    bool dynamic = false;           // Whether light changes over time
    
    // Animation properties
    double flickerRate = 0.0;       // Flicker animation rate
    double pulseRate = 0.0;         // Pulse animation rate
    double currentPhase = 0.0;      // Current animation phase
    
    // Performance properties
    QRect boundingRect;             // Cached bounding rectangle
    bool needsUpdate = true;        // Whether light needs recalculation
    
    LightSource() = default;
    LightSource(const QPoint& pos, int floor, quint8 intensity, quint8 color);
    LightSource(const QPoint& pos, int floor, quint8 intensity, const QColor& rgbColor);
    
    // Utility methods
    bool isValid() const { return intensity > 0 && enabled; }
    double getEffectiveIntensity(double time = 0.0) const;
    QColor getEffectiveColor(double time = 0.0) const;
    void updateBoundingRect();
    QString toString() const;
};

/**
 * @brief Global lighting configuration
 */
struct GlobalLightingConfig {
    // Global light settings
    QColor globalLightColor = QColor(50, 50, 50, 255);
    quint8 globalLightIntensity = 140;
    bool enableGlobalLighting = true;
    
    // Floor shading
    bool enableFloorShading = true;
    quint8 floorShadingIntensity = 128;
    QColor floorShadingColor = QColor(0, 0, 0, 128);
    
    // Light calculation
    double maxLightRadius = 10.0;
    double lightFalloffFactor = 2.0;
    bool enableLightBlending = true;
    bool enableLightCaching = true;
    
    // Performance settings
    int maxLightsPerTile = 10;
    double minLightIntensity = 0.01;
    bool enableLightCulling = true;
    bool enableBatchRendering = true;
    
    // Visual effects
    bool enableLightSmoothing = true;
    bool enableLightAnimation = true;
    bool enableLightReflection = false;
    
    GlobalLightingConfig() = default;
};

/**
 * @brief Lighting statistics
 */
struct LightingStatistics {
    int totalLights = 0;
    int activeLights = 0;
    int lightsRendered = 0;
    int lightsCulled = 0;
    double averageLightCalculationTime = 0.0;
    double averageRenderTime = 0.0;
    int cacheHits = 0;
    int cacheMisses = 0;
    
    void reset() {
        totalLights = 0;
        activeLights = 0;
        lightsRendered = 0;
        lightsCulled = 0;
        averageLightCalculationTime = 0.0;
        averageRenderTime = 0.0;
        cacheHits = 0;
        cacheMisses = 0;
    }
};

/**
 * @brief Main Enhanced Lighting System class
 */
class EnhancedLightingSystem : public QObject
{
    Q_OBJECT

public:
    explicit EnhancedLightingSystem(QObject* parent = nullptr);
    ~EnhancedLightingSystem() override;

    // Light source management
    void addLightSource(const LightSource& light);
    void addLightSource(const QPoint& position, int floor, quint8 intensity, quint8 color);
    void addLightSource(const QPoint& position, int floor, quint8 intensity, const QColor& rgbColor);
    void removeLightSource(const QPoint& position, int floor);
    void clearLightSources();
    void updateLightSource(const QPoint& position, int floor, const LightSource& newLight);
    
    // Light source queries
    QList<LightSource> getLightSources() const { return lightSources_; }
    QList<LightSource> getLightSourcesInArea(const QRect& area, int floor) const;
    LightSource* getLightSource(const QPoint& position, int floor);
    bool hasLightSource(const QPoint& position, int floor) const;
    int getLightSourceCount() const { return lightSources_.size(); }
    
    // Item-based light management
    void addLightFromItem(const Item* item, const QPoint& position, int floor);
    void removeLightFromItem(const Item* item, const QPoint& position, int floor);
    void updateLightFromItem(const Item* item, const QPoint& position, int floor);
    bool itemHasLight(const Item* item) const;
    LightSource extractLightFromItem(const Item* item, const QPoint& position, int floor) const;
    
    // Global lighting configuration
    void setGlobalLightingConfig(const GlobalLightingConfig& config);
    GlobalLightingConfig getGlobalLightingConfig() const { return globalConfig_; }
    void setGlobalLightColor(const QColor& color);
    void setGlobalLightIntensity(quint8 intensity);
    void setFloorShadingEnabled(bool enabled);
    void setFloorShadingIntensity(quint8 intensity);
    
    // Lighting calculation
    double calculateLightIntensity(const QPoint& position, int floor) const;
    QColor calculateLightColor(const QPoint& position, int floor) const;
    QColor blendLightColors(const QList<QColor>& colors, const QList<double>& intensities) const;
    double calculateLightDistance(const LightSource& light, const QPoint& position, int floor) const;
    double calculateLightFalloff(double distance, double radius, double falloffFactor) const;
    
    // Rendering methods
    void renderLighting(QPainter* painter, const QRect& viewRect, int floor, const DrawingOptions& options);
    void renderGlobalLighting(QPainter* painter, const QRect& viewRect, int floor, const DrawingOptions& options);
    void renderLightSources(QPainter* painter, const QRect& viewRect, int floor, const DrawingOptions& options);
    void renderFloorShading(QPainter* painter, const QRect& viewRect, int floor, const DrawingOptions& options);
    void renderLightEffect(QPainter* painter, const LightSource& light, const QRect& viewRect, const DrawingOptions& options);
    
    // Advanced rendering effects
    void renderLightWithComposition(QPainter* painter, const LightSource& light, const QRect& viewRect, QPainter::CompositionMode mode);
    void renderLightWithGradient(QPainter* painter, const LightSource& light, const QRect& viewRect);
    void renderLightWithBlur(QPainter* painter, const LightSource& light, const QRect& viewRect, double blurRadius);
    void renderLightReflection(QPainter* painter, const LightSource& light, const QRect& viewRect, const DrawingOptions& options);
    
    // Performance optimization
    void enableLightCaching(bool enabled) { globalConfig_.enableLightCaching = enabled; }
    void enableLightCulling(bool enabled) { globalConfig_.enableLightCulling = enabled; }
    void enableBatchRendering(bool enabled) { globalConfig_.enableBatchRendering = enabled; }
    bool isLightCachingEnabled() const { return globalConfig_.enableLightCaching; }
    bool isLightCullingEnabled() const { return globalConfig_.enableLightCulling; }
    bool isBatchRenderingEnabled() const { return globalConfig_.enableBatchRendering; }
    
    // Caching management
    void clearLightCache();
    void updateLightCache(const QRect& area, int floor);
    bool isLightCached(const QPoint& position, int floor) const;
    QColor getCachedLightColor(const QPoint& position, int floor) const;
    void cacheLightColor(const QPoint& position, int floor, const QColor& color);
    
    // Animation and dynamics
    void updateLightAnimations(double deltaTime);
    void setLightAnimationEnabled(bool enabled) { globalConfig_.enableLightAnimation = enabled; }
    bool isLightAnimationEnabled() const { return globalConfig_.enableLightAnimation; }
    void updateDynamicLights();
    
    // Statistics and monitoring
    LightingStatistics getStatistics() const { return statistics_; }
    void resetStatistics();
    void updateStatistics(int lightsProcessed, int lightsRendered, double calculationTime, double renderTime);
    
    // Integration with MapView and DrawingOptions
    void setMapView(MapView* mapView) { mapView_ = mapView; }
    MapView* getMapView() const { return mapView_; }
    void updateFromDrawingOptions(const DrawingOptions& options);
    void applyToDrawingOptions(DrawingOptions& options) const;
    
    // Utility methods
    QColor colorFromEightBit(quint8 colorIndex) const;
    quint8 colorToEightBit(const QColor& color) const;
    QString getLightingDiagnosticInfo() const;
    QVariantMap getLightingDebugInfo() const;
    
    // Configuration persistence
    void loadConfiguration(const QString& filePath);
    void saveConfiguration(const QString& filePath) const;
    void resetToDefaults();

signals:
    void lightSourceAdded(const LightSource& light);
    void lightSourceRemoved(const QPoint& position, int floor);
    void lightSourceUpdated(const LightSource& light);
    void globalLightingChanged();
    void lightingStatisticsUpdated(const LightingStatistics& stats);
    void lightAnimationUpdated(double deltaTime);

public slots:
    void onMapChanged();
    void onDrawingOptionsChanged();
    void onAnimationTimer();
    void updateLightingSystem();

private slots:
    void onStatisticsTimer();

private:
    // Core lighting calculations
    QList<LightSource> getLightsAffectingPosition(const QPoint& position, int floor) const;
    double calculateLightContribution(const LightSource& light, const QPoint& position, int floor) const;
    QColor blendLightColor(const QColor& baseColor, const QColor& lightColor, double intensity) const;
    bool isLightVisible(const LightSource& light, const QRect& viewRect, int floor) const;
    
    // Rendering optimization
    void cullInvisibleLights(const QRect& viewRect, int floor);
    void batchLightRendering(QPainter* painter, const QList<LightSource>& lights, const QRect& viewRect, const DrawingOptions& options);
    void optimizeLightCalculation(const QRect& area, int floor);
    
    // Cache management
    QString getLightCacheKey(const QPoint& position, int floor) const;
    void invalidateLightCache(const QRect& area, int floor);
    void cleanupLightCache();
    
    // Animation helpers
    void updateLightAnimation(LightSource& light, double deltaTime);
    double calculateFlickerIntensity(const LightSource& light, double time) const;
    double calculatePulseIntensity(const LightSource& light, double time) const;
    
    // Error handling
    void handleLightingError(const QString& error, const QString& context = QString()) const;

private:
    QList<LightSource> lightSources_;
    GlobalLightingConfig globalConfig_;
    LightingStatistics statistics_;
    MapView* mapView_;
    
    // Caching
    mutable QHash<QString, QColor> lightColorCache_;
    mutable QHash<QString, double> lightIntensityCache_;
    QHash<QPoint, QList<LightSource*>> spatialIndex_;
    
    // Animation
    QTimer* animationTimer_;
    double lastAnimationTime_;
    
    // Performance monitoring
    QTimer* statisticsTimer_;
    mutable QMutex cacheMutex_;
    
    // Constants
    static const double DEFAULT_LIGHT_RADIUS;
    static const double DEFAULT_FALLOFF_FACTOR;
    static const int ANIMATION_UPDATE_INTERVAL;
    static const int STATISTICS_UPDATE_INTERVAL;
    static const int MAX_CACHE_SIZE;
    static const double MIN_LIGHT_INTENSITY;
};

/**
 * @brief Light effect renderer for advanced visual effects
 */
class LightEffectRenderer : public QObject
{
    Q_OBJECT

public:
    explicit LightEffectRenderer(EnhancedLightingSystem* lightingSystem, QObject* parent = nullptr);
    ~LightEffectRenderer() override;

    // Effect rendering methods
    void renderLightHalo(QPainter* painter, const LightSource& light, const QRect& viewRect);
    void renderLightBeam(QPainter* painter, const LightSource& light, const QRect& viewRect, double angle, double width);
    void renderLightGlow(QPainter* painter, const LightSource& light, const QRect& viewRect, double glowRadius);
    void renderLightFlicker(QPainter* painter, const LightSource& light, const QRect& viewRect, double flickerIntensity);
    
    // Composition mode effects
    void renderWithMultiply(QPainter* painter, const LightSource& light, const QRect& viewRect);
    void renderWithScreen(QPainter* painter, const LightSource& light, const QRect& viewRect);
    void renderWithOverlay(QPainter* painter, const LightSource& light, const QRect& viewRect);
    void renderWithSoftLight(QPainter* painter, const LightSource& light, const QRect& viewRect);

private:
    EnhancedLightingSystem* lightingSystem_;
    
    // Effect helpers
    QRadialGradient createLightGradient(const LightSource& light, double radius) const;
    QLinearGradient createBeamGradient(const LightSource& light, double angle, double width) const;
    void applyLightBlur(QPainter* painter, const QRect& rect, double blurRadius);
};

#endif // ENHANCEDLIGHTINGSYSTEM_H
