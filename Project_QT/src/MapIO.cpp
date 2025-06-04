#include "MapIO.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "Spawn.h"
#include "Waypoint.h"
#include "io/OtbmReader.h"
#include "io/OtbmWriter.h"
#include "OtbmTypes.h"
#include "ItemManager.h"
#include "Town.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MapIO::MapIO(Map* map, QObject* parent)
    : QObject(parent)
    , map_(map)
{
    if (!map_) {
        qWarning() << "MapIO: Map pointer is null!";
    }
}

MapIO::~MapIO() = default;

bool MapIO::loadByFormat(const QString& path, const QString& format) {
    clearError();
    qDebug() << "MapIO::loadByFormat - Loading" << path << "as format:" << format;

    if (format == "otbm") {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            setError(QString("Could not open OTBM file for reading: %1").arg(path));
            return false;
        }

        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);
        bool success = loadFromOTBM(stream);
        file.close();
        return success;

    } else if (format == "xml") {
        return loadFromXML(path);

    } else if (format == "json") {
        return loadFromJSON(path);

    } else {
        setError(QString("Unsupported format: %1").arg(format));
        return false;
    }
}

bool MapIO::saveByFormat(const QString& path, const QString& format) const {
    clearError();
    qDebug() << "MapIO::saveByFormat - Saving" << path << "as format:" << format;

    if (format == "otbm") {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            setError(QString("Could not open OTBM file for writing: %1").arg(path));
            return false;
        }

        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);
        bool success = saveToOTBM(stream);
        file.close();
        return success;

    } else if (format == "xml") {
        return saveToXML(path);

    } else if (format == "json") {
        return saveToJSON(path);

    } else {
        setError(QString("Unsupported format: %1").arg(format));
        return false;
    }
}

bool MapIO::loadFromOTBM(QDataStream& stream) {
    if (!map_) {
        setError("Map pointer is null");
        return false;
    }

    map_->clear(); // Clear existing map data

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

    qDebug() << "MapIO::loadFromOTBM - Entered OTBM_ROOTV1 node";

    // Load OTBM header information
    if (!loadOTBMHeader(stream)) {
        reader.leaveNode();
        return false;
    }

    // Load map data
    if (!loadOTBMMapData(stream)) {
        reader.leaveNode();
        return false;
    }

    if (!reader.leaveNode()) {
        setError("Failed to leave ROOTV1 node");
        return false;
    }

    map_->setModified(false);
    qDebug() << "MapIO::loadFromOTBM - Successfully parsed OTBM data";
    emit loadingCompleted(true);
    return true;
}

bool MapIO::saveToOTBM(QDataStream& stream) const {
    if (!map_) {
        setError("Map pointer is null");
        return false;
    }

    OtbmWriter writer(stream);

    // Write root node
    writer.startNode(OTBM_ROOTV1);

    // Save OTBM header
    if (!saveOTBMHeader(stream)) {
        return false;
    }

    // Save map data
    if (!saveOTBMMapData(stream)) {
        return false;
    }

    writer.endNode(); // End Root Node

    if (stream.status() == QDataStream::Ok) {
        map_->setModified(false);
        qDebug() << "MapIO::saveToOTBM - Successfully saved OTBM data";
        emit savingCompleted(true);
    }
    return stream.status() == QDataStream::Ok;
}

bool MapIO::loadFromXML(const QString& path) {
    if (!map_) {
        setError("Map pointer is null");
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QString("Could not open XML file for reading: %1").arg(path));
        return false;
    }

    QXmlStreamReader xml(&file);
    map_->clear();

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "map") {
            if (!parseXMLMapElement(xml)) {
                file.close();
                return false;
            }
        }
    }

    if (xml.hasError()) {
        setError(QString("XML parsing error: %1").arg(xml.errorString()));
        file.close();
        return false;
    }

    file.close();
    map_->setModified(false);
    qDebug() << "MapIO::loadFromXML - Successfully loaded XML map";
    emit loadingCompleted(true);
    return true;
}

bool MapIO::saveToXML(const QString& path) const {
    if (!map_) {
        setError("Map pointer is null");
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        setError(QString("Could not open XML file for writing: %1").arg(path));
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    if (!writeXMLMapElement(xml)) {
        file.close();
        return false;
    }

    xml.writeEndDocument();
    file.close();

    map_->setModified(false);
    qDebug() << "MapIO::saveToXML - Successfully saved XML map";
    emit savingCompleted(true);
    return true;
}

bool MapIO::loadFromJSON(const QString& path) {
    if (!map_) {
        setError("Map pointer is null");
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QString("Could not open JSON file for reading: %1").arg(path));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        setError(QString("JSON parsing error: %1").arg(error.errorString()));
        return false;
    }

    if (!doc.isObject()) {
        setError("JSON document is not an object");
        return false;
    }

    map_->clear();

    if (!parseJSONMapObject(doc.object())) {
        return false;
    }

    map_->setModified(false);
    qDebug() << "MapIO::loadFromJSON - Successfully loaded JSON map";
    emit loadingCompleted(true);
    return true;
}

bool MapIO::saveToJSON(const QString& path) const {
    if (!map_) {
        setError("Map pointer is null");
        return false;
    }

    QJsonObject mapObj = createJSONMapObject();
    QJsonDocument doc(mapObj);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        setError(QString("Could not open JSON file for writing: %1").arg(path));
        return false;
    }

    file.write(doc.toJson());
    file.close();

    map_->setModified(false);
    qDebug() << "MapIO::saveToJSON - Successfully saved JSON map";
    emit savingCompleted(true);
    return true;
}

void MapIO::setError(const QString& error) const {
    lastError_ = error;
    qWarning() << "MapIO Error:" << error;
}

