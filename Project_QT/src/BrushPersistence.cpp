#include "BrushPersistence.h"
#include "BrushManager.h"
#include "Brush.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>
#include <QStandardPaths>
#include <QApplication>

// Static constants - EXACT wxwidgets compatibility
const QString BrushPersistence::XML_ROOT_ELEMENT = "materials"; // wxwidgets uses "materials" not "brushes"
const QString BrushPersistence::JSON_FORMAT_IDENTIFIER = "qt_brush_collection";
const QString BrushPersistence::BRUSH_FILE_VERSION = "1.0";
// EXACT wxwidgets brush type strings (from brush.cpp line 116-127)
const QStringList BrushPersistence::SERIALIZABLE_BRUSH_TYPES = {
    "ground", "border", "wall", "wall decoration", "carpet",
    "table", "doodad", "creature", "spawn", "house", "waypoint", "eraser"
};

BrushPersistence::BrushPersistence(BrushManager* brushManager, QObject* parent)
    : QObject(parent)
    , brushManager_(brushManager)
    , defaultSaveFormat_(JSON_FORMAT)
    , autoBackupEnabled_(true)
{
    Q_ASSERT(brushManager_);
}

BrushPersistence::~BrushPersistence() = default;

bool BrushPersistence::saveBrushes(const QString& filePath, FileFormat format, SaveMode mode) {
    if (!brushManager_) {
        setLastError("No brush manager available", "BrushManager is null");
        return false;
    }
    
    clearLastError();
    
    // Determine actual format
    FileFormat actualFormat = (format == AUTO_DETECT) ? defaultSaveFormat_ : format;
    
    // Create backup if enabled
    if (autoBackupEnabled_ && QFile::exists(filePath)) {
        if (!createBackup(filePath)) {
            qWarning() << "Failed to create backup for" << filePath;
        }
    }
    
    // Get brushes to save based on mode
    QStringList brushNames;
    switch (mode) {
        case SAVE_ALL:
            brushNames = brushManager_->getBrushes().keys();
            break;
        case SAVE_USER_DEFINED:
            for (const QString& name : brushManager_->getBrushes().keys()) {
                if (isBrushUserDefined(name)) {
                    brushNames.append(name);
                }
            }
            break;
        case SAVE_MODIFIED:
            for (const QString& name : brushManager_->getBrushes().keys()) {
                if (isBrushModified(name)) {
                    brushNames.append(name);
                }
            }
            break;
        case SAVE_SELECTED:
            // This would be set externally before calling - use modified brushes as fallback
            brushNames = brushManager_->getModifiedBrushes();
            break;
    }
    
    if (brushNames.isEmpty()) {
        setLastError("No brushes to save", QString("Save mode: %1").arg(mode));
        return false;
    }
    
    // Serialize brushes
    QList<BrushSerializationData> brushData;
    int current = 0;
    for (const QString& name : brushNames) {
        emit saveProgress(current++, brushNames.size(), name);
        
        Brush* brush = brushManager_->getBrush(name);
        if (brush) {
            BrushSerializationData data = serializeBrush(brush);
            if (validateBrushData(data)) {
                brushData.append(data);
            } else {
                qWarning() << "Failed to validate brush data for" << name;
            }
        }
    }
    
    // Create metadata
    QVariantMap metadata;
    metadata["version"] = BRUSH_FILE_VERSION;
    metadata["timestamp"] = QDateTime::currentDateTime().toSecsSinceEpoch();
    metadata["application"] = QApplication::applicationName();
    metadata["count"] = brushData.size();
    metadata["save_mode"] = mode;
    
    // Save based on format
    bool success = false;
    switch (actualFormat) {
        case XML_FORMAT:
            success = saveToXML(filePath, brushData, metadata);
            break;
        case JSON_FORMAT:
            success = saveToJSON(filePath, brushData, metadata);
            break;
        case BINARY_FORMAT:
            setLastError("Binary format not yet implemented", "Use XML or JSON format");
            return false;
        default:
            setLastError("Unknown file format", QString("Format: %1").arg(actualFormat));
            return false;
    }
    
    if (success) {
        // Clear modified flags for saved brushes
        if (mode == SAVE_MODIFIED || mode == SAVE_ALL) {
            for (const QString& name : brushNames) {
                modifiedBrushes_.remove(name);
            }
        }
        
        emit saveProgress(brushNames.size(), brushNames.size(), "Complete");
        qDebug() << "Successfully saved" << brushData.size() << "brushes to" << filePath;
    }
    
    return success;
}

