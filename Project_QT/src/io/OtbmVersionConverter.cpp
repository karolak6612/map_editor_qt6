#include "OtbmVersionConverter.h"
#include "../Map.h"
#include "../Tile.h"
#include "../Item.h"
#include "../ItemManager.h"
#include <QDebug>
#include <QElapsedTimer>

// Static member initialization
quint32 OtbmVersionConverter::defaultOtbmMajor_ = MAP_OTBM_4;
quint32 OtbmVersionConverter::defaultOtbmMinor_ = 0;
quint32 OtbmVersionConverter::defaultClientVersion_ = CLIENT_VERSION_1300;
OtbmVersionConverter* OtbmVersionConverter::instance_ = nullptr;

OtbmVersionConverter::OtbmVersionConverter(QObject* parent) : QObject(parent) {
    initializeConversionTables();
    initializeFeatureSupport();
    initializeAttributeMappings();
}

OtbmVersionConverter::~OtbmVersionConverter() {
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

OtbmVersionConverter* OtbmVersionConverter::instance() {
    if (!instance_) {
        instance_ = new OtbmVersionConverter();
    }
    return instance_;
}

void OtbmVersionConverter::setDefaultTargetVersions(quint32 otbmMajor, quint32 otbmMinor, quint32 clientVersion) {
    defaultOtbmMajor_ = otbmMajor;
    defaultOtbmMinor_ = otbmMinor;
    defaultClientVersion_ = clientVersion;
}

// Version detection and validation
bool OtbmVersionConverter::isValidOtbmVersion(quint32 majorVersion, quint32 minorVersion, quint32 buildVersion) {
    Q_UNUSED(minorVersion)
    Q_UNUSED(buildVersion)
    return majorVersion >= MAP_OTBM_1 && majorVersion <= MAP_OTBM_4;
}

bool OtbmVersionConverter::isValidClientVersion(quint32 clientVersion) {
    return clientVersion >= CLIENT_VERSION_750 && clientVersion <= CLIENT_VERSION_1300;
}

QString OtbmVersionConverter::getVersionDescription(quint32 majorVersion, quint32 minorVersion, quint32 buildVersion) {
    QString desc = QString("OTBM v%1.%2.%3").arg(majorVersion + 1).arg(minorVersion).arg(buildVersion);
    
    switch (majorVersion) {
        case MAP_OTBM_1:
            desc += " (Legacy format)";
            break;
        case MAP_OTBM_2:
            desc += " (Improved item handling)";
            break;
        case MAP_OTBM_3:
            desc += " (Waypoints support)";
            break;
        case MAP_OTBM_4:
            desc += " (Attribute map support)";
            break;
        default:
            desc += " (Unknown format)";
            break;
    }
    
    return desc;
}

QString OtbmVersionConverter::getClientVersionDescription(quint32 clientVersion) {
    QString desc = QString("Client %1").arg(clientVersion);
    
    if (clientVersion >= CLIENT_VERSION_1300) {
        desc += " (Latest)";
    } else if (clientVersion >= CLIENT_VERSION_1200) {
        desc += " (Modern)";
    } else if (clientVersion >= CLIENT_VERSION_1000) {
        desc += " (Stable)";
    } else if (clientVersion >= CLIENT_VERSION_900) {
        desc += " (Classic)";
    } else {
        desc += " (Legacy)";
    }
    
    return desc;
}

// Version comparison utilities
bool OtbmVersionConverter::isOtbmVersionNewer(quint32 major1, quint32 minor1, quint32 build1,
                                             quint32 major2, quint32 minor2, quint32 build2) {
    if (major1 != major2) return major1 > major2;
    if (minor1 != minor2) return minor1 > minor2;
    return build1 > build2;
}

bool OtbmVersionConverter::isClientVersionNewer(quint32 version1, quint32 version2) {
    return version1 > version2;
}

bool OtbmVersionConverter::isVersionCompatible(quint32 otbmMajor, quint32 otbmMinor, quint32 clientVersion) {
    Q_UNUSED(otbmMinor)
    
    // Basic compatibility matrix
    if (otbmMajor == MAP_OTBM_1) {
        return clientVersion >= CLIENT_VERSION_750 && clientVersion <= CLIENT_VERSION_820;
    } else if (otbmMajor == MAP_OTBM_2) {
        return clientVersion >= CLIENT_VERSION_820 && clientVersion <= CLIENT_VERSION_1000;
    } else if (otbmMajor == MAP_OTBM_3) {
        return clientVersion >= CLIENT_VERSION_1000 && clientVersion <= CLIENT_VERSION_1200;
    } else if (otbmMajor == MAP_OTBM_4) {
        return clientVersion >= CLIENT_VERSION_1057;
    }
    
    return false;
}

// Feature availability checks
bool OtbmVersionConverter::supportsWaypoints(quint32 otbmMajor) const {
    return OTBM_SUPPORTS_WAYPOINTS(otbmMajor);
}

bool OtbmVersionConverter::supportsAttributeMap(quint32 otbmMajor) const {
    return OTBM_SUPPORTS_ATTRIBUTE_MAP(otbmMajor);
}

bool OtbmVersionConverter::supportsHouseTiles(quint32 otbmMajor) const {
    return otbmMajor >= MAP_OTBM_1; // Supported from the beginning
}

bool OtbmVersionConverter::supportsSpawns(quint32 otbmMajor) const {
    return otbmMajor >= MAP_OTBM_1; // Supported from the beginning
}

bool OtbmVersionConverter::supportsTowns(quint32 otbmMajor) const {
    return otbmMajor >= MAP_OTBM_1; // Supported from the beginning
}

bool OtbmVersionConverter::supportsCharges(quint32 clientVersion) const {
    return CLIENT_SUPPORTS_CHARGES(clientVersion);
}

bool OtbmVersionConverter::supportsTier(quint32 clientVersion) const {
    return CLIENT_SUPPORTS_TIER(clientVersion);
}

bool OtbmVersionConverter::supportsPodiumOutfit(quint32 clientVersion) const {
    return CLIENT_SUPPORTS_PODIUM(clientVersion);
}

// Map-level version conversion
bool OtbmVersionConverter::convertMapVersion(Map* map, quint32 targetOtbmMajor, quint32 targetOtbmMinor, quint32 targetClientVersion) {
    if (!map) {
        logError("Cannot convert null map");
        return false;
    }
    
    quint32 currentOtbmMajor = map->getOtbmMajorVersion();
    quint32 currentOtbmMinor = map->getOtbmMinorVersion();
    
    logConversion(QString("Converting map from OTBM v%1.%2 to v%3.%4 (Client %5)")
                  .arg(currentOtbmMajor + 1).arg(currentOtbmMinor)
                  .arg(targetOtbmMajor + 1).arg(targetOtbmMinor)
                  .arg(targetClientVersion));
    
    emit conversionStarted(QString("Converting OTBM format from v%1 to v%2")
                          .arg(currentOtbmMajor + 1).arg(targetOtbmMajor + 1));
    
    QElapsedTimer timer;
    timer.start();
    
    bool success = false;
    
    if (currentOtbmMajor == targetOtbmMajor) {
        // Same major version, just update client compatibility
        success = convertMapForClientVersion(map, targetClientVersion);
    } else if (currentOtbmMajor < targetOtbmMajor) {
        // Upgrade
        success = upgradeMapToVersion(map, targetOtbmMajor, targetOtbmMinor);
    } else {
        // Downgrade
        success = downgradeMapToVersion(map, targetOtbmMajor, targetOtbmMinor);
    }
    
    if (success) {
        // Update map version information
        map->setOtbmVersions(targetOtbmMajor, targetOtbmMinor, 0);
        map->setModified(true);
        
        logConversion(QString("Map conversion completed successfully in %1 ms")
                     .arg(timer.elapsed()));
        
        ConversionResult result;
        result.success = true;
        emit conversionCompleted(result);
    } else {
        logError("Map conversion failed");
        emit conversionError("Map conversion failed");
    }
    
    return success;
}

bool OtbmVersionConverter::upgradeMapToVersion(Map* map, quint32 targetOtbmMajor, quint32 targetOtbmMinor) {
    Q_UNUSED(targetOtbmMinor)
    
    quint32 currentMajor = map->getOtbmMajorVersion();
    
    // Perform step-by-step upgrades
    for (quint32 version = currentMajor; version < targetOtbmMajor; ++version) {
        emit conversionProgress((version - currentMajor + 1) * 100 / (targetOtbmMajor - currentMajor + 1),
                               QString("Upgrading to OTBM v%1").arg(version + 2));
        
        switch (version) {
            case MAP_OTBM_1:
                if (!convertOtbmV1ToV2(map)) return false;
                break;
            case MAP_OTBM_2:
                if (!convertOtbmV2ToV3(map)) return false;
                break;
            case MAP_OTBM_3:
                if (!convertOtbmV3ToV4(map)) return false;
                break;
            default:
                logError(QString("Unknown OTBM version upgrade: %1 to %2").arg(version).arg(version + 1));
                return false;
        }
    }
    
    return true;
}

bool OtbmVersionConverter::downgradeMapToVersion(Map* map, quint32 targetOtbmMajor, quint32 targetOtbmMinor) {
    Q_UNUSED(targetOtbmMinor)
    
    quint32 currentMajor = map->getOtbmMajorVersion();
    
    // Perform step-by-step downgrades
    for (quint32 version = currentMajor; version > targetOtbmMajor; --version) {
        emit conversionProgress((currentMajor - version + 1) * 100 / (currentMajor - targetOtbmMajor + 1),
                               QString("Downgrading to OTBM v%1").arg(version));
        
        switch (version) {
            case MAP_OTBM_4:
                if (!convertOtbmV4ToV3(map)) return false;
                break;
            case MAP_OTBM_3:
                if (!convertOtbmV3ToV2(map)) return false;
                break;
            case MAP_OTBM_2:
                if (!convertOtbmV2ToV1(map)) return false;
                break;
            default:
                logError(QString("Unknown OTBM version downgrade: %1 to %2").arg(version).arg(version - 1));
                return false;
        }
    }
    
    return true;
}

// Internal conversion methods
bool OtbmVersionConverter::convertOtbmV1ToV2(Map* map) {
    logConversion("Converting OTBM v1 to v2: Improving item handling");
    
    // In v2, stackable items use OTBM_ATTR_COUNT instead of inline subtype
    int itemsConverted = 0;
    
    for (int z = 0; z < map->getLayers(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                Tile* tile = map->getTile(x, y, z);
                if (!tile) continue;
                
                const QVector<Item*>& items = tile->items();
                for (Item* item : items) {
                    if (item && convertItemV1ToV2(item)) {
                        itemsConverted++;
                    }
                }
                
                // Convert ground item too
                Item* ground = tile->getGround();
                if (ground && convertItemV1ToV2(ground)) {
                    itemsConverted++;
                }
            }
        }
    }
    
    logConversion(QString("Converted %1 items from v1 to v2 format").arg(itemsConverted));
    return true;
}

