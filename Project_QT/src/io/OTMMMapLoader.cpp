#include "OTMMMapLoader.h"
#include "../Map.h"
#include "../Tile.h"
#include "../Item.h"
#include "../ItemManager.h"
#include <QDebug>
#include <QDataStream>

OTMMMapLoader::OTMMMapLoader(QObject* parent)
    : QObject(parent)
{
    qDebug() << "OTMMMapLoader: Initialized";
}

OTMMMapLoader::~OTMMMapLoader() = default;

bool OTMMMapLoader::canLoad(const QString& filePath) const {
    Q_UNUSED(filePath)
    // For now, assume all files can be loaded
    // In a real implementation, we'd check file headers
    return true;
}

bool OTMMMapLoader::canSave(const QString& filePath) const {
    Q_UNUSED(filePath)
    return true;
}

bool OTMMMapLoader::load(Map* map, QDataStream& stream) {
    if (!map) {
        setError("Map pointer is null");
        return false;
    }

    clearError();
    map->clear(); // Clear existing map data

    // OTMM format header
    quint32 magic;
    stream >> magic;
    if (magic != OTMM_MAGIC) {
        setError(QString("Invalid OTMM magic number. Expected: %1, Got: %2").arg(OTMM_MAGIC).arg(magic));
        return false;
    }

    quint16 version;
    stream >> version;
    if (version != OTMM_VERSION) {
        setError(QString("Unsupported OTMM version. Expected: %1, Got: %2").arg(OTMM_VERSION).arg(version));
        return false;
    }

    // Read map properties
    quint16 mapWidth, mapHeight;
    quint8 mapFloors;
    stream >> mapWidth >> mapHeight >> mapFloors;

    if (stream.status() != QDataStream::Ok) {
        setError("Failed to read map dimensions from OTMM file");
        return false;
    }

    // Initialize map
    map->initialize(mapWidth, mapHeight, mapFloors);

    // Read description length and description
    quint16 descLength;
    stream >> descLength;
    if (descLength > 0) {
        QByteArray descData(descLength, 0);
        stream.readRawData(descData.data(), descLength);
        map->setDescription(QString::fromUtf8(descData));
    }

    qDebug() << "OTMMMapLoader::load - Map dimensions:" << mapWidth << "x" << mapHeight << "x" << mapFloors;
    qDebug() << "OTMMMapLoader::load - Description:" << map->getDescription();

    // Read tile count
    quint32 tileCount;
    stream >> tileCount;

    qDebug() << "OTMMMapLoader::load - Reading" << tileCount << "tiles";

    // Read tiles
    for (quint32 i = 0; i < tileCount; ++i) {
        if (!loadTile(map, stream)) {
            setError(QString("Failed to load tile %1 of %2").arg(i + 1).arg(tileCount));
            return false;
        }

        // Emit progress
        if (i % 1000 == 0) {
            emit loadingProgress(i, tileCount, "Loading tiles");
        }
    }

    map->setModified(false);
    qDebug() << "OTMMMapLoader::load - Successfully loaded OTMM map";
    emit loadingCompleted(true);
    return true;
}

bool OTMMMapLoader::save(const Map* map, QDataStream& stream) const {
    if (!map) {
        setError("Map pointer is null");
        return false;
    }

    clearError();

    // Write OTMM header
    stream << static_cast<quint32>(OTMM_MAGIC);
    stream << static_cast<quint16>(OTMM_VERSION);

    // Write map properties
    stream << static_cast<quint16>(map->getWidth());
    stream << static_cast<quint16>(map->getHeight());
    stream << static_cast<quint8>(map->getFloors());

    // Write description
    QByteArray descData = map->getDescription().toUtf8();
    stream << static_cast<quint16>(descData.size());
    if (!descData.isEmpty()) {
        stream.writeRawData(descData.constData(), descData.size());
    }

    // Count non-null tiles
    quint32 tileCount = 0;
    for (int z = 0; z < map->getFloors(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                if (map->getTile(x, y, z) != nullptr) {
                    ++tileCount;
                }
            }
        }
    }

    // Write tile count
    stream << tileCount;

    qDebug() << "OTMMMapLoader::save - Saving" << tileCount << "tiles";

    // Write tiles
    quint32 savedTiles = 0;
    for (int z = 0; z < map->getFloors(); ++z) {
        for (int y = 0; y < map->getHeight(); ++y) {
            for (int x = 0; x < map->getWidth(); ++x) {
                const Tile* tile = map->getTile(x, y, z);
                if (tile) {
                    if (!saveTile(tile, stream)) {
                        setError(QString("Failed to save tile at %1,%2,%3").arg(x).arg(y).arg(z));
                        return false;
                    }
                    ++savedTiles;

                    // Emit progress
                    if (savedTiles % 1000 == 0) {
                        emit savingProgress(savedTiles, tileCount, "Saving tiles");
                    }
                }
            }
        }
    }

    if (stream.status() == QDataStream::Ok) {
        const_cast<Map*>(map)->setModified(false);
        qDebug() << "OTMMMapLoader::save - Successfully saved OTMM map";
        emit savingCompleted(true);
    }

    return stream.status() == QDataStream::Ok;
}

