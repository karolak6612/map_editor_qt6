#ifndef HOUSETOWNPERSISTENCEMANAGER_H
#define HOUSETOWNPERSISTENCEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDataStream>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>

#include "House.h"
#include "Town.h"
#include "MapPos.h"

// Forward declarations
class Map;
class OtbmReader;
class OtbmWriter;

/**
 * @brief Task 94: Houses and Towns Persistence Manager for OTBM/XML handling
 * 
 * Complete persistence system for Houses and Towns data:
 * - OTBM format support with proper attributes and sections
 * - Dedicated XML files for house/town data (houses.xml, towns.xml)
 * - Map integration for automatic persistence
 * - Backup and recovery functionality
 * - Import/export capabilities
 * - Version compatibility and migration
 * - Error handling and validation
 * - 1:1 compatibility with wxwidgets persistence format
 */

/**
 * @brief OTBM Houses and Towns Attributes (from wxwidgets compatibility)
 */
enum class OtbmHouseAttribute : quint8 {
    HOUSE_ATTR_ID = 0x01,
    HOUSE_ATTR_NAME = 0x02,
    HOUSE_ATTR_OWNER = 0x03,
    HOUSE_ATTR_PAID_UNTIL = 0x04,
    HOUSE_ATTR_WARNINGS = 0x05,
    HOUSE_ATTR_LAST_WARNING = 0x06,
    HOUSE_ATTR_RENT = 0x07,
    HOUSE_ATTR_TOWN_ID = 0x08,
    HOUSE_ATTR_SIZE = 0x09,
    HOUSE_ATTR_GUILD_HALL = 0x0A,
    HOUSE_ATTR_DOORS = 0x0B,
    HOUSE_ATTR_BEDS = 0x0C,
    HOUSE_ATTR_TILES = 0x0D
};

enum class OtbmTownAttribute : quint8 {
    TOWN_ATTR_ID = 0x01,
    TOWN_ATTR_NAME = 0x02,
    TOWN_ATTR_TEMPLE_POS = 0x03,
    TOWN_ATTR_HOUSES = 0x04,
    TOWN_ATTR_KEYWORDS = 0x05
};

/**
 * @brief Main Houses and Towns Persistence Manager
 */
class HouseTownPersistenceManager : public QObject
{
    Q_OBJECT

public:
    explicit HouseTownPersistenceManager(QObject* parent = nullptr);
    ~HouseTownPersistenceManager() override;

    // Component setup
    void setMap(Map* map);
    Map* getMap() const { return map_; }

    // OTBM persistence
    bool saveHousesToOTBM(OtbmWriter& writer) const;
    bool loadHousesFromOTBM(OtbmReader& reader);
    bool saveTownsToOTBM(OtbmWriter& writer) const;
    bool loadTownsFromOTBM(OtbmReader& reader);

    // XML persistence
    bool saveHousesToXML(const QString& filePath) const;
    bool loadHousesFromXML(const QString& filePath);
    bool saveTownsToXML(const QString& filePath) const;
    bool loadTownsFromXML(const QString& filePath);

    // Combined operations
    bool saveAllToXML(const QString& housesPath, const QString& townsPath) const;
    bool loadAllFromXML(const QString& housesPath, const QString& townsPath);
    bool saveAllToOTBM(OtbmWriter& writer) const;
    bool loadAllFromOTBM(OtbmReader& reader);

    // Auto-persistence
    void enableAutoPersistence(bool enabled);
    bool isAutoPersistenceEnabled() const { return autoPersistence_; }
    void setAutoPersistenceInterval(int seconds);
    int getAutoPersistenceInterval() const { return autoPersistenceInterval_; }

    // Backup and recovery
    bool createBackup(const QString& backupDir) const;
    bool restoreFromBackup(const QString& backupDir);
    QStringList getAvailableBackups(const QString& backupDir) const;
    bool deleteBackup(const QString& backupDir, const QString& backupName);