bool BrushPersistence::loadBrushes(const QString& filePath, FileFormat format) {
    if (!brushManager_) {
        setLastError("No brush manager available", "BrushManager is null");
        return false;
    }
    
    if (!QFile::exists(filePath)) {
        setLastError("File not found", filePath);
        return false;
    }
    
    clearLastError();
    
    // Determine actual format
    FileFormat actualFormat = (format == AUTO_DETECT) ? detectFileFormat(filePath) : format;
    
    // Validate file format
    if (!validateBrushFile(filePath, actualFormat)) {
        setLastError("Invalid brush file format", filePath);
        return false;
    }
    
    // Load based on format
    QList<BrushSerializationData> brushData;
    QVariantMap metadata;
    bool success = false;
    
    switch (actualFormat) {
        case XML_FORMAT:
            success = loadFromXML(filePath, brushData, metadata);
            break;
        case JSON_FORMAT:
            success = loadFromJSON(filePath, brushData, metadata);
            break;
        case BINARY_FORMAT:
            setLastError("Binary format not yet implemented", "Use XML or JSON format");
            return false;
        default:
            setLastError("Unknown file format", QString("Format: %1").arg(actualFormat));
            return false;
    }
    
    if (!success) {
        return false;
    }
    
    // Deserialize and add brushes
    int current = 0;
    int successCount = 0;
    for (const BrushSerializationData& data : brushData) {
        emit loadProgress(current++, brushData.size(), data.name);
        
        if (validateBrushData(data)) {
            Brush* brush = deserializeBrush(data);
            if (brush) {
                if (brushManager_->addBrush(brush)) {
                    successCount++;
                    
                    // Update state tracking
                    if (data.isUserDefined) {
                        userDefinedBrushes_.insert(data.name);
                    }
                    if (data.isModified) {
                        modifiedBrushes_.insert(data.name);
                    }
                    brushTimestamps_[data.name] = data.timestamp;
                    
                    emit brushLoaded(data.name, filePath);
                } else {
                    qWarning() << "Failed to add brush to manager:" << data.name;
                }
            } else {
                qWarning() << "Failed to deserialize brush:" << data.name;
            }
        } else {
            qWarning() << "Invalid brush data:" << data.name;
        }
    }
    
    emit loadProgress(brushData.size(), brushData.size(), "Complete");
    qDebug() << "Successfully loaded" << successCount << "of" << brushData.size() << "brushes from" << filePath;
    
    return successCount > 0;
}

BrushSerializationData BrushPersistence::serializeBrush(Brush* brush) const {
    BrushSerializationData data;
    
    if (!brush) {
        return data;
    }
    
    // Basic properties
    data.name = brush->name();
    data.type = brushTypeToString(brush->type());
    data.id = brush->getID();
    data.version = BRUSH_FILE_VERSION;
    data.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
    data.isUserDefined = isBrushUserDefined(brush->name());
    data.isModified = isBrushModified(brush->name());
    
    // Extract brush-specific properties
    data.properties = extractBrushProperties(brush);
    
    // Extract dependencies
    data.dependencies = extractBrushDependencies(brush);
    
    // Custom data (for user-defined brushes)
    if (data.isUserDefined) {
        // Note: getCustomData() method doesn't exist in base Brush class
        // This would need to be implemented per brush type or as a base method
        // data.customData = brush->getCustomData();
    }
    
    return data;
}

Brush* BrushPersistence::deserializeBrush(const BrushSerializationData& data) const {
    if (!validateBrushData(data)) {
        return nullptr;
    }
    
    // Create brush based on type
    Brush* brush = brushManager_->createBrush(this->stringToBrushType(data.type));
    if (!brush) {
        qWarning() << "Failed to create brush of type:" << data.type;
        return nullptr;
    }
    
    // Set basic properties
    brush->setName(data.name);
    // Note: setId() method doesn't exist in base Brush class - ID is set during construction
    
    // Apply brush-specific properties
    applyBrushProperties(brush, data.properties);
    
    // Apply custom data for user-defined brushes
    if (data.isUserDefined && !data.customData.isEmpty()) {
        // Note: setCustomData() method doesn't exist in base Brush class
        // This would need to be implemented per brush type or as a base method
        // brush->setCustomData(data.customData);
    }
    
    return brush;
}

