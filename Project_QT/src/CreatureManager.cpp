#include "CreatureManager.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

CreatureManager::CreatureManager() {
    // Constructor implementation (can be empty for now)
}

CreatureManager::~CreatureManager() {
    // Destructor implementation (can be empty for now)
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

bool CreatureManager::loadCreaturesFromXml(const QString& filePath) {
    // Clear existing data
    creaturePropertiesMap_.clear();
    creatureNameToIdMap_.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "CreatureManager::loadCreaturesFromXml - Could not open file:" << filePath << file.errorString();
        return false;
    }

    QXmlStreamReader reader(&file);

    if (!reader.readNextStartElement() || reader.name().toString() != QLatin1String("creatures")) {
        qWarning() << "CreatureManager::loadCreaturesFromXml - Expected <creatures> as root element, found:" << reader.name().toString();
        file.close();
        return false;
    }

    while (reader.readNextStartElement()) {
        if (reader.name().toString() == QLatin1String("creature")) {
            CreatureProperties props;
            QXmlStreamAttributes attributes = reader.attributes();

            if (!attributes.hasAttribute(QLatin1String("looktype"))) {
                qWarning() << "CreatureManager::loadCreaturesFromXml - Creature missing mandatory 'looktype' attribute. Skipping.";
                reader.skipCurrentElement();
                continue;
            }

            bool conversionOk = false;
            int looktype = attributes.value(QLatin1String("looktype")).toInt(&conversionOk);
            if (!conversionOk) {
                qWarning() << "CreatureManager::loadCreaturesFromXml - Invalid 'looktype' attribute for creature:" << attributes.value(QLatin1String("looktype")) << ". Skipping.";
                reader.skipCurrentElement();
                continue;
            }

            props.id = looktype;
            props.outfit.lookType = looktype;
            props.name = attributes.value(QLatin1String("name")).toString();

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
                 qWarning() << "CreatureManager::loadCreaturesFromXml - Duplicate creature ID (looktype):" << props.id << ". Overwriting.";
            }
            creaturePropertiesMap_[props.id] = props;

            if (!props.name.isEmpty()) {
                QString lowerName = props.name.toLower();
                if (creatureNameToIdMap_.contains(lowerName)) {
                    qWarning() << "CreatureManager::loadCreaturesFromXml - Duplicate creature name:" << props.name << "(ID:" << props.id << "). Overwriting previous ID:" << creatureNameToIdMap_.value(lowerName);
                }
                creatureNameToIdMap_[lowerName] = props.id;
            } else {
                 qWarning() << "CreatureManager::loadCreaturesFromXml - Creature with ID" << props.id << "has empty name. It won't be accessible by name.";
            }

            reader.skipCurrentElement(); // Skip to the end of the <creature> element
        } else {
            qWarning() << "CreatureManager::loadCreaturesFromXml - Unexpected element:" << reader.name().toString();
            reader.skipCurrentElement();
        }
    }

    if (reader.hasError()) {
        qWarning() << "CreatureManager::loadCreaturesFromXml - XML parsing error:" << reader.errorString();
        file.close();
        return false;
    }

    file.close();
    return true;
}
