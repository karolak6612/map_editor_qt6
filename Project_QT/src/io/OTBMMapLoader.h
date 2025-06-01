#ifndef OTBMMAPLOADER_H
#define OTBMMAPLOADER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QHash>
#include <QList>
#include <QElapsedTimer>
#include "QtBinaryFile.h"
#include "MapFormatManager.h"

// Forward declarations
class Map;
class Tile;
class Item;
class Spawn;
class House;
class Town;
class Waypoint;

/**
 * @brief OTBM Map Loader for Task 84
 * 
 * Complete implementation of OTBM (OpenTibia Binary Map) format loader:
 * - 1:1 compatibility with wxwidgets OTBM loader
 * - Support for all OTBM versions (1-4)
 * - Complete structure restoration with all components
 * - Performance optimization and error handling
 * - Integration with Qt binary file system
 * - Full MapView integration and progress tracking
 */

/**
 * @brief OTBM version enumeration (1:1 wxwidgets compatibility)
 */
enum class OTBMVersion : quint32 {
    OTBM_VERSION_1 = 0,
    OTBM_VERSION_2 = 1,
    OTBM_VERSION_3 = 2,
    OTBM_VERSION_4 = 3
};

/**
 * @brief OTBM node types (1:1 wxwidgets compatibility)
 */
enum class OTBMNodeType : quint8 {
    OTBM_ROOT_V1 = 1,
    OTBM_MAP_DATA = 2,
    OTBM_ITEM_DEF = 3,
    OTBM_TILE_AREA = 4,
    OTBM_TILE = 5,
    OTBM_ITEM = 6,
    OTBM_TILE_SQUARE = 7,
    OTBM_TILE_REF = 8,
    OTBM_SPAWNS = 9,
    OTBM_SPAWN_AREA = 10,
    OTBM_MONSTER = 11,
    OTBM_TOWNS = 12,
    OTBM_TOWN = 13,
    OTBM_HOUSETILE = 14,
    OTBM_WAYPOINTS = 15,
    OTBM_WAYPOINT = 16,
    OTBM_ROOT_V2 = 17,
    OTBM_ROOT_V3 = 18,
    OTBM_ROOT_V4 = 19
};

/**
 * @brief OTBM tile attributes (1:1 wxwidgets compatibility)
 */
enum class OTBMTileAttribute : quint8 {
    OTBM_ATTR_TILE_FLAGS = 1,
    OTBM_ATTR_ITEM = 2,
    OTBM_ATTR_DEPOT_ID = 3,
    OTBM_ATTR_SPAWN_FILE = 4,
    OTBM_ATTR_RUNE_CHARGES = 5,
    OTBM_ATTR_HOUSE_FILE = 6,
    OTBM_ATTR_HOUSEDOORID = 7,
    OTBM_ATTR_COUNT = 8,
    OTBM_ATTR_DURATION = 9,
    OTBM_ATTR_DECAYING_STATE = 10,
    OTBM_ATTR_WRITTENDATE = 11,
    OTBM_ATTR_WRITTENBY = 12,
    OTBM_ATTR_SLEEPERGUID = 13,
    OTBM_ATTR_SLEEPSTART = 14,
    OTBM_ATTR_CHARGES = 15,
    OTBM_ATTR_CONTAINER_ITEMS = 16,
    OTBM_ATTR_NAME = 17,
    OTBM_ATTR_ARTICLE = 18,
    OTBM_ATTR_PLURALNAME = 19,
    OTBM_ATTR_WEIGHT = 20,
    OTBM_ATTR_ATTACK = 21,
    OTBM_ATTR_DEFENSE = 22,
    OTBM_ATTR_EXTRADEFENSE = 23,
    OTBM_ATTR_ARMOR = 24,
    OTBM_ATTR_ATTACKSPEED = 25,
    OTBM_ATTR_HITCHANCE = 26,
    OTBM_ATTR_SHOOTRANGE = 27,
    OTBM_ATTR_TEXT = 28,
    OTBM_ATTR_WRITTENDATE_EX = 29,
    OTBM_ATTR_WRITTENBY_EX = 30,
    OTBM_ATTR_DESCRIPTION = 31,
    OTBM_ATTR_TELE_DEST = 32,
    OTBM_ATTR_UNIQUE_ID = 33,
    OTBM_ATTR_ACTION_ID = 34
};

