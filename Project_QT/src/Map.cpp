#include "Map.h"
#include "Tile.h" // Minimal Tile.h created in previous step
#include "Selection.h" // Include Selection.h
#include "Spawn.h" // Include Spawn.h for Spawn object details
#include "Waypoint.h" // Include Waypoint.h for Waypoint object details
#include <QDebug>
#include <QSet> // Include QSet for updateSelection
#include <QVector3D> // Used if MapPos was not defined in Map.h, but MapPos is used.
// #include <algorithm> // Not needed if using QList::removeOne

// Note: MapPos struct is assumed to be defined in Map.h as per previous step.
// If it were not, it would need to be defined here or included.

Map::Map(QObject *parent) : QObject(parent) {
    initialize(0, 0, 0);
    selection_ = new Selection(this);
}

Map::Map(int width, int height, int floors, const QString& description, QObject *parent) : QObject(parent) {
    initialize(width, height, floors, description);
    selection_ = new Selection(this);
}

Map::~Map() {
    // It's generally safer to delete objects that might depend on the map (like selection_)
    // *before* clearing the map's core data (like tiles_).
    delete selection_;
    selection_ = nullptr;
    clear();
}

void Map::initialize(int width, int height, int floors, const QString& description) {
    clear(); // Clear existing map data first

    width_ = width;
    height_ = height;
    floors_ = floors;
    description_ = description;

    if (width_ > 0 && height_ > 0 && floors_ > 0) {
        tiles_.resize(width_ * height_ * floors_);
        tiles_.fill(nullptr); // Initialize all tile pointers to nullptr
    }
    qDebug() << "Map initialized with dimensions:" << width_ << "x" << height_ << "x" << floors_;
    emit dimensionsChanged(width_, height_, floors_);
    emit mapChanged(); // Map structure changed
}

void Map::clear() {
    qDebug() << "Clearing map. Deleting" << tiles_.count() << "tile slots.";
    for (Tile* tile : tiles_) {
        if (tile) {
            delete tile;
        }
    }
    tiles_.clear();

    // For Spawns, Houses, Waypoints: If Map owns them, they should be deleted.
    // Consider using qDeleteAll for QList<T*> if ownership is established.
    qDeleteAll(spawns_); // Delete all Spawn objects
    spawns_.clear();
    qDeleteAll(houses_); // If Map owns House objects (assuming similar pattern for future)
    houses_.clear();
    qDeleteAll(waypoints_); // If Map owns Waypoint objects (assuming similar pattern for future)
    waypoints_.clear();

    width_ = 0;
    height_ = 0;
    floors_ = 0;
    description_.clear();

    emit mapChanged();
    emit dimensionsChanged(0, 0, 0);
}

int Map::width() const {
    return width_;
}

int Map::height() const {
    return height_;
}

int Map::floors() const {
    return floors_;
}

QString Map::description() const {
    return description_;
}

void Map::setDescription(const QString& description) {
    if (description_ != description) {
        description_ = description;
        emit mapChanged(); // Or a more specific signal like metadataChanged()
    }
}

bool Map::isCoordValid(int x, int y, int z) const {
    return x >= 0 && x < width_ && 
           y >= 0 && y < height_ && 
           z >= 0 && z < floors_;
}

int Map::getTileIndex(int x, int y, int z) const {
    if (!isCoordValid(x, y, z)) {
        return -1;
    }
    return (z * height_ + y) * width_ + x;
}

Tile* Map::getTile(int x, int y, int z) const {
    int index = getTileIndex(x, y, z);
    if (index != -1 && index < tiles_.size()) { // Check index < tiles_.size() for safety
        return tiles_.value(index); // QVector::value is safe for out-of-bounds
    }
    return nullptr;
}

Tile* Map::getTile(const MapPos& pos) const {
    return getTile(pos.x, pos.y, pos.z);
}

bool Map::setTile(int x, int y, int z, Tile* tile) {
    int index = getTileIndex(x, y, z);
    if (index == -1) {
        qWarning() << "setTile: Invalid coordinates (" << x << "," << y << "," << z << ")";
        if (tile) { // If a tile was passed but coords are invalid, caller might leak it.
            qWarning() << "setTile: Tile provided for invalid coordinates will not be managed by map.";
        }
        return false;
    }

    if (index >= tiles_.size()) { // Should not happen if getTileIndex uses isCoordValid properly
         qWarning() << "setTile: Index out of bounds (" << index << ")";
         return false;
    }

    if (tiles_[index] != nullptr && tiles_[index] != tile) {
        delete tiles_[index]; // Delete existing tile if different
    }
    tiles_[index] = tile; 

    if (tile) { // Update tile's own coordinates if it stores them
        tile->x = x;
        tile->y = y;
        tile->z = z;
    }

    emit mapChanged();
    emit tileChanged(x, y, z);
    return true;
}

