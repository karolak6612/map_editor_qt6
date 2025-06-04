#include "CreatureManager.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

// Static instance
CreatureManager* CreatureManager::s_instance = nullptr;

CreatureManager::CreatureManager(QObject* parent) : QObject(parent), loaded_(false) {
    // Private constructor for singleton
}

CreatureManager::~CreatureManager() {
    clear();
    // Clear static instance pointer when destroyed
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

CreatureManager& CreatureManager::getInstance() {
    if (!s_instance) {
        // Parent to QApplication for automatic cleanup
        QObject* appParent = QCoreApplication::instance();
        s_instance = new CreatureManager(appParent);

        // Store reference in QApplication for easy access
        if (appParent) {
            appParent->setProperty("CreatureManager", QVariant::fromValue(s_instance));
        }
    }
    return *s_instance;
}

void CreatureManager::destroyInstance() {
    if (s_instance) {
        // Remove from QApplication properties
        QObject* appParent = QCoreApplication::instance();
        if (appParent) {
            appParent->setProperty("CreatureManager", QVariant());
        }

        delete s_instance;
        s_instance = nullptr;
    }
}

bool CreatureManager::hasCreature(int id) const {
    return creaturePropertiesMap_.contains(id);
}

bool CreatureManager::hasCreature(const QString& name) const {
    return creatureNameToIdMap_.contains(name.toLower());
}

CreatureProperties CreatureManager::getCreatureProperties(int id) const {
    if (hasCreature(id)) {
        return creaturePropertiesMap_.value(id);
    } else {
        qWarning() << "CreatureManager::getCreatureProperties - Creature with ID" << id << "not found.";
        return CreatureProperties(); // Return default-constructed object
    }
}

CreatureProperties CreatureManager::getCreatureProperties(const QString& name) const {
    if (hasCreature(name)) {
        int id = creatureNameToIdMap_.value(name.toLower());
        return getCreatureProperties(id);
    } else {
        qWarning() << "CreatureManager::getCreatureProperties - Creature with name" << name << "not found.";
        return CreatureProperties(); // Return default-constructed object
    }
}

// --- Enhanced Creature Management ---

CreatureProperties* CreatureManager::addMissingCreatureType(const QString& name, bool isNpc) {
    if (hasCreature(name)) {
        qWarning() << "CreatureManager::addMissingCreatureType - Creature with name" << name << "already exists.";
        return nullptr;
    }

    // Find next available ID
    int newId = 1;
    while (hasCreature(newId)) {
        newId++;
    }

    CreatureProperties props;
    props.id = newId;
    props.name = name;
    props.isNpc = isNpc;
    props.missing = true;
    props.standard = false;
    props.outfit.lookType = 130; // Default missing creature look

    creaturePropertiesMap_[newId] = props;
    creatureNameToIdMap_[name.toLower()] = newId;

    emit creatureAdded(name, newId);
    return &creaturePropertiesMap_[newId];
}

CreatureProperties* CreatureManager::addCreatureType(const QString& name, bool isNpc, const Outfit& outfit) {
    if (hasCreature(name)) {
        qWarning() << "CreatureManager::addCreatureType - Creature with name" << name << "already exists.";
        return nullptr;
    }

    // Find next available ID
    int newId = 1;
    while (hasCreature(newId)) {
        newId++;
    }

    CreatureProperties props;
    props.id = newId;
    props.name = name;
    props.isNpc = isNpc;
    props.missing = false;
    props.standard = false;
    props.outfit = outfit;

    creaturePropertiesMap_[newId] = props;
    creatureNameToIdMap_[name.toLower()] = newId;

    emit creatureAdded(name, newId);
    return &creaturePropertiesMap_[newId];
}

bool CreatureManager::hasMissing() const {
    for (const auto& props : creaturePropertiesMap_) {
        if (props.missing) {
            return true;
        }
    }
    return false;
}

// --- Collection Methods ---

QList<CreatureProperties> CreatureManager::getAllCreatures() const {
    QList<CreatureProperties> result;
    for (const auto& props : creaturePropertiesMap_) {
        result.append(props);
    }
    return result;
}

QList<CreatureProperties> CreatureManager::getCreaturesByType(bool isNpc) const {
    QList<CreatureProperties> result;
    for (const auto& props : creaturePropertiesMap_) {
        if (props.isNpc == isNpc) {
            result.append(props);
        }
    }
    return result;
}

QStringList CreatureManager::getCreatureNames() const {
    QStringList result;
    for (const auto& props : creaturePropertiesMap_) {
        result.append(props.name);
    }
    result.sort();
    return result;
}

int CreatureManager::getCreatureCount() const {
    return creaturePropertiesMap_.size();
}

// --- Utility Methods ---

void CreatureManager::clear() {
    creaturePropertiesMap_.clear();
    creatureNameToIdMap_.clear();
    loaded_ = false;
    emit creaturesCleared();
}

bool CreatureManager::isLoaded() const {
    return loaded_;
}

// --- Save/Export Functionality ---

bool CreatureManager::saveToXML(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "CreatureManager::saveToXML - Could not open file for writing:" << filePath << file.errorString();
        return false;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    writer.writeStartElement("creatures");

    for (const auto& props : creaturePropertiesMap_) {
        if (!props.standard) { // Only save non-standard creatures
            writer.writeStartElement("creature");

            writer.writeAttribute("name", props.name);
            writer.writeAttribute("type", props.isNpc ? "npc" : "monster");
            writer.writeAttribute("looktype", QString::number(props.outfit.lookType));

            if (props.outfit.lookItem != 0) {
                writer.writeAttribute("lookitem", QString::number(props.outfit.lookItem));
            }
            if (props.outfit.lookMount != 0) {
                writer.writeAttribute("lookmount", QString::number(props.outfit.lookMount));
            }
            if (props.outfit.lookAddon != 0) {
                writer.writeAttribute("lookaddon", QString::number(props.outfit.lookAddon));
            }
            if (props.outfit.lookHead != 0) {
                writer.writeAttribute("lookhead", QString::number(props.outfit.lookHead));
            }
            if (props.outfit.lookBody != 0) {
                writer.writeAttribute("lookbody", QString::number(props.outfit.lookBody));
            }
            if (props.outfit.lookLegs != 0) {
                writer.writeAttribute("looklegs", QString::number(props.outfit.lookLegs));
            }
            if (props.outfit.lookFeet != 0) {
                writer.writeAttribute("lookfeet", QString::number(props.outfit.lookFeet));
            }
            if (props.outfit.lookMountHead != 0) {
                writer.writeAttribute("lookmounthead", QString::number(props.outfit.lookMountHead));
            }
            if (props.outfit.lookMountBody != 0) {
                writer.writeAttribute("lookmountbody", QString::number(props.outfit.lookMountBody));
            }
            if (props.outfit.lookMountLegs != 0) {
                writer.writeAttribute("lookmountlegs", QString::number(props.outfit.lookMountLegs));
            }
            if (props.outfit.lookMountFeet != 0) {
                writer.writeAttribute("lookmountfeet", QString::number(props.outfit.lookMountFeet));
            }

            writer.writeEndElement(); // creature
        }
    }

    writer.writeEndElement(); // creatures
    writer.writeEndDocument();

    file.close();
    return true;
}

bool CreatureManager::loadCreaturesFromXml(const QString& filePath, QString& error, QStringList& warnings, bool standard) {
    // Clear existing data
    clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QString("Could not open file: %1 - %2").arg(filePath, file.errorString());
        qWarning() << "CreatureManager::loadCreaturesFromXml -" << error;
        return false;
    }

    QXmlStreamReader reader(&file);

    if (!reader.readNextStartElement() || reader.name().toString() != QLatin1String("creatures")) {
        error = QString("Expected <creatures> as root element, found: %1").arg(reader.name().toString());
        qWarning() << "CreatureManager::loadCreaturesFromXml -" << error;
        file.close();
        return false;
    }

    while (reader.readNextStartElement()) {
        if (reader.name().toString() == QLatin1String("creature")) {
            CreatureProperties props;
            QXmlStreamAttributes attributes = reader.attributes();

            if (!attributes.hasAttribute(QLatin1String("looktype"))) {
                QString warning = "Creature missing mandatory 'looktype' attribute. Skipping.";
                warnings.append(warning);
                qWarning() << "CreatureManager::loadCreaturesFromXml -" << warning;
                reader.skipCurrentElement();
                continue;
            }

            bool conversionOk = false;
            int looktype = attributes.value(QLatin1String("looktype")).toInt(&conversionOk);
            if (!conversionOk) {
                QString warning = QString("Invalid 'looktype' attribute for creature: %1. Skipping.").arg(attributes.value(QLatin1String("looktype")).toString());
                warnings.append(warning);
                qWarning() << "CreatureManager::loadCreaturesFromXml -" << warning;
                reader.skipCurrentElement();
                continue;
            }

            props.id = looktype;
            props.outfit.lookType = looktype;
            props.name = attributes.value(QLatin1String("name")).toString();
            props.standard = standard;

            QString typeStr = attributes.value(QLatin1String("type")).toString();
            props.isNpc = (typeStr.toLower() == QLatin1String("npc"));

            // Optional outfit attributes
            if (attributes.hasAttribute(QLatin1String("lookitem"))) {
                props.outfit.lookItem = attributes.value(QLatin1String("lookitem")).toInt();
            }
            if (attributes.hasAttribute(QLatin1String("lookmount"))) {
                props.outfit.lookMount = attributes.value(QLatin1String("lookmount")).toInt();
            }
            if (attributes.hasAttribute(QLatin1String("lookaddon"))) {
                props.outfit.lookAddon = attributes.value(QLatin1String("lookaddon")).toInt();
            }
            if (attributes.hasAttribute(QLatin1String("lookhead"))) {
                props.outfit.lookHead = attributes.value(QLatin1String("lookhead")).toInt();
            }
            if (attributes.hasAttribute(QLatin1String("lookbody"))) {
                props.outfit.lookBody = attributes.value(QLatin1String("lookbody")).toInt();
            }
            if (attributes.hasAttribute(QLatin1String("looklegs"))) {
                props.outfit.lookLegs = attributes.value(QLatin1String("looklegs")).toInt();
            }
            if (attributes.hasAttribute(QLatin1String("lookfeet"))) {
                props.outfit.lookFeet = attributes.value(QLatin1String("lookfeet")).toInt();
            }
            if (attributes.hasAttribute(QLatin1String("lookmounthead"))) {
                props.outfit.lookMountHead = attributes.value(QLatin1String("lookmounthead")).toInt();
            }
            if (attributes.hasAttribute(QLatin1String("lookmountbody"))) {
                props.outfit.lookMountBody = attributes.value(QLatin1String("lookmountbody")).toInt();
            }
            if (attributes.hasAttribute(QLatin1String("lookmountlegs"))) {
                props.outfit.lookMountLegs = attributes.value(QLatin1String("lookmountlegs")).toInt();
            }
            if (attributes.hasAttribute(QLatin1String("lookmountfeet"))) {
                props.outfit.lookMountFeet = attributes.value(QLatin1String("lookmountfeet")).toInt();
            }
            
            if (creaturePropertiesMap_.contains(props.id)) {
                QString warning = QString("Duplicate creature ID (looktype): %1. Overwriting.").arg(props.id);
                warnings.append(warning);
                qWarning() << "CreatureManager::loadCreaturesFromXml -" << warning;
            }
            creaturePropertiesMap_[props.id] = props;

            if (!props.name.isEmpty()) {
                QString lowerName = props.name.toLower();
                if (creatureNameToIdMap_.contains(lowerName)) {
                    QString warning = QString("Duplicate creature name: %1 (ID: %2). Overwriting previous ID: %3").arg(props.name).arg(props.id).arg(creatureNameToIdMap_.value(lowerName));
                    warnings.append(warning);
                    qWarning() << "CreatureManager::loadCreaturesFromXml -" << warning;
                }
                creatureNameToIdMap_[lowerName] = props.id;
            } else {
                QString warning = QString("Creature with ID %1 has empty name. It won't be accessible by name.").arg(props.id);
                warnings.append(warning);
                qWarning() << "CreatureManager::loadCreaturesFromXml -" << warning;
            }

            reader.skipCurrentElement(); // Skip to the end of the <creature> element
        } else {
            QString warning = QString("Unexpected element: %1").arg(reader.name().toString());
            warnings.append(warning);
            qWarning() << "CreatureManager::loadCreaturesFromXml -" << warning;
            reader.skipCurrentElement();
        }
    }

    if (reader.hasError()) {
        error = QString("XML parsing error: %1").arg(reader.errorString());
        qWarning() << "CreatureManager::loadCreaturesFromXml -" << error;
        file.close();
        return false;
    }

    file.close();
    loaded_ = true;
    emit creaturesLoaded();
    return true;
}