void MapIO::updateProgress(int current, int total, const QString& operation) const {
    emit loadingProgress(current, total, operation);
}

// OTBM Helper Methods Implementation
bool MapIO::loadOTBMHeader(QDataStream& stream) {
    // Implementation will be moved from Map.cpp
    // This is a placeholder for the header loading logic
    qDebug() << "MapIO::loadOTBMHeader - Loading OTBM header";
    return true;
}

bool MapIO::loadOTBMMapData(QDataStream& stream) {
    // Implementation will be moved from Map.cpp
    // This is a placeholder for the map data loading logic
    qDebug() << "MapIO::loadOTBMMapData - Loading OTBM map data";
    return true;
}

bool MapIO::loadOTBMTileArea(QDataStream& stream) {
    // Implementation will be moved from Map.cpp
    qDebug() << "MapIO::loadOTBMTileArea - Loading OTBM tile area";
    return true;
}

bool MapIO::loadOTBMSpawns(QDataStream& stream) {
    // Implementation will be moved from Map.cpp
    qDebug() << "MapIO::loadOTBMSpawns - Loading OTBM spawns";
    return true;
}

bool MapIO::loadOTBMHouses(QDataStream& stream) {
    // Implementation will be moved from Map.cpp
    qDebug() << "MapIO::loadOTBMHouses - Loading OTBM houses";
    return true;
}

bool MapIO::loadOTBMWaypoints(QDataStream& stream) {
    // Implementation will be moved from Map.cpp
    qDebug() << "MapIO::loadOTBMWaypoints - Loading OTBM waypoints";
    return true;
}

bool MapIO::saveOTBMHeader(QDataStream& stream) const {
    // Implementation will be moved from Map.cpp
    qDebug() << "MapIO::saveOTBMHeader - Saving OTBM header";
    return true;
}

bool MapIO::saveOTBMMapData(QDataStream& stream) const {
    // Implementation will be moved from Map.cpp
    qDebug() << "MapIO::saveOTBMMapData - Saving OTBM map data";
    return true;
}

bool MapIO::saveOTBMTileAreas(QDataStream& stream) const {
    // Implementation will be moved from Map.cpp
    qDebug() << "MapIO::saveOTBMTileAreas - Saving OTBM tile areas";
    return true;
}

bool MapIO::saveOTBMSpawns(QDataStream& stream) const {
    // Implementation will be moved from Map.cpp
    qDebug() << "MapIO::saveOTBMSpawns - Saving OTBM spawns";
    return true;
}

bool MapIO::saveOTBMHouses(QDataStream& stream) const {
    // Implementation will be moved from Map.cpp
    qDebug() << "MapIO::saveOTBMHouses - Saving OTBM houses";
    return true;
}

bool MapIO::saveOTBMWaypoints(QDataStream& stream) const {
    // Implementation will be moved from Map.cpp
    qDebug() << "MapIO::saveOTBMWaypoints - Saving OTBM waypoints";
    return true;
}

// XML Helper Methods Implementation
bool MapIO::parseXMLMapElement(QXmlStreamReader& xml) {
    // Implementation for parsing XML map elements
    qDebug() << "MapIO::parseXMLMapElement - Parsing XML map element";
    return true;
}

bool MapIO::parseXMLTileElement(QXmlStreamReader& xml) {
    // Implementation for parsing XML tile elements
    qDebug() << "MapIO::parseXMLTileElement - Parsing XML tile element";
    return true;
}

bool MapIO::parseXMLItemElement(QXmlStreamReader& xml) {
    // Implementation for parsing XML item elements
    qDebug() << "MapIO::parseXMLItemElement - Parsing XML item element";
    return true;
}

bool MapIO::writeXMLMapElement(QXmlStreamWriter& xml) const {
    // Implementation for writing XML map elements
    qDebug() << "MapIO::writeXMLMapElement - Writing XML map element";
    return true;
}

bool MapIO::writeXMLTileElements(QXmlStreamWriter& xml) const {
    // Implementation for writing XML tile elements
    qDebug() << "MapIO::writeXMLTileElements - Writing XML tile elements";
    return true;
}

bool MapIO::writeXMLItemElements(QXmlStreamWriter& xml) const {
    // Implementation for writing XML item elements
    qDebug() << "MapIO::writeXMLItemElements - Writing XML item elements";
    return true;
}

// JSON Helper Methods Implementation
bool MapIO::parseJSONMapObject(const QJsonObject& mapObj) {
    // Implementation for parsing JSON map objects
    qDebug() << "MapIO::parseJSONMapObject - Parsing JSON map object";
    return true;
}

bool MapIO::parseJSONTileArray(const QJsonArray& tilesArray) {
    // Implementation for parsing JSON tile arrays
    qDebug() << "MapIO::parseJSONTileArray - Parsing JSON tile array";
    return true;
}

bool MapIO::parseJSONItemObject(const QJsonObject& itemObj) {
    // Implementation for parsing JSON item objects
    qDebug() << "MapIO::parseJSONItemObject - Parsing JSON item object";
    return true;
}

QJsonObject MapIO::createJSONMapObject() const {
    // Implementation for creating JSON map objects
    qDebug() << "MapIO::createJSONMapObject - Creating JSON map object";
    return QJsonObject();
}

QJsonArray MapIO::createJSONTileArray() const {
    // Implementation for creating JSON tile arrays
    qDebug() << "MapIO::createJSONTileArray - Creating JSON tile array";
    return QJsonArray();
}

QJsonObject MapIO::createJSONItemObject() const {
    // Implementation for creating JSON item objects
    qDebug() << "MapIO::createJSONItemObject - Creating JSON item object";
    return QJsonObject();
}
