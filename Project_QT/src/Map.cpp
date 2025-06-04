#include "Map.h"
#include "MapIO.h"         // For I/O operations
#include "MapEntityManager.h" // Task 011: Extracted entity management
#include "Tile.h"
#include "Item.h"          // For Item objects on tiles
#include "Selection.h"
#include "Spawn.h"
#include "Waypoint.h"
#include "Waypoints.h"
#include "io/OtbmReader.h" // For OTBM reading logic
#include "io/OtbmWriter.h" // For OTBM writing logic
#include "OtbmTypes.h"     // For OTBM node and attribute types
#include "ItemManager.h"   // For ItemManager::getInstancePtr()
#include "Town.h"
#include "MapIterator.h"   // For MapIterator implementation
#include <QDebug>
#include <QSet>
#include <QVector3D>
#include <QReadLocker>     // For thread safety
#include <QWriteLocker>    // For thread safety
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>          // For qFloor function
#include <cmath>           // For std::floor as alternative

// Note: MapPos struct is assumed to be defined in Map.h as per previous step.
// If it were not, it would need to be defined here or included.

Map::Map(QObject *parent) : QObject(parent) {
    initialize(0, 0, 0);
    selection_ = new Selection(this);
    waypoints_ = new Waypoints(*this, this);
    mapIO_ = new MapIO(this, this);

    // Task 011: Initialize entity manager for mandate M6 compliance
    entityManager_ = new MapEntityManager(this, this);
    m_modified = false;
    m_otbmMajorVersion = 0;
    m_otbmMinorVersion = 0;
    m_otbmBuildVersion = 0;
    m_otbmVersionDescription = QString();
    m_externalSpawnFile = QString();
    m_externalHouseFile = QString();
    m_otbItemsMajorVersion = 0;
    m_otbItemsMinorVersion = 0;
}

Map::Map(int width, int height, int floors, const QString& description, QObject *parent) : QObject(parent) {
    initialize(width, height, floors, description);
    selection_ = new Selection(this);
    waypoints_ = new Waypoints(*this, this);
    mapIO_ = new MapIO(this, this);

    // Task 011: Initialize entity manager for mandate M6 compliance
    entityManager_ = new MapEntityManager(this, this);
    m_modified = false;
    m_otbmMajorVersion = 0;
    m_otbmMinorVersion = 0;
    m_otbmBuildVersion = 0;
    m_otbmVersionDescription = QString();
    m_externalSpawnFile = QString();
    m_externalHouseFile = QString();
    m_otbItemsMajorVersion = 0;
    m_otbItemsMinorVersion = 0;
}

Map::~Map() {
    // It's generally safer to delete objects that might depend on the map (like selection_)
    // *before* clearing the map's core data (like tiles_).
    delete selection_;
    selection_ = nullptr;
    delete waypoints_;
    waypoints_ = nullptr;
    delete mapIO_;
    mapIO_ = nullptr;

    // Task 011: Clean up entity manager
    delete entityManager_;
    entityManager_ = nullptr;
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

    m_modified = false;
    m_otbmMajorVersion = 0;
    m_otbmMinorVersion = 0;
    m_otbmBuildVersion = 0;
    m_otbmVersionDescription.clear();

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

    m_modified = false;
    m_otbmMajorVersion = 0;
    m_otbmMinorVersion = 0;
    m_otbmBuildVersion = 0;
    m_otbmVersionDescription.clear();

    qDeleteAll(m_towns);
    m_towns.clear();
    // qDeleteAll(m_waypoints); // This should be correct now
    // m_waypoints.clear(); // This should be correct now
    // The Waypoint related lists were modified in Map.h, ensure these point to m_waypoints
    // Actually, getWaypoints() in Map.cpp returns waypoints_ (old member)
    // This needs to be fixed if Map.h now uses m_waypoints for the public getter.
    // Map.h change: const QList<Waypoint*>& getWaypoints() const { return m_waypoints; }
    // So, Map.cpp should clear m_waypoints.
    // The old addWaypoint/removeWaypoint in Map.cpp also operate on waypoints_
    // These should be updated or removed if they are not used by OTBM loading yet.
    // For now, stick to the plan: clear m_towns and m_waypoints.
    // The old waypoints_ will be cleared by its own qDeleteAll(waypoints_) if it's still there.
    // Based on Map.h changes, waypoints_ is GONE. So operate on m_waypoints.
    qDeleteAll(m_waypoints);
    m_waypoints.clear();

    m_externalSpawnFile.clear();
    m_externalHouseFile.clear();
    m_otbItemsMajorVersion = 0;
    m_otbItemsMinorVersion = 0;

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
        setModified(true);
        emit mapChanged(); // Or a more specific signal like metadataChanged()
    }
}

