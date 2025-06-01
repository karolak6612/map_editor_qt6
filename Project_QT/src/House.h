#ifndef HOUSE_H
#define HOUSE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QSet>
#include <QHash>
#include <QVariantMap>
#include <QDateTime>
#include <QRect>
#include <QDomDocument>  // Task 66: XML support
#include <QDomElement>   // Task 66: XML support
#include <QXmlStreamReader>  // Task 86: Enhanced XML I/O
#include <QXmlStreamWriter>  // Task 86: Enhanced XML I/O
#include "Map.h" // For MapPos

// Forward declarations
class Map;
class Tile;
class Town;

/**
 * @brief Task 86: Enhanced House System
 *
 * Complete implementation of house management system:
 * - Enhanced data models with full property support
 * - XML I/O for houses.xml files
 * - Management dialogs and UI tools
 * - Integration with map editor and tile system
 * - Visual representation and editing capabilities
 * - 1:1 compatibility with wxwidgets house system
 */

/**
 * @brief House access level enumeration (1:1 wxwidgets compatibility)
 */
enum class HouseAccessLevel {
    None = 0,
    Guest = 1,
    Subowner = 2,
    Owner = 3
};

/**
 * @brief House door information structure
 */
struct HouseDoor {
    MapPos position;
    quint16 doorId = 0;
    QString name;
    bool isLocked = false;
    QStringList accessList;

    HouseDoor() = default;
    HouseDoor(const MapPos& pos, quint16 id = 0, const QString& doorName = QString())
        : position(pos), doorId(id), name(doorName) {}

    bool operator==(const HouseDoor& other) const {
        return position == other.position && doorId == other.doorId;
    }
};

/**
 * @brief House bed information structure
 */
struct HouseBed {
    MapPos position;
    quint16 bedId = 0;
    QString ownerName;
    QDateTime lastUsed;

    HouseBed() = default;
    HouseBed(const MapPos& pos, quint16 id = 0, const QString& owner = QString())
        : position(pos), bedId(id), ownerName(owner) {}

    bool operator==(const HouseBed& other) const {
        return position == other.position && bedId == other.bedId;
    }
};