/**
 * @brief OTBM tile flags (1:1 wxwidgets compatibility)
 */
enum class OTBMTileFlags : quint32 {
    TILESTATE_NONE = 0,
    TILESTATE_PROTECTIONZONE = 1 << 0,
    TILESTATE_DEPRECATED_HOUSE = 1 << 1,
    TILESTATE_NOPVPZONE = 1 << 2,
    TILESTATE_NOLOGOUT = 1 << 3,
    TILESTATE_PVPZONE = 1 << 4,
    TILESTATE_REFRESH = 1 << 5,
    TILESTATE_HOUSE = 1 << 6,
    TILESTATE_BED = 1 << 7,
    TILESTATE_DEPOT = 1 << 8
};

/**
 * @brief OTBM loading statistics
 */
struct OTBMLoadingStatistics {
    int totalTiles = 0;
    int totalItems = 0;
    int totalSpawns = 0;
    int totalMonsters = 0;
    int totalHouses = 0;
    int totalTowns = 0;
    int totalWaypoints = 0;
    int totalTileAreas = 0;
    int totalHouseTiles = 0;
    double loadingTime = 0.0;
    OTBMVersion version = OTBMVersion::OTBM_VERSION_1;
    QString mapDescription;
    QStringList warnings;
    QStringList errors;
    
    void reset() {
        totalTiles = 0;
        totalItems = 0;
        totalSpawns = 0;
        totalMonsters = 0;
        totalHouses = 0;
        totalTowns = 0;
        totalWaypoints = 0;
        totalTileAreas = 0;
        totalHouseTiles = 0;
        loadingTime = 0.0;
        version = OTBMVersion::OTBM_VERSION_1;
        mapDescription.clear();
        warnings.clear();
        errors.clear();
    }
};

/**
 * @brief Main OTBM Map Loader class
 */
class OTBMMapLoader : public QObject
{
    Q_OBJECT

public:
    explicit OTBMMapLoader(QObject* parent = nullptr);
    ~OTBMMapLoader() override;

    // Main loading operations
    bool loadMap(Map* map, const QString& filePath);
    bool loadMapFromHandle(Map* map, QtNodeFileReadHandle* handle);
    bool loadMapFromMemory(Map* map, const QByteArray& data);
    
    // Version detection and validation
    OTBMVersion detectVersion(const QString& filePath) const;
    OTBMVersion detectVersionFromHandle(QtNodeFileReadHandle* handle) const;
    bool isVersionSupported(OTBMVersion version) const;
    QList<OTBMVersion> getSupportedVersions() const;
    
    // Component loading
    bool loadSpawns(Map* map, const QString& filePath);
    bool loadHouses(Map* map, const QString& filePath);
    bool loadWaypoints(Map* map, const QString& filePath);
    
    // Statistics and monitoring
    OTBMLoadingStatistics getLoadingStatistics() const { return statistics_; }
    void resetStatistics();
    QStringList getLastWarnings() const { return statistics_.warnings; }
    QStringList getLastErrors() const { return statistics_.errors; }
    
    // Configuration
    void setProgressCallback(std::function<void(int, int, const QString&)> callback) { progressCallback_ = callback; }
    void setValidationEnabled(bool enabled) { validationEnabled_ = enabled; }
    void setStrictMode(bool enabled) { strictMode_ = enabled; }
    bool isValidationEnabled() const { return validationEnabled_; }
    bool isStrictMode() const { return strictMode_; }
    