    // Import/export
    bool exportHouses(const QString& filePath, const QString& format = "xml") const;
    bool importHouses(const QString& filePath, const QString& format = "xml");
    bool exportTowns(const QString& filePath, const QString& format = "xml") const;
    bool importTowns(const QString& filePath, const QString& format = "xml");

    // Validation and error handling
    bool validateHousesData() const;
    bool validateTownsData() const;
    QStringList getLastErrors() const { return lastErrors_; }
    void clearErrors();

    // Statistics
    QVariantMap getStatistics() const;
    void resetStatistics();

public slots:
    void onMapModified();
    void onHouseAdded(House* house);
    void onHouseRemoved(quint32 houseId);
    void onHouseModified(House* house);
    void onTownAdded(Town* town);
    void onTownRemoved(quint32 townId);
    void onTownModified(Town* town);
    void onAutoPersistenceTimer();

signals:
    void persistenceCompleted(bool success);
    void persistenceError(const QString& error);
    void backupCreated(const QString& backupPath);
    void dataImported(int housesCount, int townsCount);
    void dataExported(int housesCount, int townsCount);

private:
    // OTBM house operations
    bool saveHouseToOTBM(OtbmWriter& writer, House* house) const;
    bool loadHouseFromOTBM(OtbmReader& reader, House*& house);
    void writeHouseAttributes(OtbmWriter& writer, House* house) const;
    bool readHouseAttributes(OtbmReader& reader, House* house);

    // OTBM town operations
    bool saveTownToOTBM(OtbmWriter& writer, Town* town) const;
    bool loadTownFromOTBM(OtbmReader& reader, Town*& town);
    void writeTownAttributes(OtbmWriter& writer, Town* town) const;
    bool readTownAttributes(OtbmReader& reader, Town* town);

    // XML house operations
    bool saveHouseToXML(QXmlStreamWriter& writer, House* house) const;
    bool loadHouseFromXML(QXmlStreamReader& reader, House*& house);
    void writeHouseElement(QXmlStreamWriter& writer, House* house) const;
    bool readHouseElement(QXmlStreamReader& reader, House* house);

    // XML town operations
    bool saveTownToXML(QXmlStreamWriter& writer, Town* town) const;
    bool loadTownFromXML(QXmlStreamReader& reader, Town*& town);
    void writeTownElement(QXmlStreamWriter& writer, Town* town) const;
    bool readTownElement(QXmlStreamReader& reader, Town* town);

    // Utility methods
    QString generateBackupName() const;
    QString getBackupPath(const QString& backupDir, const QString& backupName) const;
    bool createDirectory(const QString& path) const;
    bool copyFile(const QString& source, const QString& destination) const;
    bool validateXMLFile(const QString& filePath) const;
    bool validateOTBMData(const QByteArray& data) const;

    // Error handling
    void addError(const QString& error);
    void logError(const QString& operation, const QString& error);
    void logWarning(const QString& operation, const QString& warning);

    // Statistics tracking
    void updateStatistics(const QString& operation, bool success);
    void incrementCounter(const QString& counter);

private:
    Map* map_;
    
    // Auto-persistence
    bool autoPersistence_;
    int autoPersistenceInterval_;
    QTimer* autoPersistenceTimer_;
    
    // Error handling
    mutable QMutex errorMutex_;
    QStringList lastErrors_;
    
    // Statistics
    mutable QMutex statsMutex_;
    QVariantMap statistics_;
    
    // File paths
    QString defaultHousesPath_;
    QString defaultTownsPath_;
    QString defaultBackupDir_;
};

/**
 * @brief Houses XML Serializer
 */
class HousesXMLSerializer
{
public:
    static bool serialize(const QList<House*>& houses, QXmlStreamWriter& writer);
    static bool deserialize(QXmlStreamReader& reader, QList<House*>& houses);
    