class House : public QObject {
    Q_OBJECT
    Q_PROPERTY(quint32 id READ getId WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(quint32 townId READ getTownId WRITE setTownId NOTIFY townIdChanged)
    Q_PROPERTY(quint32 rent READ getRent WRITE setRent NOTIFY rentChanged)
    Q_PROPERTY(bool isGuildHall READ isGuildHall WRITE setGuildHall NOTIFY guildHallChanged)
    Q_PROPERTY(QString owner READ getOwner WRITE setOwner NOTIFY ownerChanged)
    Q_PROPERTY(QDateTime paidUntil READ getPaidUntil WRITE setPaidUntil NOTIFY paidUntilChanged)
    Q_PROPERTY(int size READ getSize NOTIFY sizeChanged)

public:
    explicit House(QObject* parent = nullptr);
    House(quint32 id, const QString& name = QString(), QObject* parent = nullptr);
    explicit House(const House& other, QObject* parent = nullptr);
    ~House() override;

    // Core properties
    quint32 getId() const;
    void setId(quint32 id);

    QString getName() const;
    void setName(const QString& name);

    QString getOwner() const;
    void setOwner(const QString& owner);

    quint32 getRent() const;
    void setRent(quint32 rent);

    // Task 86: Enhanced properties
    QDateTime getPaidUntil() const;
    void setPaidUntil(const QDateTime& date);

    QString getDescription() const;
    void setDescription(const QString& description);

    QStringList getKeywords() const;
    void setKeywords(const QStringList& keywords);
    void addKeyword(const QString& keyword);
    void removeKeyword(const QString& keyword);

    quint32 getTownId() const;
    void setTownId(quint32 townId);

    bool isGuildHall() const;
    void setGuildHall(bool isGuildHall);

    // Entry/Exit management
    MapPos getEntryPosition() const;
    void setEntryPosition(const MapPos& position);

    MapPos getExitPosition() const;
    void setExitPosition(const MapPos& position);

    // Tile management
    void addTile(const MapPos& position);
    void removeTile(const MapPos& position);
    void addTile(Tile* tile);
    void removeTile(Tile* tile);
    bool containsTile(const MapPos& position) const;
    bool containsTile(Tile* tile) const;

    const QList<MapPos>& getTilePositions() const;
    QList<Tile*> getTiles(Map* map) const;
    int getTileCount() const;
    int getSize() const; // Non-blocking tiles count

    // House management
    void clean(Map* map); // Remove house from all tiles
    void clear(); // Clear all tiles from house

    // Task 86: Enhanced door management
    void addDoor(const HouseDoor& door);
    void removeDoor(const MapPos& position);
    HouseDoor getDoor(const MapPos& position) const;
    QList<HouseDoor> getDoors() const;
    bool hasDoor(const MapPos& position) const;
    void clearDoors();
    quint8 getEmptyDoorId() const;
    MapPos getDoorPositionById(quint8 doorId) const;
    void setDoorId(const MapPos& position, quint8 doorId);

    // Task 86: Enhanced bed management
    void addBed(const HouseBed& bed);
    void removeBed(const MapPos& position);
    HouseBed getBed(const MapPos& position) const;
    QList<HouseBed> getBeds() const;
    bool hasBed(const MapPos& position) const;
    void clearBeds();

    // Task 86: Access management
    void addGuest(const QString& playerName);
    void removeGuest(const QString& playerName);
    bool isGuest(const QString& playerName) const;
    QStringList getGuests() const;
    void clearGuests();

    void addSubowner(const QString& playerName);
    void removeSubowner(const QString& playerName);
    bool isSubowner(const QString& playerName) const;
    QStringList getSubowners() const;
    void clearSubowners();

    HouseAccessLevel getAccessLevel(const QString& playerName) const;
    bool hasAccess(const QString& playerName, HouseAccessLevel minLevel = HouseAccessLevel::Guest) const;

    // Validation
    bool isValid() const;
    QString getValidationError() const;
    bool canBeDeleted() const;

    // Utility
    QString getDescription() const;
    quint32 memsize() const;
    QVariantMap getStatistics() const;

    // Serialization support
    bool serialize(QDataStream& stream) const;
    bool deserialize(QDataStream& stream);

    // Task 66: XML serialization support
    bool loadFromXml(const QDomElement& element);
    bool saveToXml(QDomDocument& doc, QDomElement& parent) const;
    static bool loadHousesFromXML(const QString& filePath, QList<House*>& houses, QStringList& errors);
    static bool saveHousesToXML(const QString& filePath, const QList<House*>& houses, QStringList& errors);

    // Task 86: Enhanced XML I/O
    bool loadFromXmlStream(QXmlStreamReader& xml);
    bool saveToXmlStream(QXmlStreamWriter& xml) const;

    // Task 86: Comparison operators
    bool operator==(const House& other) const;
    bool operator!=(const House& other) const { return !(*this == other); }
    bool operator<(const House& other) const { return getId() < other.getId(); }

    // Task 73: Enhanced house operations
    void removeLooseItems(Map* map);
    void setPZOnTiles(Map* map, bool pz = true);
    void assignDoorIDs(Map* map);
    void clearDoorIDs(Map* map);
    bool isDoorIDUsed(quint8 doorId) const;

    // Task 73: House area calculations
    QRect getBoundingRect() const;
    QList<QRect> getAreas() const;
    int getArea() const;
    bool isConnected(Map* map) const;

    // Task 73: Advanced tile operations
    QList<Tile*> getTilesInArea(Map* map, const QRect& area) const;
    Tile* getTileAt(Map* map, const MapPos& position) const;
    void removeInvalidTiles(Map* map);
    void updateTileHouseReferences(Map* map);

    // Task 73: House utilities
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    QString getHouseInfo() const;
    bool hasValidExit(Map* map) const;

signals:
    // Task 86: Enhanced signals
    void idChanged(quint32 newId);
    void nameChanged(const QString& newName);
    void townIdChanged(quint32 newTownId);
    void rentChanged(quint32 newRent);
    void guildHallChanged(bool isGuildHall);
    void ownerChanged(const QString& newOwner);
    void paidUntilChanged(const QDateTime& date);
    void sizeChanged(int newSize);
    void descriptionChanged(const QString& newDescription);

    void houseChanged();
    void tileAdded(const MapPos& position);
    void tileRemoved(const MapPos& position);
    void doorAdded(const HouseDoor& door);
    void doorRemoved(const MapPos& position);
    void bedAdded(const HouseBed& bed);
    void bedRemoved(const MapPos& position);
    void accessChanged();

public slots:
    void onTileChanged(const MapPos& position);

private:
    // Core properties
    quint32 id_;
    QString name_;
    QString owner_;
    quint32 rent_;
    quint32 townId_;
    bool isGuildHall_;

    // Task 86: Enhanced properties
    QDateTime paidUntil_;
    QString description_;
    QStringList keywords_;

    MapPos entryPosition_;
    MapPos exitPosition_;

    // Tile management
    QList<MapPos> tilePositions_;
    QMap<MapPos, quint8> doorIds_; // Position -> Door ID mapping

    // Task 86: Enhanced structures
    QList<HouseDoor> doors_;
    QList<HouseBed> beds_;
    QStringList guests_;
    QStringList subowners_;

    // Helper methods
    void updateTileHouseId(Map* map, const MapPos& position, bool add);
    void emitSizeChanged();
    void validateAndEmitChanges();
};

// Task 86: Enhanced House collection management class
class Houses : public QObject {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit Houses(QObject* parent = nullptr);
    ~Houses() override;

