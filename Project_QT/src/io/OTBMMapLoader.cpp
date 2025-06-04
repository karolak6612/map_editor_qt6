#include "OTBMMapLoader.h"
#include "OtbmReader.h"
#include "OtbmWriter.h"
#include "../OtbmTypes.h"
#include "../Map.h"
#include "../Tile.h"
#include "../Item.h"
#include "../ItemManager.h"
#include "../Town.h"
#include "../Waypoint.h"
#include <QDebug>
#include <QDataStream>

OTBMMapLoader::OTBMMapLoader(QObject* parent)
    : QObject(parent)
{
    qDebug() << "OTBMMapLoader: Initialized";
}

OTBMMapLoader::~OTBMMapLoader() = default;

bool OTBMMapLoader::canLoad(const QString& filePath) const {
    Q_UNUSED(filePath)
    // For now, assume all files can be loaded
    // In a real implementation, we'd check file headers
    return true;
}

bool OTBMMapLoader::canSave(const QString& filePath) const {
    Q_UNUSED(filePath)
    return true;
}

bool OTBMMapLoader::load(Map* map, QDataStream& stream) {
    if (!map) {
        setError("Map pointer is null");
        return false;
    }

    clearError();
    map->clear(); // Clear existing map data

    OtbmReader reader(stream);
    ItemManager* itemManager = ItemManager::getInstancePtr();

    quint8 rootNodeType;
    if (!reader.enterNode(rootNodeType)) {
        setError("Could not enter root node");
        return false;
    }

    if (rootNodeType != OTBM_ROOTV1) {
        setError(QString("Root node type is not OTBM_ROOTV1. Got: %1").arg(rootNodeType));
        reader.leaveNode();
        return false;
    }

    qDebug() << "OTBMMapLoader::load - Entered OTBM_ROOTV1 node";

    // Load OTBM header information
    if (!loadHeader(map, reader)) {
        reader.leaveNode();
        return false;
    }

    // Load map data
    if (!loadMapData(map, reader)) {
        reader.leaveNode();
        return false;
    }

    if (!reader.leaveNode()) {
        setError("Failed to leave ROOTV1 node");
        return false;
    }

    map->setModified(false);
    qDebug() << "OTBMMapLoader::load - Successfully parsed OTBM data";
    emit loadingCompleted(true);
    return true;
}

bool OTBMMapLoader::save(const Map* map, QDataStream& stream) const {
    if (!map) {
        setError("Map pointer is null");
        return false;
    }

    clearError();
    OtbmWriter writer(stream);

    // Write root node
    writer.startNode(OTBM_ROOTV1);

    // Save OTBM header
    if (!saveHeader(map, writer)) {
        return false;
    }

    // Save map data
    if (!saveMapData(map, writer)) {
        return false;
    }

    writer.endNode(); // End Root Node

    if (stream.status() == QDataStream::Ok) {
        const_cast<Map*>(map)->setModified(false);
        qDebug() << "OTBMMapLoader::save - Successfully saved OTBM data";
        emit savingCompleted(true);
    }
    return stream.status() == QDataStream::Ok;
}

QString OTBMMapLoader::getLastError() const {
    return lastError_;
}

void OTBMMapLoader::clearError() const {
    lastError_.clear();
}

