#ifndef MAPFORMATMANAGER_H
#define MAPFORMATMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QHash>
#include <QList>
#include <QFile>
#include <QDataStream>
#include <QMutex>

// Forward declarations
class Map;
class MapView;
class ItemManager;

/**
 * @brief Comprehensive Map Format Manager for Task 84
 * 
 * Complete implementation of map loaders and converters for Qt6 map editor:
 * - Full BinaryFile I/O replacement with Qt (QFile, QDataStream)
 * - Support for all known map formats (OTBM, OTMM, XML, JSON)
 * - Format identification and version detection
 * - Version conversion logic with full compatibility
 * - Full structure restoration and MapView integration
 * - 1:1 compatibility with wxwidgets map I/O system
 */

/**
 * @brief Map format enumeration (1:1 wxwidgets compatibility)
 */
enum class MapFormat {
    UNKNOWN = 0,
    OTBM = 1,           // OpenTibia Binary Map (primary format)
    OTMM = 2,           // OpenTibia Memory Map (legacy format)
    XML = 3,            // XML format for components
    JSON = 4,           // JSON format for modern serialization
    OTBM_COMPRESSED = 5 // Compressed OTBM format
};

/**
 * @brief Map version structure (1:1 wxwidgets compatibility)
 */
struct MapVersion {
    enum OTBMVersion {
        OTBM_VERSION_1 = 0,
        OTBM_VERSION_2 = 1,
        OTBM_VERSION_3 = 2,
        OTBM_VERSION_4 = 3
    };
    
    enum ClientVersion {
        CLIENT_VERSION_NONE = 0,
        CLIENT_VERSION_740 = 740,
        CLIENT_VERSION_750 = 750,
        CLIENT_VERSION_760 = 760,
        CLIENT_VERSION_770 = 770,
        CLIENT_VERSION_780 = 780,
        CLIENT_VERSION_790 = 790,
        CLIENT_VERSION_792 = 792,
        CLIENT_VERSION_800 = 800,
        CLIENT_VERSION_810 = 810,
        CLIENT_VERSION_811 = 811,
        CLIENT_VERSION_820 = 820,
        CLIENT_VERSION_830 = 830,
        CLIENT_VERSION_840 = 840,
        CLIENT_VERSION_841 = 841,
        CLIENT_VERSION_842 = 842,
        CLIENT_VERSION_850 = 850,
        CLIENT_VERSION_854 = 854,
        CLIENT_VERSION_860 = 860,
        CLIENT_VERSION_861 = 861,
        CLIENT_VERSION_862 = 862,
        CLIENT_VERSION_870 = 870,
        CLIENT_VERSION_871 = 871,
        CLIENT_VERSION_872 = 872,
        CLIENT_VERSION_873 = 873,
        CLIENT_VERSION_900 = 900,
        CLIENT_VERSION_910 = 910,
        CLIENT_VERSION_920 = 920,
        CLIENT_VERSION_940 = 940,
        CLIENT_VERSION_944 = 944,
        CLIENT_VERSION_953 = 953,
        CLIENT_VERSION_960 = 960,
        CLIENT_VERSION_961 = 961,
        CLIENT_VERSION_963 = 963,
        CLIENT_VERSION_970 = 970,
        CLIENT_VERSION_980 = 980,
        CLIENT_VERSION_981 = 981,
        CLIENT_VERSION_982 = 982,
        CLIENT_VERSION_983 = 983,
        CLIENT_VERSION_985 = 985,
        CLIENT_VERSION_986 = 986,
        CLIENT_VERSION_1010 = 1010,
        CLIENT_VERSION_1020 = 1020,
        CLIENT_VERSION_1021 = 1021,
        CLIENT_VERSION_1030 = 1030,
        CLIENT_VERSION_1031 = 1031,
        CLIENT_VERSION_1035 = 1035,
        CLIENT_VERSION_1036 = 1036,
        CLIENT_VERSION_1038 = 1038,
        CLIENT_VERSION_1057 = 1057,
        CLIENT_VERSION_1058 = 1058,
        CLIENT_VERSION_1059 = 1059,
        CLIENT_VERSION_1060 = 1060,
        CLIENT_VERSION_1061 = 1061,
        CLIENT_VERSION_1062 = 1062,
        CLIENT_VERSION_1063 = 1063,
        CLIENT_VERSION_1064 = 1064,
        CLIENT_VERSION_1092 = 1092,
        CLIENT_VERSION_1093 = 1093,
        CLIENT_VERSION_1094 = 1094,
        CLIENT_VERSION_1095 = 1095,
        CLIENT_VERSION_1096 = 1096,
        CLIENT_VERSION_1097 = 1097,
        CLIENT_VERSION_1098 = 1098,
        CLIENT_VERSION_1099 = 1099,
        CLIENT_VERSION_1100 = 1100,
        CLIENT_VERSION_1110 = 1110,
        CLIENT_VERSION_1132 = 1132,
        CLIENT_VERSION_1140 = 1140,
        CLIENT_VERSION_1150 = 1150,
        CLIENT_VERSION_1171 = 1171,
        CLIENT_VERSION_1180 = 1180,
        CLIENT_VERSION_1185 = 1185,
        CLIENT_VERSION_1200 = 1200,
        CLIENT_VERSION_1210 = 1210,
        CLIENT_VERSION_1215 = 1215,
        CLIENT_VERSION_1220 = 1220,
        CLIENT_VERSION_1240 = 1240,
        CLIENT_VERSION_1250 = 1250,
        CLIENT_VERSION_1260 = 1260,
        CLIENT_VERSION_1270 = 1270,
        CLIENT_VERSION_1280 = 1280,
        CLIENT_VERSION_1281 = 1281,
        CLIENT_VERSION_1300 = 1300
    };
    
