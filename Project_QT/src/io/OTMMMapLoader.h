#ifndef OTMMMAPLOADER_H
#define OTMMMAPLOADER_H

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
 * @brief OTMM Map Loader for Task 84
 * 
 * Complete implementation of OTMM (OpenTibia Memory Map) format loader:
 * - 1:1 compatibility with wxwidgets OTMM loader
 * - Support for OTMM version 1 (legacy format)
 * - Complete structure restoration with all components
 * - Performance optimization and error handling
 * - Integration with Qt binary file system
 * - Full MapView integration and progress tracking
 */

/**
 * @brief OTMM version enumeration (1:1 wxwidgets compatibility)
 */
enum class OTMMVersion : quint32 {
    OTMM_VERSION_1 = 0
};

/**
 * @brief OTMM node types (1:1 wxwidgets compatibility)
 */
enum class OTMMNodeType : quint8 {
    OTMM_ROOT = 1,
    OTMM_MAP_DATA = 2,
    OTMM_TILE_DATA = 3,
    OTMM_TILE = 4,
    OTMM_HOUSETILE = 5,
    OTMM_ITEM = 6,
    OTMM_SPAWN_DATA = 7,
    OTMM_SPAWN_AREA = 8,
    OTMM_MONSTER = 9,
    OTMM_NPC = 10,
    OTMM_TOWN_DATA = 11,
    OTMM_TOWN = 12,
    OTMM_HOUSE_DATA = 13,
    OTMM_HOUSE = 14,
    OTMM_DESCRIPTION = 15,
    OTMM_EDITOR = 16
};

/**
 * @brief OTMM tile attributes (1:1 wxwidgets compatibility)
 */
enum class OTMMTileAttribute : quint8 {
    OTMM_ATTR_TILE_FLAGS = 1
};

/**
 * @brief OTMM item attributes (1:1 wxwidgets compatibility)
 */
enum class OTMMItemAttribute : quint8 {
    OTMM_ATTR_DESCRIPTION = 1,
    OTMM_ATTR_ACTION_ID = 2,
    OTMM_ATTR_UNIQUE_ID = 3,
    OTMM_ATTR_TEXT = 4,
    OTMM_ATTR_DESC = 5,
    OTMM_ATTR_TELE_DEST = 6,
    OTMM_ATTR_ITEM = 7,
    OTMM_ATTR_SUBTYPE = 8,
    OTMM_ATTR_DEPOT_ID = 9,
    OTMM_ATTR_DOOR_ID = 10,
    OTMM_ATTR_DURATION = 11
};

/**
 * @brief OTMM loading statistics
 */
struct OTMMLoadingStatistics {
    int totalTiles = 0;
    int totalItems = 0;
    int totalSpawns = 0;
    int totalMonsters = 0;
    int totalNPCs = 0;
    int totalHouses = 0;
    int totalTowns = 0;
    int totalHouseTiles = 0;
    double loadingTime = 0.0;
    OTMMVersion version = OTMMVersion::OTMM_VERSION_1;
    QString mapDescription;
    QString editorInfo;
    QStringList warnings;
    QStringList errors;
    
    void reset() {
        totalTiles = 0;
        totalItems = 0;
        totalSpawns = 0;
        totalMonsters = 0;
        totalNPCs = 0;
        totalHouses = 0;
        totalTowns = 0;
        totalHouseTiles = 0;
        loadingTime = 0.0;
        version = OTMMVersion::OTMM_VERSION_1;
        mapDescription.clear();
        editorInfo.clear();
        warnings.clear();
        errors.clear();
    }
};

/**
 * @brief Main OTMM Map Loader class
 */
class OTMMMapLoader : public QObject
{
    Q_OBJECT

public:
    explicit OTMMMapLoader(QObject* parent = nullptr);
    ~OTMMMapLoader() override;

    // Main loading operations
    bool loadMap(Map* map, const QString& filePath);
    bool loadMapFromHandle(Map* map, QtNodeFileReadHandle* handle);
    bool loadMapFromMemory(Map* map, const QByteArray& data);
    
    // Version detection and validation
    OTMMVersion detectVersion(const QString& filePath) const;
    OTMMVersion detectVersionFromHandle(QtNodeFileReadHandle* handle) const;
    bool isVersionSupported(OTMMVersion version) const;
    QList<OTMMVersion> getSupportedVersions() const;
    