// FIXED: Current floor tracking for brush operations
int Map::getCurrentFloor() const {
    return floors_ > 0 ? currentFloor_ : 7; // Default to ground level if no floors
}

void Map::setCurrentFloor(int floor) {
    if (floor >= 0 && floor < floors_) {
        currentFloor_ = floor;
    } else {
        qWarning() << "Map::setCurrentFloor: Invalid floor" << floor << "- valid range is 0 to" << (floors_ - 1);
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
    QReadLocker locker(&mapLock_);
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
    setModified(true);
    emit mapChanged();
    emit tileChanged(x, y, z);
    return true;
}

Tile* Map::createTile(int x, int y, int z) {
    QWriteLocker locker(&mapLock_);

    if (!isCoordValid(x, y, z)) {
        qWarning() << "createTile: Invalid coordinates (" << x << "," << y << "," << z << ")";
        return nullptr;
    }

    int index = getTileIndex(x, y, z);
    if (index >= tiles_.size()) { // Should not happen
         qWarning() << "createTile: Index out of bounds (" << index << ")";
         return nullptr;
    }

    bool hadTile = (tiles_.value(index) != nullptr);
    if (hadTile) { // Use .value for safety, though index should be valid
        delete tiles_[index];
        tiles_[index] = nullptr;
    }

    Tile* newTile = new Tile(x, y, z, this); // Pass coordinates and parent
    tiles_[index] = newTile;

    // Update tile count cache
    if (!hadTile) {
        ++tileCount_;
        tileCountDirty_ = false; // We updated the count manually
    }

    setModified(true);
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

// Task 52: Enhanced border update methods with Qt integration
void Map::requestBorderUpdate(const QPointF& tilePos) {
    QList<QVector3D> affectedTiles;

    int x = qFloor(tilePos.x());
    int y = qFloor(tilePos.y());
    int z = qFloor(tilePos.z());

    // Add the tile itself and its neighbors
    affectedTiles.append(QVector3D(x, y, z));

    // Add orthogonal neighbors
    const int dx[] = {0, 0, 1, -1};
    const int dy[] = {-1, 1, 0, 0};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (isCoordValid(nx, ny, z)) {
            affectedTiles.append(QVector3D(nx, ny, z));
        }
    }

    // Emit signals for Qt rendering system
    emit borderUpdateRequested(affectedTiles);
    emit tilesChanged(affectedTiles);

    qDebug() << "Map::requestBorderUpdate called for tile:" << tilePos
             << "affecting" << affectedTiles.size() << "tiles";
}

void Map::requestBorderUpdate(const QList<QPointF>& tilePositions) {
    if (tilePositions.isEmpty()) {
        return;
    }

    QList<QVector3D> affectedTiles;

    for (const QPointF& pos : tilePositions) {
        int x = qFloor(pos.x());
        int y = qFloor(pos.y());
        int z = qFloor(pos.z());

        // Add the tile itself
        affectedTiles.append(QVector3D(x, y, z));

        // Add orthogonal neighbors
        const int dx[] = {0, 0, 1, -1};
        const int dy[] = {-1, 1, 0, 0};

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (isCoordValid(nx, ny, z)) {
                QVector3D neighbor(nx, ny, z);
                if (!affectedTiles.contains(neighbor)) {
                    affectedTiles.append(neighbor);
                }
            }
        }
    }

    // Emit signals for Qt rendering system
    emit borderUpdateRequested(affectedTiles);
    emit tilesChanged(affectedTiles);

    qDebug() << "Map::requestBorderUpdate called for" << tilePositions.size()
             << "positions affecting" << affectedTiles.size() << "tiles";
}