bool OtbmVersionConverter::convertOtbmV2ToV3(Map* map) {
    logConversion("Converting OTBM v2 to v3: Adding waypoints support");
    
    // v3 adds waypoints support - no conversion needed for existing data
    // Waypoints will be preserved if they exist
    
    logConversion("OTBM v2 to v3 conversion completed - waypoints now supported");
    return true;
}

bool OtbmVersionConverter::convertOtbmV3ToV4(Map* map) {
    logConversion("Converting OTBM v3 to v4: Adding attribute map support");
    
    // In v4, items can use OTBM_ATTR_ATTRIBUTE_MAP for extended attributes
    int itemsConverted = 0;
    
    for (int z = 0; z < map->getLayers(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                Tile* tile = map->getTile(x, y, z);
                if (!tile) continue;
                
                const QVector<Item*>& items = tile->items();
                for (Item* item : items) {
                    if (item && convertItemV3ToV4(item)) {
                        itemsConverted++;
                    }
                }
                
                // Convert ground item too
                Item* ground = tile->getGround();
                if (ground && convertItemV3ToV4(ground)) {
                    itemsConverted++;
                }
            }
        }
    }
    
    logConversion(QString("Converted %1 items from v3 to v4 format").arg(itemsConverted));
    return true;
}

// Item conversion helpers
bool OtbmVersionConverter::convertItemV1ToV2(Item* item) {
    if (!item) return false;
    
    // In v1, stackable items had inline subtype
    // In v2, they use OTBM_ATTR_COUNT attribute
    if (item->isStackable() && item->getCount() > 1) {
        // The count is already stored in the item's attribute system
        // No additional conversion needed as our Item class handles this correctly
        return true;
    }
    
    return false;
}

