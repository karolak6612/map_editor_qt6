#include "Map.h"
#include "Tile.h" // Minimal Tile.h created in previous step
#include "Selection.h" // Include Selection.h
#include <QDebug>
#include <QSet> // Include QSet for updateSelection
#include <QVector3D> // Used if MapPos was not defined in Map.h, but MapPos is used.

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
    // For now, as per instruction, just clearing the lists.
    // Consider using qDeleteAll for QList<T*> if ownership is established.
    // qDeleteAll(spawns_); // If Map owns Spawn objects
    spawns_.clear();
    // qDeleteAll(houses_); // If Map owns House objects
    houses_.clear();
    // qDeleteAll(waypoints_); // If Map owns Waypoint objects
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

// Entity List Stubs
void Map::addSpawn(Spawn* spawn) {
    if (spawn) {
        spawns_.append(spawn);
        qDebug() << "Spawn added (stub)";
        emit mapChanged();
    }
}

void Map::removeSpawn(Spawn* spawn) {
    if (spawns_.removeOne(spawn)) { // removeOne returns true if item was removed
        qDebug() << "Spawn removed (stub)";
        // Note: Does not delete spawn, just removes from list. Caller manages memory if needed.
        emit mapChanged();
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
        waypoints_.append(waypoint);
        qDebug() << "Waypoint added (stub)";
        emit mapChanged();
    }
}

void Map::removeWaypoint(Waypoint* waypoint) {
    if (waypoints_.removeOne(waypoint)) {
        qDebug() << "Waypoint removed (stub)";
        emit mapChanged();
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
