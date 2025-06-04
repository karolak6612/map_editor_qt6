#include "MapEntityManager.h"
#include "Map.h"
#include "Spawn.h"
#include "House.h"
#include "Waypoint.h"
#include "Town.h"
#include <QDebug>

MapEntityManager::MapEntityManager(Map* map, QObject *parent)
    : QObject(parent)
    , map_(map)
{
    connectMapSignals();
}

MapEntityManager::~MapEntityManager()
{
    disconnectMapSignals();
    clearAllEntities();
}

// Spawn management
void MapEntityManager::addSpawn(Spawn* spawn)
{
    if (!spawn) {
        qWarning() << "MapEntityManager::addSpawn - Attempted to add a null spawn.";
        return;
    }

    if (!validateSpawn(spawn)) {
        qWarning() << "MapEntityManager::addSpawn - Spawn validation failed.";
        return;
    }

    if (!spawns_.contains(spawn)) {
        spawns_.append(spawn);
        emit spawnAdded(spawn);
        emitEntityCountChanged();
        qDebug() << "MapEntityManager::addSpawn - Spawn added at position" 
                 << spawn->position().x << "," << spawn->position().y << "," << spawn->position().z;
    } else {
        qDebug() << "MapEntityManager::addSpawn - Spawn already exists in the list.";
    }
}

void MapEntityManager::removeSpawn(Spawn* spawn)
{
    if (!spawn) {
        qWarning() << "MapEntityManager::removeSpawn - Attempted to remove a null spawn.";
        return;
    }

    if (spawns_.removeOne(spawn)) {
        emit spawnRemoved(spawn);
        emitEntityCountChanged();
        qDebug() << "MapEntityManager::removeSpawn - Spawn removed from map.";
        delete spawn;
    } else {
        qDebug() << "MapEntityManager::removeSpawn - Spawn not found in the list.";
    }
}

void MapEntityManager::removeSpawn(const MapPos& position)
{
    Spawn* spawn = getSpawn(position);
    if (spawn) {
        removeSpawn(spawn);
    }
}

const QList<Spawn*>& MapEntityManager::getSpawns() const
{
    return spawns_;
}

Spawn* MapEntityManager::getSpawn(const MapPos& position) const
{
    for (Spawn* spawn : spawns_) {
        if (spawn && spawn->position().x == position.x && 
            spawn->position().y == position.y && 
            spawn->position().z == position.z) {
            return spawn;
        }
    }
    return nullptr;
}

void MapEntityManager::clearSpawns()
{
    if (!spawns_.isEmpty()) {
        deleteSpawns();
        spawns_.clear();
        emitEntityCountChanged();
        qDebug() << "MapEntityManager::clearSpawns - All spawns cleared.";
    }
}

// House management
void MapEntityManager::addHouse(House* house)
{
    if (!house) {
        qWarning() << "MapEntityManager::addHouse - Attempted to add a null house.";
        return;
    }

    if (!validateHouse(house)) {
        qWarning() << "MapEntityManager::addHouse - House validation failed.";
        return;
    }

    // Check for duplicate house ID
    for (House* existingHouse : houses_) {
        if (existingHouse && existingHouse->getId() == house->getId()) {
            qWarning() << "MapEntityManager::addHouse - House with ID" << house->getId() << "already exists.";
            return;
        }
    }

    houses_.append(house);
    emit houseAdded(house);
    emitEntityCountChanged();
    qDebug() << "MapEntityManager::addHouse - House" << house->getName() << "ID:" << house->getId() << "added.";
}

void MapEntityManager::removeHouse(House* house)
{
    if (!house) {
        qWarning() << "MapEntityManager::removeHouse - Attempted to remove a null house.";
        return;
    }

    int index = houses_.indexOf(house);
    if (index != -1) {
        houses_.removeAt(index);
        emit houseRemoved(house);
        emitEntityCountChanged();
        qDebug() << "MapEntityManager::removeHouse - House" << house->getName() << "ID:" << house->getId() << "removed.";
        delete house;
    } else {
        qDebug() << "MapEntityManager::removeHouse - House not found in the list.";
    }
}

void MapEntityManager::removeHouse(quint32 houseId)
{
    House* house = getHouse(houseId);
    if (house) {
        removeHouse(house);
    }
}

const QList<House*>& MapEntityManager::getHouses() const
{
    return houses_;
}

House* MapEntityManager::getHouse(quint32 houseId) const
{
    for (House* house : houses_) {
        if (house && house->getId() == houseId) {
            return house;
        }
    }
    return nullptr;
}