Tile* Map::createTile(int x, int y, int z) {
    if (!isCoordValid(x, y, z)) {
        qWarning() << "createTile: Invalid coordinates (" << x << "," << y << "," << z << ")";
        return nullptr;
    }

    int index = getTileIndex(x, y, z);
    if (index >= tiles_.size()) { // Should not happen
         qWarning() << "createTile: Index out of bounds (" << index << ")";
         return nullptr;
    }

    if (tiles_.value(index) != nullptr) { // Use .value for safety, though index should be valid
        delete tiles_[index];
        tiles_[index] = nullptr;
    }

    Tile* newTile = new Tile(x, y, z, this); // Pass coordinates and parent
    tiles_[index] = newTile;

    emit mapChanged();
    emit tileChanged(x, y, z);
    return newTile;
}


// --- New methods for commands and brushes ---

Tile* Map::getOrCreateTile(const QPointF& pos) {
    // Assuming QPointF can represent 3D, or z comes from current floor context
    // For this stub, if QPointF doesn't have .z(), this will need adjustment or z=0.
    // The prompt uses qFloor(pos.z()) which implies pos.z() is meaningful.
    // If QPointF is strictly 2D in this project, MapView's current floor should be used for z.
    // For now, proceeding with pos.z(), assuming it's valid or will be adapted.
    return getOrCreateTile(qFloor(pos.x()), qFloor(pos.y()), qFloor(pos.z()));
}

Tile* Map::getOrCreateTile(int x, int y, int z) {
    if (!isCoordValid(x,y,z)) {
        qWarning() << "Map::getOrCreateTile: Invalid coordinates (" << x << "," << y << "," << z << ")";
        return nullptr;
    }
    Tile* tile = getTile(x, y, z);
    if (!tile) {
        // createTile handles adding to tiles_ vector, setting tile coordinates, and emitting signals.
        tile = createTile(x, y, z);
    }
    return tile;
}

void Map::removeTile(const QPointF& pos) {
    removeTile(qFloor(pos.x()), qFloor(pos.y()), qFloor(pos.z()));
}

void Map::removeTile(int x, int y, int z) {
    if (!isCoordValid(x,y,z)) {
        qWarning() << "Map::removeTile: Invalid coordinates (" << x << "," << y << "," << z << ")";
        return;
    }
    int index = getTileIndex(x,y,z);
    if (index != -1 && index < tiles_.size() && tiles_[index] != nullptr) {
        // Full removal logic depends on whether the tile must be empty first.
        // For now, this is a placeholder for what PlaceWallCommand might call in its undo.
        // A robust version would check tile->isEmpty() or similar.
        // delete tiles_[index]; // If map owns tiles and this means full deletion
        // tiles_[index] = nullptr;
        // emit tileChanged(x,y,z);
        // emit mapChanged();
        qDebug() << "Map::removeTile stub called for" << x << "," << y << "," << z << ". Actual removal logic (checking if empty, etc.) deferred.";
    } else {
         qDebug() << "Map::removeTile: No tile to remove at" << x << "," << y << "," << z << "or index invalid.";
    }
}

void Map::setGround(const QPointF& pos, quint16 groundItemId) {
    Tile* tile = getOrCreateTile(pos);
    if (tile) {
        // Tile class should handle the logic of creating/deleting Item objects for ground.
        tile->setGroundById(groundItemId); // Assumes Tile::setGroundById(quint16) will be created
        qDebug() << "Map::setGround called for tile" << pos << "with ID" << groundItemId;
        // Tile::setGroundById should emit its own tileChanged/visualChanged signals.
        // We emit map's tileChanged here to ensure views observing Map are notified.
        emit tileChanged(qFloor(pos.x()), qFloor(pos.y()), qFloor(pos.z()));
    } else {
        qWarning() << "Map::setGround: Could not get/create tile at" << pos;
    }
}

void Map::removeGround(const QPointF& pos) {
    // Use qFloor for converting QPointF to integer coordinates
    Tile* tile = getTile(qFloor(pos.x()), qFloor(pos.y()), qFloor(pos.z()));
    if (tile) {
        tile->removeGround(); // Assumes Tile::removeGround() will be created
        qDebug() << "Map::removeGround called for tile" << pos;
        // Tile::removeGround should emit its own signals.
        emit tileChanged(qFloor(pos.x()), qFloor(pos.y()), qFloor(pos.z()));
    } else {
        qWarning() << "Map::removeGround: Tile not found at" << pos << ". Nothing to remove.";
    }
}

void Map::requestBorderUpdate(const QPointF& tilePos) {
    Q_UNUSED(tilePos); // Mark as unused if no specific logic yet
    qDebug() << "Map::requestBorderUpdate (placeholder) called for tile:" << tilePos;
    // This would typically add tilePos and its orthogonal neighbors to a list
    // of tiles that need their border graphics recalculated.
    // For now, just emit a generic signal for the specific tile.
    emit tileChanged(qFloor(tilePos.x()), qFloor(tilePos.y()), qFloor(tilePos.z()));
}

