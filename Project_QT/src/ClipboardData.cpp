#include "ClipboardData.h"
#include "Map.h"    // Access to Map, Tile, MapPos
#include "Tile.h"   // For Tile class definition (if not fully opaque)
#include "Item.h"   // For Item class definition
#include "Creature.h" // For Creature class definition
#include "Spawn.h"  // For Spawn class definition

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <algorithm> // For std::min/max

// --- Constructor & Destructor ---

ClipboardData::ClipboardData()
    : selectionWidth_(0), selectionHeight_(0), selectionDepth_(0) {
}

ClipboardData::~ClipboardData() {
}

// --- Helper: Tile to ClipboardTileData ---

ClipboardTileData ClipboardData::tileToClipboardTileData(const Tile* tile, const MapPos& selectionOrigin) {
    ClipboardTileData ctd;
    if (!tile) {
        qWarning() << "tileToClipboardTileData: Called with null tile.";
        // Return default ctd, which will have hasGround/Creature/Spawn = false
        return ctd;
    }

    // Relative Position
    ctd.relativePosition = MapPos(
        tile->x() - selectionOrigin.x,
        tile->y() - selectionOrigin.y,
        tile->z() - selectionOrigin.z
    );

    // Ground Item
    const Item* groundItem = tile->getGround(); // Assuming Tile::getGround() returns Item*
    if (groundItem) {
        ctd.ground.id = groundItem->getServerId(); // Corrected method name
        // ctd.ground.countOrSubType = groundItem->getCountOrSubType(); // Example
        // Populate ctd.ground.properties if needed
        ctd.hasGround = true;
    }

    // Top Items
    for (const Item* item : tile->items()) { // Corrected method name
        if (item) {
            ClipboardItemData cid;
            cid.id = item->getServerId(); // Corrected method name
            // cid.countOrSubType = item->getCountOrSubType(); // Example
            // Populate cid.properties if needed
            ctd.items.append(cid);
        }
    }
    
    // Creature
    const Creature* creature = tile->creature(); // Corrected method name
    if (creature) {
        ctd.creature.name = creature->name(); // Corrected method name
        // ctd.creature.offset = ...; // If creatures have sub-tile offsets
        // Populate ctd.creature.properties if needed
        ctd.hasCreature = true;
    }

    // Spawn
    const Spawn* spawn = tile->spawn(); // Corrected method name
    if (spawn) {
        ctd.spawn.radius = spawn->radius();
        ctd.spawn.interval = spawn->interval();
        ctd.spawn.maxCreatures = spawn->maxCreatures();
        ctd.spawn.creatureNames = spawn->creatureNames();
        // Populate ctd.spawn.properties if needed
        ctd.hasSpawn = true;
    }

    // Tile Flags
    ctd.tileFlags = tile->getMapFlags(); // Corrected method name

    return ctd;
}

// --- Main Population Method ---

void ClipboardData::populateFromSelection(const QSet<MapPos>& selection, const Map& map) {
    copiedTiles_.clear();
    selectionWidth_ = 0;
    selectionHeight_ = 0;
    selectionDepth_ = 0;

    if (selection.isEmpty()) {
        return;
    }

    MapPos minPos(99999, 99999, 15); // Arbitrarily large values for min
    MapPos maxPos(0, 0, 0);      // Arbitrarily small values for max

    for (const MapPos& pos : selection) {
        minPos.x = std::min(minPos.x, pos.x);
        minPos.y = std::min(minPos.y, pos.y);
        minPos.z = std::min(minPos.z, pos.z);
        maxPos.x = std::max(maxPos.x, pos.x);
        maxPos.y = std::max(maxPos.y, pos.y);
        maxPos.z = std::max(maxPos.z, pos.z);
    }
    const MapPos selectionOrigin = minPos;

    for (const MapPos& pos : selection) {
        const Tile* tile = map.getTile(pos);
        // We always add a tile data entry, even for null tiles in selection,
        // to preserve the shape of the selection. The relativePosition will be correct.
        copiedTiles_.append(tileToClipboardTileData(tile, selectionOrigin));
    }

    if (!selection.isEmpty()) {
        selectionWidth_ = maxPos.x - minPos.x + 1;
        selectionHeight_ = maxPos.y - minPos.y + 1;
        selectionDepth_ = maxPos.z - minPos.z + 1;
    }
}


