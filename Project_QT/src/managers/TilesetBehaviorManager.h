#ifndef TILESETBEHAVIORMANAGER_H
#define TILESETBEHAVIORMANAGER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QVariantMap>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QElapsedTimer>

#include "TilesetManager.h"
#include "ui/TilesetEditorDialog.h"

// Forward declarations
class Map;
class Tile;
class Item;
class Brush;
class DoorBrush;
class WallBrush;
class GroundBrush;
class MapView;

/**
 * @brief Task 91: Tileset Behavior Manager for special tileset handling
 * 
 * Manages special tileset behaviors and their integration with map editing:
 * - Door tileset behavior (palettedoor functionality)
 * - Wall tileset behavior with auto-connection
 * - Ground tileset behavior with border handling
 * - TileLocation flag handling based on tileset type
 * - Integration with map editing operations
 * - Behavior configuration and persistence
 * - Performance optimization for behavior checks
 * - 1:1 compatibility with wxwidgets tileset behaviors
 */

/**
 * @brief Door Tileset Configuration (palettedoor)
 */
struct DoorTilesetConfig {
    bool autoPlaceOnWalls;
    bool autoDetectDoorType;
    bool preserveOrientation;
    bool updateAdjacentWalls;
    bool lockByDefault;
    QMap<quint16, QString> doorTypeMapping; // item ID -> door type
    
    DoorTilesetConfig() 
        : autoPlaceOnWalls(true)
        , autoDetectDoorType(true)
        , preserveOrientation(true)
        , updateAdjacentWalls(true)
        , lockByDefault(false) {}
};

/**
 * @brief Wall Tileset Configuration
 */
struct WallTilesetConfig {
    bool autoConnect;
    bool updateBorders;
    bool calculateAlignment;
    bool handleCorners;
    bool supportDoors;
    QMap<quint16, QString> wallTypeMapping; // item ID -> wall type
    QSet<quint16> cornerItems;
    QSet<quint16> doorCompatibleItems;
    
    WallTilesetConfig()
        : autoConnect(true)
        , updateBorders(true)
        , calculateAlignment(true)
        , handleCorners(true)
        , supportDoors(true) {}
};

/**
 * @brief Ground Tileset Configuration
 */
struct GroundTilesetConfig {
    bool autoCalculateBorders;
    bool blendWithAdjacent;
    bool supportTransitions;
    bool handleElevation;
    QMap<quint16, int> groundPriority; // item ID -> priority
    QMap<quint16, QSet<quint16>> transitionItems; // base item -> transition items
    
    GroundTilesetConfig()
        : autoCalculateBorders(true)
        , blendWithAdjacent(true)
        , supportTransitions(true)
        , handleElevation(false) {}
};

/**
 * @brief Tileset Behavior Rule
 */
struct TilesetBehaviorRule {
    QString tilesetName;
    QString categoryName;
    TilesetBehavior::Type behaviorType;
    QVariantMap conditions;
    QVariantMap actions;
    int priority;
    bool enabled;
    
    TilesetBehaviorRule()
        : behaviorType(TilesetBehavior::NORMAL)
        , priority(0)
        , enabled(true) {}
};

/**
 * @brief Main Tileset Behavior Manager
 */
class TilesetBehaviorManager : public QObject
{
    Q_OBJECT

public:
    explicit TilesetBehaviorManager(QObject* parent = nullptr);
    ~TilesetBehaviorManager() override;

    // Initialization
    bool initialize(TilesetManager* tilesetManager);
    void shutdown();
    bool isInitialized() const { return initialized_; }

    // Behavior registration
    void registerTilesetBehavior(const QString& tilesetName, const TilesetBehavior& behavior);
    void unregisterTilesetBehavior(const QString& tilesetName);
    TilesetBehavior getTilesetBehavior(const QString& tilesetName) const;
    bool hasTilesetBehavior(const QString& tilesetName) const;

    // Special tileset configuration
    void configureDoorTileset(const QString& tilesetName, const DoorTilesetConfig& config);
    void configureWallTileset(const QString& tilesetName, const WallTilesetConfig& config);
    void configureGroundTileset(const QString& tilesetName, const GroundTilesetConfig& config);
    