void Map::requestBorderUpdate(const QRect& area) {
    QList<QVector3D> affectedTiles;

    // Process all tiles in the area plus a border around it
    QRect expandedArea = area.adjusted(-1, -1, 1, 1);

    for (int x = expandedArea.left(); x <= expandedArea.right(); ++x) {
        for (int y = expandedArea.top(); y <= expandedArea.bottom(); ++y) {
            for (int z = 0; z < floors_; ++z) {
                if (isCoordValid(x, y, z)) {
                    affectedTiles.append(QVector3D(x, y, z));
                }
            }
        }
    }

    // Emit signals for Qt rendering system
    emit borderUpdateRequested(affectedTiles);
    emit visualUpdateNeeded(expandedArea);

    qDebug() << "Map::requestBorderUpdate called for area:" << area
             << "affecting" << affectedTiles.size() << "tiles";
}

void Map::requestWallUpdate(const QPointF& tilePos) {
    Q_UNUSED(tilePos); // Mark as unused if no specific logic yet
    qDebug() << "Map::requestWallUpdate (placeholder) called for tile:" << tilePos;
    // This would trigger logic to check the walls on this tile and its neighbors
    // and update their appearance based on connections (e.g., changing wall item IDs).
    emit tileChanged(qFloor(tilePos.x()), qFloor(tilePos.y()), qFloor(tilePos.z()));
}


// Entity List Implementations - Task 011: Delegate to entity manager
void Map::addSpawn(Spawn* spawn) {
    if (entityManager_) {
        entityManager_->addSpawn(spawn);
        setModified(true);
        emit mapChanged();
    }
}

void Map::removeSpawn(Spawn* spawn) {
    if (entityManager_) {
        entityManager_->removeSpawn(spawn);
        setModified(true);
        emit mapChanged();
    }
}

const QList<Spawn*>& Map::getSpawns() const {
    if (entityManager_) {
        return entityManager_->getSpawns();
    }
    return spawns_; // Fallback to local list
}

void Map::addHouse(House* house) {
    if (entityManager_) {
        entityManager_->addHouse(house);
        setModified(true);
        emit mapChanged();
    }
}

void Map::removeHouse(House* house) {
    if (entityManager_) {
        entityManager_->removeHouse(house);
        setModified(true);
        emit mapChanged();
    }
}

const QList<House*>& Map::getHouses() const {
    if (entityManager_) {
        return entityManager_->getHouses();
    }
    return houses_; // Fallback to local list
}

void Map::addWaypoint(Waypoint* waypoint) {
    if (waypoint && waypoints_) {
        waypoints_->addWaypoint(waypoint);
        // Also add to legacy list for OTBM compatibility
        if (!m_waypoints.contains(waypoint)) {
            m_waypoints.append(waypoint);
        }
        setModified(true);
        emit waypointAdded(waypoint);
        emit waypointsChanged();
        emit mapChanged();
    } else {
        qWarning("Map::addWaypoint - Attempted to add null waypoint or waypoints collection not initialized.");
    }
}