// --- JSON Helper Implementations ---

QJsonObject ClipboardData::itemDataToJsonObject(const ClipboardItemData& itemData) const {
    QJsonObject itemJson;
    itemJson["id"] = itemData.id;
    itemJson["countOrSubType"] = itemData.countOrSubType;
    if (!itemData.properties.isEmpty()) {
        itemJson["properties"] = QJsonObject::fromVariantMap(itemData.properties);
    }
    return itemJson;
}

ClipboardItemData ClipboardData::jsonObjectToItemData(const QJsonObject& itemJson) {
    ClipboardItemData itemData;
    itemData.id = itemJson["id"].toInt(0);
    itemData.countOrSubType = itemJson["countOrSubType"].toInt(0);
    if (itemJson.contains("properties") && itemJson["properties"].isObject()) {
        itemData.properties = itemJson["properties"].toObject().toVariantMap();
    }
    return itemData;
}

QJsonObject ClipboardData::creatureDataToJsonObject(const ClipboardCreatureData& creatureData) const {
    QJsonObject creatureJson;
    creatureJson["name"] = creatureData.name;
    // creatureJson["offsetX"] = creatureData.offset.x; // Example for offset
    // creatureJson["offsetY"] = creatureData.offset.y;
    // creatureJson["offsetZ"] = creatureData.offset.z;
    if (!creatureData.properties.isEmpty()) {
        creatureJson["properties"] = QJsonObject::fromVariantMap(creatureData.properties);
    }
    return creatureJson;
}

ClipboardCreatureData ClipboardData::jsonObjectToCreatureData(const QJsonObject& creatureJson) {
    ClipboardCreatureData creatureData;
    creatureData.name = creatureJson["name"].toString();
    // creatureData.offset.x = creatureJson["offsetX"].toInt(0); // Example
    // creatureData.offset.y = creatureJson["offsetY"].toInt(0);
    // creatureData.offset.z = creatureJson["offsetZ"].toInt(0);
    if (creatureJson.contains("properties") && creatureJson["properties"].isObject()) {
        creatureData.properties = creatureJson["properties"].toObject().toVariantMap();
    }
    return creatureData;
}

QJsonObject ClipboardData::spawnDataToJsonObject(const ClipboardSpawnData& spawnData) const {
    QJsonObject spawnJson;
    spawnJson["radius"] = spawnData.radius;
    spawnJson["interval"] = spawnData.interval;
    spawnJson["maxCreatures"] = spawnData.maxCreatures;
    spawnJson["creatureNames"] = QJsonArray::fromStringList(spawnData.creatureNames);
    if (!spawnData.properties.isEmpty()) {
        spawnJson["properties"] = QJsonObject::fromVariantMap(spawnData.properties);
    }
    return spawnJson;
}

ClipboardSpawnData ClipboardData::jsonObjectToSpawnData(const QJsonObject& spawnJson) {
    ClipboardSpawnData spawnData;
    spawnData.radius = spawnJson["radius"].toInt(0);
    spawnData.interval = spawnJson["interval"].toInt(10000);
    spawnData.maxCreatures = spawnJson["maxCreatures"].toInt(1);
    if (spawnJson.contains("creatureNames") && spawnJson["creatureNames"].isArray()) {
        QJsonArray namesArray = spawnJson["creatureNames"].toArray();
        for (const QJsonValue& val : namesArray) {
            spawnData.creatureNames.append(val.toString());
        }
    }
    if (spawnJson.contains("properties") && spawnJson["properties"].isObject()) {
        spawnData.properties = spawnJson["properties"].toObject().toVariantMap();
    }
    return spawnData;
}