    OTBMVersion otbm = OTBM_VERSION_1;
    ClientVersion client = CLIENT_VERSION_NONE;
    
    MapVersion() = default;
    MapVersion(OTBMVersion otbm, ClientVersion client) : otbm(otbm), client(client) {}
    
    bool operator==(const MapVersion& other) const {
        return otbm == other.otbm && client == other.client;
    }
    
    bool operator!=(const MapVersion& other) const {
        return !(*this == other);
    }
    
    QString toString() const;
    static MapVersion fromString(const QString& str);
    bool isValid() const { return otbm >= OTBM_VERSION_1 && client != CLIENT_VERSION_NONE; }
};

/**
 * @brief Format detection result
 */
struct FormatDetectionResult {
    MapFormat format = MapFormat::UNKNOWN;
    MapVersion version;
    QString description;
    bool isValid = false;
    QString errorMessage;
    QVariantMap metadata;
    
    FormatDetectionResult() = default;
    FormatDetectionResult(MapFormat fmt, const MapVersion& ver, const QString& desc)
        : format(fmt), version(ver), description(desc), isValid(true) {}
};

/**
 * @brief Map loading statistics
 */
struct MapLoadingStatistics {
    int totalTiles = 0;
    int totalItems = 0;
    int totalCreatures = 0;
    int totalSpawns = 0;
    int totalHouses = 0;
    int totalTowns = 0;
    int totalWaypoints = 0;
    double loadingTime = 0.0;
    QString formatUsed;
    MapVersion versionLoaded;
    QStringList warnings;
    QStringList errors;
    
    void reset() {
        totalTiles = 0;
        totalItems = 0;
        totalCreatures = 0;
        totalSpawns = 0;
        totalHouses = 0;
        totalTowns = 0;
        totalWaypoints = 0;
        loadingTime = 0.0;
        formatUsed.clear();
        versionLoaded = MapVersion();
        warnings.clear();
        errors.clear();
    }
};

/**
 * @brief Main Map Format Manager class
 */
class MapFormatManager : public QObject
{
    Q_OBJECT

public:
    explicit MapFormatManager(QObject* parent = nullptr);
    ~MapFormatManager() override;

    // Format detection and identification
    FormatDetectionResult detectFormat(const QString& filePath) const;
    FormatDetectionResult detectFormatFromHeader(const QByteArray& header) const;
    FormatDetectionResult detectFormatFromExtension(const QString& filePath) const;
    MapFormat getFormatFromExtension(const QString& extension) const;
    QString getFormatName(MapFormat format) const;
    QStringList getSupportedExtensions() const;
    QStringList getSupportedFormats() const;
    