void Map::removeWaypoint(Waypoint* waypoint) {
    if (waypoint && waypoints_) {
        QString waypointName = waypoint->name();
        // Remove from legacy list first
        m_waypoints.removeOne(waypoint);
        // Remove from waypoints collection (this will delete the waypoint)
        waypoints_->removeWaypoint(waypoint);
        setModified(true);
        emit waypointRemoved(waypoint);
        emit waypointRemoved(waypointName);
        emit waypointsChanged();
        emit mapChanged();
    } else {
        qWarning("Map::removeWaypoint - Attempted to remove null waypoint or waypoints collection not initialized.");
    }
}

void Map::removeWaypoint(const QString& name) {
    if (waypoints_) {
        Waypoint* waypoint = waypoints_->getWaypoint(name);
        if (waypoint) {
            removeWaypoint(waypoint);
        }
    }
}

Waypoint* Map::getWaypoint(const QString& name) const {
    if (waypoints_) {
        return waypoints_->getWaypoint(name);
    }
    return nullptr;
}

const QList<Waypoint*> Map::getWaypoints() const {
    if (waypoints_) {
        return waypoints_->getAllWaypoints();
    }
    return QList<Waypoint*>(); // Return empty list if waypoints collection not initialized
}

// Task 71: Additional waypoint methods for full functionality
Waypoint* Map::findWaypoint(const QString& name) const {
    return getWaypoint(name); // Delegate to existing method
}

Waypoint* Map::findWaypointAt(const MapPos& position) const {
    return findWaypointAt(position.x, position.y, position.z);
}

Waypoint* Map::findWaypointAt(int x, int y, int z) const {
    if (!waypoints_) {
        return nullptr;
    }

    QList<Waypoint*> allWaypoints = waypoints_->getAllWaypoints();
    for (Waypoint* waypoint : allWaypoints) {
        if (waypoint && waypoint->position().x == x &&
            waypoint->position().y == y && waypoint->position().z == z) {
            return waypoint;
        }
    }
    return nullptr;
}

QList<Waypoint*> Map::findWaypointsInArea(const QRect& area, int z) const {
    QList<Waypoint*> result;
    if (!waypoints_) {
        return result;
    }

    QList<Waypoint*> allWaypoints = waypoints_->getAllWaypoints();
    for (Waypoint* waypoint : allWaypoints) {
        if (waypoint && waypoint->position().z == z) {
            QPoint waypointPos(waypoint->position().x, waypoint->position().y);
            if (area.contains(waypointPos)) {
                result.append(waypoint);
            }
        }
    }
    return result;
}

bool Map::hasWaypoint(const QString& name) const {
    return findWaypoint(name) != nullptr;
}

bool Map::hasWaypointAt(const MapPos& position) const {
    return findWaypointAt(position) != nullptr;
}

int Map::getWaypointCount() const {
    if (waypoints_) {
        return waypoints_->getWaypointCount();
    }
    return 0;
}

void Map::clearWaypoints() {
    if (waypoints_) {
        waypoints_->clear();
        m_waypoints.clear();
        setModified(true);
        emit waypointsCleared();
        emit waypointsChanged();
        emit mapChanged();
    }
}

// Task 71: Waypoint validation and utilities
bool Map::isValidWaypointName(const QString& name) const {
    QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        return false;
    }

    // Check if name already exists (case-insensitive)
    return !hasWaypoint(trimmedName);
}

QString Map::generateUniqueWaypointName(const QString& baseName) const {
    QString base = baseName.isEmpty() ? "Waypoint" : baseName;
    QString candidateName = base;
    int counter = 1;

    while (hasWaypoint(candidateName)) {
        candidateName = QString("%1 %2").arg(base).arg(counter);
        counter++;
    }

    return candidateName;
}

QStringList Map::getWaypointNames() const {
    QStringList names;
    if (waypoints_) {
        QList<Waypoint*> allWaypoints = waypoints_->getAllWaypoints();
        for (Waypoint* waypoint : allWaypoints) {
            if (waypoint) {
                names.append(waypoint->name());
            }
        }
        names.sort(Qt::CaseInsensitive);
    }
    return names;
}