    // Error handling
    QString getLastError() const { return lastError_; }
    bool hasError() const { return !lastError_.isEmpty(); }
    void clearError() { lastError_.clear(); }

signals:
    void loadingStarted(const QString& filePath);
    void loadingProgress(int current, int total, const QString& operation);
    void loadingCompleted(const OTBMLoadingStatistics& stats);
    void loadingFailed(const QString& error);
    void warningOccurred(const QString& warning);

public slots:
    void onProgressUpdate(int current, int total, const QString& operation);

private:
    // Core loading implementation
    bool loadMapStructure(Map* map, QtNodeFileReadHandle* handle);
    bool loadRootNode(Map* map, QtBinaryNode* rootNode);
    bool loadMapDataNode(Map* map, QtBinaryNode* mapDataNode);
    bool loadTileAreaNode(Map* map, QtBinaryNode* tileAreaNode);
    bool loadTileNode(Map* map, QtBinaryNode* tileNode, quint16 baseX, quint16 baseY, quint8 floor);
    bool loadItemNode(Map* map, QtBinaryNode* itemNode, Tile* tile);
    bool loadSpawnsNode(Map* map, QtBinaryNode* spawnsNode);
    bool loadSpawnAreaNode(Map* map, QtBinaryNode* spawnAreaNode);
    bool loadMonsterNode(Map* map, QtBinaryNode* monsterNode, Spawn* spawn);
    bool loadTownsNode(Map* map, QtBinaryNode* townsNode);
    bool loadTownNode(Map* map, QtBinaryNode* townNode);
    bool loadHouseTileNode(Map* map, QtBinaryNode* houseTileNode, quint16 x, quint16 y, quint8 floor);
    bool loadWaypointsNode(Map* map, QtBinaryNode* waypointsNode);
    bool loadWaypointNode(Map* map, QtBinaryNode* waypointNode);
    
    // Attribute loading
    bool loadTileAttributes(Tile* tile, QtBinaryNode* node);
    bool loadItemAttributes(Item* item, QtBinaryNode* node);
    bool loadTileAttribute(Tile* tile, OTBMTileAttribute attribute, QtBinaryNode* node);
    bool loadItemAttribute(Item* item, OTBMTileAttribute attribute, QtBinaryNode* node);
    
    // Data parsing helpers
    bool parseMapHeader(QtBinaryNode* rootNode, Map* map);
    bool parseMapDescription(QtBinaryNode* mapDataNode, Map* map);
    bool parseSpawnFile(QtBinaryNode* node, QString& spawnFile);
    bool parseHouseFile(QtBinaryNode* node, QString& houseFile);
    bool parseTileFlags(QtBinaryNode* node, quint32& flags);
    bool parseItemId(QtBinaryNode* node, quint16& itemId);
    bool parseItemCount(QtBinaryNode* node, quint8& count);
    bool parseItemCharges(QtBinaryNode* node, quint16& charges);
    bool parseActionId(QtBinaryNode* node, quint16& actionId);
    bool parseUniqueId(QtBinaryNode* node, quint16& uniqueId);
    bool parseText(QtBinaryNode* node, QString& text);
    bool parseDescription(QtBinaryNode* node, QString& description);
    bool parseTeleportDestination(QtBinaryNode* node, quint16& x, quint16& y, quint8& z);
    
    // Version-specific loading
    bool loadMapV1(Map* map, QtBinaryNode* rootNode);
    bool loadMapV2(Map* map, QtBinaryNode* rootNode);
    bool loadMapV3(Map* map, QtBinaryNode* rootNode);
    bool loadMapV4(Map* map, QtBinaryNode* rootNode);
    
    // Validation and error handling
    bool validateMapStructure(Map* map) const;
    bool validateTileData(Tile* tile) const;
    bool validateItemData(Item* item) const;
    bool validateNodeType(QtBinaryNode* node, OTBMNodeType expectedType) const;
    bool validateNodeSize(QtBinaryNode* node, qint64 minSize, qint64 maxSize = -1) const;
    
    void setError(const QString& error);
    void addWarning(const QString& warning);
    void updateProgress(int current, int total, const QString& operation);
    
    // Performance optimization
    void optimizeMapLoading(Map* map);
    void preloadTileAreas(Map* map, QtBinaryNode* mapDataNode);
    void cacheFrequentlyUsedItems(Map* map);
    
