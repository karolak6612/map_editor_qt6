#include "Spawns.h"
#include "Tile.h"
#include "Spawn.h"
#include <QDebug>

Spawns::Spawns(QObject* parent)
    : QObject(parent) {
    // Initialize empty spawn collection
}

Spawns::~Spawns() {
    clear();
}

void Spawns::addSpawn(Tile* tile) {
    if (!tile || !tile->getSpawn()) {
        qWarning() << "Spawns::addSpawn called with invalid tile or tile without spawn";
        return;
    }
    
    MapPos position = tile->getPosition();
    if (spawnPositions_.insert(position).second) {
        emit spawnAdded(position);
        qDebug() << "Spawn added at position" << position.x << position.y << position.z;
    }
}

void Spawns::removeSpawn(Tile* tile) {
    if (!tile) {
        qWarning() << "Spawns::removeSpawn called with null tile";
        return;
    }
    
    MapPos position = tile->getPosition();
    if (spawnPositions_.remove(position)) {
        emit spawnRemoved(position);
        qDebug() << "Spawn removed from position" << position.x << position.y << position.z;
    }
}

void Spawns::addSpawn(const MapPos& position) {
    if (spawnPositions_.insert(position).second) {
        emit spawnAdded(position);
        qDebug() << "Spawn added at position" << position.x << position.y << position.z;
    }
}

void Spawns::removeSpawn(const MapPos& position) {
    if (spawnPositions_.remove(position)) {
        emit spawnRemoved(position);
        qDebug() << "Spawn removed from position" << position.x << position.y << position.z;
    }
}

bool Spawns::hasSpawn(const MapPos& position) const {
    return spawnPositions_.contains(position);
}

bool Spawns::hasSpawn(Tile* tile) const {
    if (!tile) {
        return false;
    }
    return hasSpawn(tile->getPosition());
}

int Spawns::count() const {
    return spawnPositions_.size();
}

bool Spawns::isEmpty() const {
    return spawnPositions_.isEmpty();
}

void Spawns::clear() {
    if (!spawnPositions_.isEmpty()) {
        spawnPositions_.clear();
        emit spawnsCleared();
        qDebug() << "All spawns cleared";
    }
}

QSet<MapPos> Spawns::getSpawnPositions() const {
    return spawnPositions_;
}

QList<MapPos> Spawns::getSpawnPositionsList() const {
    return spawnPositions_.values();
}

// Iterator support
QSet<MapPos>::iterator Spawns::begin() {
    return spawnPositions_.begin();
}

QSet<MapPos>::iterator Spawns::end() {
    return spawnPositions_.end();
}

QSet<MapPos>::const_iterator Spawns::begin() const {
    return spawnPositions_.begin();
}

QSet<MapPos>::const_iterator Spawns::end() const {
    return spawnPositions_.end();
}

QSet<MapPos>::const_iterator Spawns::find(const MapPos& position) const {
    return spawnPositions_.find(position);
}

void Spawns::erase(QSet<MapPos>::iterator it) {
    if (it != spawnPositions_.end()) {
        MapPos position = *it;
        spawnPositions_.erase(it);
        emit spawnRemoved(position);
    }
}

quint32 Spawns::memsize() const {
    quint32 size = sizeof(Spawns);
    size += spawnPositions_.size() * sizeof(MapPos);
    return size;
}

#include "Spawns.moc"