QJsonObject ClipboardData::tileDataToJsonObject(const ClipboardTileData& tileData) const {
    QJsonObject tileJson;
    QJsonObject posJson;
    posJson["x"] = tileData.relativePosition.x;
    posJson["y"] = tileData.relativePosition.y;
    posJson["z"] = tileData.relativePosition.z;
    tileJson["relativePosition"] = posJson;

    tileJson["hasGround"] = tileData.hasGround;
    if (tileData.hasGround) {
        tileJson["ground"] = itemDataToJsonObject(tileData.ground);
    }

    QJsonArray itemsArray;
    for (const ClipboardItemData& item : tileData.items) {
        itemsArray.append(itemDataToJsonObject(item));
    }
    if (!itemsArray.isEmpty()) {
        tileJson["items"] = itemsArray;
    }

    tileJson["hasCreature"] = tileData.hasCreature;
    if (tileData.hasCreature) {
        tileJson["creature"] = creatureDataToJsonObject(tileData.creature);
    }

    tileJson["hasSpawn"] = tileData.hasSpawn;
    if (tileData.hasSpawn) {
        tileJson["spawn"] = spawnDataToJsonObject(tileData.spawn);
    }
    tileJson["tileFlags"] = static_cast<qint64>(tileData.tileFlags); // Store as number
    return tileJson;
}

ClipboardTileData ClipboardData::jsonObjectToTileData(const QJsonObject& tileJson) {
    ClipboardTileData tileData;
    if (tileJson.contains("relativePosition") && tileJson["relativePosition"].isObject()) {
        QJsonObject posJson = tileJson["relativePosition"].toObject();
        tileData.relativePosition.x = posJson["x"].toInt(0);
        tileData.relativePosition.y = posJson["y"].toInt(0);
        tileData.relativePosition.z = posJson["z"].toInt(0);
    }

    tileData.hasGround = tileJson["hasGround"].toBool(false);
    if (tileData.hasGround && tileJson.contains("ground") && tileJson["ground"].isObject()) {
        tileData.ground = jsonObjectToItemData(tileJson["ground"].toObject());
    }

    if (tileJson.contains("items") && tileJson["items"].isArray()) {
        QJsonArray itemsArray = tileJson["items"].toArray();
        for (const QJsonValue& val : itemsArray) {
            if (val.isObject()) {
                tileData.items.append(jsonObjectToItemData(val.toObject()));
            }
        }
    }
    
    tileData.hasCreature = tileJson["hasCreature"].toBool(false);
    if (tileData.hasCreature && tileJson.contains("creature") && tileJson["creature"].isObject()) {
        tileData.creature = jsonObjectToCreatureData(tileJson["creature"].toObject());
    }

    tileData.hasSpawn = tileJson["hasSpawn"].toBool(false);
    if (tileData.hasSpawn && tileJson.contains("spawn") && tileJson["spawn"].isObject()) {
        tileData.spawn = jsonObjectToSpawnData(tileJson["spawn"].toObject());
    }
    tileData.tileFlags = static_cast<quint32>(tileJson["tileFlags"].toVariant().toULongLong());
    return tileData;
}


// --- Serialization & Deserialization ---

QByteArray ClipboardData::serializeToJson() const {
    QJsonObject rootObj;
    rootObj["selectionWidth"] = selectionWidth_;
    rootObj["selectionHeight"] = selectionHeight_;
    rootObj["selectionDepth"] = selectionDepth_;

    QJsonArray tilesArray;
    for (const ClipboardTileData& tileData : copiedTiles_) {
        tilesArray.append(tileDataToJsonObject(tileData));
    }
    rootObj["tiles"] = tilesArray;

    QJsonDocument doc(rootObj);
    return doc.toJson(QJsonDocument::Compact); // Use Compact for smaller size
}