bool OTBMMapLoader::loadHeader(Map* map, OtbmReader& reader) {
    // Read OTBM version attributes
    quint8 attrId;
    while (reader.nextAttributeId(attrId)) {
        quint16 attrDataLen;
        if (!(reader.stream() >> attrDataLen)) {
            setError(QString("Failed to read data length for ROOTV1 attribute %1").arg(attrId));
            return false;
        }
        
        QByteArray attrData = reader.readData(attrDataLen);
        if (reader.stream().status() != QDataStream::Ok && attrDataLen > 0) {
            setError(QString("Failed to read data for ROOTV1 attribute %1").arg(attrId));
            return false;
        }

        QDataStream valueStream(attrData);
        valueStream.setByteOrder(QDataStream::LittleEndian);

        switch (static_cast<OTBM_RootAttribute>(attrId)) {
            case OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_MAJOR:
                if (attrDataLen == sizeof(quint32)) {
                    quint32 majorVersion;
                    valueStream >> majorVersion;
                    map->setOtbmMajorVersion(majorVersion);
                } else {
                    qWarning() << "OTBMMapLoader::loadHeader - Incorrect data length for OTBM_ROOT_ATTR_VERSION_MAJOR";
                }
                break;
            case OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_MINOR:
                if (attrDataLen == sizeof(quint32)) {
                    quint32 minorVersion;
                    valueStream >> minorVersion;
                    map->setOtbmMinorVersion(minorVersion);
                } else {
                    qWarning() << "OTBMMapLoader::loadHeader - Incorrect data length for OTBM_ROOT_ATTR_VERSION_MINOR";
                }
                break;
            case OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_BUILD:
                if (attrDataLen == sizeof(quint32)) {
                    quint32 buildVersion;
                    valueStream >> buildVersion;
                    map->setOtbmBuildVersion(buildVersion);
                } else {
                    qWarning() << "OTBMMapLoader::loadHeader - Incorrect data length for OTBM_ROOT_ATTR_VERSION_BUILD";
                }
                break;
            case OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_DESC_STRING:
                map->setOtbmVersionDescription(QString::fromUtf8(attrData));
                break;
            default:
                qDebug() << "OTBMMapLoader::loadHeader - Skipped unknown attribute" << attrId << "in ROOTV1 node";
                break;
        }
    }

    // Read map dimensions and OTB item versions
    quint16 mapWidth, mapHeight;
    quint32 otbItemsMajor, otbItemsMinor;
    
    if (!(reader.stream() >> mapWidth)) {
        setError("Failed to read map width from root node stream");
        return false;
    }
    
    if (!(reader.stream() >> mapHeight)) {
        setError("Failed to read map height from root node stream");
        return false;
    }
    
    if (!(reader.stream() >> otbItemsMajor)) {
        setError("Failed to read OTB items major version from root node stream");
        return false;
    }
    
    if (!(reader.stream() >> otbItemsMinor)) {
        setError("Failed to read OTB items minor version from root node stream");
        return false;
    }

    // Set map properties
    map->setWidth(mapWidth);
    map->setHeight(mapHeight);
    map->setOtbItemsMajorVersion(otbItemsMajor);
    map->setOtbItemsMinorVersion(otbItemsMinor);

    qDebug() << "OTBMMapLoader::loadHeader - Map dimensions:" << mapWidth << "x" << mapHeight;
    qDebug() << "OTBMMapLoader::loadHeader - OTB Items Version:" << otbItemsMajor << "." << otbItemsMinor;

    return true;
}