    DoorTilesetConfig getDoorTilesetConfig(const QString& tilesetName) const;
    WallTilesetConfig getWallTilesetConfig(const QString& tilesetName) const;
    GroundTilesetConfig getGroundTilesetConfig(const QString& tilesetName) const;

    // Behavior queries
    bool isDoorTileset(const QString& tilesetName) const;
    bool isWallTileset(const QString& tilesetName) const;
    bool isGroundTileset(const QString& tilesetName) const;
    bool isDecorationTileset(const QString& tilesetName) const;
    bool isCreatureTileset(const QString& tilesetName) const;

    // Item behavior queries
    bool isItemInDoorTileset(quint16 itemId) const;
    bool isItemInWallTileset(quint16 itemId) const;
    bool isItemInGroundTileset(quint16 itemId) const;
    QString getItemTilesetName(quint16 itemId) const;
    TilesetBehavior::Type getItemBehaviorType(quint16 itemId) const;

    // Map editing integration
    bool shouldApplyBehavior(const QString& tilesetName, Map* map, const QPoint& position) const;
    void applyTilesetBehavior(const QString& tilesetName, Map* map, const QPoint& position, quint16 itemId);
    void handleItemPlacement(Map* map, const QPoint& position, quint16 itemId);
    void handleItemRemoval(Map* map, const QPoint& position, quint16 itemId);

    // Door tileset specific methods (palettedoor)
    void handleDoorPlacement(Map* map, const QPoint& position, quint16 doorItemId);
    void handleDoorRemoval(Map* map, const QPoint& position, quint16 doorItemId);
    bool canPlaceDoorOnTile(Map* map, const QPoint& position) const;
    quint16 getOptimalDoorItem(Map* map, const QPoint& position, const QString& doorType) const;

    // Wall tileset specific methods
    void handleWallPlacement(Map* map, const QPoint& position, quint16 wallItemId);
    void handleWallRemoval(Map* map, const QPoint& position, quint16 wallItemId);
    void updateWallConnections(Map* map, const QPoint& position);
    void calculateWallAlignment(Map* map, const QPoint& position);

    // Ground tileset specific methods
    void handleGroundPlacement(Map* map, const QPoint& position, quint16 groundItemId);
    void handleGroundRemoval(Map* map, const QPoint& position, quint16 groundItemId);
    void updateGroundBorders(Map* map, const QPoint& position);
    void calculateGroundTransitions(Map* map, const QPoint& position);

    // Rule management
    void addBehaviorRule(const TilesetBehaviorRule& rule);
    void removeBehaviorRule(const QString& ruleName);
    void updateBehaviorRule(const QString& ruleName, const TilesetBehaviorRule& rule);
    QList<TilesetBehaviorRule> getBehaviorRules(const QString& tilesetName) const;
    QList<TilesetBehaviorRule> getAllBehaviorRules() const;

    // Configuration persistence
    bool loadConfiguration(const QString& filePath);
    bool saveConfiguration(const QString& filePath) const;
    void resetToDefaults();

    // Performance and statistics
    QVariantMap getStatistics() const;
    void resetStatistics();
    void optimizePerformance();

public slots:
    void onTilesetAdded(const QString& tilesetName);
    void onTilesetRemoved(const QString& tilesetName);
    void onTilesetModified(const QString& tilesetName);
    void onItemAddedToTileset(const QString& tilesetName, const QString& categoryName, quint16 itemId);
    void onItemRemovedFromTileset(const QString& tilesetName, const QString& categoryName, quint16 itemId);

signals:
    void behaviorRegistered(const QString& tilesetName, TilesetBehavior::Type type);
    void behaviorUnregistered(const QString& tilesetName);
    void behaviorApplied(const QString& tilesetName, const QPoint& position, quint16 itemId);
    void doorPlaced(const QPoint& position, quint16 doorItemId);
    void wallConnected(const QPoint& position, quint16 wallItemId);
    void groundBorderUpdated(const QPoint& position, quint16 groundItemId);

private:
    // Internal behavior application
    void applyDoorBehavior(Map* map, const QPoint& position, quint16 itemId);
    void applyWallBehavior(Map* map, const QPoint& position, quint16 itemId);
    void applyGroundBehavior(Map* map, const QPoint& position, quint16 itemId);
    void applyDecorationBehavior(Map* map, const QPoint& position, quint16 itemId);
    void applyCreatureBehavior(Map* map, const QPoint& position, quint16 itemId);

