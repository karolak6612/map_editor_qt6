#include "Town.h"
#include <QFile>          // Task 66: XML file I/O
#include <QTextStream>    // Task 66: XML file I/O
#include <QDomDocument>   // Task 66: XML support
#include <QDomElement>    // Task 66: XML support
#include <QDebug>         // Task 66: Debug output

Town::Town(quint32 id, const QString& name, const MapPos& templePosition)
    : m_id(id), m_name(name), m_templePosition(templePosition) {
}

quint32 Town::getId() const {
    return m_id;
}

void Town::setId(quint32 id) {
    m_id = id;
}

QString Town::getName() const {
    return m_name;
}

void Town::setName(const QString& name) {
    m_name = name;
}

MapPos Town::getTemplePosition() const {
    return m_templePosition;
}

void Town::setTemplePosition(const MapPos& position) {
    m_templePosition = position;
}

// Task 66: XML serialization implementation
bool Town::loadFromXml(const QDomElement& element) {
    // Load basic town properties
    m_id = element.attribute("id", "0").toUInt();
    m_name = element.attribute("name", "");

    // Load temple position
    m_templePosition.x = element.attribute("templex", "0").toInt();
    m_templePosition.y = element.attribute("templey", "0").toInt();
    m_templePosition.z = element.attribute("templez", "0").toInt();

    return true;
}

bool Town::saveToXml(QDomDocument& doc, QDomElement& parent) const {
    QDomElement townElement = doc.createElement("town");

    // Save basic town properties
    townElement.setAttribute("id", m_id);
    townElement.setAttribute("name", m_name);

    // Save temple position
    townElement.setAttribute("templex", m_templePosition.x);
    townElement.setAttribute("templey", m_templePosition.y);
    townElement.setAttribute("templez", m_templePosition.z);

    parent.appendChild(townElement);
    return true;
}

bool Town::loadTownsFromXML(const QString& filePath, QList<Town*>& towns, QStringList& errors) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errors << QString("Cannot open towns file: %1").arg(filePath);
        return false;
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;

    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        errors << QString("XML parse error in %1 at line %2, column %3: %4")
                  .arg(filePath).arg(errorLine).arg(errorColumn).arg(errorMsg);
        file.close();
        return false;
    }

    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "towns") {
        errors << QString("Invalid towns XML file: root element should be 'towns'");
        return false;
    }

    // Clear existing towns
    qDeleteAll(towns);
    towns.clear();

    // Load towns
    QDomNodeList townNodes = root.elementsByTagName("town");
    for (int i = 0; i < townNodes.count(); ++i) {
        QDomElement townElement = townNodes.at(i).toElement();
        if (!townElement.isNull()) {
            Town* town = new Town();
            if (town->loadFromXml(townElement)) {
                towns.append(town);
                qDebug() << "Loaded town:" << town->getName() << "ID:" << town->getId();
            } else {
                errors << QString("Failed to load town at index %1").arg(i);
                delete town;
            }
        }
    }

    qDebug() << "Loaded" << towns.size() << "towns from" << filePath;
    return true;
}

bool Town::saveTownsToXML(const QString& filePath, const QList<Town*>& towns, QStringList& errors) {
    QDomDocument doc;
    QDomElement root = doc.createElement("towns");
    doc.appendChild(root);

    // Save all towns
    for (Town* town : towns) {
        if (town) {
            if (!town->saveToXml(doc, root)) {
                errors << QString("Failed to save town: %1 (ID: %2)").arg(town->getName()).arg(town->getId());
            }
        }
    }

    // Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errors << QString("Cannot write towns file: %1").arg(filePath);
        return false;
    }

    QTextStream stream(&file);
    stream << doc.toString(4); // 4-space indentation
    file.close();

    qDebug() << "Saved" << towns.size() << "towns to" << filePath;
    return errors.isEmpty();
}