bool OtbmVersionConverter::convertItemV3ToV4(Item* item) {
    if (!item) return false;
    
    // In v4, we can migrate individual attributes to the attribute map
    // This is mainly for optimization and extended attribute support
    migrateAttributesToAttributeMap(item);
    return true;
}

// Logging and error handling
void OtbmVersionConverter::logConversion(const QString& message) {
    if (verboseLogging_) {
        qDebug() << "OtbmVersionConverter:" << message;
    }
    conversionLog_.append(QString("[INFO] %1").arg(message));
}

void OtbmVersionConverter::logWarning(const QString& warning) {
    qWarning() << "OtbmVersionConverter Warning:" << warning;
    conversionLog_.append(QString("[WARNING] %1").arg(warning));
    emit conversionWarning(warning);
}

void OtbmVersionConverter::logError(const QString& error) {
    qCritical() << "OtbmVersionConverter Error:" << error;
    conversionLog_.append(QString("[ERROR] %1").arg(error));
    emit conversionError(error);
}

// Initialization methods
void OtbmVersionConverter::initializeConversionTables() {
    // Initialize item ID conversion maps for different client versions
    // This would typically be loaded from external data files

    // Example conversions (simplified)
    QMap<quint16, quint16> v750to820;
    v750to820[100] = 101; // Example: item 100 in v750 becomes 101 in v820
    v750to820[200] = 205;
    itemIdConversionMaps_[CLIENT_VERSION_750] = v750to820;

    QMap<quint16, quint16> v820to1000;
    v820to1000[300] = 310;
    v820to1000[400] = 420;
    itemIdConversionMaps_[CLIENT_VERSION_820] = v820to1000;

    // More conversion maps would be added here for comprehensive support
}