    // Version detection and validation
    MapVersion detectVersion(const QString& filePath) const;
    MapVersion detectVersionFromOTBM(QDataStream& stream) const;
    MapVersion detectVersionFromOTMM(QDataStream& stream) const;
    bool isVersionSupported(const MapVersion& version) const;
    QList<MapVersion> getSupportedVersions() const;
    
    // Map loading operations
    bool loadMap(Map* map, const QString& filePath);
    bool loadMapByFormat(Map* map, const QString& filePath, MapFormat format);
    bool loadMapWithVersion(Map* map, const QString& filePath, const MapVersion& targetVersion);
    
    // Format-specific loading methods
    bool loadOTBMMap(Map* map, const QString& filePath);
    bool loadOTMMMap(Map* map, const QString& filePath);
    bool loadXMLMap(Map* map, const QString& filePath);
    bool loadJSONMap(Map* map, const QString& filePath);
    bool loadCompressedOTBMMap(Map* map, const QString& filePath);
    
    // Map saving operations
    bool saveMap(Map* map, const QString& filePath) const;
    bool saveMapByFormat(Map* map, const QString& filePath, MapFormat format) const;
    bool saveMapWithVersion(Map* map, const QString& filePath, const MapVersion& targetVersion) const;
    
    // Format-specific saving methods
    bool saveOTBMMap(Map* map, const QString& filePath) const;
    bool saveOTMMMap(Map* map, const QString& filePath) const;
    bool saveXMLMap(Map* map, const QString& filePath) const;
    bool saveJSONMap(Map* map, const QString& filePath) const;
    bool saveCompressedOTBMMap(Map* map, const QString& filePath) const;
    
    // Version conversion operations
    bool convertMapVersion(Map* map, const MapVersion& fromVersion, const MapVersion& toVersion);
    bool convertOTBMVersion(Map* map, MapVersion::OTBMVersion fromVersion, MapVersion::OTBMVersion toVersion);
    bool convertClientVersion(Map* map, MapVersion::ClientVersion fromVersion, MapVersion::ClientVersion toVersion);
    QList<MapVersion> getConversionPath(const MapVersion& fromVersion, const MapVersion& toVersion) const;
    
    // Component loading/saving (spawns, houses, waypoints)
    bool loadSpawns(Map* map, const QString& filePath);
    bool saveSpawns(Map* map, const QString& filePath) const;
    bool loadHouses(Map* map, const QString& filePath);
    bool saveHouses(Map* map, const QString& filePath) const;
    bool loadWaypoints(Map* map, const QString& filePath);
    bool saveWaypoints(Map* map, const QString& filePath) const;
    
    // Statistics and monitoring
    MapLoadingStatistics getLastLoadingStatistics() const { return lastLoadingStats_; }
    void resetStatistics();
    QStringList getLastWarnings() const { return lastLoadingStats_.warnings; }
    QStringList getLastErrors() const { return lastLoadingStats_.errors; }
    
    // Integration with managers
    void setItemManager(ItemManager* itemManager) { itemManager_ = itemManager; }
    void setMapView(MapView* mapView) { mapView_ = mapView; }
    ItemManager* getItemManager() const { return itemManager_; }
    MapView* getMapView() const { return mapView_; }
    
    // Utility methods
    QString getLastError() const { return lastError_; }
    bool hasError() const { return !lastError_.isEmpty(); }
    void clearError() { lastError_.clear(); }
    QString getFormatDescription(MapFormat format) const;
    QVariantMap getFormatCapabilities(MapFormat format) const;

signals:
    void loadingStarted(const QString& filePath, MapFormat format);
    void loadingProgress(int current, int total, const QString& currentOperation);
    void loadingCompleted(const QString& filePath, const MapLoadingStatistics& stats);
    void loadingFailed(const QString& filePath, const QString& error);
    