    // House management
    bool addHouse(House* house);
    bool removeHouse(House* house);
    bool removeHouse(quint32 houseId);
    House* getHouse(quint32 houseId) const;
    House* getHouseByName(const QString& name) const;
    House* getHouseByTile(const MapPos& position) const;

    // Collection access
    const QMap<quint32, House*>& getHouses() const;
    QList<House*> getHousesList() const;
    int getHouseCount() const;
    int count() const { return getHouseCount(); }
    bool isEmpty() const;

    // Task 86: Enhanced collection operations
    QList<House*> getAllHouses() const;
    QList<House*> getHousesByTown(quint32 townId) const;
    QList<House*> getGuildHalls() const;
    QList<House*> getHousesByOwner(const QString& owner) const;
    QList<House*> getHousesInArea(const QRect& area) const;

    // ID management
    quint32 getEmptyId() const;
    quint32 getNextId() const;
    quint32 getNextAvailableId() const;
    bool isIdUsed(quint32 id) const;
    bool isIdAvailable(quint32 houseId) const;
    QList<quint32> getUsedIds() const;

    // Task 86: Validation and statistics
    bool validateHouses() const;
    QStringList getValidationErrors() const;
    void removeInvalidHouses();
    QVariantMap getStatistics() const;
    int getTotalTiles() const;
    int getAverageSize() const;

    // Utility
    void clear();
    quint32 memsize() const;

    // Task 86: Enhanced XML I/O
    bool loadFromXml(const QString& filePath);
    bool saveToXml(const QString& filePath) const;
    bool loadFromXmlStream(QXmlStreamReader& xml);
    bool saveToXmlStream(QXmlStreamWriter& xml) const;

    // Iterator support
    QMap<quint32, House*>::iterator begin();
    QMap<quint32, House*>::iterator end();
    QMap<quint32, House*>::const_iterator begin() const;
    QMap<quint32, House*>::const_iterator end() const;
    QMap<quint32, House*>::const_iterator find(quint32 id) const;

signals:
    void houseAdded(House* house);
    void houseRemoved(quint32 houseId);
    void houseChanged(House* house);
    void countChanged(int newCount);
    void housesCleared();

public slots:
    void onHouseChanged();

private:
    QMap<quint32, House*> houses_;
    quint32 nextId_;

    void connectHouseSignals(House* house);
    void disconnectHouseSignals(House* house);
};

/**
 * @brief Task 86: House utility functions
 */
class HouseUtils
{
public:
    // Validation utilities
    static bool isValidHouseName(const QString& name);
    static bool isValidHouseId(quint32 id);
    static bool isValidRent(quint32 rent);
    static QString validateHouseData(const House* house);

    // Tile utilities
    static bool isHouseTile(const MapPos& position, const Map* map);
    static QList<MapPos> getConnectedHouseTiles(const MapPos& startPosition, const Map* map);
    static bool areHouseTilesConnected(const QList<MapPos>& tiles, const Map* map);

    // Door utilities
    static bool isDoorTile(const MapPos& position, const Map* map);
    static QList<MapPos> findHouseDoors(const QList<MapPos>& houseTiles, const Map* map);
    static bool isValidDoorPosition(const MapPos& position, const Map* map);

    // Bed utilities
    static bool isBedTile(const MapPos& position, const Map* map);
    static QList<MapPos> findHouseBeds(const QList<MapPos>& houseTiles, const Map* map);
    static bool isValidBedPosition(const MapPos& position, const Map* map);

    // Area utilities
    static QRect calculateBoundingRect(const QList<MapPos>& tiles);
    static MapPos calculateCenter(const QList<MapPos>& tiles);
    static int calculateFloorCount(const QList<MapPos>& tiles);

    // Import/Export utilities
    static QString exportHouseToString(const House* house);
    static House* importHouseFromString(const QString& data, QObject* parent = nullptr);
    static bool exportHousesToFile(const Houses* houses, const QString& filePath);
    static bool importHousesFromFile(Houses* houses, const QString& filePath);

private:
    HouseUtils() = default; // Static utility class
};

#endif // HOUSE_H