bool OTMMMapLoader::loadTile(Map* map, QDataStream& stream) {
    // Read tile position
    quint16 x, y;
    quint8 z;
    stream >> x >> y >> z;

    if (stream.status() != QDataStream::Ok) {
        setError("Failed to read tile position");
        return false;
    }

    // Validate coordinates
    if (!map->isCoordValid(x, y, z)) {
        setError(QString("Invalid tile coordinates: %1,%2,%3").arg(x).arg(y).arg(z));
        return false;
    }

    // Create tile
    Tile* tile = map->getOrCreateTile(x, y, z);
    if (!tile) {
        setError(QString("Failed to create tile at %1,%2,%3").arg(x).arg(y).arg(z));
        return false;
    }

    // Read tile flags
    quint32 flags;
    stream >> flags;
    tile->setMapFlagsValue(flags);

    // Read house tile flag
    quint8 isHouseTile;
    stream >> isHouseTile;
    tile->setHouseTile(isHouseTile != 0);

    if (isHouseTile) {
        quint8 houseDoorId;
        stream >> houseDoorId;
        tile->setHouseDoorId(houseDoorId);
    }

    // Read item count
    quint16 itemCount;
    stream >> itemCount;

    // Read items
    for (quint16 i = 0; i < itemCount; ++i) {
        if (!loadItem(tile, stream)) {
            setError(QString("Failed to load item %1 on tile %2,%3,%4").arg(i).arg(x).arg(y).arg(z));
            return false;
        }
    }

    tile->setModified(false);
    return true;
}

bool OTMMMapLoader::loadItem(Tile* tile, QDataStream& stream) {
    // Read item ID
    quint16 itemId;
    stream >> itemId;

    if (stream.status() != QDataStream::Ok) {
        setError("Failed to read item ID");
        return false;
    }

    // Create item
    ItemManager* itemManager = ItemManager::getInstancePtr();
    Item* item = itemManager->createItem(itemId);
    if (!item) {
        setError(QString("Failed to create item with ID: %1").arg(itemId));
        return false;
    }

    // Read item properties
    quint8 count;
    stream >> count;
    if (count > 0) {
        item->setCount(count);
    }

    quint16 actionId;
    stream >> actionId;
    if (actionId > 0) {
        item->setActionId(actionId);
    }

    quint16 uniqueId;
    stream >> uniqueId;
    if (uniqueId > 0) {
        item->setUniqueId(uniqueId);
    }

    // Read text length and text
    quint16 textLength;
    stream >> textLength;
    if (textLength > 0) {
        QByteArray textData(textLength, 0);
        stream.readRawData(textData.data(), textLength);
        item->setText(QString::fromUtf8(textData));
    }

    tile->addItem(item);
    return true;
}

QString OTMMMapLoader::getLastError() const {
    return lastError_;
}

void OTMMMapLoader::clearError() const {
    lastError_.clear();
}

bool OTMMMapLoader::saveTile(const Tile* tile, QDataStream& stream) const {
    // Write tile position
    stream << static_cast<quint16>(tile->getX());
    stream << static_cast<quint16>(tile->getY());
    stream << static_cast<quint8>(tile->getZ());

    // Write tile flags
    stream << static_cast<quint32>(tile->getMapFlags());

    // Write house tile flag
    stream << static_cast<quint8>(tile->isHouseTile() ? 1 : 0);
    if (tile->isHouseTile()) {
        stream << static_cast<quint8>(tile->getHouseDoorId());
    }

    // Write item count
    const QList<Item*>& items = tile->getItems();
    stream << static_cast<quint16>(items.size());

    // Write items
    for (const Item* item : items) {
        if (!saveItem(item, stream)) {
            return false;
        }
    }

    return stream.status() == QDataStream::Ok;
}

bool OTMMMapLoader::saveItem(const Item* item, QDataStream& stream) const {
    // Write item ID
    stream << static_cast<quint16>(item->getID());

    // Write item properties
    stream << static_cast<quint8>(item->getCount());
    stream << static_cast<quint16>(item->getActionId());
    stream << static_cast<quint16>(item->getUniqueId());

    // Write text
    QByteArray textData = item->getText().toUtf8();
    stream << static_cast<quint16>(textData.size());
    if (!textData.isEmpty()) {
        stream.writeRawData(textData.constData(), textData.size());
    }

    return stream.status() == QDataStream::Ok;
}

void OTMMMapLoader::setError(const QString& error) const {
    lastError_ = error;
    qWarning() << "OTMMMapLoader Error:" << error;
}
