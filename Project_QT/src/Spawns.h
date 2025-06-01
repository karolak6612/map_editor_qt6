#ifndef SPAWNS_H
#define SPAWNS_H

#include <QObject>
#include <QSet>
#include <QList>
#include "Map.h" // For MapPos

// Forward declarations
class Tile;
class Spawn;

/**
 * Spawns - Collection class for managing spawn positions on a map
 * This class tracks all spawn positions and provides methods for adding/removing spawns
 * Similar to the wxWidgets Spawns class but with Qt integration
 */
class Spawns : public QObject {
    Q_OBJECT

public:
    explicit Spawns(QObject* parent = nullptr);
    ~Spawns();

    // Spawn management
    void addSpawn(Tile* tile);
    void removeSpawn(Tile* tile);
    void addSpawn(const MapPos& position);
    void removeSpawn(const MapPos& position);
    
    // Query methods
    bool hasSpawn(const MapPos& position) const;
    bool hasSpawn(Tile* tile) const;
    int count() const;
    bool isEmpty() const;
    void clear();
    
    // Collection access
    QSet<MapPos> getSpawnPositions() const;
    QList<MapPos> getSpawnPositionsList() const;
    
    // Iterator support
    QSet<MapPos>::iterator begin();
    QSet<MapPos>::iterator end();
    QSet<MapPos>::const_iterator begin() const;
    QSet<MapPos>::const_iterator end() const;
    QSet<MapPos>::const_iterator find(const MapPos& position) const;
    
    // Utility methods
    void erase(QSet<MapPos>::iterator it);
    quint32 memsize() const;

signals:
    void spawnAdded(const MapPos& position);
    void spawnRemoved(const MapPos& position);
    void spawnsCleared();

private:
    QSet<MapPos> spawnPositions_;
};

#endif // SPAWNS_H
