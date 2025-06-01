#ifndef OTBMVERSIONCONVERTER_H
#define OTBMVERSIONCONVERTER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>
#include "../OtbmTypes.h"

// Forward declarations
class Map;
class Item;
class Tile;

/**
 * @brief OtbmVersionConverter - Handles OTBM version conversion and client compatibility
 * 
 * This class provides comprehensive version conversion logic for OTBM files,
 * ensuring compatibility between different client versions and OTBM format versions.
 * It handles item ID conversions, attribute transformations, and format upgrades/downgrades.
 */
class OtbmVersionConverter : public QObject {
    Q_OBJECT

public:
    explicit OtbmVersionConverter(QObject* parent = nullptr);
    ~OtbmVersionConverter();

    // Version detection and validation
    static bool isValidOtbmVersion(quint32 majorVersion, quint32 minorVersion = 0, quint32 buildVersion = 0);
    static bool isValidClientVersion(quint32 clientVersion);
    static QString getVersionDescription(quint32 majorVersion, quint32 minorVersion, quint32 buildVersion);
    static QString getClientVersionDescription(quint32 clientVersion);

    // Version comparison utilities
    static bool isOtbmVersionNewer(quint32 major1, quint32 minor1, quint32 build1,
                                   quint32 major2, quint32 minor2, quint32 build2);
    static bool isClientVersionNewer(quint32 version1, quint32 version2);
    static bool isVersionCompatible(quint32 otbmMajor, quint32 otbmMinor, quint32 clientVersion);

    // Map-level version conversion
    bool convertMapVersion(Map* map, quint32 targetOtbmMajor, quint32 targetOtbmMinor = 0, 
                          quint32 targetClientVersion = CLIENT_VERSION_1300);
    bool upgradeMapToVersion(Map* map, quint32 targetOtbmMajor, quint32 targetOtbmMinor = 0);
    bool downgradeMapToVersion(Map* map, quint32 targetOtbmMajor, quint32 targetOtbmMinor = 0);

    // Item-level version conversion
    bool convertItemForVersion(Item* item, quint32 otbmMajor, quint32 otbmMinor, quint32 clientVersion);
    bool upgradeItemAttributes(Item* item, quint32 fromOtbmMajor, quint32 toOtbmMajor);
    bool downgradeItemAttributes(Item* item, quint32 fromOtbmMajor, quint32 toOtbmMajor);

    // Tile-level version conversion
    bool convertTileForVersion(Tile* tile, quint32 otbmMajor, quint32 otbmMinor, quint32 clientVersion);
    bool upgradeTileAttributes(Tile* tile, quint32 fromOtbmMajor, quint32 toOtbmMajor);
    bool downgradeTileAttributes(Tile* tile, quint32 fromOtbmMajor, quint32 toOtbmMajor);

    // Item ID conversion for client compatibility
    quint16 convertItemIdForClient(quint16 itemId, quint32 fromClientVersion, quint32 toClientVersion);
    QMap<quint16, quint16> getItemIdConversionMap(quint32 fromClientVersion, quint32 toClientVersion);
    bool hasItemIdChanged(quint16 itemId, quint32 fromClientVersion, quint32 toClientVersion);

    // Attribute conversion utilities
    QVariant convertAttributeForVersion(const QString& attributeName, const QVariant& value,
                                       quint32 fromOtbmMajor, quint32 toOtbmMajor);
    QMap<QString, QVariant> convertAttributeMapForVersion(const QMap<QString, QVariant>& attributes,
                                                         quint32 fromOtbmMajor, quint32 toOtbmMajor);

    // Feature availability checks
    bool supportsWaypoints(quint32 otbmMajor) const;
    bool supportsAttributeMap(quint32 otbmMajor) const;
    bool supportsHouseTiles(quint32 otbmMajor) const;
    bool supportsSpawns(quint32 otbmMajor) const;
    bool supportsTowns(quint32 otbmMajor) const;
    bool supportsCharges(quint32 clientVersion) const;
    bool supportsTier(quint32 clientVersion) const;
    bool supportsPodiumOutfit(quint32 clientVersion) const;

    // Conversion validation
    struct ConversionResult {
        bool success = false;
        QString errorMessage;
        QStringList warnings;
        int itemsConverted = 0;
        int tilesConverted = 0;
        int attributesConverted = 0;
        int featuresRemoved = 0;
        int featuresAdded = 0;
    };

    ConversionResult validateConversion(Map* map, quint32 targetOtbmMajor, quint32 targetOtbmMinor,
                                       quint32 targetClientVersion);
    ConversionResult performConversion(Map* map, quint32 targetOtbmMajor, quint32 targetOtbmMinor,
                                      quint32 targetClientVersion);