bool ClipboardData::deserializeFromJson(const QByteArray& jsonData) {
    copiedTiles_.clear();
    selectionWidth_ = 0;
    selectionHeight_ = 0;
    selectionDepth_ = 0;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);

    if (doc.isNull()) {
        qWarning() << "ClipboardData::deserializeFromJson - Failed to parse JSON:" << error.errorString();
        return false;
    }
    if (!doc.isObject()) {
        qWarning() << "ClipboardData::deserializeFromJson - JSON document is not an object.";
        return false;
    }

    QJsonObject rootObj = doc.object();

    selectionWidth_ = rootObj["selectionWidth"].toInt(0);
    selectionHeight_ = rootObj["selectionHeight"].toInt(0);
    selectionDepth_ = rootObj["selectionDepth"].toInt(0);

    if (rootObj.contains("tiles") && rootObj["tiles"].isArray()) {
        QJsonArray tilesArray = rootObj["tiles"].toArray();
        for (const QJsonValue& tileVal : tilesArray) {
            if (tileVal.isObject()) {
                copiedTiles_.append(jsonObjectToTileData(tileVal.toObject()));
            } else {
                qWarning() << "ClipboardData::deserializeFromJson - Invalid tile data in array.";
            }
        }
    } else {
        qWarning() << "ClipboardData::deserializeFromJson - 'tiles' array missing or invalid.";
        // It might be valid for an empty selection to have no 'tiles' array.
        // If width/height/depth are 0, this might be acceptable.
        if (selectionWidth_ != 0 || selectionHeight_ != 0 || selectionDepth_ != 0) {
             // If dimensions are non-zero, missing tiles array is an error.
            return false;
        }
    }
    return true;
}

// --- Enhanced Operations ---

void ClipboardData::populateFromSelectionCopy(const QSet<MapPos>& selection, const Map& map) {
    isCutOperation_ = false;
    populateFromSelection(selection, map);
}

void ClipboardData::populateFromSelectionCut(const QSet<MapPos>& selection, const Map& map) {
    isCutOperation_ = true;
    populateFromSelection(selection, map);
}

// --- System Clipboard Integration ---

void ClipboardData::copyToSystemClipboard() const {
    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard) {
        qWarning() << "ClipboardData::copyToSystemClipboard - No system clipboard available";
        return;
    }

    QByteArray jsonData = serializeToJson();

    // Create MIME data with custom format for our clipboard data
    QMimeData* mimeData = new QMimeData();
    mimeData->setData("application/x-rme-clipboard", jsonData);
    mimeData->setText(QString::fromUtf8(jsonData)); // Also set as text for debugging

    clipboard->setMimeData(mimeData);
    qDebug() << "ClipboardData copied to system clipboard:" << jsonData.size() << "bytes";
}

bool ClipboardData::pasteFromSystemClipboard() {
    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard) {
        qWarning() << "ClipboardData::pasteFromSystemClipboard - No system clipboard available";
        return false;
    }

    const QMimeData* mimeData = clipboard->mimeData();
    if (!mimeData) {
        qWarning() << "ClipboardData::pasteFromSystemClipboard - No MIME data on clipboard";
        return false;
    }

    QByteArray jsonData;

    // Try our custom format first
    if (mimeData->hasFormat("application/x-rme-clipboard")) {
        jsonData = mimeData->data("application/x-rme-clipboard");
    } else if (mimeData->hasText()) {
        // Fallback to text format
        jsonData = mimeData->text().toUtf8();
    } else {
        qWarning() << "ClipboardData::pasteFromSystemClipboard - No compatible data on clipboard";
        return false;
    }

    bool success = deserializeFromJson(jsonData);
    if (success) {
        qDebug() << "ClipboardData pasted from system clipboard:" << jsonData.size() << "bytes";
    } else {
        qWarning() << "ClipboardData::pasteFromSystemClipboard - Failed to deserialize clipboard data";
    }

    return success;
}

// --- Utility Methods ---

void ClipboardData::clear() {
    copiedTiles_.clear();
    selectionWidth_ = 0;
    selectionHeight_ = 0;
    selectionDepth_ = 0;
    isCutOperation_ = false;
}

ClipboardData* ClipboardData::deepCopy() const {
    ClipboardData* copy = new ClipboardData();
    copy->copiedTiles_ = copiedTiles_; // QList copy constructor does deep copy of structs
    copy->selectionWidth_ = selectionWidth_;
    copy->selectionHeight_ = selectionHeight_;
    copy->selectionDepth_ = selectionDepth_;
    copy->isCutOperation_ = isCutOperation_;
    return copy;
}

int ClipboardData::getTileCount() const {
    return copiedTiles_.size();
}

int ClipboardData::getItemCount() const {
    int itemCount = 0;
    for (const ClipboardTileData& tileData : copiedTiles_) {
        if (tileData.hasGround) {
            itemCount++;
        }
        itemCount += tileData.items.size();
    }
    return itemCount;
}