// Task 71: Waypoint navigation and interaction
bool Map::centerOnWaypoint(const QString& name) {
    Waypoint* waypoint = findWaypoint(name);
    return centerOnWaypoint(waypoint);
}

bool Map::centerOnWaypoint(Waypoint* waypoint) {
    if (!waypoint) {
        return false;
    }

    // This would typically emit a signal that the MapView can connect to
    // For now, we'll emit a custom signal that can be connected to the view
    emit waypointCenterRequested(waypoint);
    return true;
}

QList<Waypoint*> Map::getWaypointsInRadius(const MapPos& center, int radius) const {
    QList<Waypoint*> result;
    if (!waypoints_ || radius < 0) {
        return result;
    }

    QList<Waypoint*> allWaypoints = waypoints_->getAllWaypoints();
    for (Waypoint* waypoint : allWaypoints) {
        if (waypoint && waypoint->position().z == center.z) {
            int dx = waypoint->position().x - center.x;
            int dy = waypoint->position().y - center.y;
            int distance = qSqrt(dx * dx + dy * dy);

            if (distance <= radius) {
                result.append(waypoint);
            }
        }
    }
    return result;
}

#include <QFile>
#include <QFileInfo>
// QDataStream is already included for loadFromOTBM/saveToOTBM

// Task 51: Enhanced load/save with format detection
bool Map::load(const QString& path) {
    QString format = detectFileFormat(path);
    return loadByFormat(path, format);
}

QString Map::detectFileFormat(const QString& path) const {
    QString extension = QFileInfo(path).suffix().toLower();

    if (extension == "otbm") {
        return "otbm";
    } else if (extension == "xml") {
        return "xml";
    } else if (extension == "json") {
        return "json";
    } else {
        // Try to detect by content
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray header = file.read(16);
            file.close();

            if (header.startsWith("<?xml")) {
                return "xml";
            } else if (header.startsWith("{") || header.startsWith("[")) {
                return "json";
            } else {
                return "otbm"; // Default to OTBM for binary files
            }
        }
    }

    return "otbm"; // Default fallback
}

bool Map::loadByFormat(const QString& path, const QString& format) {
    if (!mapIO_) {
        qWarning() << "Map::loadByFormat - MapIO not initialized";
        return false;
    }
    return mapIO_->loadByFormat(path, format);
}

bool Map::save(const QString& path) const {
    QString format = detectFileFormat(path);
    return saveByFormat(path, format);
}