bool OTBMMapLoader::loadMapData(Map* map, OtbmReader& reader) {
    quint8 mapDataNodeType;
    if (!reader.enterNode(mapDataNodeType)) {
        setError("Could not enter MAP_DATA node");
        return false;
    }

    if (mapDataNodeType != OTBM_MAP_DATA) {
        setError(QString("Expected OTBM_MAP_DATA node, got: %1").arg(mapDataNodeType));
        reader.leaveNode();
        return false;
    }

    qDebug() << "OTBMMapLoader::loadMapData - Entered OTBM_MAP_DATA node";

    // Read map attributes
    quint8 mapAttrId;
    while (reader.nextAttributeId(mapAttrId)) {
        quint16 mapAttrDataLen;
        if (!(reader.stream() >> mapAttrDataLen)) {
            qWarning() << "OTBMMapLoader::loadMapData - Failed to read data length for MAP_DATA attribute" << mapAttrId;
            break;
        }
        
        QByteArray mapAttrData = reader.readData(mapAttrDataLen);
        if (reader.stream().status() != QDataStream::Ok && mapAttrDataLen > 0) {
            qWarning() << "OTBMMapLoader::loadMapData - Failed to read data for MAP_DATA attribute" << mapAttrId;
            break;
        }

        switch (mapAttrId) {
            case OTBM_ATTR_DESCRIPTION:
                map->setDescription(QString::fromUtf8(mapAttrData));
                qDebug() << "Map Description:" << map->getDescription();
                break;
            case OTBM_ATTR_EXT_SPAWN_FILE:
                map->setExternalSpawnFile(QString::fromUtf8(mapAttrData));
                qDebug() << "External Spawn File:" << map->getExternalSpawnFile();
                break;
            case OTBM_ATTR_EXT_HOUSE_FILE:
                map->setExternalHouseFile(QString::fromUtf8(mapAttrData));
                qDebug() << "External House File:" << map->getExternalHouseFile();
                break;
            default:
                qDebug() << "OTBMMapLoader::loadMapData - Skipping MAP_DATA attribute" << mapAttrId;
                break;
        }
    }

    // Process child nodes (tile areas, towns, waypoints, etc.)
    quint8 nodeType;
    while (reader.enterNode(nodeType)) {
        if (nodeType == OTBM_TILE_AREA) {
            if (!loadTileArea(map, reader)) {
                reader.leaveNode();
                return false;
            }
        } else if (nodeType == OTBM_TOWNS) {
            if (!loadTowns(map, reader)) {
                reader.leaveNode();
                return false;
            }
        } else if (nodeType == OTBM_WAYPOINTS) {
            if (!loadWaypoints(map, reader)) {
                reader.leaveNode();
                return false;
            }
        } else {
            qWarning() << "OTBMMapLoader::loadMapData - Unexpected node type" << nodeType << "inside MAP_DATA. Skipping node";
        }
        
        if (!reader.leaveNode()) {
            setError(QString("Failed to leave node type %1 in MAP_DATA").arg(nodeType));
            return false;
        }
    }

    return reader.leaveNode(); // Leave MAP_DATA node
}

bool OTBMMapLoader::loadTileArea(Map* map, OtbmReader& reader) {
    quint16 areaBaseX, areaBaseY;
    quint8 areaBaseZ;

    reader.stream() >> areaBaseX >> areaBaseY >> areaBaseZ;
    if (reader.stream().status() != QDataStream::Ok) {
        setError("Failed to read TILE_AREA coordinates");
        return false;
    }

    qDebug() << "OTBMMapLoader::loadTileArea - Reading TILE_AREA at" << areaBaseX << areaBaseY << areaBaseZ;

    quint8 tileNodeType;
    while (reader.enterNode(tileNodeType)) {
        if (tileNodeType == OTBM_TILE || tileNodeType == OTBM_HOUSETILE) {
            if (!loadTile(map, reader, areaBaseX, areaBaseY, areaBaseZ, tileNodeType == OTBM_HOUSETILE)) {
                reader.leaveNode();
                return false;
            }
        } else {
            qWarning() << "OTBMMapLoader::loadTileArea - Unexpected node type" << tileNodeType << "inside TILE_AREA";
        }

        if (!reader.leaveNode()) {
            setError("Failed to leave tile node");
            return false;
        }
    }

    return true;
}