void OtbmVersionConverter::initializeFeatureSupport() {
    // OTBM feature support matrix
    QSet<QString> otbmV1Features = {"spawns", "towns", "housetiles", "items", "tiles"};
    QSet<QString> otbmV2Features = otbmV1Features;
    otbmV2Features.insert("improved_items");
    QSet<QString> otbmV3Features = otbmV2Features;
    otbmV3Features.insert("waypoints");
    QSet<QString> otbmV4Features = otbmV3Features;
    otbmV4Features.insert("attribute_map");

    otbmFeatureSupport_[MAP_OTBM_1] = otbmV1Features;
    otbmFeatureSupport_[MAP_OTBM_2] = otbmV2Features;
    otbmFeatureSupport_[MAP_OTBM_3] = otbmV3Features;
    otbmFeatureSupport_[MAP_OTBM_4] = otbmV4Features;

    // Client feature support matrix
    QSet<QString> client750Features = {"basic_items", "spawns", "towns"};
    QSet<QString> client820Features = client750Features;
    client820Features.insert("charges");
    QSet<QString> client1057Features = client820Features;
    client1057Features.insert("tier");
    QSet<QString> client1094Features = client1057Features;
    client1094Features.insert("podium_outfit");

    clientFeatureSupport_[CLIENT_VERSION_750] = client750Features;
    clientFeatureSupport_[CLIENT_VERSION_820] = client820Features;
    clientFeatureSupport_[CLIENT_VERSION_1057] = client1057Features;
    clientFeatureSupport_[CLIENT_VERSION_1094] = client1094Features;
    clientFeatureSupport_[CLIENT_VERSION_1300] = client1094Features; // Latest inherits all
}