    // Statistics and monitoring
    OTMMLoadingStatistics getLoadingStatistics() const { return statistics_; }
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
    void loadingCompleted(const OTMMLoadingStatistics& stats);
    void loadingFailed(const QString& error);
    void warningOccurred(const QString& warning);

public slots:
    void onProgressUpdate(int current, int total, const QString& operation);

private:
    // Core loading implementation
    bool loadMapStructure(Map* map, QtNodeFileReadHandle* handle);
    bool loadRootNode(Map* map, QtBinaryNode* rootNode);
    bool loadMapDataNode(Map* map, QtBinaryNode* mapDataNode);
    bool loadTileDataNode(Map* map, QtBinaryNode* tileDataNode);
    bool loadTileNode(Map* map, QtBinaryNode* tileNode);
    bool loadHouseTileNode(Map* map, QtBinaryNode* houseTileNode);
    bool loadItemNode(Map* map, QtBinaryNode* itemNode, Tile* tile);
    bool loadSpawnDataNode(Map* map, QtBinaryNode* spawnDataNode);
    bool loadSpawnAreaNode(Map* map, QtBinaryNode* spawnAreaNode);
    bool loadMonsterNode(Map* map, QtBinaryNode* monsterNode, Spawn* spawn);
    bool loadNPCNode(Map* map, QtBinaryNode* npcNode, Spawn* spawn);
    bool loadTownDataNode(Map* map, QtBinaryNode* townDataNode);
    bool loadTownNode(Map* map, QtBinaryNode* townNode);
    bool loadHouseDataNode(Map* map, QtBinaryNode* houseDataNode);
    bool loadHouseNode(Map* map, QtBinaryNode* houseNode);
    bool loadDescriptionNode(Map* map, QtBinaryNode* descriptionNode);
    bool loadEditorNode(Map* map, QtBinaryNode* editorNode);
    
    // Attribute loading
    bool loadTileAttributes(Tile* tile, QtBinaryNode* node);
    bool loadItemAttributes(Item* item, QtBinaryNode* node);
    bool loadTileAttribute(Tile* tile, OTMMTileAttribute attribute, QtBinaryNode* node);
    bool loadItemAttribute(Item* item, OTMMItemAttribute attribute, QtBinaryNode* node);
    
    // Data parsing helpers
    bool parseMapHeader(QtBinaryNode* rootNode, Map* map);
    bool parseMapDescription(QtBinaryNode* descriptionNode, Map* map);
    bool parseEditorInfo(QtBinaryNode* editorNode, Map* map);
    bool parseTilePosition(QtBinaryNode* node, quint16& x, quint16& y, quint8& z);
    bool parseTileFlags(QtBinaryNode* node, quint32& flags);
    bool parseItemId(QtBinaryNode* node, quint16& itemId);
    bool parseItemSubtype(QtBinaryNode* node, quint8& subtype);
    bool parseActionId(QtBinaryNode* node, quint16& actionId);
    bool parseUniqueId(QtBinaryNode* node, quint16& uniqueId);
    bool parseText(QtBinaryNode* node, QString& text);
    bool parseDescription(QtBinaryNode* node, QString& description);
    bool parseTeleportDestination(QtBinaryNode* node, quint16& x, quint16& y, quint8& z);
    bool parseDepotId(QtBinaryNode* node, quint16& depotId);
    bool parseDoorId(QtBinaryNode* node, quint8& doorId);
    bool parseDuration(QtBinaryNode* node, quint32& duration);
    
    // Validation and error handling
    bool validateMapStructure(Map* map) const;
    bool validateTileData(Tile* tile) const;
    bool validateItemData(Item* item) const;
    bool validateNodeType(QtBinaryNode* node, OTMMNodeType expectedType) const;
    bool validateNodeSize(QtBinaryNode* node, qint64 minSize, qint64 maxSize = -1) const;
    
    void setError(const QString& error);
    void addWarning(const QString& warning);
    void updateProgress(int current, int total, const QString& operation);
    
    // Performance optimization
    void optimizeMapLoading(Map* map);
    void preloadTileData(Map* map, QtBinaryNode* mapDataNode);
    void cacheFrequentlyUsedItems(Map* map);
    
