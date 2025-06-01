#ifndef MAPVERSIONCONVERTER_H
#define MAPVERSIONCONVERTER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QHash>
#include <QList>
#include <QPair>
#include <QElapsedTimer>
#include "MapFormatManager.h"

// Forward declarations
class Map;
class Tile;
class Item;
class ItemManager;

/**
 * @brief Map Version Converter for Task 84
 * 
 * Complete implementation of map version conversion system:
 * - Support for all OTBM and client version conversions
 * - Item ID mapping and translation tables
 * - Attribute and property conversion logic
 * - Tile flag and state conversion
 * - Performance optimization and error handling
 * - Full compatibility with wxwidgets conversion system
 */

/**
 * @brief Conversion operation type
 */
enum class ConversionType {
    OTBM_VERSION_CONVERSION,    // Convert between OTBM versions
    CLIENT_VERSION_CONVERSION,  // Convert between client versions
    FORMAT_CONVERSION,          // Convert between different formats
    FULL_CONVERSION            // Convert both OTBM and client versions
};

/**
 * @brief Item ID mapping entry
 */
struct ItemIdMapping {
    quint16 sourceId = 0;
    quint16 targetId = 0;
    QString sourceName;
    QString targetName;
    QVariantMap attributeChanges;
    bool isValid = false;
    
    ItemIdMapping() = default;
    ItemIdMapping(quint16 src, quint16 tgt, const QString& srcName = QString(), const QString& tgtName = QString())
        : sourceId(src), targetId(tgt), sourceName(srcName), targetName(tgtName), isValid(true) {}
};

/**
 * @brief Conversion rule for complex transformations
 */
struct ConversionRule {
    QString name;
    QString description;
    ConversionType type;
    MapVersion::OTBMVersion sourceOTBMVersion;
    MapVersion::OTBMVersion targetOTBMVersion;
    MapVersion::ClientVersion sourceClientVersion;
    MapVersion::ClientVersion targetClientVersion;
    QList<ItemIdMapping> itemMappings;
    QVariantMap attributeMappings;
    QVariantMap tileFlagMappings;
    bool isReversible = false;
    
    ConversionRule() = default;
};

/**
 * @brief Conversion statistics
 */
struct ConversionStatistics {
    int totalTiles = 0;
    int totalItems = 0;
    int itemsConverted = 0;
    int itemsUnchanged = 0;
    int itemsRemoved = 0;
    int itemsAdded = 0;
    int tilesModified = 0;
    int attributesChanged = 0;
    int flagsChanged = 0;
    double conversionTime = 0.0;
    MapVersion sourceVersion;
    MapVersion targetVersion;
    QStringList warnings;
    QStringList errors;
    QHash<quint16, int> itemIdChangeCounts;
    
    void reset() {
        totalTiles = 0;
        totalItems = 0;
        itemsConverted = 0;
        itemsUnchanged = 0;
        itemsRemoved = 0;
        itemsAdded = 0;
        tilesModified = 0;
        attributesChanged = 0;
        flagsChanged = 0;
        conversionTime = 0.0;
        sourceVersion = MapVersion();
        targetVersion = MapVersion();
        warnings.clear();
        errors.clear();
        itemIdChangeCounts.clear();
    }
};

/**
 * @brief Main Map Version Converter class
 */
class MapVersionConverter : public QObject
{
    Q_OBJECT

public:
    explicit MapVersionConverter(QObject* parent = nullptr);
    ~MapVersionConverter() override;

    // Main conversion operations
    bool convertMap(Map* map, const MapVersion& targetVersion);
    bool convertMapOTBMVersion(Map* map, MapVersion::OTBMVersion targetVersion);
    bool convertMapClientVersion(Map* map, MapVersion::ClientVersion targetVersion);
    bool convertMapFormat(Map* map, MapFormat sourceFormat, MapFormat targetFormat);
    
    // Conversion path analysis
    QList<MapVersion> getConversionPath(const MapVersion& sourceVersion, const MapVersion& targetVersion) const;
    bool isConversionSupported(const MapVersion& sourceVersion, const MapVersion& targetVersion) const;
    bool isDirectConversionAvailable(const MapVersion& sourceVersion, const MapVersion& targetVersion) const;
    QList<MapVersion> getSupportedTargetVersions(const MapVersion& sourceVersion) const;
    