bool Map::saveByFormat(const QString& path, const QString& format) const {
    if (!mapIO_) {
        qWarning() << "Map::saveByFormat - MapIO not initialized";
        return false;
    }
    return mapIO_->saveByFormat(path, format);
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


bool Map::loadFromOTBM(QDataStream& stream) {
    if (!mapIO_) {
        qWarning() << "Map::loadFromOTBM - MapIO not initialized";
        return false;
    }
    return mapIO_->loadFromOTBM(stream);
}









bool Map::saveToOTBM(QDataStream& stream) const {
    if (!mapIO_) {
        qWarning() << "Map::saveToOTBM - MapIO not initialized";
        return false;
    }
    return mapIO_->saveToOTBM(stream);
}



// Task 51: XML Serialization Implementation
bool Map::loadFromXML(const QString& path) {
    if (!mapIO_) {
        qWarning() << "Map::loadFromXML - MapIO not initialized";
        return false;
    }
    return mapIO_->loadFromXML(path);
}

bool Map::saveToXML(const QString& path) const {
    if (!mapIO_) {
        qWarning() << "Map::saveToXML - MapIO not initialized";
        return false;
    }
    return mapIO_->saveToXML(path);
}

bool Map::loadFromJSON(const QString& path) {
    if (!mapIO_) {
        qWarning() << "Map::loadFromJSON - MapIO not initialized";
        return false;
    }
    return mapIO_->loadFromJSON(path);
}

bool Map::saveToJSON(const QString& path) const {
    if (!mapIO_) {
        qWarning() << "Map::saveToJSON - MapIO not initialized";
        return false;
    }
    return mapIO_->saveToJSON(path);
}



MapIterator Map::begin() {
    QWriteLocker locker(&mapLock_);
    return MapIterator(this, false);
}

MapIterator Map::end() {
    QWriteLocker locker(&mapLock_);
    return MapIterator(this, true);
}

ConstMapIterator Map::begin() const {
    QReadLocker locker(&mapLock_);
    return ConstMapIterator(this, false);
}

ConstMapIterator Map::end() const {
    QReadLocker locker(&mapLock_);
    return ConstMapIterator(this, true);
}

// --- Performance Utilities ---

quint64 Map::getTileCount() const {
    QReadLocker locker(&mapLock_);

    if (tileCountDirty_) {
        tileCount_ = 0;
        for (const Tile* tile : tiles_) {
            if (tile) {
                ++tileCount_;
            }
        }
        tileCountDirty_ = false;
    }
    return tileCount_;
}

Tile* Map::swapTile(int x, int y, int z, Tile* newTile) {
    QWriteLocker locker(&mapLock_);

    int index = getTileIndex(x, y, z);
    if (index == -1 || index >= tiles_.size()) {
        qWarning() << "Map::swapTile: Invalid coordinates or index" << x << y << z;
        return nullptr;
    }

    Tile* oldTile = tiles_[index];
    tiles_[index] = newTile;

    if (newTile) {
        newTile->x = x;
        newTile->y = y;
        newTile->z = z;
    }

    // Update tile count cache
    if (oldTile && !newTile) {
        --tileCount_;
    } else if (!oldTile && newTile) {
        ++tileCount_;
    }

    setModified(true);
    emit tileChanged(x, y, z);
    return oldTile;
}

Tile* Map::swapTile(const MapPos& pos, Tile* newTile) {
    return swapTile(pos.x, pos.y, pos.z, newTile);
}

// --- Advanced Tile Operations ---

void Map::clearTile(int x, int y, int z) {
    QWriteLocker locker(&mapLock_);

    int index = getTileIndex(x, y, z);
    if (index != -1 && index < tiles_.size() && tiles_[index]) {
        tiles_[index] = nullptr; // Clear without deleting
        --tileCount_;
        tileCountDirty_ = false; // We updated the count manually
        setModified(true);
        emit tileChanged(x, y, z);
    }
}

void Map::clearTile(const MapPos& pos) {
    clearTile(pos.x, pos.y, pos.z);
}

bool Map::hasTile(int x, int y, int z) const {
    QReadLocker locker(&mapLock_);
    return getTile(x, y, z) != nullptr;
}

bool Map::hasTile(const MapPos& pos) const {
    return hasTile(pos.x, pos.y, pos.z);
}

// --- Thread Safety Utilities ---

void Map::lockForReading() const {
    mapLock_.lockForRead();
}

void Map::lockForWriting() {
    mapLock_.lockForWrite();
}

void Map::unlock() const {
    mapLock_.unlock();
}

void Map::unlockWrite() {
    mapLock_.unlock();
}

// --- Map Cleanup and Optimization Utilities ---

quint32 Map::cleanDuplicateItems(const QVector<QPair<quint16, quint16>>& ranges) {
    QWriteLocker locker(&mapLock_);

    quint32 removedCount = 0;

    // Iterate through all tiles and remove duplicate items
    for (Tile* tile : tiles_) {
        if (!tile) continue;

        QVector<Item*>& items = tile->items();
        QSet<quint16> seenItemIds;

        for (int i = items.size() - 1; i >= 0; --i) {
            Item* item = items[i];
            if (!item) continue;

            quint16 itemId = item->getID();

            // Check if item ID is in specified ranges (if any)
            bool inRange = ranges.isEmpty();
            if (!inRange) {
                for (const auto& range : ranges) {
                    if (itemId >= range.first && itemId <= range.second) {
                        inRange = true;
                        break;
                    }
                }
            }

            if (inRange && seenItemIds.contains(itemId)) {
                // Remove duplicate item
                items.removeAt(i);
                delete item;
                ++removedCount;
            } else if (inRange) {
                seenItemIds.insert(itemId);
            }
        }
    }

    if (removedCount > 0) {
        setModified(true);
        emit mapChanged();
        qDebug() << "Map::cleanDuplicateItems: Removed" << removedCount << "duplicate items";
    }

    return removedCount;
}

void Map::optimizeMemory() {
    QWriteLocker locker(&mapLock_);

    // Compact tiles vector by removing trailing nulls
    while (!tiles_.isEmpty() && tiles_.last() == nullptr) {
        tiles_.removeLast();
    }

    // Squeeze to free unused capacity
    tiles_.squeeze();

    // Force tile count recalculation
    tileCountDirty_ = true;

    qDebug() << "Map::optimizeMemory: Memory optimization completed";
}

void Map::rebuildTileIndex() {
    QWriteLocker locker(&mapLock_);

    // Force recalculation of tile count
    tileCountDirty_ = true;
    getTileCount(); // This will recalculate

    qDebug() << "Map::rebuildTileIndex: Tile index rebuilt, count:" << tileCount_;
}

// --- QGraphicsView Integration Helpers ---

QVector<Tile*> Map::getTilesInRegion(const QRect& region, int floor) const {
    QReadLocker locker(&mapLock_);

    QVector<Tile*> result;

    // Clamp region to map bounds
    int startX = qMax(0, region.left());
    int endX = qMin(width_ - 1, region.right());
    int startY = qMax(0, region.top());
    int endY = qMin(height_ - 1, region.bottom());

    if (floor < 0 || floor >= floors_) {
        return result; // Invalid floor
    }

    // Collect tiles in the specified region
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            Tile* tile = getTile(x, y, floor);
            if (tile) {
                result.append(tile);
            }
        }
    }

    return result;
}