bool CreatureManager::loadCreaturesFromXml(const QString& filePath, bool standard) {
    QString error;
    QStringList warnings;
    bool result = loadCreaturesFromXml(filePath, error, warnings, standard);

    if (!result) {
        qWarning() << "CreatureManager::loadCreaturesFromXml failed:" << error;
    }

    if (!warnings.isEmpty()) {
        qWarning() << "CreatureManager::loadCreaturesFromXml warnings:" << warnings.join("; ");
    }

    return result;
}

// --- OT XML Import Support ---

bool CreatureManager::importXMLFromOT(const QString& filePath, QString& error, QStringList& warnings) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QString("Could not open file: %1 - %2").arg(filePath, file.errorString());
        return false;
    }

    QXmlStreamReader reader(&file);

    // Look for monster or npc root element
    bool isNpc = false;
    if (!reader.readNextStartElement()) {
        error = "Could not read XML root element";
        file.close();
        return false;
    }

    if (reader.name().toString() == QLatin1String("monster")) {
        isNpc = false;
    } else if (reader.name().toString() == QLatin1String("npc")) {
        isNpc = true;
    } else {
        error = "This file is not a monster/npc file";
        file.close();
        return false;
    }

    QXmlStreamAttributes attributes = reader.attributes();
    if (!attributes.hasAttribute(QLatin1String("name"))) {
        error = "Couldn't read name attribute of creature node";
        file.close();
        return false;
    }

    QString name = attributes.value(QLatin1String("name")).toString();

    // Create creature properties with default outfit
    Outfit outfit;
    outfit.lookType = 130; // Default look

    // Look for look element
    while (reader.readNextStartElement()) {
        if (reader.name().toString() == QLatin1String("look")) {
            QXmlStreamAttributes lookAttrs = reader.attributes();
            if (lookAttrs.hasAttribute(QLatin1String("type"))) {
                outfit.lookType = lookAttrs.value(QLatin1String("type")).toInt();
            }
            if (lookAttrs.hasAttribute(QLatin1String("head"))) {
                outfit.lookHead = lookAttrs.value(QLatin1String("head")).toInt();
            }
            if (lookAttrs.hasAttribute(QLatin1String("body"))) {
                outfit.lookBody = lookAttrs.value(QLatin1String("body")).toInt();
            }
            if (lookAttrs.hasAttribute(QLatin1String("legs"))) {
                outfit.lookLegs = lookAttrs.value(QLatin1String("legs")).toInt();
            }
            if (lookAttrs.hasAttribute(QLatin1String("feet"))) {
                outfit.lookFeet = lookAttrs.value(QLatin1String("feet")).toInt();
            }
            if (lookAttrs.hasAttribute(QLatin1String("addons"))) {
                outfit.lookAddon = lookAttrs.value(QLatin1String("addons")).toInt();
            }
            if (lookAttrs.hasAttribute(QLatin1String("mount"))) {
                outfit.lookMount = lookAttrs.value(QLatin1String("mount")).toInt();
            }
            reader.skipCurrentElement();
        } else {
            reader.skipCurrentElement();
        }
    }

    file.close();

    // Add the creature
    CreatureProperties* props = addCreatureType(name, isNpc, outfit);
    if (props) {
        props->standard = false;
        return true;
    }

    error = QString("Failed to add creature: %1").arg(name);
    return false;
}

bool CreatureManager::importXMLFromOT(const QString& filePath) {
    QString error;
    QStringList warnings;
    bool result = importXMLFromOT(filePath, error, warnings);

    if (!result) {
        qWarning() << "CreatureManager::importXMLFromOT failed:" << error;
    }

    if (!warnings.isEmpty()) {
        qWarning() << "CreatureManager::importXMLFromOT warnings:" << warnings.join("; ");
    }

    return result;
}