House* MapEntityManager::getHouse(const QString& name) const
{
    for (House* house : houses_) {
        if (house && house->getName() == name) {
            return house;
        }
    }
    return nullptr;
}

void MapEntityManager::clearHouses()
{
    if (!houses_.isEmpty()) {
        deleteHouses();
        houses_.clear();
        emitEntityCountChanged();
        qDebug() << "MapEntityManager::clearHouses - All houses cleared.";
    }
}

// Waypoint management
void MapEntityManager::addWaypoint(Waypoint* waypoint)
{
    if (!waypoint) {
        qWarning() << "MapEntityManager::addWaypoint - Attempted to add a null waypoint.";
        return;
    }

    if (!validateWaypoint(waypoint)) {
        qWarning() << "MapEntityManager::addWaypoint - Waypoint validation failed.";
        return;
    }

    // Check for duplicate waypoint name
    if (getWaypoint(waypoint->getName())) {
        qWarning() << "MapEntityManager::addWaypoint - Waypoint with name" << waypoint->getName() << "already exists.";
        return;
    }

    waypoints_.append(waypoint);
    emit waypointAdded(waypoint);
    emitEntityCountChanged();
    qDebug() << "MapEntityManager::addWaypoint - Waypoint" << waypoint->getName() << "added.";
}

void MapEntityManager::removeWaypoint(Waypoint* waypoint)
{
    if (!waypoint) {
        qWarning() << "MapEntityManager::removeWaypoint - Attempted to remove a null waypoint.";
        return;
    }

    if (waypoints_.removeOne(waypoint)) {
        emit waypointRemoved(waypoint);
        emitEntityCountChanged();
        qDebug() << "MapEntityManager::removeWaypoint - Waypoint" << waypoint->getName() << "removed.";
        delete waypoint;
    } else {
        qDebug() << "MapEntityManager::removeWaypoint - Waypoint not found in the list.";
    }
}

void MapEntityManager::removeWaypoint(const QString& name)
{
    Waypoint* waypoint = getWaypoint(name);
    if (waypoint) {
        removeWaypoint(waypoint);
    }
}

const QList<Waypoint*>& MapEntityManager::getWaypoints() const
{
    return waypoints_;
}

Waypoint* MapEntityManager::getWaypoint(const QString& name) const
{
    for (Waypoint* waypoint : waypoints_) {
        if (waypoint && waypoint->getName() == name) {
            return waypoint;
        }
    }
    return nullptr;
}

Waypoint* MapEntityManager::getWaypoint(const MapPos& position) const
{
    for (Waypoint* waypoint : waypoints_) {
        if (waypoint && waypoint->getPosition().x == position.x && 
            waypoint->getPosition().y == position.y && 
            waypoint->getPosition().z == position.z) {
            return waypoint;
        }
    }
    return nullptr;
}

void MapEntityManager::clearWaypoints()
{
    if (!waypoints_.isEmpty()) {
        deleteWaypoints();
        waypoints_.clear();
        emitEntityCountChanged();
        qDebug() << "MapEntityManager::clearWaypoints - All waypoints cleared.";
    }
}

// Town management
void MapEntityManager::addTown(Town* town)
{
    if (!town) {
        qWarning() << "MapEntityManager::addTown - Attempted to add a null town.";
        return;
    }

    if (!validateTown(town)) {
        qWarning() << "MapEntityManager::addTown - Town validation failed.";
        return;
    }

    // Check for duplicate town ID
    for (Town* existingTown : towns_) {
        if (existingTown && existingTown->getId() == town->getId()) {
            qWarning() << "MapEntityManager::addTown - Town with ID" << town->getId() << "already exists.";
            return;
        }
    }

    towns_.append(town);
    emit townAdded(town);
    emitEntityCountChanged();
    qDebug() << "MapEntityManager::addTown - Town" << town->getName() << "ID:" << town->getId() << "added.";
}

void MapEntityManager::removeTown(Town* town)
{
    if (!town) {
        qWarning() << "MapEntityManager::removeTown - Attempted to remove a null town.";
        return;
    }

    if (towns_.removeOne(town)) {
        emit townRemoved(town);
        emitEntityCountChanged();
        qDebug() << "MapEntityManager::removeTown - Town" << town->getName() << "ID:" << town->getId() << "removed.";
        delete town;
    } else {
        qDebug() << "MapEntityManager::removeTown - Town not found in the list.";
    }
}

void MapEntityManager::removeTown(quint32 townId)
{
    Town* town = getTown(townId);
    if (town) {
        removeTown(town);
    }
}

const QList<Town*>& MapEntityManager::getTowns() const
{
    return towns_;
}

