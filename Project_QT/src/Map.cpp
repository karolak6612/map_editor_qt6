#include "Map.h"
#include "Tile.h"
#include "Item.h"          // For Item objects on tiles
#include "Selection.h"
#include "Spawn.h"
#include "Waypoint.h"
#include "io/OtbmReader.h" // For OTBM reading logic
#include "io/OtbmWriter.h" // For OTBM writing logic
#include "OtbmTypes.h"     // For OTBM node and attribute types
#include "ItemManager.h"   // For ItemManager::getInstancePtr()
#include "Town.h"
#include "Waypoint.h" // Ensure Waypoint.h is included for QList<Waypoint*>
#include <QDebug>
#include <QSet>
#include <QVector3D>
// #include <algorithm>

// Note: MapPos struct is assumed to be defined in Map.h as per previous step.
// If it were not, it would need to be defined here or included.

Map::Map(QObject *parent) : QObject(parent) {
    initialize(0, 0, 0);
    selection_ = new Selection(this);
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
    setModified(true);
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
            setModified(true);
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
            setModified(true);
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
        setModified(true);
        emit mapChanged();
    }
}

void Map::removeHouse(House* house) {
    if (houses_.removeOne(house)) {
        qDebug() << "House removed (stub)";
        setModified(true);
        emit mapChanged();
    }
}

const QList<House*>& Map::getHouses() const {
    return houses_;
}