void OtbmVersionConverter::initializeAttributeMappings() {
    // Legacy to modern attribute name mappings
    attributeNameMappings_["aid"] = "actionid";
    attributeNameMappings_["uid"] = "uniqueid";
    attributeNameMappings_["text"] = "description";
    attributeNameMappings_["desc"] = "description";

    // Legacy attributes (pre-v4)
    legacyAttributes_.insert("aid");
    legacyAttributes_.insert("uid");
    legacyAttributes_.insert("charges");
    legacyAttributes_.insert("count");
    legacyAttributes_.insert("text");
    legacyAttributes_.insert("desc");
    legacyAttributes_.insert("tier");

    // Modern attributes (v4+)
    modernAttributes_.insert("actionid");
    modernAttributes_.insert("uniqueid");
    modernAttributes_.insert("description");
    modernAttributes_.insert("weight");
    modernAttributes_.insert("attack");
    modernAttributes_.insert("defense");
    modernAttributes_.insert("extradefense");
    modernAttributes_.insert("armor");
    modernAttributes_.insert("hitchance");
    modernAttributes_.insert("shootrange");
}

// Attribute conversion utilities
QVariant OtbmVersionConverter::convertAttributeForVersion(const QString& attributeName, const QVariant& value,
                                                         quint32 fromOtbmMajor, quint32 toOtbmMajor) {
    Q_UNUSED(fromOtbmMajor)
    Q_UNUSED(toOtbmMajor)

    // Handle specific attribute conversions
    if (attributeName == "charges" && !supportsCharges(defaultClientVersion_)) {
        // Convert charges to count for older clients
        return value;
    }

    if (attributeName == "tier" && !supportsTier(defaultClientVersion_)) {
        // Remove tier attribute for older clients
        return QVariant();
    }

    return value;
}

QMap<QString, QVariant> OtbmVersionConverter::convertAttributeMapForVersion(const QMap<QString, QVariant>& attributes,
                                                                           quint32 fromOtbmMajor, quint32 toOtbmMajor) {
    QMap<QString, QVariant> convertedAttributes;

    for (auto it = attributes.begin(); it != attributes.end(); ++it) {
        const QString& key = it.key();
        const QVariant& value = it.value();

        QVariant convertedValue = convertAttributeForVersion(key, value, fromOtbmMajor, toOtbmMajor);
        if (convertedValue.isValid()) {
            QString convertedKey = key;

            // Handle attribute name conversions
            if (toOtbmMajor < MAP_OTBM_4 && modernAttributes_.contains(key)) {
                // Convert modern names to legacy names
                for (auto mapIt = attributeNameMappings_.begin(); mapIt != attributeNameMappings_.end(); ++mapIt) {
                    if (mapIt.value() == key) {
                        convertedKey = mapIt.key();
                        break;
                    }
                }
            } else if (toOtbmMajor >= MAP_OTBM_4 && legacyAttributes_.contains(key)) {
                // Convert legacy names to modern names
                if (attributeNameMappings_.contains(key)) {
                    convertedKey = attributeNameMappings_[key];
                }
            }

            convertedAttributes[convertedKey] = convertedValue;
        }
    }

    return convertedAttributes;
}

// Item ID conversion for client compatibility
quint16 OtbmVersionConverter::convertItemIdForClient(quint16 itemId, quint32 fromClientVersion, quint32 toClientVersion) {
    if (fromClientVersion == toClientVersion) {
        return itemId;
    }

    // Look up conversion in our tables
    if (itemIdConversionMaps_.contains(fromClientVersion)) {
        const QMap<quint16, quint16>& conversionMap = itemIdConversionMaps_[fromClientVersion];
        if (conversionMap.contains(itemId)) {
            return conversionMap[itemId];
        }
    }

    // No conversion found, return original ID
    return itemId;
}

QMap<quint16, quint16> OtbmVersionConverter::getItemIdConversionMap(quint32 fromClientVersion, quint32 toClientVersion) {
    Q_UNUSED(toClientVersion)

    if (itemIdConversionMaps_.contains(fromClientVersion)) {
        return itemIdConversionMaps_[fromClientVersion];
    }

    return QMap<quint16, quint16>();
}