BrushPersistence::FileFormat BrushPersistence::detectFileFormat(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return AUTO_DETECT;
    }
    
    QTextStream stream(&file);
    QString firstLine = stream.readLine().trimmed();
    
    if (firstLine.startsWith("<?xml") || firstLine.startsWith("<" + XML_ROOT_ELEMENT)) {
        return XML_FORMAT;
    } else if (firstLine.startsWith("{")) {
        // Check for JSON format identifier
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject root = doc.object();
            if (root.contains("format") && root["format"].toString() == JSON_FORMAT_IDENTIFIER) {
                return JSON_FORMAT;
            }
            return JSON_FORMAT; // Assume JSON if valid JSON
        }
    }
    
    return AUTO_DETECT;
}

bool BrushPersistence::validateBrushFile(const QString& filePath, FileFormat format) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    switch (format) {
        case XML_FORMAT: {
            QDomDocument doc;
            QString errorMsg;
            int errorLine, errorColumn;
            if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
                qWarning() << "XML parse error:" << errorMsg << "at line" << errorLine;
                return false;
            }
            return validateXMLStructure(doc);
        }
        case JSON_FORMAT: {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
            if (error.error != QJsonParseError::NoError) {
                qWarning() << "JSON parse error:" << error.errorString();
                return false;
            }
            return validateJSONStructure(doc);
        }
        default:
            return false;
    }
}

void BrushPersistence::markBrushAsModified(const QString& brushName) {
    modifiedBrushes_.insert(brushName);
    brushTimestamps_[brushName] = QDateTime::currentDateTime().toSecsSinceEpoch();
}

void BrushPersistence::markBrushAsUserDefined(const QString& brushName) {
    userDefinedBrushes_.insert(brushName);
    markBrushAsModified(brushName);
}

bool BrushPersistence::isBrushModified(const QString& brushName) const {
    return modifiedBrushes_.contains(brushName);
}

bool BrushPersistence::isBrushUserDefined(const QString& brushName) const {
    return userDefinedBrushes_.contains(brushName);
}

QStringList BrushPersistence::getAvailableCollections(const QString& directory) const {
    QStringList collections;
    QDir dir(directory);
    
    if (!dir.exists()) {
        return collections;
    }
    
    QStringList filters;
    filters << "*.xml" << "*.json";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& fileInfo : files) {
        if (validateBrushFile(fileInfo.absoluteFilePath())) {
            collections.append(fileInfo.baseName());
        }
    }
    
    return collections;
}

BrushCollectionData BrushPersistence::createCollectionFromBrushes(const QStringList& brushNames, const QString& collectionName) const {
    BrushCollectionData collection;
    collection.name = collectionName;
    collection.description = QString("Collection of %1 brushes").arg(brushNames.size());
    collection.author = QApplication::applicationName();
    collection.version = BRUSH_FILE_VERSION;
    collection.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
    
    for (const QString& name : brushNames) {
        Brush* brush = brushManager_->getBrush(name);
        if (brush) {
            collection.brushes.append(serializeBrush(brush));
        }
    }
    
    collection.metadata["brush_count"] = collection.brushes.size();
    collection.metadata["created_by"] = QApplication::applicationName();
    
    return collection;
}

// XML serialization methods - EXACT wxwidgets format
bool BrushPersistence::saveToXML(const QString& filePath, const QList<BrushSerializationData>& brushes, const QVariantMap& metadata) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setLastError("Cannot open file for writing", filePath);
        return false;
    }

    QDomDocument doc;
    QDomElement root = doc.createElement(XML_ROOT_ELEMENT); // "materials"
    doc.appendChild(root);

    // NO metadata section - wxwidgets doesn't use metadata
    // Add brushes directly to materials root (exact wxwidgets format)
    for (const BrushSerializationData& data : brushes) {
        QDomElement brushElement = serializeBrushToXML(data, doc);
        root.appendChild(brushElement);
    }

    QTextStream stream(&file);
    stream << doc.toString(2);

    return true;
}