QVector<Tile*> Map::getTilesInRadius(const MapPos& center, int radius) const {
    QReadLocker locker(&mapLock_);

    QVector<Tile*> result;

    if (radius < 0) {
        return result; // Invalid radius
    }

    // Calculate bounding box for the radius
    int startX = qMax(0, center.x - radius);
    int endX = qMin(width_ - 1, center.x + radius);
    int startY = qMax(0, center.y - radius);
    int endY = qMin(height_ - 1, center.y + radius);

    if (center.z < 0 || center.z >= floors_) {
        return result; // Invalid floor
    }

    // Collect tiles within the radius
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            // Check if point is within circular radius
            int dx = x - center.x;
            int dy = y - center.y;
            if (dx * dx + dy * dy <= radius * radius) {
                Tile* tile = getTile(x, y, center.z);
                if (tile) {
                    result.append(tile);
                }
            }
        }
    }

    return result;
}

void Map::invalidateRegion(const QRect& region, int floor) {
    QReadLocker locker(&mapLock_);

    // Clamp region to map bounds
    int startX = qMax(0, region.left());
    int endX = qMin(width_ - 1, region.right());
    int startY = qMax(0, region.top());
    int endY = qMin(height_ - 1, region.bottom());

    if (floor < 0 || floor >= floors_) {
        return; // Invalid floor
    }

    // Emit tileChanged signals for all tiles in the region
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            emit tileChanged(x, y, floor);
        }
    }

    qDebug() << "Map::invalidateRegion: Invalidated region" << region << "on floor" << floor;
}

// Task 66: Enhanced house management implementation - moved to MapEntityManager

// Delegate house methods to entity manager
House* Map::getHouse(quint32 houseId) const {
    if (entityManager_) {
        return entityManager_->getHouse(houseId);
    }
    return nullptr;
}