    // Statistics tracking
    void updateStatistics(const QString& operation, int count = 1);
    void startLoadingTimer();
    void stopLoadingTimer();

private:
    OTMMLoadingStatistics statistics_;
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
    static const QStringList OTMM_IDENTIFIERS;
    static const quint32 OTMM_SIGNATURE;
    static const int MAX_MAP_WIDTH;
    static const int MAX_MAP_HEIGHT;
    static const int MAX_MAP_LAYERS;
    static const int PROGRESS_UPDATE_INTERVAL;
};

/**
 * @brief OTMM Map Saver for saving maps in OTMM format
 */
class OTMMMapSaver : public QObject
{
    Q_OBJECT

public:
    explicit OTMMMapSaver(QObject* parent = nullptr);
    ~OTMMMapSaver() override;

    // Main saving operations
    bool saveMap(Map* map, const QString& filePath, OTMMVersion version = OTMMVersion::OTMM_VERSION_1);
    bool saveMapToHandle(Map* map, QtNodeFileWriteHandle* handle, OTMMVersion version = OTMMVersion::OTMM_VERSION_1);
    bool saveMapToMemory(Map* map, QByteArray& data, OTMMVersion version = OTMMVersion::OTMM_VERSION_1);
    
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
    bool saveMapStructure(Map* map, QtNodeFileWriteHandle* handle, OTMMVersion version);
    bool saveRootNode(Map* map, QtNodeFileWriteHandle* handle, OTMMVersion version);
    bool saveMapDataNode(Map* map, QtNodeFileWriteHandle* handle);
    bool saveTileDataNode(Map* map, QtNodeFileWriteHandle* handle);
    bool saveTileNode(Tile* tile, QtNodeFileWriteHandle* handle);
    bool saveHouseTileNode(Tile* tile, QtNodeFileWriteHandle* handle);
    bool saveItemNode(Item* item, QtNodeFileWriteHandle* handle);
    bool saveSpawnDataNode(Map* map, QtNodeFileWriteHandle* handle);
    bool saveTownDataNode(Map* map, QtNodeFileWriteHandle* handle);
    bool saveHouseDataNode(Map* map, QtNodeFileWriteHandle* handle);
    bool saveDescriptionNode(Map* map, QtNodeFileWriteHandle* handle);
    bool saveEditorNode(Map* map, QtNodeFileWriteHandle* handle);
    
    // Attribute saving
    bool saveTileAttributes(Tile* tile, QtNodeFileWriteHandle* handle);
    bool saveItemAttributes(Item* item, QtNodeFileWriteHandle* handle);
    
    // Optimization
    void optimizeMapSaving(Map* map);
    bool shouldSkipEmptyTile(Tile* tile) const;

private:
    bool compressionEnabled_;
    bool optimizationEnabled_;
    QString lastError_;
};

/**
 * @brief OTMM to OTBM converter for format migration
 */
class OTMMToOTBMConverter : public QObject
{
    Q_OBJECT

public:
    explicit OTMMToOTBMConverter(QObject* parent = nullptr);
    ~OTMMToOTBMConverter() override;

    // Conversion operations
    bool convertMap(const QString& otmmFilePath, const QString& otbmFilePath);
    bool convertMapInMemory(const QByteArray& otmmData, QByteArray& otbmData);
    
    // Configuration
    void setTargetOTBMVersion(OTBMVersion version) { targetOTBMVersion_ = version; }
    void setPreserveMetadata(bool preserve) { preserveMetadata_ = preserve; }
    OTBMVersion getTargetOTBMVersion() const { return targetOTBMVersion_; }
    bool isPreserveMetadata() const { return preserveMetadata_; }

signals:
    void conversionStarted(const QString& sourceFile, const QString& targetFile);
    void conversionProgress(int current, int total, const QString& operation);
    void conversionCompleted();
    void conversionFailed(const QString& error);

private:
    // Conversion implementation
    bool performConversion(Map* map);
    bool convertTileData(Map* map);
    bool convertItemData(Map* map);
    bool convertSpawnData(Map* map);
    bool convertHouseData(Map* map);
    bool convertTownData(Map* map);
    
    // Data mapping
    bool mapOTMMAttributesToOTBM(Item* item);
    bool mapOTMMTileFlagsToOTBM(Tile* tile);

private:
    OTBMVersion targetOTBMVersion_;
    bool preserveMetadata_;
    QString lastError_;
};

#endif // OTMMMAPLOADER_H