bool BrushPersistence::loadFromXML(const QString& filePath, QList<BrushSerializationData>& brushes, QVariantMap& metadata) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setLastError("Cannot open file for reading", filePath);
        return false;
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        setLastError("XML parse error", QString("%1 at line %2").arg(errorMsg).arg(errorLine));
        return false;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != XML_ROOT_ELEMENT) {
        setLastError("Invalid XML root element", QString("Expected: %1, Found: %2").arg(XML_ROOT_ELEMENT, root.tagName()));
        return false;
    }

    // NO metadata loading - wxwidgets doesn't use metadata

    // Load brushes - EXACT wxwidgets logic from materials.cpp
    QDomNodeList brushNodes = root.elementsByTagName("brush");
    for (int i = 0; i < brushNodes.count(); ++i) {
        QDomElement brushElement = brushNodes.at(i).toElement();
        BrushSerializationData data = deserializeBrushFromXML(brushElement);
        if (validateBrushData(data)) {
            brushes.append(data);
        }
    }

    return true;
}

QDomElement BrushPersistence::serializeBrushToXML(const BrushSerializationData& data, QDomDocument& doc) const {
    QDomElement brushElement = doc.createElement("brush");

    // EXACT wxwidgets attributes (from brush.cpp unserializeBrush)
    brushElement.setAttribute("name", data.name);
    brushElement.setAttribute("type", data.type); // Must be exact: "ground", "wall", etc.

    // Add brush-specific attributes based on type (exact wxwidgets format)
    if (data.type == "ground" || data.type == "border") {
        // GroundBrush attributes (from ground_brush.cpp load method)
        if (data.properties.contains("lookid")) {
            brushElement.setAttribute("lookid", data.properties["lookid"].toString());
        }
        if (data.properties.contains("server_lookid")) {
            brushElement.setAttribute("server_lookid", data.properties["server_lookid"].toString());
        }
        if (data.properties.contains("z-order")) {
            brushElement.setAttribute("z-order", data.properties["z-order"].toString());
        }
        if (data.properties.contains("solo_optional")) {
            brushElement.setAttribute("solo_optional", data.properties["solo_optional"].toBool() ? "true" : "false");
        }
        if (data.properties.contains("randomize")) {
            brushElement.setAttribute("randomize", data.properties["randomize"].toBool() ? "true" : "false");
        }
    } else if (data.type == "wall") {
        // WallBrush attributes
        if (data.properties.contains("lookid")) {
            brushElement.setAttribute("lookid", data.properties["lookid"].toString());
        }
        if (data.properties.contains("server_lookid")) {
            brushElement.setAttribute("server_lookid", data.properties["server_lookid"].toString());
        }
    } else if (data.type == "doodad") {
        // DoodadBrush attributes (from doodad_brush.cpp load method)
        if (data.properties.contains("lookid")) {
            brushElement.setAttribute("lookid", data.properties["lookid"].toString());
        }
        if (data.properties.contains("server_lookid")) {
            brushElement.setAttribute("server_lookid", data.properties["server_lookid"].toString());
        }
        if (data.properties.contains("on_blocking")) {
            brushElement.setAttribute("on_blocking", data.properties["on_blocking"].toBool() ? "true" : "false");
        }
        if (data.properties.contains("on_duplicate")) {
            brushElement.setAttribute("on_duplicate", data.properties["on_duplicate"].toBool() ? "true" : "false");
        }
        if (data.properties.contains("redo_borders") || data.properties.contains("reborder")) {
            brushElement.setAttribute("redo_borders", data.properties.value("redo_borders", data.properties["reborder"]).toBool() ? "true" : "false");
        }
        if (data.properties.contains("one_size")) {
            brushElement.setAttribute("one_size", data.properties["one_size"].toBool() ? "true" : "false");
        }
    }

    // NO properties, custom_data, or dependencies elements - wxwidgets stores everything as attributes
    // Child elements are added by individual brush load() methods, not by the persistence system

    return brushElement;
}