void Map::removeHouse(quint32 houseId) {
    if (entityManager_) {
        entityManager_->removeHouse(houseId);
        setModified(true);
        emit mapChanged();
    }
}

void Map::clearHouses() {
    if (entityManager_) {
        entityManager_->clearHouses();
        setModified(true);
        emit mapChanged();
    }
}

quint32 Map::getNextHouseId() const {
    if (entityManager_) {
        quint32 maxId = 0;
        const QList<House*>& houses = entityManager_->getHouses();
        for (House* house : houses) {
            if (house && house->getId() > maxId) {
                maxId = house->getId();
            }
        }
        return maxId + 1;
    }
    return 1;
}

QList<MapPos> Map::getSelectedPositions() const {
    // Stub implementation
    if (selection_) {
        // Assuming Selection class has a method to get positions
        // For example, if Selection stores them as QSet<MapPos> or QList<MapPos>
        // return selection_->getSelectedTilesAsPositions(); // Hypothetical method
        // For now, let's assume Selection has a method like this:
        // return selection_->getPositions();
        qWarning("Map::getSelectedPositions() - actual implementation depends on Selection class internals.");
        return QList<MapPos>(); // Placeholder
    }
    qWarning("Map::getSelectedPositions() called but selection_ is null.");
    return QList<MapPos>();
}

void Map::deselectPosition(const MapPos& pos) {
    // Stub implementation
    if (selection_) {
        selection_->removeTile(pos); // Assuming Selection class has removeTile(MapPos)
        // emit selectionChanged(); // If Map has such a signal
    } else {
        qWarning("Map::deselectPosition(const MapPos&) called but selection_ is null.");
    }
    Q_UNUSED(pos); // Q_UNUSED is fine here as it's a stub
}

void Map::clearSelection() {
    // Stub implementation
    if (selection_) {
        selection_->clear();
        // emit selectionChanged(); // If Map has such a signal
    } else {
        qWarning("Map::clearSelection() called but selection_ is null.");
    }
}

void Map::selectPosition(const MapPos& pos) {
    // Stub implementation
    if (selection_) {
        selection_->addTile(pos); // Assuming Selection class has addTile(MapPos)
        // emit selectionChanged(); // If Map has such a signal
    } else {
        qWarning("Map::selectPosition(const MapPos&) called but selection_ is null.");
    }
    Q_UNUSED(pos); // Q_UNUSED is fine here as it's a stub
}

// Task 66: Enhanced town management implementation - moved to MapEntityManager

// Delegate town methods to entity manager
void Map::addTown(Town* town) {
    if (entityManager_) {
        entityManager_->addTown(town);
        setModified(true);
        emit mapChanged();
    }
}

void Map::removeTown(Town* town) {
    if (entityManager_) {
        entityManager_->removeTown(town);
        setModified(true);
        emit mapChanged();
    }
}

void Map::removeTown(quint32 townId) {
    if (entityManager_) {
        entityManager_->removeTown(townId);
        setModified(true);
        emit mapChanged();
    }
}

Town* Map::getTown(quint32 townId) const {
    if (entityManager_) {
        return entityManager_->getTown(townId);
    }
    return nullptr;
}

Town* Map::getTown(const QString& townName) const {
    if (entityManager_) {
        return entityManager_->getTown(townName);
    }
    return nullptr;
}

void Map::clearTowns() {
    if (entityManager_) {
        entityManager_->clearTowns();
        setModified(true);
        emit mapChanged();
    }
}

quint32 Map::getNextTownId() const {
    if (entityManager_) {
        quint32 maxId = 0;
        const QList<Town*>& towns = entityManager_->getTowns();
        for (Town* town : towns) {
            if (town && town->getId() > maxId) {
                maxId = town->getId();
            }
        }
        return maxId + 1;
    }
    return 1;
}
