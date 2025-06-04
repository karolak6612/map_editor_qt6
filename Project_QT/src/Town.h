#ifndef TOWN_H
#define TOWN_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QVariantMap>
#include <QHash>
#include <QDateTime>
#include <QRect>
#include <QDomDocument>  // Task 66: XML support
#include <QDomElement>   // Task 66: XML support
#include <QXmlStreamReader>  // Task 86: Enhanced XML I/O
#include <QXmlStreamWriter>  // Task 86: Enhanced XML I/O
#include "MapPos.h" // Direct include for MapPos typedef

// Forward declarations
class Map;
class House;

/**
 * @brief Task 86: Enhanced Town System
 *
 * Complete implementation of town management system:
 * - Enhanced data models with full property support
 * - XML I/O for towns.xml files
 * - Management dialogs and UI tools
 * - Integration with map editor and house system
 * - Visual representation and editing capabilities
 * - 1:1 compatibility with wxwidgets town system
 */

/**
 * @brief Main Town class (1:1 wxwidgets compatibility)
 */
class Town : public QObject {
    Q_OBJECT
    Q_PROPERTY(quint32 id READ getId WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(MapPos templePosition READ getTemplePosition WRITE setTemplePosition NOTIFY templePositionChanged)

public:
    explicit Town(QObject* parent = nullptr);
    Town(quint32 id, const QString& name = QString(), const MapPos& templePosition = MapPos(), QObject* parent = nullptr);
    explicit Town(const Town& other, QObject* parent = nullptr);
    ~Town() override = default;

    // Basic properties (1:1 wxwidgets compatibility)
    quint32 getId() const;
    void setId(quint32 id);

    QString getName() const;
    void setName(const QString& name);

    MapPos getTemplePosition() const;
    void setTemplePosition(const MapPos& position);

    // Task 86: Enhanced properties
    QString getDescription() const;
    void setDescription(const QString& description);

    QStringList getKeywords() const;
    void setKeywords(const QStringList& keywords);
    void addKeyword(const QString& keyword);
    void removeKeyword(const QString& keyword);

    // Task 86: House management
    QList<quint32> getHouseIds() const;
    void addHouse(quint32 houseId);
    void removeHouse(quint32 houseId);
    bool hasHouse(quint32 houseId) const;
    int getHouseCount() const;
    void clearHouses();

    // Task 86: Area and statistics
    QRect getBoundingRect(const Map* map) const;
    MapPos getCenter(const Map* map) const;
    int getTotalHouseTiles(const Map* map) const;
    QVariantMap getStatistics(const Map* map) const;

    // Validation and utility
    bool isValid() const;
    QString getValidationError() const;
    bool canBeDeleted() const;

    // Task 66: XML serialization support
    bool loadFromXml(const QDomElement& element);
    bool saveToXml(QDomDocument& doc, QDomElement& parent) const;
    static bool loadTownsFromXML(const QString& filePath, QList<Town*>& towns, QStringList& errors);
    static bool saveTownsToXML(const QString& filePath, const QList<Town*>& towns, QStringList& errors);

    // Task 86: Enhanced XML I/O
    bool loadFromXmlStream(QXmlStreamReader& xml);
    bool saveToXmlStream(QXmlStreamWriter& xml) const;

    // Task 86: Comparison operators
    bool operator==(const Town& other) const;
    bool operator!=(const Town& other) const { return !(*this == other); }
    bool operator<(const Town& other) const { return getId() < other.getId(); }

signals:
    // Task 86: Enhanced signals
    void idChanged(quint32 newId);
    void nameChanged(const QString& newName);
    void templePositionChanged(const MapPos& newPosition);
    void descriptionChanged(const QString& newDescription);
    void houseAdded(quint32 houseId);
    void houseRemoved(quint32 houseId);
    void townChanged();

public slots:
    void onHouseChanged();

private:
    // Core properties
    quint32 m_id;
    QString m_name;
    MapPos m_templePosition;

    // Task 86: Enhanced properties
    QString m_description;
    QStringList m_keywords;
    QList<quint32> m_houseIds;

    // Helper methods
    void validateAndEmitChanges();
};

/**
 * @brief Task 86: Enhanced Towns collection management class
 */
class Towns : public QObject {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit Towns(QObject* parent = nullptr);
    ~Towns() override;

    // Town management
    bool addTown(Town* town);
    bool removeTown(Town* town);
    bool removeTown(quint32 townId);
    Town* getTown(quint32 townId) const;
    Town* getTownByName(const QString& name) const;

    // Collection operations
    int count() const { return towns_.size(); }
    bool isEmpty() const { return towns_.isEmpty(); }
    void clear();

    QList<Town*> getAllTowns() const;
    QList<Town*> getTownsInArea(const QRect& area, const Map* map) const;

    // ID management
    quint32 getNextAvailableId() const;
    bool isIdAvailable(quint32 townId) const;
    QList<quint32> getUsedIds() const;

    // Validation
    bool validateTowns() const;
    QStringList getValidationErrors() const;
    void removeInvalidTowns();

    // Statistics
    QVariantMap getStatistics(const Map* map) const;
    int getTotalHouses(const Map* map) const;

    // XML I/O
    bool loadFromXml(const QString& filePath);
    bool saveToXml(const QString& filePath) const;
    bool loadFromXmlStream(QXmlStreamReader& xml);
    bool saveToXmlStream(QXmlStreamWriter& xml) const;

    // Iteration support
    QHash<quint32, Town*>::const_iterator begin() const { return towns_.begin(); }
    QHash<quint32, Town*>::const_iterator end() const { return towns_.end(); }
    QHash<quint32, Town*>::iterator begin() { return towns_.begin(); }
    QHash<quint32, Town*>::iterator end() { return towns_.end(); }

signals:
    void townAdded(Town* town);
    void townRemoved(quint32 townId);
    void townChanged(Town* town);
    void countChanged(int newCount);
    void townsCleared();

public slots:
    void onTownChanged();

private:
    QHash<quint32, Town*> towns_;

    void connectTownSignals(Town* town);
    void disconnectTownSignals(Town* town);
};

/**
 * @brief Task 86: Town utility functions
 */
class TownUtils
{
public:
    // Validation utilities
    static bool isValidTownName(const QString& name);
    static bool isValidTownId(quint32 id);
    static QString validateTownData(const Town* town);

    // Area utilities
    static QRect calculateBoundingRect(const QList<quint32>& houseIds, const Map* map);
    static MapPos calculateCenter(const QList<quint32>& houseIds, const Map* map);
    static int calculateTotalTiles(const QList<quint32>& houseIds, const Map* map);

    // Import/Export utilities
    static QString exportTownToString(const Town* town);
    static Town* importTownFromString(const QString& data, QObject* parent = nullptr);
    static bool exportTownsToFile(const Towns* towns, const QString& filePath);
    static bool importTownsFromFile(Towns* towns, const QString& filePath);

private:
    TownUtils() = default; // Static utility class
};

#endif // TOWN_H