    // Version-specific serialization helpers
    bool shouldSerializeAttribute(const QString& attributeName, quint32 otbmMajor, quint32 clientVersion) const;
    bool shouldSerializeFeature(const QString& featureName, quint32 otbmMajor, quint32 clientVersion) const;
    QStringList getUnsupportedAttributes(quint32 otbmMajor, quint32 clientVersion) const;
    QStringList getUnsupportedFeatures(quint32 otbmMajor, quint32 clientVersion) const;

    // Error handling and logging
    void setVerboseLogging(bool enabled) { verboseLogging_ = enabled; }
    bool isVerboseLogging() const { return verboseLogging_; }
    QStringList getConversionLog() const { return conversionLog_; }
    void clearConversionLog() { conversionLog_.clear(); }

    // Static utility methods
    static OtbmVersionConverter* instance();
    static void setDefaultTargetVersions(quint32 otbmMajor, quint32 otbmMinor, quint32 clientVersion);
    static quint32 getDefaultOtbmMajorVersion() { return defaultOtbmMajor_; }
    static quint32 getDefaultOtbmMinorVersion() { return defaultOtbmMinor_; }
    static quint32 getDefaultClientVersion() { return defaultClientVersion_; }

signals:
    void conversionStarted(const QString& description);
    void conversionProgress(int percentage, const QString& currentOperation);
    void conversionCompleted(const ConversionResult& result);
    void conversionError(const QString& errorMessage);
    void conversionWarning(const QString& warningMessage);

private:
    // Internal conversion methods
    bool convertOtbmV1ToV2(Map* map);
    bool convertOtbmV2ToV3(Map* map);
    bool convertOtbmV3ToV4(Map* map);
    bool convertOtbmV4ToV3(Map* map);
    bool convertOtbmV3ToV2(Map* map);
    bool convertOtbmV2ToV1(Map* map);

    // Item conversion helpers
    bool convertItemV1ToV2(Item* item);
    bool convertItemV2ToV3(Item* item);
    bool convertItemV3ToV4(Item* item);
    bool convertItemV4ToV3(Item* item);
    bool convertItemV3ToV2(Item* item);
    bool convertItemV2ToV1(Item* item);

    // Client version specific conversions
    bool convertItemForClientVersion(Item* item, quint32 fromVersion, quint32 toVersion);
    bool convertTileForClientVersion(Tile* tile, quint32 fromVersion, quint32 toVersion);

    // Attribute handling
    void migrateAttributesToAttributeMap(Item* item);
    void migrateAttributeMapToAttributes(Item* item);
    bool isLegacyAttribute(const QString& attributeName) const;
    QString getLegacyAttributeName(const QString& modernName) const;
    QString getModernAttributeName(const QString& legacyName) const;

    // Validation helpers
    bool validateItemForVersion(Item* item, quint32 otbmMajor, quint32 clientVersion, QStringList& warnings);
    bool validateTileForVersion(Tile* tile, quint32 otbmMajor, quint32 clientVersion, QStringList& warnings);
    bool validateMapForVersion(Map* map, quint32 otbmMajor, quint32 clientVersion, QStringList& warnings);

    // Logging and error handling
    void logConversion(const QString& message);
    void logWarning(const QString& warning);
    void logError(const QString& error);

    // Member variables
    bool verboseLogging_ = false;
    QStringList conversionLog_;
    
    // Version mapping tables
    QMap<quint32, QMap<quint16, quint16>> itemIdConversionMaps_;
    QMap<QString, QString> attributeNameMappings_;
    QSet<QString> legacyAttributes_;
    QSet<QString> modernAttributes_;
    
    // Feature support matrices
    QMap<quint32, QSet<QString>> otbmFeatureSupport_;
    QMap<quint32, QSet<QString>> clientFeatureSupport_;
    
    // Static defaults
    static quint32 defaultOtbmMajor_;
    static quint32 defaultOtbmMinor_;
    static quint32 defaultClientVersion_;
    static OtbmVersionConverter* instance_;

    // Initialization
    void initializeConversionTables();
    void initializeFeatureSupport();
    void initializeAttributeMappings();
};

// Convenience macros for version checking
#define OTBM_SUPPORTS_WAYPOINTS(major) ((major) >= MAP_OTBM_3)
#define OTBM_SUPPORTS_ATTRIBUTE_MAP(major) ((major) >= MAP_OTBM_4)
#define CLIENT_SUPPORTS_CHARGES(version) ((version) >= CLIENT_VERSION_820)
#define CLIENT_SUPPORTS_TIER(version) ((version) >= CLIENT_VERSION_1057)
#define CLIENT_SUPPORTS_PODIUM(version) ((version) >= CLIENT_VERSION_1094)

#endif // OTBMVERSIONCONVERTER_H