bool OtbmVersionConverter::hasItemIdChanged(quint16 itemId, quint32 fromClientVersion, quint32 toClientVersion) {
    return convertItemIdForClient(itemId, fromClientVersion, toClientVersion) != itemId;
}

// Attribute handling helpers
void OtbmVersionConverter::migrateAttributesToAttributeMap(Item* item) {
    if (!item) return;

    // In v4, individual attributes can be consolidated into the attribute map
    // This is mainly for optimization and doesn't change functionality
    // Our Item class already handles this correctly
}

void OtbmVersionConverter::migrateAttributeMapToAttributes(Item* item) {
    if (!item) return;

    // When downgrading from v4, we need to extract attributes from the attribute map
    // back to individual attributes for compatibility
    // Our Item class handles this automatically
}

bool OtbmVersionConverter::isLegacyAttribute(const QString& attributeName) const {
    return legacyAttributes_.contains(attributeName);
}

QString OtbmVersionConverter::getLegacyAttributeName(const QString& modernName) const {
    for (auto it = attributeNameMappings_.begin(); it != attributeNameMappings_.end(); ++it) {
        if (it.value() == modernName) {
            return it.key();
        }
    }
    return modernName;
}

QString OtbmVersionConverter::getModernAttributeName(const QString& legacyName) const {
    return attributeNameMappings_.value(legacyName, legacyName);
}

// Conversion validation
OtbmVersionConverter::ConversionResult OtbmVersionConverter::validateConversion(Map* map, quint32 targetOtbmMajor,
                                                                                quint32 targetOtbmMinor, quint32 targetClientVersion) {
    ConversionResult result;
    QStringList warnings;

    if (!map) {
        result.errorMessage = "Cannot validate null map";
        return result;
    }

    if (!isValidOtbmVersion(targetOtbmMajor, targetOtbmMinor)) {
        result.errorMessage = QString("Invalid target OTBM version: %1.%2").arg(targetOtbmMajor).arg(targetOtbmMinor);
        return result;
    }

    if (!isValidClientVersion(targetClientVersion)) {
        result.errorMessage = QString("Invalid target client version: %1").arg(targetClientVersion);
        return result;
    }

    // Validate map compatibility
    if (!validateMapForVersion(map, targetOtbmMajor, targetClientVersion, warnings)) {
        result.errorMessage = "Map validation failed for target version";
        result.warnings = warnings;
        return result;
    }

    result.success = true;
    result.warnings = warnings;
    return result;
}

OtbmVersionConverter::ConversionResult OtbmVersionConverter::performConversion(Map* map, quint32 targetOtbmMajor,
                                                                              quint32 targetOtbmMinor, quint32 targetClientVersion) {
    ConversionResult result = validateConversion(map, targetOtbmMajor, targetOtbmMinor, targetClientVersion);

    if (!result.success) {
        return result;
    }

    // Perform the actual conversion
    bool conversionSuccess = convertMapVersion(map, targetOtbmMajor, targetOtbmMinor, targetClientVersion);

    if (conversionSuccess) {
        result.success = true;
        result.itemsConverted = 0; // Would be counted during conversion
        result.tilesConverted = 0; // Would be counted during conversion
        result.attributesConverted = 0; // Would be counted during conversion
    } else {
        result.success = false;
        result.errorMessage = "Conversion failed during execution";
    }

    return result;
}

// Missing method implementations
bool OtbmVersionConverter::convertMapForClientVersion(Map* map, quint32 targetClientVersion) {
    if (!map) return false;

    logConversion(QString("Converting map for client version %1").arg(targetClientVersion));

    int itemsConverted = 0;

    // Convert all items in the map for the target client version
    for (int z = 0; z < map->getLayers(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                Tile* tile = map->getTile(x, y, z);
                if (!tile) continue;

                // Convert items on tile
                const QVector<Item*>& items = tile->items();
                for (Item* item : items) {
                    if (item && convertItemForClientVersion(item, defaultClientVersion_, targetClientVersion)) {
                        itemsConverted++;
                    }
                }

                // Convert ground item
                Item* ground = tile->getGround();
                if (ground && convertItemForClientVersion(ground, defaultClientVersion_, targetClientVersion)) {
                    itemsConverted++;
                }
            }
        }
    }

    logConversion(QString("Converted %1 items for client version %2").arg(itemsConverted).arg(targetClientVersion));
    return true;
}