bool OTBMMapLoader::loadTile(Map* map, OtbmReader& reader, quint16 areaBaseX, quint16 areaBaseY, quint8 areaBaseZ, bool isHouseTile) {
    quint8 relX, relY;
    reader.stream() >> relX >> relY;
    if (reader.stream().status() != QDataStream::Ok) {
        setError("Failed to read TILE relative coordinates");
        return false;
    }

    int tileX = areaBaseX + relX;
    int tileY = areaBaseY + relY;
    int tileZ = areaBaseZ;

    if (!map->isCoordValid(tileX, tileY, tileZ)) {
        qWarning() << "OTBMMapLoader::loadTile - Tile coordinates" << tileX << tileY << tileZ << "are out of map bounds. Skipping tile";
        return true; // Skip but don't fail
    }

    Tile* tile = map->getOrCreateTile(tileX, tileY, tileZ);
    if (!tile) {
        setError(QString("Failed to get/create tile at %1,%2,%3").arg(tileX).arg(tileY).arg(tileZ));
        return false;
    }

    if (isHouseTile) {
        tile->setHouseTile(true);
    }

    // Read tile attributes
    quint8 tileAttrId;
    while (reader.nextAttributeId(tileAttrId)) {
        quint16 tileAttrDataLen;
        if (!(reader.stream() >> tileAttrDataLen)) {
            break;
        }

        QByteArray tileAttrData = reader.readData(tileAttrDataLen);
        QDataStream tileValueStream(tileAttrData);
        tileValueStream.setByteOrder(QDataStream::LittleEndian);

        if (tileAttrId == OTBM_ATTR_TILE_FLAGS) {
            quint32 flags = 0;
            if (tileAttrDataLen == sizeof(quint32)) {
                tileValueStream >> flags;
                tile->setMapFlagsValue(flags);
            } else {
                qWarning() << "OTBMMapLoader::loadTile - Incorrect TILE_FLAGS length";
            }
        } else if (tileAttrId == OTBM_ATTR_HOUSEDOORID && isHouseTile) {
            quint8 houseDoorId = 0;
            if (tileAttrDataLen == sizeof(quint8)) {
                tileValueStream >> houseDoorId;
                tile->setHouseDoorId(houseDoorId);
            } else {
                qWarning() << "OTBMMapLoader::loadTile - Incorrect HOUSEDOORID length";
            }
        } else {
            qDebug() << "OTBMMapLoader::loadTile - Skipping TILE attribute" << tileAttrId;
        }
    }

    // Read items on tile
    quint8 itemNodeType;
    while (reader.enterNode(itemNodeType)) {
        if (itemNodeType == OTBM_ITEM) {
            ItemManager* itemManager = ItemManager::getInstancePtr();
            Item* item = reader.readItem(itemManager, map->getOtbmMajorVersion(),
                                       map->getOtbItemsMajorVersion(), map->getOtbItemsMinorVersion());
            if (item) {
                tile->addItem(item);
            } else {
                qDebug() << "OTBMMapLoader::loadTile - Failed to read item on tile" << tileX << tileY << tileZ;
            }
        } else {
            qWarning() << "OTBMMapLoader::loadTile - Unexpected node type" << itemNodeType << "inside TILE";
        }

        if (!reader.leaveNode()) {
            setError("Failed to leave item node");
            return false;
        }
    }

    tile->setModified(false);
    return true;
}

bool OTBMMapLoader::loadTowns(Map* map, OtbmReader& reader) {
    qDebug() << "OTBMMapLoader::loadTowns - Reading OTBM_TOWNS";

    quint8 townNodeType;
    while (reader.enterNode(townNodeType)) {
        if (townNodeType == OTBM_TOWN) {
            quint32 townId;
            QString townName;
            quint16 tempX, tempY;
            quint8 tempZ;

            if (!(reader.stream() >> townId)) {
                setError("Failed to read town ID");
                reader.leaveNode();
                return false;
            }

            townName = reader.readString();
            if (reader.stream().status() != QDataStream::Ok) {
                setError("Failed to read town name string");
                reader.leaveNode();
                return false;
            }

            if (!(reader.stream() >> tempX >> tempY >> tempZ)) {
                setError("Failed to read town temple position");
                reader.leaveNode();
                return false;
            }

            MapPos templePos(tempX, tempY, tempZ);
            Town* newTown = new Town(townId, townName, templePos);
            map->addTown(newTown);
            qDebug() << "Loaded Town:" << newTown->getName() << "ID:" << newTown->getId() << "Pos:" << templePos.x << templePos.y << templePos.z;
        } else {
            qWarning() << "OTBMMapLoader::loadTowns - Unexpected node type" << townNodeType << "inside OTBM_TOWNS";
        }

        if (!reader.leaveNode()) {
            setError("Failed to leave TOWN node");
            return false;
        }
    }

    return true;
}