    // Item ID conversion
    bool convertItemIds(Map* map, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion);
    quint16 convertItemId(quint16 sourceId, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion) const;
    QList<ItemIdMapping> getItemIdMappings(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion) const;
    bool hasItemIdMapping(quint16 itemId, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion) const;
    
    // Attribute and property conversion
    bool convertItemAttributes(Map* map, const MapVersion& sourceVersion, const MapVersion& targetVersion);
    bool convertTileFlags(Map* map, MapVersion::OTBMVersion sourceVersion, MapVersion::OTBMVersion targetVersion);
    bool convertItemProperties(Item* item, const MapVersion& sourceVersion, const MapVersion& targetVersion);
    bool convertTileProperties(Tile* tile, const MapVersion& sourceVersion, const MapVersion& targetVersion);
    
    // Conversion rules management
    void addConversionRule(const ConversionRule& rule);
    void removeConversionRule(const QString& ruleName);
    ConversionRule getConversionRule(const QString& ruleName) const;
    QList<ConversionRule> getConversionRules() const { return conversionRules_; }
    QList<ConversionRule> getApplicableRules(const MapVersion& sourceVersion, const MapVersion& targetVersion) const;
    
    // Item ID mapping management
    void loadItemIdMappings(const QString& filePath);
    void saveItemIdMappings(const QString& filePath) const;
    void addItemIdMapping(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion, const ItemIdMapping& mapping);
    void removeItemIdMapping(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion, quint16 sourceId);
    void clearItemIdMappings();
    
    // Statistics and monitoring
    ConversionStatistics getConversionStatistics() const { return statistics_; }
    void resetStatistics();
    QStringList getLastWarnings() const { return statistics_.warnings; }
    QStringList getLastErrors() const { return statistics_.errors; }
    
    // Configuration
    void setItemManager(ItemManager* itemManager) { itemManager_ = itemManager; }
    void setProgressCallback(std::function<void(int, int, const QString&)> callback) { progressCallback_ = callback; }
    void setValidationEnabled(bool enabled) { validationEnabled_ = enabled; }
    void setBackupEnabled(bool enabled) { backupEnabled_ = enabled; }
    void setStrictMode(bool enabled) { strictMode_ = enabled; }
    
    ItemManager* getItemManager() const { return itemManager_; }
    bool isValidationEnabled() const { return validationEnabled_; }
    bool isBackupEnabled() const { return backupEnabled_; }
    bool isStrictMode() const { return strictMode_; }
    
    // Error handling
    QString getLastError() const { return lastError_; }
    bool hasError() const { return !lastError_.isEmpty(); }
    void clearError() { lastError_.clear(); }

signals:
    void conversionStarted(const MapVersion& sourceVersion, const MapVersion& targetVersion);
    void conversionProgress(int current, int total, const QString& operation);
    void conversionCompleted(const ConversionStatistics& stats);
    void conversionFailed(const QString& error);
    void warningOccurred(const QString& warning);
    void itemIdConverted(quint16 sourceId, quint16 targetId, const QString& itemName);

public slots:
    void onProgressUpdate(int current, int total, const QString& operation);

private:
    // Core conversion implementation
    bool performConversion(Map* map, const MapVersion& sourceVersion, const MapVersion& targetVersion);
    bool performOTBMVersionConversion(Map* map, MapVersion::OTBMVersion sourceVersion, MapVersion::OTBMVersion targetVersion);
    bool performClientVersionConversion(Map* map, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion);
    bool performMultiStepConversion(Map* map, const QList<MapVersion>& conversionPath);
    
    // Item conversion implementation
    bool convertMapItems(Map* map, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion);
    bool convertTileItems(Tile* tile, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion);
    bool convertSingleItem(Item* item, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion);
    
    // Attribute conversion implementation
    bool convertItemAttributesForVersion(Item* item, const MapVersion& sourceVersion, const MapVersion& targetVersion);
    bool convertTileFlagsForVersion(Tile* tile, MapVersion::OTBMVersion sourceVersion, MapVersion::OTBMVersion targetVersion);
    QVariantMap convertAttributeMap(const QVariantMap& sourceAttributes, const MapVersion& sourceVersion, const MapVersion& targetVersion);
    
