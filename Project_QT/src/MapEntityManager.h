#ifndef MAPENTITYMANAGER_H
#define MAPENTITYMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include "MapPos.h"

// Forward declarations
class Map;
class Spawn;
class House;
class Waypoint;
class Town;

/**
 * @brief MapEntityManager - Helper class for managing Map entities
 * 
 * This class extracts all entity management logic from Map.cpp to comply with mandate M6.
 * It handles spawns, houses, waypoints, and towns management operations.
 * 
 * Task 011: Refactor large source files - Extract entity management from Map.cpp
 */
class MapEntityManager : public QObject
{
    Q_OBJECT

public:
    explicit MapEntityManager(Map* map, QObject *parent = nullptr);
    ~MapEntityManager() override;

    // Spawn management
    void addSpawn(Spawn* spawn);
    void removeSpawn(Spawn* spawn);
    void removeSpawn(const MapPos& position);
    const QList<Spawn*>& getSpawns() const;
    Spawn* getSpawn(const MapPos& position) const;
    void clearSpawns();

    // House management
    void addHouse(House* house);
    void removeHouse(House* house);
    void removeHouse(quint32 houseId);
    const QList<House*>& getHouses() const;
    House* getHouse(quint32 houseId) const;
    House* getHouse(const QString& name) const;
    void clearHouses();

    // Waypoint management
    void addWaypoint(Waypoint* waypoint);
    void removeWaypoint(Waypoint* waypoint);
    void removeWaypoint(const QString& name);
    const QList<Waypoint*>& getWaypoints() const;
    Waypoint* getWaypoint(const QString& name) const;
    Waypoint* getWaypoint(const MapPos& position) const;
    void clearWaypoints();

    // Town management
    void addTown(Town* town);
    void removeTown(Town* town);
    void removeTown(quint32 townId);
    const QList<Town*>& getTowns() const;
    Town* getTown(quint32 townId) const;
    Town* getTown(const QString& name) const;
    void clearTowns();

    // Bulk operations
    void clearAllEntities();
    int getTotalEntityCount() const;

    // Entity validation
    bool validateSpawn(Spawn* spawn) const;
    bool validateHouse(House* house) const;
    bool validateWaypoint(Waypoint* waypoint) const;
    bool validateTown(Town* town) const;

    // Entity search and filtering
    QList<Spawn*> getSpawnsInArea(const MapPos& topLeft, const MapPos& bottomRight) const;
    QList<House*> getHousesInArea(const MapPos& topLeft, const MapPos& bottomRight) const;
    QList<Waypoint*> getWaypointsInArea(const MapPos& topLeft, const MapPos& bottomRight) const;

    // Entity statistics
    int getSpawnCount() const { return spawns_.size(); }
    int getHouseCount() const { return houses_.size(); }
    int getWaypointCount() const { return waypoints_.size(); }
    int getTownCount() const { return towns_.size(); }

signals:
    // Entity change signals
    void spawnAdded(Spawn* spawn);
    void spawnRemoved(Spawn* spawn);
    void houseAdded(House* house);
    void houseRemoved(House* house);
    void waypointAdded(Waypoint* waypoint);
    void waypointRemoved(Waypoint* waypoint);
    void townAdded(Town* town);
    void townRemoved(Town* town);

    // Bulk change signals
    void entitiesCleared();
    void entityCountChanged(int totalCount);

private slots:
    void onMapCleared();

private:
    Map* map_;
    
    // Entity storage
    QList<Spawn*> spawns_;
    QList<House*> houses_;
    QList<Waypoint*> waypoints_;
    QList<Town*> towns_;

    // General entity storage for polymorphic access
    QList<QObject*> entities_;

    // Helper methods
    void connectMapSignals();
    void disconnectMapSignals();
    bool isPositionValid(const MapPos& position) const;
    void emitEntityCountChanged();

    // Entity cleanup helpers
    void deleteSpawns();
    void deleteHouses();
    void deleteWaypoints();
    void deleteTowns();
};

#endif // MAPENTITYMANAGER_H