bool OTBMMapLoader::loadWaypoints(Map* map, OtbmReader& reader) {
    qDebug() << "OTBMMapLoader::loadWaypoints - Reading OTBM_WAYPOINTS";

    quint8 waypointNodeType;
    while (reader.enterNode(waypointNodeType)) {
        if (waypointNodeType == OTBM_WAYPOINT) {
            QString waypointName;
            quint16 tempX, tempY;
            quint8 tempZ;

            waypointName = reader.readString();
            if (reader.stream().status() != QDataStream::Ok) {
                setError("Failed to read waypoint name string");
                reader.leaveNode();
                return false;
            }

            if (!(reader.stream() >> tempX >> tempY >> tempZ)) {
                setError("Failed to read waypoint position");
                reader.leaveNode();
                return false;
            }

            MapPos waypointPos(tempX, tempY, tempZ);
            Waypoint* newWaypoint = new Waypoint(waypointName, waypointPos);
            map->addWaypoint(newWaypoint);
            qDebug() << "Loaded Waypoint:" << newWaypoint->getName() << "Pos:" << waypointPos.x << waypointPos.y << waypointPos.z;
        } else {
            qWarning() << "OTBMMapLoader::loadWaypoints - Unexpected node type" << waypointNodeType << "inside OTBM_WAYPOINTS";
        }

        if (!reader.leaveNode()) {
            setError("Failed to leave WAYPOINT node");
            return false;
        }
    }

    return true;
}

bool OTBMMapLoader::saveHeader(const Map* map, OtbmWriter& writer) const {
    // Write OTBM version attributes for ROOTV1
    writer.writeAttributeU32(OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_MAJOR, map->getOtbmMajorVersion());
    writer.writeAttributeU32(OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_MINOR, map->getOtbmMinorVersion());
    writer.writeAttributeU32(OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_BUILD, map->getOtbmBuildVersion());

    if (!map->getOtbmVersionDescription().isEmpty()) {
        writer.writeAttributeString(OTBM_RootAttribute::OTBM_ROOT_ATTR_VERSION_DESC_STRING, map->getOtbmVersionDescription());
    }

    qDebug() << "OTBMMapLoader::saveHeader - Wrote OTBM map format version info: Major" << map->getOtbmMajorVersion()
             << "Minor" << map->getOtbmMinorVersion() << "Build" << map->getOtbmBuildVersion()
             << "Desc:" << map->getOtbmVersionDescription();

    // Write map dimensions and OTB item versions as direct properties of the root node
    writer.writeU16(map->getWidth());
    writer.writeU16(map->getHeight());
    writer.writeU32(map->getOtbItemsMajorVersion());
    writer.writeU32(map->getOtbItemsMinorVersion());

    qDebug() << "OTBMMapLoader::saveHeader - Wrote map dimensions:" << map->getWidth() << "x" << map->getHeight();
    qDebug() << "OTBMMapLoader::saveHeader - Wrote OTB Items Version: Major" << map->getOtbItemsMajorVersion() << "Minor" << map->getOtbItemsMinorVersion();

    return true;
}

bool OTBMMapLoader::saveMapData(const Map* map, OtbmWriter& writer) const {
    // Start Map Data Node
    writer.beginNode(OTBM_MAP_DATA);

    // Write Map Attributes
    if (!map->getDescription().isEmpty()) {
        writer.writeAttributeString(OTBM_ATTR_DESCRIPTION, map->getDescription());
    }
    if (!map->getExternalSpawnFile().isEmpty()) {
        writer.writeAttributeString(OTBM_ATTR_EXT_SPAWN_FILE, map->getExternalSpawnFile());
    }
    if (!map->getExternalHouseFile().isEmpty()) {
        writer.writeAttributeString(OTBM_ATTR_EXT_HOUSE_FILE, map->getExternalHouseFile());
    }

    // Save tile areas
    if (!saveTileAreas(map, writer)) {
        return false;
    }

    // Save towns
    if (!saveTowns(map, writer)) {
        return false;
    }

    // Save waypoints (if OTBM version supports them)
    if (map->getOtbmMajorVersion() >= 2) {
        if (!saveWaypoints(map, writer)) {
            return false;
        }
    }

    writer.endNode(); // End Map Data Node
    return true;
}