    // Helper methods
    void buildItemTilesetCache();
    void updateItemTilesetCache(const QString& tilesetName);
    void clearItemTilesetCache();
    bool evaluateConditions(const QVariantMap& conditions, Map* map, const QPoint& position) const;
    void executeActions(const QVariantMap& actions, Map* map, const QPoint& position, quint16 itemId);

    // TileLocation flag handling
    void updateTileLocationFlags(Map* map, const QPoint& position, const TilesetBehavior& behavior);
    void setTileLocationFlag(Map* map, const QPoint& position, const QString& flagName, bool value);
    bool getTileLocationFlag(Map* map, const QPoint& position, const QString& flagName) const;

    // Performance optimization
    void optimizeBehaviorLookup();
    void cacheFrequentlyUsedBehaviors();
    void cleanupUnusedCache();

private:
    // Core components
    TilesetManager* tilesetManager_;
    bool initialized_;

    // Behavior storage
    QMap<QString, TilesetBehavior> tilesetBehaviors_;
    QMap<QString, DoorTilesetConfig> doorConfigs_;
    QMap<QString, WallTilesetConfig> wallConfigs_;
    QMap<QString, GroundTilesetConfig> groundConfigs_;
    QList<TilesetBehaviorRule> behaviorRules_;

    // Performance caches
    mutable QMutex cacheMutex_;
    QMap<quint16, QString> itemTilesetCache_; // item ID -> tileset name
    QMap<quint16, TilesetBehavior::Type> itemBehaviorCache_; // item ID -> behavior type
    QSet<QString> doorTilesets_;
    QSet<QString> wallTilesets_;
    QSet<QString> groundTilesets_;
    QSet<QString> decorationTilesets_;
    QSet<QString> creatureTilesets_;

    // Statistics
    mutable QMutex statsMutex_;
    QVariantMap statistics_;
    QElapsedTimer performanceTimer_;
    int behaviorsApplied_;
    int cacheHits_;
    int cacheMisses_;

    // Configuration
    QString configurationFile_;
    QTimer* cacheCleanupTimer_;

    // Constants
    static const int CACHE_CLEANUP_INTERVAL = 300000; // 5 minutes
    static const int MAX_CACHE_SIZE = 10000;
};

/**
 * @brief Tileset Behavior Factory for creating common behaviors
 */
class TilesetBehaviorFactory : public QObject
{
    Q_OBJECT

public:
    static TilesetBehavior createDoorTilesetBehavior();
    static TilesetBehavior createWallTilesetBehavior();
    static TilesetBehavior createGroundTilesetBehavior();
    static TilesetBehavior createDecorationTilesetBehavior();
    static TilesetBehavior createCreatureTilesetBehavior();

    static DoorTilesetConfig createDefaultDoorConfig();
    static WallTilesetConfig createDefaultWallConfig();
    static GroundTilesetConfig createDefaultGroundConfig();

    static TilesetBehaviorRule createDoorPlacementRule(const QString& tilesetName);
    static TilesetBehaviorRule createWallConnectionRule(const QString& tilesetName);
    static TilesetBehaviorRule createGroundBorderRule(const QString& tilesetName);
};

/**
 * @brief Tileset Behavior Validator for configuration validation
 */
class TilesetBehaviorValidator : public QObject
{
    Q_OBJECT

public:
    static bool validateTilesetBehavior(const TilesetBehavior& behavior, QStringList& errors);
    static bool validateDoorConfig(const DoorTilesetConfig& config, QStringList& errors);
    static bool validateWallConfig(const WallTilesetConfig& config, QStringList& errors);
    static bool validateGroundConfig(const GroundTilesetConfig& config, QStringList& errors);
    static bool validateBehaviorRule(const TilesetBehaviorRule& rule, QStringList& errors);

    static QStringList getRecommendations(const TilesetBehavior& behavior);
    static QStringList optimizeBehavior(TilesetBehavior& behavior);
};

#endif // TILESETBEHAVIORMANAGER_H