    static bool serializeHouse(House* house, QXmlStreamWriter& writer);
    static bool deserializeHouse(QXmlStreamReader& reader, House*& house);

private:
    static void writeHouseProperties(House* house, QXmlStreamWriter& writer);
    static bool readHouseProperties(QXmlStreamReader& reader, House* house);
    static void writeHouseTiles(House* house, QXmlStreamWriter& writer);
    static bool readHouseTiles(QXmlStreamReader& reader, House* house);
    static void writeHouseDoors(House* house, QXmlStreamWriter& writer);
    static bool readHouseDoors(QXmlStreamReader& reader, House* house);
};

/**
 * @brief Towns XML Serializer
 */
class TownsXMLSerializer
{
public:
    static bool serialize(const QList<Town*>& towns, QXmlStreamWriter& writer);
    static bool deserialize(QXmlStreamReader& reader, QList<Town*>& towns);
    
    static bool serializeTown(Town* town, QXmlStreamWriter& writer);
    static bool deserializeTown(QXmlStreamReader& reader, Town*& town);

private:
    static void writeTownProperties(Town* town, QXmlStreamWriter& writer);
    static bool readTownProperties(QXmlStreamReader& reader, Town* town);
    static void writeTownHouses(Town* town, QXmlStreamWriter& writer);
    static bool readTownHouses(QXmlStreamReader& reader, Town* town);
    static void writeTownKeywords(Town* town, QXmlStreamWriter& writer);
    static bool readTownKeywords(QXmlStreamReader& reader, Town* town);
};

/**
 * @brief Houses OTBM Serializer
 */
class HousesOTBMSerializer
{
public:
    static bool serialize(const QList<House*>& houses, OtbmWriter& writer);
    static bool deserialize(OtbmReader& reader, QList<House*>& houses);
    
    static bool serializeHouse(House* house, OtbmWriter& writer);
    static bool deserializeHouse(OtbmReader& reader, House*& house);

private:
    static void writeHouseAttributes(House* house, OtbmWriter& writer);
    static bool readHouseAttributes(OtbmReader& reader, House* house);
    static void writeHouseTileList(House* house, OtbmWriter& writer);
    static bool readHouseTileList(OtbmReader& reader, House* house);
};

/**
 * @brief Towns OTBM Serializer
 */
class TownsOTBMSerializer
{
public:
    static bool serialize(const QList<Town*>& towns, OtbmWriter& writer);
    static bool deserialize(OtbmReader& reader, QList<Town*>& towns);
    
    static bool serializeTown(Town* town, OtbmWriter& writer);
    static bool deserializeTown(OtbmReader& reader, Town*& town);

private:
    static void writeTownAttributes(Town* town, OtbmWriter& writer);
    static bool readTownAttributes(OtbmReader& reader, Town* town);
    static void writeTownHouseList(Town* town, OtbmWriter& writer);
    static bool readTownHouseList(OtbmReader& reader, Town* town);
};

/**
 * @brief Houses and Towns Data Validator
 */
class HouseTownDataValidator
{
public:
    static bool validateHouse(House* house, QStringList& errors);
    static bool validateTown(Town* town, QStringList& errors);
    static bool validateHouseList(const QList<House*>& houses, QStringList& errors);
    static bool validateTownList(const QList<Town*>& towns, QStringList& errors);
    static bool validateHouseTownConsistency(const QList<House*>& houses, const QList<Town*>& towns, QStringList& errors);

private:
    static bool validateHouseId(quint32 houseId, QStringList& errors);
    static bool validateTownId(quint32 townId, QStringList& errors);
    static bool validateHouseName(const QString& name, QStringList& errors);
    static bool validateTownName(const QString& name, QStringList& errors);
    static bool validatePosition(const MapPos& position, QStringList& errors);
    static bool validateRent(quint32 rent, QStringList& errors);
};

#endif // HOUSETOWNPERSISTENCEMANAGER_H