bool OTBMMapLoader::saveTileAreas(const Map* map, OtbmWriter& writer) const {
    qDebug() << "OTBMMapLoader::saveTileAreas - Map dimensions:" << map->getWidth() << "x" << map->getHeight() << "x" << map->getFloors();

    for (int z = 0; z < map->getFloors(); ++z) {
        for (int areaY = 0; areaY < map->getHeight(); areaY += 256) {
            for (int areaX = 0; areaX < map->getWidth(); areaX += 256) {
                // Check if this area actually contains any tiles that need saving
                bool areaHasTiles = false;
                for (int relY = 0; relY < 256; ++relY) {
                    if (areaY + relY >= map->getHeight()) break;
                    for (int relX = 0; relX < 256; ++relX) {
                        if (areaX + relX >= map->getWidth()) break;
                        if (map->getTile(areaX + relX, areaY + relY, z) != nullptr) {
                            areaHasTiles = true;
                            break;
                        }
                    }
                    if (areaHasTiles) break;
                }

                if (areaHasTiles) {
                    writer.beginNode(OTBM_TILE_AREA);
                    writer.writeU16(areaX);
                    writer.writeU16(areaY);
                    writer.writeByte(z);

                    // Save tiles within this area
                    for (int relY = 0; relY < 256; ++relY) {
                        if (areaY + relY >= map->getHeight()) break;
                        for (int relX = 0; relX < 256; ++relX) {
                            if (areaX + relX >= map->getWidth()) break;

                            const Tile* tile = map->getTile(areaX + relX, areaY + relY, z);
                            if (tile) {
                                if (!saveTile(tile, writer, relX, relY)) {
                                    return false;
                                }
                            }
                        }
                    }
                    writer.endNode(); // End TileArea Node
                }
            }
        }
    }

    return true;
}

bool OTBMMapLoader::saveTile(const Tile* tile, OtbmWriter& writer, int relX, int relY) const {
    // Begin Tile Node (OTBM_TILE or OTBM_HOUSETILE)
    writer.beginNode(tile->isHouseTile() ? OTBM_HOUSETILE : OTBM_TILE);
    writer.writeByte(static_cast<quint8>(relX));
    writer.writeByte(static_cast<quint8>(relY));

    // Write Tile Attributes
    if (tile->getMapFlags() != 0) {
        writer.writeAttributeU32(OTBM_ATTR_TILE_FLAGS, static_cast<quint32>(tile->getMapFlags()));
    }
    if (tile->isHouseTile() && tile->getHouseDoorId() != 0) {
        writer.writeAttributeByte(OTBM_ATTR_HOUSEDOORID, tile->getHouseDoorId());
    }

    // Write Items on Tile
    const QList<Item*>& items = tile->getItems();
    for (const Item* item : items) {
        if (item) {
            writer.writeItemNode(item, 0, 0, 0); // Version parameters would be passed from map
        }
    }

    writer.endNode(); // End Tile Node
    return true;
}

bool OTBMMapLoader::saveTowns(const Map* map, OtbmWriter& writer) const {
    const QList<Town*>& towns = map->getTowns();
    if (!towns.isEmpty()) {
        writer.beginNode(OTBM_TOWNS);
        for (const Town* town : towns) {
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

    return true;
}

bool OTBMMapLoader::saveWaypoints(const Map* map, OtbmWriter& writer) const {
    const QList<Waypoint*>& waypoints = map->getWaypoints();
    if (!waypoints.isEmpty()) {
        writer.beginNode(OTBM_WAYPOINTS);
        for (const Waypoint* waypoint : waypoints) {
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
    } else if (!waypoints.isEmpty()) {
        qWarning() << "OTBMMapLoader::saveWaypoints - Waypoints exist but are not saved due to OTBM version being less than 3";
    }

    return true;
}

void OTBMMapLoader::setError(const QString& error) const {
    lastError_ = error;
    qWarning() << "OTBMMapLoader Error:" << error;
}