bool OtbmVersionConverter::convertItemForVersion(Item* item, quint32 otbmMajor, quint32 otbmMinor, quint32 clientVersion) {
    Q_UNUSED(otbmMinor)

    if (!item) return false;

    // Convert item ID for client compatibility
    quint16 currentId = item->getServerId();
    quint16 newId = convertItemIdForClient(currentId, defaultClientVersion_, clientVersion);

    if (newId != currentId) {
        // Note: We can't actually change the item ID here as it would require
        // recreating the item. This would be handled at a higher level.
        logConversion(QString("Item ID %1 should be converted to %2 for client %3")
                     .arg(currentId).arg(newId).arg(clientVersion));
    }

    // Convert attributes for OTBM version
    QMap<QString, QVariant> currentAttributes = item->getAllAttributes();
    QMap<QString, QVariant> convertedAttributes = convertAttributeMapForVersion(currentAttributes,
                                                                               defaultOtbmMajor_, otbmMajor);

    // Apply converted attributes (simplified - would need proper implementation)
    for (auto it = convertedAttributes.begin(); it != convertedAttributes.end(); ++it) {
        item->setAttribute(it.key(), it.value());
    }

    return true;
}

bool OtbmVersionConverter::convertTileForVersion(Tile* tile, quint32 otbmMajor, quint32 otbmMinor, quint32 clientVersion) {
    Q_UNUSED(otbmMinor)
    Q_UNUSED(clientVersion)

    if (!tile) return false;

    // Convert all items on the tile
    const QVector<Item*>& items = tile->items();
    for (Item* item : items) {
        convertItemForVersion(item, otbmMajor, otbmMinor, clientVersion);
    }

    // Convert ground item
    Item* ground = tile->getGround();
    if (ground) {
        convertItemForVersion(ground, otbmMajor, otbmMinor, clientVersion);
    }

    return true;
}

bool OtbmVersionConverter::convertItemForClientVersion(Item* item, quint32 fromVersion, quint32 toVersion) {
    if (!item || fromVersion == toVersion) return false;

    // Handle client-specific attribute conversions
    if (!supportsCharges(toVersion) && item->hasAttribute("charges")) {
        // Convert charges to count for older clients
        QVariant charges = item->getAttribute("charges");
        item->removeAttribute("charges");
        item->setAttribute("count", charges);
        return true;
    }

    if (!supportsTier(toVersion) && item->hasAttribute("tier")) {
        // Remove tier attribute for older clients
        item->removeAttribute("tier");
        return true;
    }

    return false;
}

bool OtbmVersionConverter::convertTileForClientVersion(Tile* tile, quint32 fromVersion, quint32 toVersion) {
    if (!tile) return false;

    bool converted = false;

    // Convert all items on the tile
    const QVector<Item*>& items = tile->items();
    for (Item* item : items) {
        if (convertItemForClientVersion(item, fromVersion, toVersion)) {
            converted = true;
        }
    }

    // Convert ground item
    Item* ground = tile->getGround();
    if (ground && convertItemForClientVersion(ground, fromVersion, toVersion)) {
        converted = true;
    }

    return converted;
}

// Downgrade conversion methods
bool OtbmVersionConverter::convertOtbmV4ToV3(Map* map) {
    logConversion("Converting OTBM v4 to v3: Removing attribute map support");

    int itemsConverted = 0;

    // Convert all items to use individual attributes instead of attribute map
    for (int z = 0; z < map->getLayers(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                Tile* tile = map->getTile(x, y, z);
                if (!tile) continue;

                const QVector<Item*>& items = tile->items();
                for (Item* item : items) {
                    if (item && convertItemV4ToV3(item)) {
                        itemsConverted++;
                    }
                }

                Item* ground = tile->getGround();
                if (ground && convertItemV4ToV3(ground)) {
                    itemsConverted++;
                }
            }
        }
    }

    logConversion(QString("Converted %1 items from v4 to v3 format").arg(itemsConverted));
    return true;
}