    // Conversion rule application
    bool applyConversionRule(Map* map, const ConversionRule& rule);
    bool applyItemMappings(Map* map, const QList<ItemIdMapping>& mappings);
    bool applyAttributeMappings(Map* map, const QVariantMap& attributeMappings);
    bool applyTileFlagMappings(Map* map, const QVariantMap& tileFlagMappings);
    
    // Validation and error handling
    bool validateConversionInput(Map* map, const MapVersion& sourceVersion, const MapVersion& targetVersion);
    bool validateConversionResult(Map* map, const MapVersion& targetVersion);
    bool validateItemIdMapping(const ItemIdMapping& mapping) const;
    bool validateConversionRule(const ConversionRule& rule) const;
    
    void setError(const QString& error);
    void addWarning(const QString& warning);
    void updateProgress(int current, int total, const QString& operation);
    
    // Data loading and initialization
    void loadBuiltInConversionRules();
    void loadBuiltInItemIdMappings();
    void initializeVersionSupport();
    
    // Performance optimization
    void optimizeConversion(Map* map);
    void preloadItemMappings(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion);
    void cacheFrequentlyUsedMappings();
    
    // Statistics tracking
    void updateStatistics(const QString& operation, int count = 1);
    void trackItemIdChange(quint16 sourceId, quint16 targetId);
    void startConversionTimer();
    void stopConversionTimer();
    
    // Backup and recovery
    bool createBackup(Map* map);
    bool restoreFromBackup(Map* map);
    void clearBackup();

private:
    ItemManager* itemManager_;
    
    // Conversion data
    QList<ConversionRule> conversionRules_;
    QHash<QPair<MapVersion::ClientVersion, MapVersion::ClientVersion>, QHash<quint16, ItemIdMapping>> itemIdMappings_;
    QHash<MapVersion, QList<MapVersion>> supportedConversions_;
    
    // State tracking
    ConversionStatistics statistics_;
    QString lastError_;
    
    // Configuration
    bool validationEnabled_;
    bool backupEnabled_;
    bool strictMode_;
    std::function<void(int, int, const QString&)> progressCallback_;
    
    // Performance tracking
    QElapsedTimer conversionTimer_;
    int currentProgress_;
    int totalProgress_;
    
    // Caching for performance
    QHash<quint16, ItemIdMapping> mappingCache_;
    QHash<QString, ConversionRule> ruleCache_;
    
    // Backup data
    QByteArray mapBackup_;
    bool hasBackup_;
    
    // Constants
    static const int PROGRESS_UPDATE_INTERVAL;
    static const int MAX_CONVERSION_STEPS;
    static const QString BUILTIN_MAPPINGS_RESOURCE;
    static const QString BUILTIN_RULES_RESOURCE;
};

/**
 * @brief Item ID mapping table manager
 */
class ItemIdMappingManager : public QObject
{
    Q_OBJECT

public:
    explicit ItemIdMappingManager(QObject* parent = nullptr);
    ~ItemIdMappingManager() override;

    // Mapping table operations
    bool loadMappingTable(const QString& filePath);
    bool saveMappingTable(const QString& filePath) const;
    bool loadMappingTableFromResource(const QString& resourcePath);
    
    // Mapping queries
    ItemIdMapping getMapping(quint16 sourceId, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion) const;
    QList<ItemIdMapping> getAllMappings(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion) const;
    bool hasMapping(quint16 sourceId, MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion) const;
    
    // Mapping management
    void addMapping(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion, const ItemIdMapping& mapping);
    void removeMapping(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion, quint16 sourceId);
    void clearMappings();
    void clearMappingsForVersion(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion);
    
    // Statistics
    int getMappingCount() const;
    int getMappingCount(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion) const;
    QList<QPair<MapVersion::ClientVersion, MapVersion::ClientVersion>> getSupportedVersionPairs() const;

signals:
    void mappingAdded(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion, const ItemIdMapping& mapping);
    void mappingRemoved(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion, quint16 sourceId);
    void mappingsCleared();

private:
    void initializeBuiltInMappings();
    QString getMappingKey(MapVersion::ClientVersion sourceVersion, MapVersion::ClientVersion targetVersion) const;

private:
    QHash<QString, QHash<quint16, ItemIdMapping>> mappingTables_;
};

#endif // MAPVERSIONCONVERTER_H