BrushSerializationData BrushPersistence::deserializeBrushFromXML(const QDomElement& element) const {
    BrushSerializationData data;

    // EXACT wxwidgets attributes (from brush.cpp unserializeBrush)
    data.name = element.attribute("name");
    data.type = element.attribute("type");

    // Extract brush-specific properties from attributes (exact wxwidgets format)
    if (data.type == "ground" || data.type == "border") {
        // GroundBrush attributes (from ground_brush.cpp load method)
        if (element.hasAttribute("lookid")) {
            data.properties["lookid"] = element.attribute("lookid").toUShort();
        }
        if (element.hasAttribute("server_lookid")) {
            data.properties["server_lookid"] = element.attribute("server_lookid").toUShort();
        }
        if (element.hasAttribute("z-order")) {
            data.properties["z-order"] = element.attribute("z-order").toInt();
        }
        if (element.hasAttribute("solo_optional")) {
            data.properties["solo_optional"] = element.attribute("solo_optional").toLower() == "true";
        }
        if (element.hasAttribute("randomize")) {
            data.properties["randomize"] = element.attribute("randomize").toLower() == "true";
        }
    } else if (data.type == "wall") {
        // WallBrush attributes
        if (element.hasAttribute("lookid")) {
            data.properties["lookid"] = element.attribute("lookid").toUShort();
        }
        if (element.hasAttribute("server_lookid")) {
            data.properties["server_lookid"] = element.attribute("server_lookid").toUShort();
        }
    } else if (data.type == "doodad") {
        // DoodadBrush attributes (from doodad_brush.cpp load method)
        if (element.hasAttribute("lookid")) {
            data.properties["lookid"] = element.attribute("lookid").toUShort();
        }
        if (element.hasAttribute("server_lookid")) {
            data.properties["server_lookid"] = element.attribute("server_lookid").toUShort();
        }
        if (element.hasAttribute("on_blocking")) {
            data.properties["on_blocking"] = element.attribute("on_blocking").toLower() == "true";
        }
        if (element.hasAttribute("on_duplicate")) {
            data.properties["on_duplicate"] = element.attribute("on_duplicate").toLower() == "true";
        }
        if (element.hasAttribute("redo_borders") || element.hasAttribute("reborder")) {
            data.properties["redo_borders"] = element.attribute("redo_borders", element.attribute("reborder")).toLower() == "true";
        }
        if (element.hasAttribute("one_size")) {
            data.properties["one_size"] = element.attribute("one_size").toLower() == "true";
        }
    }

    // Set defaults for Qt-specific fields (not in wxwidgets)
    data.id = 0; // wxwidgets doesn't store brush ID in XML
    data.version = BRUSH_FILE_VERSION;
    data.timestamp = QDateTime::currentDateTime().toSecsSinceEpoch();
    data.isUserDefined = true; // Assume loaded brushes are user-defined
    data.isModified = false;

    // NO properties, custom_data, or dependencies elements - wxwidgets stores everything as attributes

    return data;
}

// JSON serialization methods
bool BrushPersistence::saveToJSON(const QString& filePath, const QList<BrushSerializationData>& brushes, const QVariantMap& metadata) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setLastError("Cannot open file for writing", filePath);
        return false;
    }

    QJsonObject root;
    root["format"] = JSON_FORMAT_IDENTIFIER;
    root["version"] = BRUSH_FILE_VERSION;

    // Add metadata
    QJsonObject metaObject;
    for (auto it = metadata.begin(); it != metadata.end(); ++it) {
        metaObject[it.key()] = QJsonValue::fromVariant(it.value());
    }
    root["metadata"] = metaObject;

    // Add brushes
    QJsonArray brushArray;
    for (const BrushSerializationData& data : brushes) {
        QJsonObject brushObject = serializeBrushToJSON(data);
        brushArray.append(brushObject);
    }
    root["brushes"] = brushArray;

    QJsonDocument doc(root);
    QTextStream stream(&file);
    stream << doc.toJson();

    return true;
}

bool BrushPersistence::loadFromJSON(const QString& filePath, QList<BrushSerializationData>& brushes, QVariantMap& metadata) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setLastError("Cannot open file for reading", filePath);
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        setLastError("JSON parse error", error.errorString());
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("format") || root["format"].toString() != JSON_FORMAT_IDENTIFIER) {
        setLastError("Invalid JSON format identifier", "Expected: " + JSON_FORMAT_IDENTIFIER);
        return false;
    }

    // Load metadata
    if (root.contains("metadata")) {
        QJsonObject metaObject = root["metadata"].toObject();
        for (auto it = metaObject.begin(); it != metaObject.end(); ++it) {
            metadata[it.key()] = it.value().toVariant();
        }
    }

    // Load brushes
    if (root.contains("brushes")) {
        QJsonArray brushArray = root["brushes"].toArray();
        for (const QJsonValue& value : brushArray) {
            QJsonObject brushObject = value.toObject();
            BrushSerializationData data = deserializeBrushFromJSON(brushObject);
            if (validateBrushData(data)) {
                brushes.append(data);
            }
        }
    }

    return true;
}