    // Statistics tracking
    void updateStatistics(const QString& operation, int count = 1);
    void startLoadingTimer();
    void stopLoadingTimer();

private:
    OTBMLoadingStatistics statistics_;
    QString lastError_;
    
    // Configuration
    bool validationEnabled_;
    bool strictMode_;
    std::function<void(int, int, const QString&)> progressCallback_;
    
    // Performance tracking
    QElapsedTimer loadingTimer_;
    int currentProgress_;
    int totalProgress_;
    
    // Caching for performance
    QHash<quint16, Item*> itemCache_;
    QHash<QString, Spawn*> spawnCache_;
    QHash<quint32, House*> houseCache_;
    
    // Constants
    static const QStringList OTBM_IDENTIFIERS;
    static const quint32 OTBM_SIGNATURE;
    static const int MAX_MAP_WIDTH;
    static const int MAX_MAP_HEIGHT;
    static const int MAX_MAP_LAYERS;
    static const int PROGRESS_UPDATE_INTERVAL;
};

/**
 * @brief OTBM Map Saver for saving maps in OTBM format
 */
class OTBMMapSaver : public QObject
{
    Q_OBJECT

public:
    explicit OTBMMapSaver(QObject* parent = nullptr);
    ~OTBMMapSaver() override;

    // Main saving operations
    bool saveMap(Map* map, const QString& filePath, OTBMVersion version = OTBMVersion::OTBM_VERSION_4);
    bool saveMapToHandle(Map* map, QtNodeFileWriteHandle* handle, OTBMVersion version = OTBMVersion::OTBM_VERSION_4);
    bool saveMapToMemory(Map* map, QByteArray& data, OTBMVersion version = OTBMVersion::OTBM_VERSION_4);
    
    // Component saving
    bool saveSpawns(Map* map, const QString& filePath);
    bool saveHouses(Map* map, const QString& filePath);
    bool saveWaypoints(Map* map, const QString& filePath);
    
    // Configuration
    void setCompressionEnabled(bool enabled) { compressionEnabled_ = enabled; }
    void setOptimizationEnabled(bool enabled) { optimizationEnabled_ = enabled; }
    bool isCompressionEnabled() const { return compressionEnabled_; }
    bool isOptimizationEnabled() const { return optimizationEnabled_; }

signals:
    void savingStarted(const QString& filePath);
    void savingProgress(int current, int total, const QString& operation);
    void savingCompleted();
    void savingFailed(const QString& error);

private:
    // Core saving implementation
    bool saveMapStructure(Map* map, QtNodeFileWriteHandle* handle, OTBMVersion version);
    bool saveRootNode(Map* map, QtNodeFileWriteHandle* handle, OTBMVersion version);
    bool saveMapDataNode(Map* map, QtNodeFileWriteHandle* handle);
    bool saveTileAreaNodes(Map* map, QtNodeFileWriteHandle* handle);
    bool saveTileNode(Tile* tile, QtNodeFileWriteHandle* handle, quint16 x, quint16 y, quint8 z);
    bool saveItemNode(Item* item, QtNodeFileWriteHandle* handle);
    bool saveSpawnsNode(Map* map, QtNodeFileWriteHandle* handle);
    bool saveTownsNode(Map* map, QtNodeFileWriteHandle* handle);
    bool saveWaypointsNode(Map* map, QtNodeFileWriteHandle* handle);
    
    // Attribute saving
    bool saveTileAttributes(Tile* tile, QtNodeFileWriteHandle* handle);
    bool saveItemAttributes(Item* item, QtNodeFileWriteHandle* handle);
    
    // Optimization
    void optimizeMapSaving(Map* map);
    bool shouldSkipEmptyTile(Tile* tile) const;
    bool shouldCompressTileArea(const QList<Tile*>& tiles) const;

private:
    bool compressionEnabled_;
    bool optimizationEnabled_;
    QString lastError_;
};

#endif // OTBMMAPLOADER_H