    void savingStarted(const QString& filePath, MapFormat format);
    void savingProgress(int current, int total, const QString& currentOperation);
    void savingCompleted(const QString& filePath);
    void savingFailed(const QString& filePath, const QString& error);
    
    void conversionStarted(const MapVersion& fromVersion, const MapVersion& toVersion);
    void conversionProgress(int current, int total, const QString& currentOperation);
    void conversionCompleted(const MapVersion& fromVersion, const MapVersion& toVersion);
    void conversionFailed(const QString& error);

public slots:
    void onMapChanged();
    void onItemManagerChanged();

private:
    // Format detection implementation
    FormatDetectionResult detectOTBMFormat(const QString& filePath) const;
    FormatDetectionResult detectOTMMFormat(const QString& filePath) const;
    FormatDetectionResult detectXMLFormat(const QString& filePath) const;
    FormatDetectionResult detectJSONFormat(const QString& filePath) const;
    
    // Version detection implementation
    MapVersion readOTBMVersion(QDataStream& stream) const;
    MapVersion readOTMMVersion(QDataStream& stream) const;
    
    // Loading implementation helpers
    bool loadMapStructure(Map* map, QDataStream& stream, MapFormat format, const MapVersion& version);
    bool loadTileData(Map* map, QDataStream& stream, const MapVersion& version);
    bool loadItemData(Map* map, QDataStream& stream, const MapVersion& version);
    bool loadSpawnData(Map* map, QDataStream& stream, const MapVersion& version);
    bool loadHouseData(Map* map, QDataStream& stream, const MapVersion& version);
    bool loadWaypointData(Map* map, QDataStream& stream, const MapVersion& version);
    
    // Saving implementation helpers
    bool saveMapStructure(Map* map, QDataStream& stream, MapFormat format, const MapVersion& version) const;
    bool saveTileData(Map* map, QDataStream& stream, const MapVersion& version) const;
    bool saveItemData(Map* map, QDataStream& stream, const MapVersion& version) const;
    bool saveSpawnData(Map* map, QDataStream& stream, const MapVersion& version) const;
    bool saveHouseData(Map* map, QDataStream& stream, const MapVersion& version) const;
    bool saveWaypointData(Map* map, QDataStream& stream, const MapVersion& version) const;
    
    // Version conversion implementation
    bool performVersionConversion(Map* map, const MapVersion& fromVersion, const MapVersion& toVersion);
    bool convertItemIds(Map* map, MapVersion::ClientVersion fromVersion, MapVersion::ClientVersion toVersion);
    bool convertTileFlags(Map* map, MapVersion::OTBMVersion fromVersion, MapVersion::OTBMVersion toVersion);
    bool convertAttributes(Map* map, const MapVersion& fromVersion, const MapVersion& toVersion);
    
    // Error handling and validation
    void setError(const QString& error);
    void addWarning(const QString& warning);
    bool validateMapStructure(Map* map) const;
    bool validateVersion(const MapVersion& version) const;
    
    // Statistics tracking
    void updateLoadingStatistics(const QString& operation, int processed);
    void startLoadingTimer();
    void stopLoadingTimer();

private:
    ItemManager* itemManager_;
    MapView* mapView_;
    
    // State tracking
    mutable QString lastError_;
    mutable MapLoadingStatistics lastLoadingStats_;
    
    // Format support mapping
    QHash<QString, MapFormat> extensionToFormat_;
    QHash<MapFormat, QString> formatToName_;
    QHash<MapFormat, QStringList> formatToExtensions_;
    QHash<MapFormat, QVariantMap> formatCapabilities_;
    
    // Version support
    QList<MapVersion> supportedVersions_;
    QHash<MapVersion::ClientVersion, QHash<quint16, quint16>> itemIdConversionTables_;
    
    // Performance monitoring
    mutable QElapsedTimer loadingTimer_;
    mutable QMutex operationMutex_;
    
    // Constants
    static const QStringList OTBM_MAGIC_NUMBERS;
    static const QStringList OTMM_MAGIC_NUMBERS;
    static const int MAX_HEADER_SIZE;
    static const int DEFAULT_BUFFER_SIZE;
};

#endif // MAPFORMATMANAGER_H