QJsonObject BrushPersistence::serializeBrushToJSON(const BrushSerializationData& data) const {
    QJsonObject brushObject;

    // Basic properties
    brushObject["name"] = data.name;
    brushObject["type"] = data.type;
    brushObject["id"] = static_cast<qint64>(data.id);
    brushObject["version"] = data.version;
    brushObject["timestamp"] = data.timestamp;
    brushObject["user_defined"] = data.isUserDefined;
    brushObject["modified"] = data.isModified;

    // Properties
    if (!data.properties.isEmpty()) {
        brushObject["properties"] = QJsonObject::fromVariantMap(data.properties);
    }

    // Custom data
    if (!data.customData.isEmpty()) {
        brushObject["custom_data"] = QJsonObject::fromVariantMap(data.customData);
    }

    // Dependencies
    if (!data.dependencies.isEmpty()) {
        QJsonArray depsArray;
        for (const QString& dep : data.dependencies) {
            depsArray.append(dep);
        }
        brushObject["dependencies"] = depsArray;
    }

    return brushObject;
}

BrushSerializationData BrushPersistence::deserializeBrushFromJSON(const QJsonObject& object) const {
    BrushSerializationData data;

    // Basic properties
    data.name = object["name"].toString();
    data.type = object["type"].toString();
    data.id = static_cast<quint32>(object["id"].toInt());
    data.version = object["version"].toString();
    data.timestamp = object["timestamp"].toVariant().toLongLong();
    data.isUserDefined = object["user_defined"].toBool();
    data.isModified = object["modified"].toBool();

    // Properties
    if (object.contains("properties")) {
        data.properties = object["properties"].toObject().toVariantMap();
    }

    // Custom data
    if (object.contains("custom_data")) {
        data.customData = object["custom_data"].toObject().toVariantMap();
    }

    // Dependencies
    if (object.contains("dependencies")) {
        QJsonArray depsArray = object["dependencies"].toArray();
        for (const QJsonValue& value : depsArray) {
            data.dependencies.append(value.toString());
        }
    }

    return data;
}

// Helper methods implementation
QVariantMap BrushPersistence::extractBrushProperties(Brush* brush) const {
    QVariantMap properties;

    if (!brush) {
        return properties;
    }

    // Extract common brush properties
    // Note: These methods don't exist in base Brush class - would need to be added or use dynamic_cast
    // properties["size"] = brush->getSize();
    // properties["opacity"] = brush->getOpacity();
    // properties["hardness"] = brush->getHardness();
    // properties["spacing"] = brush->getSpacing();
    // properties["angle"] = brush->getAngle();
    // properties["roundness"] = brush->getRoundness();
    // properties["flow"] = brush->getFlow();
    // properties["blend_mode"] = static_cast<int>(brush->getBlendMode());

    // Extract basic properties that do exist
    properties["look_id"] = brush->getLookID();

    // Extract brush-specific properties based on type
    switch (static_cast<int>(brush->type())) {
        case static_cast<int>(Brush::Type::Ground):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to GroundBrush
            // properties["ground_id"] = brush->getGroundId();
            // properties["border_enabled"] = brush->isBorderEnabled();
            break;
        case static_cast<int>(Brush::Type::Wall):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to WallBrush
            // properties["wall_type"] = static_cast<int>(brush->getWallType());
            // properties["wall_alignment"] = static_cast<int>(brush->getWallAlignment());
            break;
        case static_cast<int>(Brush::Type::Creature):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to CreatureBrush
            // properties["creature_id"] = brush->getCreatureId();
            // properties["spawn_time"] = brush->getSpawnTime();
            break;
        case static_cast<int>(Brush::Type::Waypoint):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to WaypointBrush
            // properties["waypoint_name"] = brush->getWaypointName();
            // properties["waypoint_type"] = static_cast<int>(brush->getWaypointType());
            break;
        default:
            // Extract generic properties
            break;
    }

    return properties;
}