Town* MapEntityManager::getTown(quint32 townId) const
{
    for (Town* town : towns_) {
        if (town && town->getId() == townId) {
            return town;
        }
    }
    return nullptr;
}

Town* MapEntityManager::getTown(const QString& name) const
{
    for (Town* town : towns_) {
        if (town && town->getName() == name) {
            return town;
        }
    }
    return nullptr;
}

void MapEntityManager::clearTowns()
{
    if (!towns_.isEmpty()) {
        deleteTowns();
        towns_.clear();
        emitEntityCountChanged();
        qDebug() << "MapEntityManager::clearTowns - All towns cleared.";
    }
}

// Bulk operations
void MapEntityManager::clearAllEntities()
{
    clearSpawns();
    clearHouses();
    clearWaypoints();
    clearTowns();
    emit entitiesCleared();
    qDebug() << "MapEntityManager::clearAllEntities - All entities cleared.";
}

int MapEntityManager::getTotalEntityCount() const
{
    return spawns_.size() + houses_.size() + waypoints_.size() + towns_.size();
}

// Entity validation
bool MapEntityManager::validateSpawn(Spawn* spawn) const
{
    if (!spawn) return false;
    return isPositionValid(spawn->position());
}

bool MapEntityManager::validateHouse(House* house) const
{
    if (!house) return false;
    return house->getId() > 0 && !house->getName().isEmpty();
}

bool MapEntityManager::validateWaypoint(Waypoint* waypoint) const
{
    if (!waypoint) return false;
    return !waypoint->getName().isEmpty() && isPositionValid(waypoint->getPosition());
}

bool MapEntityManager::validateTown(Town* town) const
{
    if (!town) return false;
    return town->getId() > 0 && !town->getName().isEmpty();
}

// Entity search and filtering
QList<Spawn*> MapEntityManager::getSpawnsInArea(const MapPos& topLeft, const MapPos& bottomRight) const
{
    QList<Spawn*> result;
    for (Spawn* spawn : spawns_) {
        if (spawn) {
            const MapPos& pos = spawn->position();
            if (pos.x >= topLeft.x && pos.x <= bottomRight.x &&
                pos.y >= topLeft.y && pos.y <= bottomRight.y &&
                pos.z >= topLeft.z && pos.z <= bottomRight.z) {
                result.append(spawn);
            }
        }
    }
    return result;
}

QList<House*> MapEntityManager::getHousesInArea(const MapPos& topLeft, const MapPos& bottomRight) const
{
    QList<House*> result;
    // Note: Houses might need area checking based on their tiles, not just a single position
    // This is a simplified implementation
    for (House* house : houses_) {
        if (house) {
            // Assuming House has a getPosition() method or similar
            // This would need to be adapted based on actual House implementation
            result.append(house);
        }
    }
    return result;
}

QList<Waypoint*> MapEntityManager::getWaypointsInArea(const MapPos& topLeft, const MapPos& bottomRight) const
{
    QList<Waypoint*> result;
    for (Waypoint* waypoint : waypoints_) {
        if (waypoint) {
            const MapPos& pos = waypoint->getPosition();
            if (pos.x >= topLeft.x && pos.x <= bottomRight.x &&
                pos.y >= topLeft.y && pos.y <= bottomRight.y &&
                pos.z >= topLeft.z && pos.z <= bottomRight.z) {
                result.append(waypoint);
            }
        }
    }
    return result;
}

// Private slots
void MapEntityManager::onMapCleared()
{
    clearAllEntities();
}

// Private helper methods
void MapEntityManager::connectMapSignals()
{
    if (map_) {
        connect(map_, &Map::mapChanged, this, &MapEntityManager::onMapCleared);
    }
}

void MapEntityManager::disconnectMapSignals()
{
    if (map_) {
        disconnect(map_, nullptr, this, nullptr);
    }
}

bool MapEntityManager::isPositionValid(const MapPos& position) const
{
    if (!map_) return false;
    return map_->isCoordValid(position.x, position.y, position.z);
}

void MapEntityManager::emitEntityCountChanged()
{
    emit entityCountChanged(getTotalEntityCount());
}

// Entity cleanup helpers
void MapEntityManager::deleteSpawns()
{
    for (Spawn* spawn : spawns_) {
        delete spawn;
    }
}

void MapEntityManager::deleteHouses()
{
    for (House* house : houses_) {
        delete house;
    }
}

void MapEntityManager::deleteWaypoints()
{
    for (Waypoint* waypoint : waypoints_) {
        delete waypoint;
    }
}

void MapEntityManager::deleteTowns()
{
    for (Town* town : towns_) {
        delete town;
    }
}