void Map::requestWallUpdate(const QPointF& tilePos) {
    Q_UNUSED(tilePos); // Mark as unused if no specific logic yet
    qDebug() << "Map::requestWallUpdate (placeholder) called for tile:" << tilePos;
    // This would trigger logic to check the walls on this tile and its neighbors
    // and update their appearance based on connections (e.g., changing wall item IDs).
    emit tileChanged(qFloor(tilePos.x()), qFloor(tilePos.y()), qFloor(tilePos.z()));
}


// Entity List Implementations
void Map::addSpawn(Spawn* spawn) {
    if (spawn) {
        if (!spawns_.contains(spawn)) { // Optional: prevent duplicates if not desired
            spawns_.append(spawn);
            qDebug() << "Spawn added to map at position (" << spawn->position().x << "," << spawn->position().y << "," << spawn->position().z << ")";
            // emit spawnAdded(spawn); // Optional: more specific signal
            emit mapChanged();
        } else {
            qDebug() << "Map::addSpawn - Spawn already exists in the list.";
        }
    } else {
        qWarning() << "Map::addSpawn - Attempted to add a null spawn.";
    }
}

void Map::removeSpawn(Spawn* spawn) {
    if (spawn) {
        if (spawns_.removeOne(spawn)) { // removeOne returns true if item was found and removed
            qDebug() << "Spawn removed from map and deleted.";
            delete spawn; // Delete the spawn object as the map owns it
            // emit spawnRemoved(spawn); // Optional: more specific signal
            emit mapChanged();
        } else {
            qDebug() << "Map::removeSpawn - Spawn not found in the list.";
        }
    } else {
        qWarning() << "Map::removeSpawn - Attempted to remove a null spawn.";
    }
}

const QList<Spawn*>& Map::getSpawns() const {
    return spawns_;
}

void Map::addHouse(House* house) {
    if (house) {
        houses_.append(house);
        qDebug() << "House added (stub)";
        emit mapChanged();
    }
}

void Map::removeHouse(House* house) {
    if (houses_.removeOne(house)) {
        qDebug() << "House removed (stub)";
        emit mapChanged();
    }
}

const QList<House*>& Map::getHouses() const {
    return houses_;
}

void Map::addWaypoint(Waypoint* waypoint) {
    if (waypoint) {
        if (!waypoints_.contains(waypoint)) { // Optional: prevent duplicates
            waypoints_.append(waypoint);
            qDebug() << "Waypoint" << waypoint->name() << "added to map at position (" << waypoint->position().x << "," << waypoint->position().y << "," << waypoint->position().z << ")";
            // emit waypointAdded(waypoint); // Optional: more specific signal
            emit mapChanged();
        } else {
            qWarning() << "Map::addWaypoint - Waypoint" << waypoint->name() << "already exists.";
        }
    } else {
        qWarning("Map::addWaypoint - Attempted to add null waypoint.");
    }
}

void Map::removeWaypoint(Waypoint* waypoint) {
    if (waypoint) {
        if (waypoints_.removeOne(waypoint)) { // removeOne returns true if item was found and removed
            qDebug() << "Waypoint" << waypoint->name() << "removed from map and deleted.";
            delete waypoint; // Map owns the waypoint
            // emit waypointRemoved(waypoint); // Optional: more specific signal
            emit mapChanged();
        } else {
            qWarning() << "Map::removeWaypoint - Waypoint" << waypoint->name() << "not found in map.";
        }
    } else {
        qWarning("Map::removeWaypoint - Attempted to remove null waypoint.");
    }
}

const QList<Waypoint*>& Map::getWaypoints() const {
    return waypoints_;
}

// Load/Save Stubs
bool Map::load(const QString& path) {
    qDebug() << "Map::load not implemented, path:" << path;
    // Placeholder: clear current map and emit signals
    // initialize(defaultWidth, defaultHeight, defaultFloors, "Loaded Map Placeholder");
    return false;
}

bool Map::save(const QString& path) const {
    qDebug() << "Map::save not implemented, path:" << path;
    return false;
}

// Selection method implementations
Selection* Map::getSelection() const {
    return selection_;
}

void Map::updateSelection(const QSet<MapPos>& newSelection) {
    if (selection_) {
        selection_->clear();
        for (const MapPos& pos : newSelection) {
            selection_->addTile(pos);
        }
        // Optionally, emit a signal here if Map has a selectionChanged signal
        // emit selectionChangedSignal(); // Example
        qDebug() << "Map selection updated with" << newSelection.count() << "tiles.";
    } else {
        qWarning("Map::updateSelection called but selection_ is null.");
    }
}