void BrushPersistence::applyBrushProperties(Brush* brush, const QVariantMap& properties) const {
    if (!brush || properties.isEmpty()) {
        return;
    }

    // Apply common brush properties
    // Note: These methods don't exist in base Brush class - would need to be added or use dynamic_cast
    // if (properties.contains("size")) {
    //     brush->setSize(properties["size"].toInt());
    // }
    // if (properties.contains("opacity")) {
    //     brush->setOpacity(properties["opacity"].toDouble());
    // }
    // if (properties.contains("hardness")) {
    //     brush->setHardness(properties["hardness"].toDouble());
    // }
    // if (properties.contains("spacing")) {
    //     brush->setSpacing(properties["spacing"].toDouble());
    // }
    // if (properties.contains("angle")) {
    //     brush->setAngle(properties["angle"].toDouble());
    // }
    // if (properties.contains("roundness")) {
    //     brush->setRoundness(properties["roundness"].toDouble());
    // }
    // if (properties.contains("flow")) {
    //     brush->setFlow(properties["flow"].toDouble());
    // }
    // if (properties.contains("blend_mode")) {
    //     brush->setBlendMode(static_cast<Brush::BlendMode>(properties["blend_mode"].toInt()));
    // }

    // Apply brush-specific properties
    switch (static_cast<int>(brush->type())) {
        case static_cast<int>(Brush::Type::Ground):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to GroundBrush
            // if (properties.contains("ground_id")) {
            //     brush->setGroundId(properties["ground_id"].toUInt());
            // }
            // if (properties.contains("border_enabled")) {
            //     brush->setBorderEnabled(properties["border_enabled"].toBool());
            // }
            break;
        case static_cast<int>(Brush::Type::Wall):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to WallBrush
            // if (properties.contains("wall_type")) {
            //     brush->setWallType(static_cast<Brush::WallType>(properties["wall_type"].toInt()));
            // }
            // if (properties.contains("wall_alignment")) {
            //     brush->setWallAlignment(static_cast<Brush::WallAlignment>(properties["wall_alignment"].toInt()));
            // }
            break;
        case static_cast<int>(Brush::Type::Creature):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to CreatureBrush
            // if (properties.contains("creature_id")) {
            //     brush->setCreatureId(properties["creature_id"].toUInt());
            // }
            // if (properties.contains("spawn_time")) {
            //     brush->setSpawnTime(properties["spawn_time"].toInt());
            // }
            break;
        case static_cast<int>(Brush::Type::Waypoint):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to WaypointBrush
            // if (properties.contains("waypoint_name")) {
            //     brush->setWaypointName(properties["waypoint_name"].toString());
            // }
            // if (properties.contains("waypoint_type")) {
            //     brush->setWaypointType(static_cast<Brush::WaypointType>(properties["waypoint_type"].toInt()));
            // }
            break;
        default:
            // Apply generic properties
            break;
    }
}

QStringList BrushPersistence::extractBrushDependencies(Brush* brush) const {
    QStringList dependencies;

    if (!brush) {
        return dependencies;
    }

    // Extract dependencies based on brush type
    switch (static_cast<int>(brush->type())) {
        case static_cast<int>(Brush::Type::Ground):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to GroundBrush
            // if (brush->getGroundId() > 0) {
            //     dependencies.append(QString("item:%1").arg(brush->getGroundId()));
            // }
            break;
        case static_cast<int>(Brush::Type::Wall):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to WallBrush
            // for (quint32 itemId : brush->getWallItemIds()) {
            //     dependencies.append(QString("item:%1").arg(itemId));
            // }
            break;
        case static_cast<int>(Brush::Type::Creature):
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to CreatureBrush
            // if (brush->getCreatureId() > 0) {
            //     dependencies.append(QString("creature:%1").arg(brush->getCreatureId()));
            // }
            break;
        case static_cast<int>(Brush::Type::Doodad):  // Use Doodad instead of DOODAD_BRUSH
            // Note: These methods don't exist in base Brush class - would need dynamic_cast to DoodadBrush
            // for (quint32 itemId : brush->getDoodadItemIds()) {
            //     dependencies.append(QString("item:%1").arg(itemId));
            // }
            break;
        default:
            break;
    }

    return dependencies;
}