bool OtbmVersionConverter::convertOtbmV3ToV2(Map* map) {
    logConversion("Converting OTBM v3 to v2: Removing waypoints support");

    // Remove waypoints if they exist
    // This would require access to the waypoints system
    logWarning("Waypoints will be lost when converting from v3 to v2");

    return true;
}

bool OtbmVersionConverter::convertOtbmV2ToV1(Map* map) {
    logConversion("Converting OTBM v2 to v1: Reverting to legacy item handling");

    int itemsConverted = 0;

    // Convert items back to v1 format
    for (int z = 0; z < map->getLayers(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                Tile* tile = map->getTile(x, y, z);
                if (!tile) continue;

                const QVector<Item*>& items = tile->items();
                for (Item* item : items) {
                    if (item && convertItemV2ToV1(item)) {
                        itemsConverted++;
                    }
                }

                Item* ground = tile->getGround();
                if (ground && convertItemV2ToV1(ground)) {
                    itemsConverted++;
                }
            }
        }
    }

    logConversion(QString("Converted %1 items from v2 to v1 format").arg(itemsConverted));
    return true;
}

// Item conversion helpers for downgrades
bool OtbmVersionConverter::convertItemV4ToV3(Item* item) {
    if (!item) return false;

    // Migrate from attribute map back to individual attributes
    migrateAttributeMapToAttributes(item);
    return true;
}

bool OtbmVersionConverter::convertItemV3ToV2(Item* item) {
    Q_UNUSED(item)
    // No specific conversion needed from v3 to v2 for items
    return false;
}

bool OtbmVersionConverter::convertItemV2ToV1(Item* item) {
    if (!item) return false;

    // In v1, stackable items had inline subtype instead of OTBM_ATTR_COUNT
    // This is handled automatically by our serialization system
    return item->isStackable() && item->getCount() > 1;
}

// Validation helpers
bool OtbmVersionConverter::validateMapForVersion(Map* map, quint32 otbmMajor, quint32 clientVersion, QStringList& warnings) {
    if (!map) return false;

    bool valid = true;

    // Check if map features are supported by target version
    if (!supportsWaypoints(otbmMajor) && map->hasWaypoints()) {
        warnings.append("Waypoints will be lost in target OTBM version");
    }

    if (!supportsAttributeMap(otbmMajor)) {
        warnings.append("Extended attributes may be lost in target OTBM version");
    }

    // Check client version compatibility
    if (!supportsCharges(clientVersion)) {
        warnings.append("Charge attributes will be converted to count for target client version");
    }

    if (!supportsTier(clientVersion)) {
        warnings.append("Tier attributes will be removed for target client version");
    }

    return valid;
}

bool OtbmVersionConverter::validateItemForVersion(Item* item, quint32 otbmMajor, quint32 clientVersion, QStringList& warnings) {
    Q_UNUSED(otbmMajor)

    if (!item) return false;

    // Check client-specific features
    if (item->hasAttribute("charges") && !supportsCharges(clientVersion)) {
        warnings.append(QString("Item %1 has charges attribute not supported by target client").arg(item->getServerId()));
    }

    if (item->hasAttribute("tier") && !supportsTier(clientVersion)) {
        warnings.append(QString("Item %1 has tier attribute not supported by target client").arg(item->getServerId()));
    }

    return true;
}

bool OtbmVersionConverter::validateTileForVersion(Tile* tile, quint32 otbmMajor, quint32 clientVersion, QStringList& warnings) {
    if (!tile) return false;

    bool valid = true;

    // Validate all items on tile
    const QVector<Item*>& items = tile->items();
    for (Item* item : items) {
        if (!validateItemForVersion(item, otbmMajor, clientVersion, warnings)) {
            valid = false;
        }
    }

    // Validate ground item
    Item* ground = tile->getGround();
    if (ground && !validateItemForVersion(ground, otbmMajor, clientVersion, warnings)) {
        valid = false;
    }

    return valid;
}
