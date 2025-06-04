#ifndef SPAWNVISUALIZATIONMANAGER_H
#define SPAWNVISUALIZATIONMANAGER_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QMap>
#include <QSet>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QPixmap>
#include <QElapsedTimer>
#include <QVariantMap>

#include "MapPos.h"
#include "ui/SpawnItem.h"

// Forward declarations
class Map;
class MapView;
class MapScene;
class Spawn;
class Tile;
class SpawnBrush;

/**
 * @brief Task 92: Spawn Visualization Manager for finalizing spawn display
 * 
 * Complete spawn visualization system with wxwidgets compatibility:
 * - Manages all SpawnItem instances in MapScene
 * - Handles spawn marker rendering (purple flame from SPRITE_SPAWN)
 * - Manages spawn radius indicators and creature positions
 * - Integrates with SpawnBrush application and map updates
 * - Handles drawing flags and tile state updates
 * - Provides dynamic visual updates when spawn attributes change
 * - Maintains 1:1 compatibility with wxwidgets spawn visualization
 * - Optimizes performance for large numbers of spawns
 */

/**
 * @brief Spawn Drawing Flags (from wxwidgets compatibility)
 */
enum class SpawnDrawingFlag : quint32 {
    None = 0x0000,
    ShowMarker = 0x0001,        // Show spawn marker (purple flame)
    ShowRadius = 0x0002,        // Show spawn radius circle
    ShowCreatures = 0x0004,     // Show creature positions
    ShowInfo = 0x0008,          // Show spawn information text
    Selected = 0x0010,          // Spawn is selected (different color)
    Highlighted = 0x0020,       // Spawn is highlighted (hover)
    Animated = 0x0040,          // Enable animation effects
    DebugMode = 0x0080          // Show debug information
};
Q_DECLARE_FLAGS(SpawnDrawingFlags, SpawnDrawingFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(SpawnDrawingFlags)

/**
 * @brief Spawn Visual Style Configuration
 */
struct SpawnVisualStyle {
    // Marker appearance (purple flame from wxwidgets)
    QColor markerColor;
    QColor markerSelectedColor;
    QColor markerHighlightColor;
    int markerSize;
    
    // Radius appearance
    QColor radiusColor;
    QColor radiusSelectedColor;
    int radiusLineWidth;
    Qt::PenStyle radiusLineStyle;
    int radiusOpacity;
    
    // Creature position appearance
    QColor creaturePositionColor;
    int creaturePositionSize;
    
    // Text appearance
    QColor textColor;
    QFont textFont;
    
    // Animation settings
    bool enableAnimation;
    int animationDuration;
    double animationIntensity;
    
    SpawnVisualStyle() {
        // Default values matching wxwidgets
        markerColor = QColor(128, 0, 128);           // Purple
        markerSelectedColor = QColor(255, 0, 255);   // Bright purple
        markerHighlightColor = QColor(192, 64, 192); // Light purple
        markerSize = 16;
        
        radiusColor = QColor(128, 0, 128, 100);      // Semi-transparent purple
        radiusSelectedColor = QColor(255, 0, 255, 150);
        radiusLineWidth = 1;
        radiusLineStyle = Qt::DashLine;
        radiusOpacity = 100;
        
        creaturePositionColor = QColor(64, 255, 64); // Green
        creaturePositionSize = 4;
        
        textColor = QColor(255, 255, 255);
        textFont = QFont("Arial", 8);
        
        enableAnimation = true;
        animationDuration = 2000; // 2 seconds
        animationIntensity = 0.3;
    }
};

/**
 * @brief Main Spawn Visualization Manager
 */
class SpawnVisualizationManager : public QObject
{
    Q_OBJECT

public:
    explicit SpawnVisualizationManager(QObject* parent = nullptr);
    ~SpawnVisualizationManager() override;

    // Component setup
    void setMap(Map* map);
    void setMapView(MapView* mapView);
    void setMapScene(MapScene* scene);
    Map* getMap() const { return map_; }
    MapView* getMapView() const { return mapView_; }
    MapScene* getMapScene() const { return scene_; }

    // Visualization control
    void setShowSpawns(bool show);
    bool isShowSpawns() const { return showSpawns_; }
    
    void setShowSpawnRadius(bool show);
    bool isShowSpawnRadius() const { return showSpawnRadius_; }
    
    void setShowCreaturePositions(bool show);
    bool isShowCreaturePositions() const { return showCreaturePositions_; }
    
    void setShowSpawnInfo(bool show);
    bool isShowSpawnInfo() const { return showSpawnInfo_; }

    // Drawing flags management
    void setSpawnDrawingFlags(Spawn* spawn, SpawnDrawingFlags flags);
    SpawnDrawingFlags getSpawnDrawingFlags(Spawn* spawn) const;
    void updateSpawnDrawingFlags(Spawn* spawn);
    void clearSpawnDrawingFlags(Spawn* spawn);

    // Visual style management
    void setVisualStyle(const SpawnVisualStyle& style);
    SpawnVisualStyle getVisualStyle() const { return visualStyle_; }
    void resetVisualStyle();

    // Spawn item management
    SpawnItem* createSpawnItem(Spawn* spawn);
    void removeSpawnItem(Spawn* spawn);
    SpawnItem* getSpawnItem(Spawn* spawn) const;
    QList<SpawnItem*> getAllSpawnItems() const;
    void updateSpawnItem(Spawn* spawn);
    void refreshAllSpawnItems();

    // Spawn visualization updates
    void updateSpawnVisualization(Spawn* spawn);
    void updateSpawnRadius(Spawn* spawn);
    void updateSpawnMarker(Spawn* spawn);
    void updateCreaturePositions(Spawn* spawn);
    void updateSpawnSelection(Spawn* spawn, bool selected);

    // Map integration
    void onSpawnAdded(Spawn* spawn);
    void onSpawnRemoved(Spawn* spawn);
    void onSpawnModified(Spawn* spawn);
    void onSpawnAttributeChanged(Spawn* spawn, const QString& attribute);

    // Tile flag integration
    void updateTileFlags(Tile* tile, bool hasSpawn);
    void notifyBorderSystem(Tile* tile);
    void handleTileStateUpdate(Tile* tile);

    // Performance and optimization
    void setUpdateInterval(int milliseconds);
    int getUpdateInterval() const { return updateInterval_; }
    void optimizeForLargeSpawnCount(bool optimize);
    bool isOptimizedForLargeSpawnCount() const { return optimizeForLargeSpawnCount_; }

    // Statistics and monitoring
    QVariantMap getStatistics() const;
    void resetStatistics();
    int getSpawnItemCount() const;
    int getVisibleSpawnItemCount() const;

public slots:
    void refreshVisualization();
    void updateAllSpawnItems();
    void clearAllSpawnItems();
    void onMapChanged();
    void onViewportChanged();
    void onZoomChanged(double zoom);
    void onFloorChanged(int floor);

signals:
    void spawnItemCreated(SpawnItem* item);
    void spawnItemRemoved(SpawnItem* item);
    void spawnItemUpdated(SpawnItem* item);
    void visualizationRefreshed();
    void drawingFlagsChanged(Spawn* spawn, SpawnDrawingFlags flags);

private slots:
    void onUpdateTimer();
    void onAnimationTimer();

private:
    // Core components
    Map* map_;
    MapView* mapView_;
    MapScene* scene_;
    
    // Visualization settings
    bool showSpawns_;
    bool showSpawnRadius_;
    bool showCreaturePositions_;
    bool showSpawnInfo_;
    SpawnVisualStyle visualStyle_;
    
    // Spawn item management
    QMap<Spawn*, SpawnItem*> spawnItems_;
    QMap<Spawn*, SpawnDrawingFlags> spawnDrawingFlags_;
    QMutex spawnItemsMutex_;
    
    // Update management
    QTimer* updateTimer_;
    QTimer* animationTimer_;
    int updateInterval_;
    QSet<Spawn*> pendingUpdates_;
    
    // Performance optimization
    bool optimizeForLargeSpawnCount_;
    int maxVisibleSpawns_;
    QElapsedTimer performanceTimer_;
    
    // Statistics
    mutable QMutex statsMutex_;
    QVariantMap statistics_;
    int spawnItemsCreated_;
    int spawnItemsRemoved_;
    int visualizationUpdates_;
    
    // Helper methods
    void createSpawnItemInternal(Spawn* spawn);
    void removeSpawnItemInternal(Spawn* spawn);
    void updateSpawnItemInternal(SpawnItem* item, Spawn* spawn);
    bool isSpawnVisible(Spawn* spawn) const;
    void applyVisualStyle(SpawnItem* item) const;
    void updateStatistics();
    
    // wxwidgets compatibility
    void applyWxWidgetsCompatibility(SpawnItem* item) const;
    QPixmap createSpawnMarkerPixmap(const SpawnVisualStyle& style, bool selected = false) const;
    
    // Constants
    static const int DEFAULT_UPDATE_INTERVAL = 100; // milliseconds
    static const int DEFAULT_ANIMATION_INTERVAL = 50; // milliseconds
    static const int MAX_VISIBLE_SPAWNS_OPTIMIZED = 1000;
};

/**
 * @brief Spawn Drawing Flag Manager for tile state integration
 */
class SpawnDrawingFlagManager : public QObject
{
    Q_OBJECT

public:
    explicit SpawnDrawingFlagManager(QObject* parent = nullptr);
    ~SpawnDrawingFlagManager() override = default;

    // Flag management
    void setTileSpawnFlag(Tile* tile, bool hasSpawn);
    bool getTileSpawnFlag(Tile* tile) const;
    void updateTileSpawnFlags(Map* map);
    void clearTileSpawnFlags(Map* map);

    // Border system integration
    void notifyBorderSystemSpawnChange(Tile* tile, bool hasSpawn);
    void updateBorderizationForSpawn(Map* map, const MapPos& position);

    // Drawing flag interpretation
    void interpretDrawingFlags(Spawn* spawn, SpawnDrawingFlags& flags) const;
    void applyDrawingFlags(SpawnItem* item, SpawnDrawingFlags flags) const;

signals:
    void tileSpawnFlagChanged(Tile* tile, bool hasSpawn);
    void borderSystemNotified(Tile* tile);

private:
    void updateTileFlagsInternal(Tile* tile, bool hasSpawn);
    void notifyAdjacentTiles(Map* map, const MapPos& position);
};

/**
 * @brief Spawn Attribute Reflector for dynamic visual updates
 */
class SpawnAttributeReflector : public QObject
{
    Q_OBJECT

public:
    explicit SpawnAttributeReflector(SpawnVisualizationManager* manager, QObject* parent = nullptr);
    ~SpawnAttributeReflector() override = default;

    // Attribute monitoring
    void monitorSpawn(Spawn* spawn);
    void unmonitorSpawn(Spawn* spawn);
    void checkSpawnAttributes(Spawn* spawn);
    void updateVisualizationForAttribute(Spawn* spawn, const QString& attribute);

public slots:
    void onSpawnAttributeChanged(Spawn* spawn, const QString& attribute, const QVariant& oldValue, const QVariant& newValue);

signals:
    void attributeVisualizationUpdateRequired(Spawn* spawn, const QString& attribute);

private:
    SpawnVisualizationManager* manager_;
    QMap<Spawn*, QVariantMap> spawnAttributeCache_;
    QMutex cacheMutex_;
    
    void updateAttributeCache(Spawn* spawn);
    bool hasAttributeChanged(Spawn* spawn, const QString& attribute) const;
};

#endif // SPAWNVISUALIZATIONMANAGER_H