QString BrushPersistence::brushTypeToString(Brush::Type type) const {
    // EXACT wxwidgets brush type strings (from brush.cpp line 116-127)
    switch (type) {
        case Brush::Type::Raw: return "ground"; // RAW maps to ground in wxwidgets
        case Brush::Type::Ground: return "ground";
        case Brush::Type::OptionalBorder: return "border";
        case Brush::Type::Wall: return "wall";
        case Brush::Type::WallDecoration: return "wall decoration";
        case Brush::Type::Carpet: return "carpet";
        case Brush::Type::Table: return "table";
        case Brush::Type::Doodad: return "doodad";
        case Brush::Type::Creature: return "creature";
        case Brush::Type::Spawn: return "spawn";
        case Brush::Type::House: return "house";
        case Brush::Type::Waypoint: return "waypoint";
        case Brush::Type::Eraser: return "eraser";
        default: return "ground"; // Default fallback to ground
    }
}

Brush::Type BrushPersistence::stringToBrushType(const QString& typeString) const {
    // EXACT wxwidgets brush type strings (from brush.cpp line 116-127)
    if (typeString == "ground") return Brush::Type::Ground;
    if (typeString == "border") return Brush::Type::OptionalBorder;
    if (typeString == "wall") return Brush::Type::Wall;
    if (typeString == "wall decoration") return Brush::Type::WallDecoration;
    if (typeString == "carpet") return Brush::Type::Carpet;
    if (typeString == "table") return Brush::Type::Table;
    if (typeString == "doodad") return Brush::Type::Doodad;
    if (typeString == "creature") return Brush::Type::Creature;
    if (typeString == "spawn") return Brush::Type::Spawn;
    if (typeString == "house") return Brush::Type::House;
    if (typeString == "waypoint") return Brush::Type::Waypoint;
    if (typeString == "eraser") return Brush::Type::Eraser;
    return Brush::Type::Ground; // Default fallback to ground
}

bool BrushPersistence::validateBrushData(const BrushSerializationData& data) const {
    // Basic validation
    if (data.name.isEmpty() || data.type.isEmpty()) {
        return false;
    }

    // Check if brush type is supported
    if (!SERIALIZABLE_BRUSH_TYPES.contains(data.type)) {
        qWarning() << "Unsupported brush type:" << data.type;
        return false;
    }

    // Validate version compatibility
    if (data.version != BRUSH_FILE_VERSION) {
        qWarning() << "Version mismatch for brush" << data.name << "- Expected:" << BRUSH_FILE_VERSION << "Got:" << data.version;
        // Allow loading but warn about potential compatibility issues
    }

    return true;
}

bool BrushPersistence::validateXMLStructure(const QDomDocument& doc) const {
    QDomElement root = doc.documentElement();
    if (root.tagName() != XML_ROOT_ELEMENT) {
        return false;
    }

    // Check for required elements
    QDomNodeList brushNodes = root.elementsByTagName("brush");
    if (brushNodes.isEmpty()) {
        qWarning() << "No brush elements found in XML";
        return false;
    }

    // Validate each brush element
    for (int i = 0; i < brushNodes.count(); ++i) {
        QDomElement brushElement = brushNodes.at(i).toElement();
        if (!brushElement.hasAttribute("name") || !brushElement.hasAttribute("type")) {
            qWarning() << "Invalid brush element at index" << i;
            return false;
        }
    }

    return true;
}

bool BrushPersistence::validateJSONStructure(const QJsonDocument& doc) const {
    QJsonObject root = doc.object();

    // Check format identifier
    if (!root.contains("format") || root["format"].toString() != JSON_FORMAT_IDENTIFIER) {
        return false;
    }

    // Check for brushes array
    if (!root.contains("brushes") || !root["brushes"].isArray()) {
        qWarning() << "No brushes array found in JSON";
        return false;
    }

    QJsonArray brushArray = root["brushes"].toArray();
    if (brushArray.isEmpty()) {
        qWarning() << "Empty brushes array in JSON";
        return false;
    }

    // Validate each brush object
    for (const QJsonValue& value : brushArray) {
        if (!value.isObject()) {
            qWarning() << "Invalid brush object in JSON array";
            return false;
        }

        QJsonObject brushObject = value.toObject();
        if (!brushObject.contains("name") || !brushObject.contains("type")) {
            qWarning() << "Invalid brush object - missing name or type";
            return false;
        }
    }

    return true;
}

void BrushPersistence::setLastError(const QString& error, const QString& details) {
    lastError_ = error;
    lastErrorDetails_ = details;
    emit errorOccurred(error, details);
    qWarning() << "BrushPersistence error:" << error << "-" << details;
}

void BrushPersistence::clearLastError() {
    lastError_.clear();
    lastErrorDetails_.clear();
}