void Map::addWaypoint(Waypoint* waypoint) {
    if (waypoint) {
        if (!m_waypoints.contains(waypoint)) { // Optional: prevent duplicates
            m_waypoints.append(waypoint);
            qDebug() << "Waypoint" << waypoint->name() << "added to map at position (" << waypoint->position().x << "," << waypoint->position().y << "," << waypoint->position().z << ")";
            setModified(true);
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
        if (m_waypoints.removeOne(waypoint)) { // removeOne returns true if item was found and removed
            qDebug() << "Waypoint" << waypoint->name() << "removed from map and deleted.";
            delete waypoint; // Map owns the waypoint
            setModified(true);
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
    // This const method should now refer to m_waypoints as per Map.h
    // The Map.h has: const QList<Waypoint*>& getWaypoints() const { return m_waypoints; }
    // So, this implementation in Map.cpp might be redundant if it was just returning the member.
    // However, if Map.h only *declares* it and Map.cpp *defines* it, this change is needed.
    // Looking at Map.h, the getter is defined inline. This means this Map.cpp version of getWaypoints()
    // is either dead code or there's a mix-up.
    // For safety, I will update this one too, though it might be shadowed by an inline getter in the header.
    return m_waypoints;
}

#include <QFile>
#include <QFileInfo>
// QDataStream is already included for loadFromOTBM/saveToOTBM

// Load/Save Stubs
bool Map::load(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Map::load - Could not open file for reading:" << path << "Error:" << file.errorString();
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian); // Ensure consistent byte order for initial checks if any, or for reader

    qDebug() << "Map::load - Attempting to load from OTBM file:" << path;
    bool success = loadFromOTBM(stream); // Delegate to the new method

    if (success) {
        qDebug() << "Map::load - Successfully loaded from OTBM file:" << path;
    } else {
        qWarning() << "Map::load - Failed to load from OTBM file:" << path << "Stream status:" << stream.status();
    }

    file.close();
    return success;
}

bool Map::save(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Map::save - Could not open file for writing:" << path << "Error:" << file.errorString();
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian); // Ensure consistent byte order

    qDebug() << "Map::save - Attempting to save to OTBM file:" << path;
    bool success = saveToOTBM(stream); // Delegate to the new method

    if (success) {
        qDebug() << "Map::save - Successfully saved to OTBM file:" << path;
    } else {
        qWarning() << "Map::save - Failed to save to OTBM file:" << path << "Stream status:" << stream.status();
    }

    file.close();
    return success;
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
    clear(); // Clear existing map data

    OtbmReader reader(stream);
    ItemManager* itemManager = ItemManager::getInstancePtr(); // Get instance for item creation

    quint8 rootNodeType;
    if (!reader.enterNode(rootNodeType)) {
        qWarning() << "Map::loadFromOTBM - Could not enter root node.";
        return false;
    }

    if (rootNodeType != OTBM_ROOTV1) {
        qWarning() << "Map::loadFromOTBM - Root node type is not OTBM_ROOTV1. Got:" << rootNodeType;
        reader.leaveNode();
        return false;
    }
    qDebug() << "Map::loadFromOTBM - Entered OTBM_ROOTV1 node.";

    // Read attributes of ROOTV1 node
    quint8 rootAttrId;
    while(reader.nextAttributeId(rootAttrId)) {
        quint16 rootAttrDataLen;
        if (!(reader.stream() >> rootAttrDataLen)) {
            qWarning() << "Map::loadFromOTBM - Failed to read data length for ROOTV1 attribute" << rootAttrId;
            reader.leaveNode(); return false;
        }
        QByteArray rootAttrData = reader.readData(rootAttrDataLen);
        if (reader.stream().status() != QDataStream::Ok && rootAttrDataLen > 0) {
            qWarning() << "Map::loadFromOTBM - Failed to read data for ROOTV1 attribute" << rootAttrId;
            reader.leaveNode(); return false;
        }

        QDataStream valueStream(rootAttrData);
        valueStream.setByteOrder(QDataStream::LittleEndian);

       switch (static_cast<OTBM_RootAttribute>(rootAttrId)) { // Cast to new enum
           case OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_MAJOR:
               if (rootAttrDataLen == sizeof(quint32)) valueStream >> m_otbmMajorVersion;
               else qWarning() << "Map::loadFromOTBM - Incorrect data length for OTBM_ROOT_ATTR_VERSION_MAJOR";
               qDebug() << "OTBM Major Version:" << m_otbmMajorVersion;
               break;
           case OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_MINOR:
               if (rootAttrDataLen == sizeof(quint32)) valueStream >> m_otbmMinorVersion;
               else qWarning() << "Map::loadFromOTBM - Incorrect data length for OTBM_ROOT_ATTR_VERSION_MINOR";
               qDebug() << "OTBM Minor Version:" << m_otbmMinorVersion;
               break;
           case OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_BUILD:
               if (rootAttrDataLen == sizeof(quint32)) valueStream >> m_otbmBuildVersion; // Assuming build is also quint32
               else qWarning() << "Map::loadFromOTBM - Incorrect data length for OTBM_ROOT_ATTR_VERSION_BUILD";
               qDebug() << "OTBM Build Version:" << m_otbmBuildVersion;
               break;
           case OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_DESC_STRING:
               m_otbmVersionDescription = QString::fromUtf8(rootAttrData);
               qDebug() << "OTBM Version Description:" << m_otbmVersionDescription;
                // TODO (Task51): Implement client version specific logic.
                // Based on m_otbmMajorVersion, m_otbmMinorVersion, m_otbmBuildVersion,
                // some map properties or item interpretations might need adjustment.
                // For example, if (m_otbmMajorVersion < X) { /* handle old format */ }
               break;
           default:
               qDebug() << "Map::loadFromOTBM - Skipped unknown attribute" << rootAttrId << "in ROOTV1 node.";
               // reader.skipBytes(rootAttrDataLen); // Already consumed by readData
               break;
       }
    }

    qDebug() << "Map::loadFromOTBM - Finished reading ROOTV1 attributes. Attempting to read map dimensions and OTB item versions.";

    quint16 loadedMapWidth, loadedMapHeight;
    if (!(reader.stream() >> loadedMapWidth)) {
        qWarning() << "Map::loadFromOTBM - Failed to read map width from root node stream.";
        reader.leaveNode(); // Leave ROOTV1
        return false;
    }
    this->width_ = loadedMapWidth; // Set map width directly

    if (!(reader.stream() >> loadedMapHeight)) {
        qWarning() << "Map::loadFromOTBM - Failed to read map height from root node stream.";
        reader.leaveNode(); // Leave ROOTV1
        return false;
    }
    this->height_ = loadedMapHeight; // Set map height directly

    if (!(reader.stream() >> m_otbItemsMajorVersion)) {
        qWarning() << "Map::loadFromOTBM - Failed to read OTB items major version from root node stream.";
        reader.leaveNode(); // Leave ROOTV1
        return false;
    }
    qDebug() << "Map::loadFromOTBM - Read OTB Items Major Version:" << m_otbItemsMajorVersion;

    if (!(reader.stream() >> m_otbItemsMinorVersion)) {
        qWarning() << "Map::loadFromOTBM - Failed to read OTB items minor version from root node stream.";
        reader.leaveNode(); // Leave ROOTV1
        return false;
    }
    qDebug() << "Map::loadFromOTBM - Read OTB Items Minor Version:" << m_otbItemsMinorVersion;

    // Note: Map floors are not read here; they are determined by TILE_AREA nodes.
    // The tiles_ vector in Map needs to be able to accommodate tiles from any floor.
    // A common approach is to resize tiles_ dynamically as new floors are encountered
    // or pre-allocate to a max number of floors (e.g. 16) and then trim if necessary.
    // The current Map::initialize (called by constructors) sets up tiles_ with initial dimensions.
    // Since clear() is called at the start of loadFromOTBM, width_ and height_ are reset.
    // We've now set width_ and height_. The floors_ member and tiles_ resizing needs to be handled
    // correctly before or during TILE_AREA processing. For now, we assume this is handled.

    // TODO (Task51-MapVer): If there were fundamental changes to OTBM_MAP_DATA,
    // OTBM_TILE_AREA, or OTBM_TILE node structures in different OTBM versions,
    // conditional parsing logic based on m_otbmMajorVersion (map format version)
    // would be needed here. Current parsing assumes a generally stable structure
    // for these core nodes across versions handled.

    quint8 mapDataNodeType;
    if (!reader.enterNode(mapDataNodeType)) {
        qWarning() << "Map::loadFromOTBM - Could not enter MAP_DATA node.";
        reader.leaveNode(); // Leave ROOTV1
        return false;
    }

    if (mapDataNodeType != OTBM_MAP_DATA) {
        qWarning() << "Map::loadFromOTBM - Expected OTBM_MAP_DATA node, got:" << mapDataNodeType;
        reader.leaveNode(); // Leave MAP_DATA (or whatever it was)
        reader.leaveNode(); // Leave ROOTV1
        return false;
    }
    qDebug() << "Map::loadFromOTBM - Entered OTBM_MAP_DATA node.";

    QString mapDescription;
    // These are not typically set at map level in OTBM but rather discovered
    // For now, we'll use them if present, otherwise derive from tile areas.
    quint16 initialMapWidth = 0, initialMapHeight = 0, initialMapFloors = 0;

    quint8 mapAttrId;
    while(reader.nextAttributeId(mapAttrId)) {
        quint16 mapAttrDataLen;
        if (!(reader.stream() >> mapAttrDataLen)) {
             qWarning() << "Map::loadFromOTBM - Failed to read data length for MAP_DATA attribute" << mapAttrId;
             break;
        }
        QByteArray mapAttrData = reader.readData(mapAttrDataLen);
        if (reader.stream().status() != QDataStream::Ok && mapAttrDataLen > 0) {
            qWarning() << "Map::loadFromOTBM - Failed to read data for MAP_DATA attribute" << mapAttrId;
            break;
        }
        QDataStream mapValueStream(mapAttrData);
        mapValueStream.setByteOrder(QDataStream::LittleEndian);

        switch (mapAttrId) {
            case OTBM_ATTR_DESCRIPTION:
                mapDescription = QString::fromUtf8(mapAttrData);
                qDebug() << "Map Description:" << mapDescription;
                break;
            case OTBM_ATTR_EXT_SPAWN_FILE:
                m_externalSpawnFile = QString::fromUtf8(mapAttrData);
                qDebug() << "External Spawn File:" << m_externalSpawnFile;
                break;
            case OTBM_ATTR_EXT_HOUSE_FILE:
                m_externalHouseFile = QString::fromUtf8(mapAttrData);
                qDebug() << "External House File:" << m_externalHouseFile;
                break;
            // OTBM_ATTR_MAP_WIDTH, OTBM_ATTR_MAP_HEIGHT etc. are not standard OTBM attributes for MAP_DATA.
            // Dimensions are typically derived from TILE_AREA nodes.
            default:
                qDebug() << "Map::loadFromOTBM - Skipping MAP_DATA attribute" << mapAttrId;
                break;
        }
    }
    setDescription(mapDescription); // Set map description if read

    // The map dimensions will be determined by the extent of TILE_AREA nodes.
    // We might need to do a first pass to find all TILE_AREAs and determine bounds,
    // then initialize the map, then do a second pass to populate tiles.
    // For simplicity in this step, we'll initialize/expand the map as we find areas.
    // This is not ideal for performance or correctness if areas are not ordered.
    // A better approach is to collect all area data first or use min/max extents.
    // For now, we'll assume initialize(0,0,0) was called and expand as needed.
    // This part is complex and will be simplified for this initial implementation.
    // Actual robust dimension handling needs pre-scan or dynamic resizing.
    // Let's assume for now the map is pre-sized or we handle it very simply.
    // If map is not pre-sized, initialize with some default or first area's size.
    // The current `initialize` clears the map, so it should be called once with final dimensions.
    // This will be a placeholder: actual dimensions will be determined by TILE_AREAs.
    // For now, let's assume the map is large enough or will be resized later.

    quint8 nodeType;
    while (reader.enterNode(nodeType)) {
        if (nodeType == OTBM_TILE_AREA) {
            quint16 areaBaseX, areaBaseY;
            quint8 areaBaseZ;
            reader.stream() >> areaBaseX >> areaBaseY >> areaBaseZ;
            if (reader.stream().status() != QDataStream::Ok) {
                qWarning() << "Map::loadFromOTBM - Failed to read TILE_AREA coordinates.";
                reader.leaveNode(); break;
            }
            qDebug() << "Map::loadFromOTBM - Reading TILE_AREA at" << areaBaseX << areaBaseY << areaBaseZ;

            // Placeholder: Ensure map is large enough. This is where dynamic resizing or pre-scan would be needed.
            // For example: updateMapDimensions(areaBaseX + 255, areaBaseY + 255, areaBaseZ);
            if (!isCoordValid(areaBaseX, areaBaseY, areaBaseZ) ||
                !isCoordValid(areaBaseX + 255, areaBaseY + 255, areaBaseZ)) { // Rough check
                // If map has not been initialized with proper dimensions yet.
                // This is where a pre-scan to determine overall map dimensions would be crucial.
                // For this step, we'll assume initialize() was called or we are just logging.
                qWarning() << "Map::loadFromOTBM - TILE_AREA exceeds current map bounds or map not initialized. Area:" << areaBaseX << areaBaseY << areaBaseZ;
                // To continue robustly, we'd need to skip this entire area's content.
                // This requires skipping all its child tiles and their items.
                // For now, we might fail or try to process if in bounds.
                // Let's assume the map was initialized to a default or from header if those attributes existed.
                // If not, this will likely try to access out of bounds.
                // The provided Map::initialize clears everything, so it must be called once with final dimensions.
            }


            quint8 tileNodeType;
            while(reader.enterNode(tileNodeType)) {
                if (tileNodeType == OTBM_TILE || tileNodeType == OTBM_HOUSETILE) {
                    quint8 relX, relY;
                    reader.stream() >> relX >> relY; // Tile relative coordinates
                    if (reader.stream().status() != QDataStream::Ok) {
                        qWarning() << "Map::loadFromOTBM - Failed to read TILE relative coordinates.";
                        reader.leaveNode(); break;
                    }

                    MapPos currentTilePos(areaBaseX + relX, areaBaseY + relY, areaBaseZ);
                    // Ensure map is large enough before calling getOrCreateTile.
                    // This is a critical part that needs robust dimension handling.
                    // For now, if map wasn't pre-initialized to full size, getOrCreateTile might fail or try to access out of bounds.
                    if (!isCoordValid(currentTilePos.x, currentTilePos.y, currentTilePos.z)) {
                         qWarning() << "Map::loadFromOTBM - Tile coordinates" << currentTilePos.x << currentTilePos.y << currentTilePos.z << "are out of map bounds. Skipping tile.";
                         // We need to skip this tile's attributes and items. This is complex.
                         // For now, we'll just leave the node and continue, which might lead to errors.
                         // A robust skip involves parsing until OTBM_NODE_END.
                         reader.leaveNode(); // Attempt to leave this tile node
                         continue; // Skip to next tile node in area
                    }

                    Tile* tile = getOrCreateTile(currentTilePos.x, currentTilePos.y, currentTilePos.z);
                    if (!tile) {
                        qWarning() << "Map::loadFromOTBM - Failed to get/create tile at" << currentTilePos.x << currentTilePos.y << currentTilePos.z;
                        reader.leaveNode(); break;
                    }
                    if (tileNodeType == OTBM_HOUSETILE) tile->setHouseTile(true);
                    // tile->setModified(false); // Tile reflects persistent state - This needs to be after attributes/items


                    quint8 tileAttrId;
                    while(reader.nextAttributeId(tileAttrId)) {
                        quint16 tileAttrDataLen;
                        if (!(reader.stream() >> tileAttrDataLen)) { /* error */ break; }
                        QByteArray tileAttrData = reader.readData(tileAttrDataLen);
                        QDataStream tileValueStream(tileAttrData);
                        tileValueStream.setByteOrder(QDataStream::LittleEndian);

                        if (tileAttrId == OTBM_ATTR_TILE_FLAGS) {
                            quint32 flags = 0;
                            if (tileAttrDataLen == sizeof(quint32)) tileValueStream >> flags; else qWarning("Incorrect TILE_FLAGS length");
                            tile->setMapFlagsValue(flags);
                        } else if (tileAttrId == OTBM_ATTR_HOUSEDOORID && tileNodeType == OTBM_HOUSETILE) {
                            quint8 houseDoorId = 0;
                            if (tileAttrDataLen == sizeof(quint8)) tileValueStream >> houseDoorId; else qWarning("Incorrect HOUSEDOORID length");
                            tile->setHouseDoorId(houseDoorId);
                        } else {
                             qDebug() << "Map::loadFromOTBM - Skipping TILE/HOUSETILE attribute" << tileAttrId;
                        }
                    }

                    quint8 itemNodeType;
                    while(reader.enterNode(itemNodeType)) {
                         if (itemNodeType == OTBM_ITEM) {
                            // Pass client version to item reading if ItemManager or Item needs it
                            Item* item = reader.readItem(itemManager, m_otbItemsMajorVersion, m_otbItemsMinorVersion); // NEW
                            // TODO (Task51): If item deserialization is version-dependent, OtbmReader::readItem
                            // might need to accept version parameters, or Item::unserializeOtbmAttributes
                            // might need to access the map's version via its parent or a global accessor.
                            if (item) {
                                tile->addItem(item);
                            } else {
                                qDebug() << "Map::loadFromOTBM - Failed to read item on tile" << currentTilePos.x << currentTilePos.y << currentTilePos.z;
                            }
                        } else {
                            qWarning() << "Map::loadFromOTBM - Unexpected node type" << itemNodeType << "inside TILE, expected OTBM_ITEM. Skipping node.";
                            // reader.skipNode(); // Placeholder for skipping unknown child nodes
                        }
                        if (!reader.leaveNode()) { qWarning() << "Map::loadFromOTBM - Failed to leave item node."; return false; }
                    }
                    if (tile) { // After all attributes and items for the tile have been read
                        tile->setModified(false);
                    }
                } else {
                     qWarning() << "Map::loadFromOTBM - Unexpected node type" << tileNodeType << "inside TILE_AREA, expected OTBM_TILE or OTBM_HOUSETILE. Skipping node.";
                     // reader.skipNode();
                }
                if (!reader.leaveNode()) { qWarning() << "Map::loadFromOTBM - Failed to leave tile node."; return false; }
            }
        } else if (nodeType == OTBM_TOWNS) {
            qDebug() << "Map::loadFromOTBM - Reading OTBM_TOWNS.";
            quint8 townNodeType;
            while(reader.enterNode(townNodeType)) {
                if (townNodeType == OTBM_TOWN) {
                    quint32 townId;
                    QString townName;
                    MapPos templePos;
                    quint16 tempX, tempY;
                    quint8 tempZ;

                    if (!(reader.stream() >> townId)) {
                        qWarning() << "Map::loadFromOTBM - Failed to read town ID.";
                        reader.leaveNode(); break;
                    }

                    townName = reader.readString(); // Assuming this method exists or is implemented.
                    if (reader.stream().status() != QDataStream::Ok) {
                        qWarning() << "Map::loadFromOTBM - Failed to read town name string.";
                        reader.leaveNode(); break;
                    }

                    if (!(reader.stream() >> tempX >> tempY >> tempZ)) {
                        qWarning() << "Map::loadFromOTBM - Failed to read town temple position.";
                        reader.leaveNode(); break;
                    }
                    templePos.x = tempX;
                    templePos.y = tempY;
                    templePos.z = tempZ;

                    Town* newTown = new Town(townId, townName, templePos);
                    m_towns.append(newTown);
                    qDebug() << "Loaded Town:" << newTown->getName() << "ID:" << newTown->getId() << "Pos:" << templePos.x << templePos.y << templePos.z;

                } else {
                    qWarning() << "Map::loadFromOTBM - Unexpected node type" << townNodeType << "inside OTBM_TOWNS.";
                    // reader.skipNode(); // Placeholder
                }
                if (!reader.leaveNode()) { qWarning() << "Map::loadFromOTBM - Failed to leave TOWN node."; return false; }
            }
        } else if (nodeType == OTBM_WAYPOINTS) {
            qDebug() << "Map::loadFromOTBM - Reading OTBM_WAYPOINTS.";
            quint8 waypointNodeType;
            while(reader.enterNode(waypointNodeType)) {
                if (waypointNodeType == OTBM_WAYPOINT) {
                    QString waypointName;
                    MapPos waypointPos;
                    quint16 tempX, tempY;
                    quint8 tempZ;

                    waypointName = reader.readString(); // Assuming this method exists or is implemented.
                    if (reader.stream().status() != QDataStream::Ok) {
                        qWarning() << "Map::loadFromOTBM - Failed to read waypoint name string.";
                        reader.leaveNode(); break;
                    }

                    if (!(reader.stream() >> tempX >> tempY >> tempZ)) {
                        qWarning() << "Map::loadFromOTBM - Failed to read waypoint position.";
                        reader.leaveNode(); break;
                    }
                    waypointPos.x = tempX;
                    waypointPos.y = tempY;
                    waypointPos.z = tempZ;

                    Waypoint* newWaypoint = new Waypoint(waypointName, waypointPos);
                    m_waypoints.append(newWaypoint);
                    qDebug() << "Loaded Waypoint:" << newWaypoint->getName() << "Pos:" << waypointPos.x << waypointPos.y << waypointPos.z;

                } else {
                    qWarning() << "Map::loadFromOTBM - Unexpected node type" << waypointNodeType << "inside OTBM_WAYPOINTS.";
                    // reader.skipNode(); // Placeholder
                }
                if (!reader.leaveNode()) { qWarning() << "Map::loadFromOTBM - Failed to leave WAYPOINT node."; return false; }
            }
        }
        else {
            qWarning() << "Map::loadFromOTBM - Unexpected node type" << nodeType << "inside MAP_DATA. Skipping node.";
            // reader.skipNode();
        }
        if (!reader.leaveNode()) {
            qWarning() << "Map::loadFromOTBM - Failed to leave node type" << nodeType << "in MAP_DATA.";
            return false;
        }
    }

    if (!reader.leaveNode()) { qWarning() << "Map::loadFromOTBM - Failed to leave MAP_DATA node."; return false; }
    if (!reader.leaveNode()) { qWarning() << "Map::loadFromOTBM - Failed to leave ROOTV1 node."; return false; }

    // Here, the map dimensions should be finalized based on read data if not fixed.
    // If map was initialized with 0,0,0, it needs proper sizing now.
    // For this step, proper dimension handling is deferred.
    setModified(false); // Map is now in a clean state reflecting the loaded file.
    qDebug() << "Map::loadFromOTBM - Successfully parsed OTBM data. Map set to unmodified.";
    emit mapChanged();
    return true;
}


bool Map::saveToOTBM(QDataStream& stream) const {
    OtbmWriter writer(stream);

    // Start Root Node
    writer.beginNode(OTBM_ROOTV1);

    // Write OTBM version attributes for ROOTV1
    writer.writeAttributeU32(OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_MAJOR, m_otbmMajorVersion);
    writer.writeAttributeU32(OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_MINOR, m_otbmMinorVersion);
    writer.writeAttributeU32(OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_BUILD, m_otbmBuildVersion);
    if (!m_otbmVersionDescription.isEmpty()) {
        writer.writeAttributeString(OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_DESC_STRING, m_otbmVersionDescription);
    }
    qDebug() << "Map::saveToOTBM - Wrote OTBM version info: Major" << m_otbmMajorVersion
             << "Minor" << m_otbmMinorVersion << "Build" << m_otbmBuildVersion
             << "Desc:" << m_otbmVersionDescription;

    // TODO (Task51-MapVer): If saving to different OTBM target versions is required,
    // logic would be needed here to adjust the output structure. For example:
    // - Omitting nodes like OTBM_WAYPOINTS if target version < 3. (Already handled for waypoints).
    // - Converting any map-level data to older representations if necessary.
    // Currently, the map is saved in its native/loaded OTBM version format.

    // Start Map Data Node
    writer.beginNode(OTBM_MAP_DATA);

    // Write Map Attributes
    if (!description_.isEmpty()) {
        writer.writeAttributeString(OTBM_ATTR_DESCRIPTION, description_);
    }
    if (!m_externalSpawnFile.isEmpty()) {
        writer.writeAttributeString(OTBM_ATTR_EXT_SPAWN_FILE, m_externalSpawnFile);
    }
    if (!m_externalHouseFile.isEmpty()) {
        writer.writeAttributeString(OTBM_ATTR_EXT_HOUSE_FILE, m_externalHouseFile);
    }

    // TODO: Write Item Definition version if stored (major/minor items.otb version)
    // Example:
    // writer.writeAttributeU32(OTBM_ATTR_ITEM_MAJOR_VERSION, ItemManager::getMajorVersion());
    // writer.writeAttributeU32(OTBM_ATTR_ITEM_MINOR_VERSION, ItemManager::getMinorVersion());

    // TODO (Task51): Implement client version specific logic for saving.
    // If the current map data needs to be converted to an older/specific format
    // based on a target client version (if that's a feature), do it here.
    // For now, we save using the map's loaded/current version.

    // Iterate through the map to write TileArea nodes
    // OTBM typically stores tiles in 256x256 areas per floor.
    // We need to determine the actual extents of the map to avoid writing empty areas if possible,
    // but the format expects areas to cover the declared map width/height.
    // For simplicity, iterate based on current map dimensions, grouped into areas.

    qDebug() << "Map::saveToOTBM - Map dimensions:" << width_ << "x" << height_ << "x" << floors_;

    for (int z = 0; z < floors_; ++z) {
        for (int areaY = 0; areaY < height_; areaY += 256) {
            for (int areaX = 0; areaX < width_; areaX += 256) {
                // Check if this area actually contains any tiles that need saving
                bool areaHasTiles = false;
                for (int relY = 0; relY < 256; ++relY) {
                    if (areaY + relY >= height_) break; // Past map boundary
                    for (int relX = 0; relX < 256; ++relX) {
                        if (areaX + relX >= width_) break; // Past map boundary
                        if (getTile(areaX + relX, areaY + relY, z) != nullptr) {
                            areaHasTiles = true;
                            break;
                        }
                    }
                    if (areaHasTiles) break;
                }

                if (areaHasTiles) {
                    writer.beginNode(OTBM_TILE_AREA);
                    writer.writeU16(areaX); // Area base X
                    writer.writeU16(areaY); // Area base Y
                    writer.writeByte(z);    // Area base Z (floor)

                    // Iterate tiles within this area
                    for (int relY = 0; relY < 256; ++relY) {
                        if (areaY + relY >= height_) break;
                        for (int relX = 0; relX < 256; ++relX) {
                            if (areaX + relX >= width_) break;

                            Tile* tile = getTile(areaX + relX, areaY + relY, z);
                            if (tile) {
                                // Begin Tile Node (OTBM_TILE or OTBM_HOUSETILE)
                                writer.beginNode(tile->isHouseTile() ? OTBM_HOUSETILE : OTBM_TILE);
                                writer.writeByte(static_cast<quint8>(relX)); // Relative X
                                writer.writeByte(static_cast<quint8>(relY)); // Relative Y

                                // Write Tile Attributes
                                if (tile->getMapFlags() != 0) { // Only write if not default
                                    writer.writeAttributeU32(OTBM_ATTR_TILE_FLAGS, static_cast<quint32>(tile->getMapFlags()));
                                }
                                if (tile->isHouseTile() && tile->getHouseDoorId() != 0) { // Example for house door ID
                                     writer.writeAttributeByte(OTBM_ATTR_HOUSEDOORID, tile->getHouseDoorId());
                                }
                                // TODO: Other tile-specific attributes (e.g. from a QMap on Tile)

                                // Write Items on Tile
                                const QList<Item*>& items = tile->getItems();
                                for (const Item* item : items) {
                                    if (item) {
                                       writer.writeItemNode(item, m_otbItemsMajorVersion, m_otbItemsMinorVersion); // NEW
                                       // TODO (Task51): If item serialization is version-dependent, OtbmWriter::writeItemNode
                                       // or Item::serializeOtbmAttributes might need version parameters.
                                    }
                                }
                                writer.endNode(); // End Tile Node
                            }
                        }
                    }
                    writer.endNode(); // End TileArea Node
                }
            }
        }
    }

    // TODO: Write other top-level nodes like Spawns, Waypoints, Houses
    // Example for Spawns:
    // if (!spawns_.isEmpty()) {
    //     writer.beginNode(OTBM_SPAWNS);
    //     for (const Spawn* spawn : spawns_) {
    //         // spawn->serializeOtbmNode(writer); // Assuming Spawn has such a method
    //     }
    //     writer.endNode(); // End Spawns Node
    // }

    // Write Towns
    if (!m_towns.isEmpty()) {
        writer.beginNode(OTBM_TOWNS);
        for (const Town* town : m_towns) {
            if (town) {
                writer.beginNode(OTBM_TOWN);
                writer.writeU32(town->getId());
                writer.writeString(town->getName());
                writer.writeU16(town->getTemplePosition().x);
                writer.writeU16(town->getTemplePosition().y);
                writer.writeByte(static_cast<quint8>(town->getTemplePosition().z));
                writer.endNode(); // End OTBM_TOWN
            }
        }
        writer.endNode(); // End OTBM_TOWNS
    }

    // Write Waypoints
    // Assuming m_otbmMajorVersion is 0-indexed from OTBM_ROOT_ATTR_VERSION_MAJOR.
    // OTBM v1 = 0, v2 = 1, v3 = 2. Waypoints supported from v3 onwards.
    if (m_otbmMajorVersion >= 2) {
        if (!m_waypoints.isEmpty()) {
            writer.beginNode(OTBM_WAYPOINTS);
            for (const Waypoint* waypoint : m_waypoints) {
                if (waypoint) {
                    writer.beginNode(OTBM_WAYPOINT);
                    writer.writeString(waypoint->getName());
                    writer.writeU16(waypoint->getPosition().x);
                    writer.writeU16(waypoint->getPosition().y);
                    writer.writeByte(static_cast<quint8>(waypoint->getPosition().z));
                    writer.endNode(); // End OTBM_WAYPOINT
                }
            }
            writer.endNode(); // End OTBM_WAYPOINTS
        }
    } else if (!m_waypoints.isEmpty()) {
        qWarning() << "Map::saveToOTBM - Waypoints exist but are not saved due to OTBM version being less than 3 (actual version index:" << m_otbmMajorVersion << ")";
    }

    writer.endNode(); // End Map Data Node
    writer.endNode(); // End Root Node

    if (stream.status() == QDataStream::Ok) {
        Map* nonConstThis = const_cast<Map*>(this);
        nonConstThis->setModified(false);
        qDebug() << "Map::saveToOTBM - Successfully saved OTBM data. Map set to unmodified.";
    }
    return stream.status() == QDataStream::Ok;
}
