#include "House.h"
#include "Map.h"
#include "Tile.h"
#include <QDebug>
#include <QDataStream>
#include <QFile>          // Task 66: XML file I/O
#include <QTextStream>    // Task 66: XML file I/O
#include <QDomDocument>   // Task 66: XML support
#include <QDomElement>    // Task 66: XML support

// House implementation
House::House(QObject* parent)
    : QObject(parent),
      id_(0),
      name_(""),
      owner_(""),
      rent_(0),
      townId_(0),
      isGuildHall_(false),
      entryPosition_(0, 0, 0),
      exitPosition_(0, 0, 0) {
}

House::House(quint32 id, const QString& name, QObject* parent)
    : QObject(parent),
      id_(id),
      name_(name),
      owner_(""),
      rent_(0),
      townId_(0),
      isGuildHall_(false),
      entryPosition_(0, 0, 0),
      exitPosition_(0, 0, 0) {
}

House::~House() {
    // Note: We don't clean tiles here as it requires Map access
    // The Map or Houses collection should call clean() before deletion
}

// Core properties
quint32 House::getId() const {
    return id_;
}

void House::setId(quint32 id) {
    if (id_ != id) {
        id_ = id;
        emit houseChanged();
    }
}

QString House::getName() const {
    return name_;
}

void House::setName(const QString& name) {
    if (name_ != name) {
        name_ = name;
        emit houseChanged();
    }
}

QString House::getOwner() const {
    return owner_;
}

void House::setOwner(const QString& owner) {
    if (owner_ != owner) {
        owner_ = owner;
        emit houseChanged();
    }
}

int House::getRent() const {
    return rent_;
}

void House::setRent(int rent) {
    if (rent_ != rent) {
        rent_ = rent;
        emit houseChanged();
    }
}

quint32 House::getTownId() const {
    return townId_;
}

void House::setTownId(quint32 townId) {
    if (townId_ != townId) {
        townId_ = townId;
        emit houseChanged();
    }
}

bool House::isGuildHall() const {
    return isGuildHall_;
}

void House::setGuildHall(bool isGuildHall) {
    if (isGuildHall_ != isGuildHall) {
        isGuildHall_ = isGuildHall;
        emit houseChanged();
    }
}

// Entry/Exit management
MapPos House::getEntryPosition() const {
    return entryPosition_;
}

void House::setEntryPosition(const MapPos& position) {
    if (entryPosition_ != position) {
        entryPosition_ = position;
        emit houseChanged();
    }
}

MapPos House::getExitPosition() const {
    return exitPosition_;
}

void House::setExitPosition(const MapPos& position) {
    if (exitPosition_ != position) {
        exitPosition_ = position;
        emit houseChanged();
    }
}

// Tile management
void House::addTile(const MapPos& position) {
    if (!tilePositions_.contains(position)) {
        tilePositions_.append(position);
        emit tileAdded(position);
        emit houseChanged();
    }
}

void House::removeTile(const MapPos& position) {
    if (tilePositions_.removeOne(position)) {
        // Also remove any door ID for this position
        doorIds_.remove(position);
        emit tileRemoved(position);
        emit houseChanged();
    }
}

void House::addTile(Tile* tile) {
    if (tile) {
        addTile(tile->mapPos());
    }
}

void House::removeTile(Tile* tile) {
    if (tile) {
        removeTile(tile->mapPos());
    }
}

bool House::containsTile(const MapPos& position) const {
    return tilePositions_.contains(position);
}

bool House::containsTile(Tile* tile) const {
    return tile && containsTile(tile->mapPos());
}

const QList<MapPos>& House::getTilePositions() const {
    return tilePositions_;
}

QList<Tile*> House::getTiles(Map* map) const {
    QList<Tile*> tiles;
    if (map) {
        for (const MapPos& pos : tilePositions_) {
            Tile* tile = map->getTile(pos.x, pos.y, pos.z);
            if (tile) {
                tiles.append(tile);
            }
        }
    }
    return tiles;
}

int House::getTileCount() const {
    return tilePositions_.size();
}

int House::getSize() const {
    // This would require Map access to check if tiles are blocking
    // For now, return tile count as approximation
    return getTileCount();
}

// House management
void House::clean(Map* map) {
    if (!map) return;
    
    // Remove house ID from all tiles
    for (const MapPos& pos : tilePositions_) {
        Tile* tile = map->getTile(pos.x, pos.y, pos.z);
        if (tile && tile->getHouseId() == id_) {
            tile->setHouseId(0);
        }
    }
}

void House::clear() {
    tilePositions_.clear();
    doorIds_.clear();
    emit houseChanged();
}

// Door management
quint8 House::getEmptyDoorId() const {
    // Find the lowest unused door ID
    for (quint8 id = 1; id <= 255; ++id) {
        bool used = false;
        for (auto it = doorIds_.begin(); it != doorIds_.end(); ++it) {
            if (it.value() == id) {
                used = true;
                break;
            }
        }
        if (!used) {
            return id;
        }
    }
    return 0; // No empty ID found
}

MapPos House::getDoorPositionById(quint8 doorId) const {
    for (auto it = doorIds_.begin(); it != doorIds_.end(); ++it) {
        if (it.value() == doorId) {
            return it.key();
        }
    }
    return MapPos(0, 0, 0); // Not found
}

void House::setDoorId(const MapPos& position, quint8 doorId) {
    if (containsTile(position)) {
        if (doorId == 0) {
            doorIds_.remove(position);
        } else {
            doorIds_[position] = doorId;
        }
        emit houseChanged();
    }
}

// Validation
bool House::isValid() const {
    return id_ > 0 && !name_.isEmpty() && !tilePositions_.isEmpty();
}

QString House::getValidationError() const {
    if (id_ == 0) return "House ID cannot be 0";
    if (name_.isEmpty()) return "House name cannot be empty";
    if (tilePositions_.isEmpty()) return "House must have at least one tile";
    return QString();
}

// Utility
QString House::getDescription() const {
    QString desc = QString("House %1: %2").arg(id_).arg(name_);
    if (!owner_.isEmpty()) {
        desc += QString(" (Owner: %1)").arg(owner_);
    }
    if (rent_ > 0) {
        desc += QString(" (Rent: %1)").arg(rent_);
    }
    if (isGuildHall_) {
        desc += " [Guild Hall]";
    }
    desc += QString(" (%1 tiles)").arg(tilePositions_.size());
    return desc;
}

quint32 House::memsize() const {
    quint32 size = sizeof(House);
    size += name_.size() * sizeof(QChar);
    size += owner_.size() * sizeof(QChar);
    size += tilePositions_.size() * sizeof(MapPos);
    size += doorIds_.size() * (sizeof(MapPos) + sizeof(quint8));
    return size;
}

// Serialization support
bool House::serialize(QDataStream& stream) const {
    stream << id_ << name_ << owner_ << rent_ << townId_ << isGuildHall_;
    stream << entryPosition_.x << entryPosition_.y << entryPosition_.z;
    stream << exitPosition_.x << exitPosition_.y << exitPosition_.z;
    stream << tilePositions_.size();
    for (const MapPos& pos : tilePositions_) {
        stream << pos.x << pos.y << pos.z;
    }
    stream << doorIds_.size();
    for (auto it = doorIds_.begin(); it != doorIds_.end(); ++it) {
        stream << it.key().x << it.key().y << it.key().z << it.value();
    }
    return stream.status() == QDataStream::Ok;
}

bool House::deserialize(QDataStream& stream) {
    int tileCount, doorCount;
    stream >> id_ >> name_ >> owner_ >> rent_ >> townId_ >> isGuildHall_;
    stream >> entryPosition_.x >> entryPosition_.y >> entryPosition_.z;
    stream >> exitPosition_.x >> exitPosition_.y >> exitPosition_.z;
    stream >> tileCount;
    
    tilePositions_.clear();
    for (int i = 0; i < tileCount; ++i) {
        MapPos pos;
        stream >> pos.x >> pos.y >> pos.z;
        tilePositions_.append(pos);
    }
    
    stream >> doorCount;
    doorIds_.clear();
    for (int i = 0; i < doorCount; ++i) {
        MapPos pos;
        quint8 doorId;
        stream >> pos.x >> pos.y >> pos.z >> doorId;
        doorIds_[pos] = doorId;
    }
    
    return stream.status() == QDataStream::Ok;
}

// Task 66: XML serialization implementation
bool House::loadFromXml(const QDomElement& element) {
    // Load basic house properties
    id_ = element.attribute("houseid", "0").toUInt();
    name_ = element.attribute("name", "");
    rent_ = element.attribute("rent", "0").toInt();
    townId_ = element.attribute("townid", "0").toUInt();
    isGuildHall_ = element.attribute("guildhall", "false").toLower() == "true";

    // Load entry position (exit in wxwidgets terminology)
    entryPosition_.x = element.attribute("entryx", "0").toInt();
    entryPosition_.y = element.attribute("entryy", "0").toInt();
    entryPosition_.z = element.attribute("entryz", "0").toInt();

    // For now, set exit position same as entry (can be enhanced later)
    exitPosition_ = entryPosition_;

    // Load tile positions from child elements
    tilePositions_.clear();
    doorIds_.clear();

    QDomNodeList tileNodes = element.elementsByTagName("tile");
    for (int i = 0; i < tileNodes.count(); ++i) {
        QDomElement tileElement = tileNodes.at(i).toElement();
        if (!tileElement.isNull()) {
            MapPos pos;
            pos.x = tileElement.attribute("x", "0").toInt();
            pos.y = tileElement.attribute("y", "0").toInt();
            pos.z = tileElement.attribute("z", "0").toInt();
            tilePositions_.append(pos);

            // Load door ID if present
            if (tileElement.hasAttribute("doorid")) {
                quint8 doorId = tileElement.attribute("doorid", "0").toUInt();
                doorIds_[pos] = doorId;
            }
        }
    }

    return true;
}

bool House::saveToXml(QDomDocument& doc, QDomElement& parent) const {
    QDomElement houseElement = doc.createElement("house");

    // Save basic house properties
    houseElement.setAttribute("houseid", id_);
    houseElement.setAttribute("name", name_);
    houseElement.setAttribute("rent", rent_);
    if (townId_ > 0) {
        houseElement.setAttribute("townid", townId_);
    }
    if (isGuildHall_) {
        houseElement.setAttribute("guildhall", "true");
    }

    // Save entry position
    houseElement.setAttribute("entryx", entryPosition_.x);
    houseElement.setAttribute("entryy", entryPosition_.y);
    houseElement.setAttribute("entryz", entryPosition_.z);

    // Save tile positions
    for (const MapPos& pos : tilePositions_) {
        QDomElement tileElement = doc.createElement("tile");
        tileElement.setAttribute("x", pos.x);
        tileElement.setAttribute("y", pos.y);
        tileElement.setAttribute("z", pos.z);

        // Save door ID if present
        if (doorIds_.contains(pos)) {
            tileElement.setAttribute("doorid", doorIds_[pos]);
        }

        houseElement.appendChild(tileElement);
    }

    parent.appendChild(houseElement);
    return true;
}

bool House::loadHousesFromXML(const QString& filePath, QList<House*>& houses, QStringList& errors) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errors << QString("Cannot open houses file: %1").arg(filePath);
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
    if (root.tagName() != "houses") {
        errors << QString("Invalid houses XML file: root element should be 'houses'");
        return false;
    }

    // Clear existing houses
    qDeleteAll(houses);
    houses.clear();

    // Load houses
    QDomNodeList houseNodes = root.elementsByTagName("house");
    for (int i = 0; i < houseNodes.count(); ++i) {
        QDomElement houseElement = houseNodes.at(i).toElement();
        if (!houseElement.isNull()) {
            House* house = new House();
            if (house->loadFromXml(houseElement)) {
                houses.append(house);
                qDebug() << "Loaded house:" << house->getName() << "ID:" << house->getId();
            } else {
                errors << QString("Failed to load house at index %1").arg(i);
                delete house;
            }
        }
    }

    qDebug() << "Loaded" << houses.size() << "houses from" << filePath;
    return true;
}

bool House::saveHousesToXML(const QString& filePath, const QList<House*>& houses, QStringList& errors) {
    QDomDocument doc;
    QDomElement root = doc.createElement("houses");
    doc.appendChild(root);

    // Save all houses
    for (House* house : houses) {
        if (house) {
            if (!house->saveToXml(doc, root)) {
                errors << QString("Failed to save house: %1 (ID: %2)").arg(house->getName()).arg(house->getId());
            }
        }
    }

    // Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errors << QString("Cannot write houses file: %1").arg(filePath);
        return false;
    }

    QTextStream stream(&file);
    stream << doc.toString(4); // 4-space indentation
    file.close();

    qDebug() << "Saved" << houses.size() << "houses to" << filePath;
    return errors.isEmpty();
}

// Houses collection implementation
Houses::Houses(QObject* parent)
    : QObject(parent), nextId_(1) {
}

Houses::~Houses() {
    clear();
}

void Houses::addHouse(House* house) {
    if (!house) return;
    
    quint32 id = house->getId();
    if (id == 0) {
        id = getEmptyId();
        house->setId(id);
    }
    
    if (!houses_.contains(id)) {
        houses_[id] = house;
        house->setParent(this);
        if (id >= nextId_) {
            nextId_ = id + 1;
        }
        emit houseAdded(house);
    }
}

void Houses::removeHouse(House* house) {
    if (house) {
        removeHouse(house->getId());
    }
}

void Houses::removeHouse(quint32 houseId) {
    auto it = houses_.find(houseId);
    if (it != houses_.end()) {
        House* house = it.value();
        houses_.erase(it);
        house->setParent(nullptr);
        emit houseRemoved(houseId);
        delete house;
    }
}

House* Houses::getHouse(quint32 houseId) const {
    return houses_.value(houseId, nullptr);
}

House* Houses::getHouseByName(const QString& name) const {
    for (House* house : houses_) {
        if (house && house->getName().compare(name, Qt::CaseInsensitive) == 0) {
            return house;
        }
    }
    return nullptr;
}

const QMap<quint32, House*>& Houses::getHouses() const {
    return houses_;
}

QList<House*> Houses::getHousesList() const {
    return houses_.values();
}

int Houses::getHouseCount() const {
    return houses_.size();
}

bool Houses::isEmpty() const {
    return houses_.isEmpty();
}

quint32 Houses::getEmptyId() const {
    while (houses_.contains(nextId_)) {
        ++nextId_;
    }
    return nextId_;
}

quint32 Houses::getNextId() const {
    return nextId_;
}

bool Houses::isIdUsed(quint32 id) const {
    return houses_.contains(id);
}

void Houses::clear() {
    qDeleteAll(houses_);
    houses_.clear();
    nextId_ = 1;
    emit housesCleared();
}

quint32 Houses::memsize() const {
    quint32 size = sizeof(Houses);
    for (House* house : houses_) {
        if (house) {
            size += house->memsize();
        }
    }
    return size;
}

// Iterator support
QMap<quint32, House*>::iterator Houses::begin() {
    return houses_.begin();
}

QMap<quint32, House*>::iterator Houses::end() {
    return houses_.end();
}

QMap<quint32, House*>::const_iterator Houses::begin() const {
    return houses_.begin();
}

QMap<quint32, House*>::const_iterator Houses::end() const {
    return houses_.end();
}

QMap<quint32, House*>::const_iterator Houses::find(quint32 id) const {
    return houses_.find(id);
}

// Task 73: Enhanced house operations implementation
void House::removeLooseItems(Map* map) {
    if (!map) return;

    for (const MapPos& pos : tilePositions_) {
        Tile* tile = map->getTile(pos);
        if (!tile) continue;

        // Remove loose items (items that are moveable)
        auto& items = tile->getItems();
        for (auto it = items.begin(); it != items.end(); ) {
            Item* item = *it;
            if (item && item->isMoveable()) {
                delete item;
                it = items.erase(it);
            } else {
                ++it;
            }
        }
    }

    emit houseChanged();
}

void House::setPZOnTiles(Map* map, bool pz) {
    if (!map) return;

    for (const MapPos& pos : tilePositions_) {
        Tile* tile = map->getTile(pos);
        if (tile) {
            tile->setPZ(pz);
        }
    }

    emit houseChanged();
}

void House::assignDoorIDs(Map* map) {
    if (!map) return;

    quint8 nextDoorId = 1;

    for (const MapPos& pos : tilePositions_) {
        Tile* tile = map->getTile(pos);
        if (!tile) continue;

        // Find doors on this tile
        for (Item* item : tile->getItems()) {
            if (Door* door = dynamic_cast<Door*>(item)) {
                if (door->getDoorID() == 0) {
                    // Find next available door ID
                    while (isDoorIDUsed(nextDoorId)) {
                        nextDoorId++;
                        if (nextDoorId == 0) nextDoorId = 1; // Wrap around, skip 0
                    }

                    door->setDoorID(nextDoorId);
                    doorIds_[pos] = nextDoorId;
                    nextDoorId++;
                }
            }
        }
    }

    emit houseChanged();
}

void House::clearDoorIDs(Map* map) {
    if (!map) return;

    for (const MapPos& pos : tilePositions_) {
        Tile* tile = map->getTile(pos);
        if (!tile) continue;

        // Clear door IDs on this tile
        for (Item* item : tile->getItems()) {
            if (Door* door = dynamic_cast<Door*>(item)) {
                door->setDoorID(0);
            }
        }
    }

    doorIds_.clear();
    emit houseChanged();
}

bool House::isDoorIDUsed(quint8 doorId) const {
    return doorIds_.values().contains(doorId);
}

// Task 73: House area calculations
QRect House::getBoundingRect() const {
    if (tilePositions_.isEmpty()) {
        return QRect();
    }

    int minX = tilePositions_.first().x;
    int maxX = minX;
    int minY = tilePositions_.first().y;
    int maxY = minY;

    for (const MapPos& pos : tilePositions_) {
        minX = qMin(minX, pos.x);
        maxX = qMax(maxX, pos.x);
        minY = qMin(minY, pos.y);
        maxY = qMax(maxY, pos.y);
    }

    return QRect(minX, minY, maxX - minX + 1, maxY - minY + 1);
}

QList<QRect> House::getAreas() const {
    QList<QRect> areas;
    QSet<MapPos> processedTiles;

    for (const MapPos& pos : tilePositions_) {
        if (processedTiles.contains(pos)) {
            continue;
        }

        // Find connected area starting from this position
        QList<MapPos> areaPositions;
        QList<MapPos> toProcess;
        toProcess.append(pos);

        while (!toProcess.isEmpty()) {
            MapPos currentPos = toProcess.takeFirst();
            if (processedTiles.contains(currentPos)) {
                continue;
            }

            processedTiles.insert(currentPos);
            areaPositions.append(currentPos);

            // Check adjacent positions
            QList<MapPos> adjacent = {
                MapPos(currentPos.x - 1, currentPos.y, currentPos.z),
                MapPos(currentPos.x + 1, currentPos.y, currentPos.z),
                MapPos(currentPos.x, currentPos.y - 1, currentPos.z),
                MapPos(currentPos.x, currentPos.y + 1, currentPos.z)
            };

            for (const MapPos& adjPos : adjacent) {
                if (tilePositions_.contains(adjPos) && !processedTiles.contains(adjPos)) {
                    toProcess.append(adjPos);
                }
            }
        }

        // Calculate bounding rect for this area
        if (!areaPositions.isEmpty()) {
            int minX = areaPositions.first().x;
            int maxX = minX;
            int minY = areaPositions.first().y;
            int maxY = minY;

            for (const MapPos& areaPos : areaPositions) {
                minX = qMin(minX, areaPos.x);
                maxX = qMax(maxX, areaPos.x);
                minY = qMin(minY, areaPos.y);
                maxY = qMax(maxY, areaPos.y);
            }

            areas.append(QRect(minX, minY, maxX - minX + 1, maxY - minY + 1));
        }
    }

    return areas;
}

int House::getArea() const {
    return tilePositions_.size();
}

bool House::isConnected(Map* map) const {
    if (!map || tilePositions_.isEmpty()) {
        return true; // Empty house is considered connected
    }

    QSet<MapPos> visited;
    QList<MapPos> toVisit;
    toVisit.append(tilePositions_.first());

    while (!toVisit.isEmpty()) {
        MapPos currentPos = toVisit.takeFirst();
        if (visited.contains(currentPos)) {
            continue;
        }

        visited.insert(currentPos);

        // Check adjacent positions
        QList<MapPos> adjacent = {
            MapPos(currentPos.x - 1, currentPos.y, currentPos.z),
            MapPos(currentPos.x + 1, currentPos.y, currentPos.z),
            MapPos(currentPos.x, currentPos.y - 1, currentPos.z),
            MapPos(currentPos.x, currentPos.y + 1, currentPos.z)
        };

        for (const MapPos& adjPos : adjacent) {
            if (tilePositions_.contains(adjPos) && !visited.contains(adjPos)) {
                toVisit.append(adjPos);
            }
        }
    }

    return visited.size() == tilePositions_.size();
}

// Task 73: Advanced tile operations
QList<Tile*> House::getTilesInArea(Map* map, const QRect& area) const {
    QList<Tile*> tiles;
    if (!map) return tiles;

    for (const MapPos& pos : tilePositions_) {
        if (area.contains(pos.x, pos.y)) {
            Tile* tile = map->getTile(pos);
            if (tile) {
                tiles.append(tile);
            }
        }
    }

    return tiles;
}

Tile* House::getTileAt(Map* map, const MapPos& position) const {
    if (!map || !tilePositions_.contains(position)) {
        return nullptr;
    }

    return map->getTile(position);
}

void House::removeInvalidTiles(Map* map) {
    if (!map) return;

    QList<MapPos> invalidTiles;

    for (const MapPos& pos : tilePositions_) {
        Tile* tile = map->getTile(pos);
        if (!tile || tile->getHouseID() != id_) {
            invalidTiles.append(pos);
        }
    }

    for (const MapPos& pos : invalidTiles) {
        tilePositions_.removeAll(pos);
        doorIds_.remove(pos);
    }

    if (!invalidTiles.isEmpty()) {
        emit houseChanged();
    }
}

void House::updateTileHouseReferences(Map* map) {
    if (!map) return;

    for (const MapPos& pos : tilePositions_) {
        Tile* tile = map->getTile(pos);
        if (tile && tile->getHouseID() != id_) {
            tile->setHouseID(id_);
        }
    }

    emit houseChanged();
}

// Task 73: House utilities
QJsonObject House::toJson() const {
    QJsonObject json;
    json["id"] = static_cast<qint64>(id_);
    json["name"] = name_;
    json["owner"] = owner_;
    json["rent"] = rent_;
    json["townId"] = static_cast<qint64>(townId_);
    json["isGuildHall"] = isGuildHall_;

    QJsonObject entryPos;
    entryPos["x"] = entryPosition_.x;
    entryPos["y"] = entryPosition_.y;
    entryPos["z"] = entryPosition_.z;
    json["entryPosition"] = entryPos;

    QJsonObject exitPos;
    exitPos["x"] = exitPosition_.x;
    exitPos["y"] = exitPosition_.y;
    exitPos["z"] = exitPosition_.z;
    json["exitPosition"] = exitPos;

    QJsonArray tilesArray;
    for (const MapPos& pos : tilePositions_) {
        QJsonObject tileObj;
        tileObj["x"] = pos.x;
        tileObj["y"] = pos.y;
        tileObj["z"] = pos.z;
        if (doorIds_.contains(pos)) {
            tileObj["doorId"] = doorIds_[pos];
        }
        tilesArray.append(tileObj);
    }
    json["tiles"] = tilesArray;

    return json;
}

void House::fromJson(const QJsonObject& json) {
    id_ = static_cast<quint32>(json["id"].toInt());
    name_ = json["name"].toString();
    owner_ = json["owner"].toString();
    rent_ = json["rent"].toInt();
    townId_ = static_cast<quint32>(json["townId"].toInt());
    isGuildHall_ = json["isGuildHall"].toBool();

    QJsonObject entryPos = json["entryPosition"].toObject();
    entryPosition_.x = entryPos["x"].toInt();
    entryPosition_.y = entryPos["y"].toInt();
    entryPosition_.z = entryPos["z"].toInt();

    QJsonObject exitPos = json["exitPosition"].toObject();
    exitPosition_.x = exitPos["x"].toInt();
    exitPosition_.y = exitPos["y"].toInt();
    exitPosition_.z = exitPos["z"].toInt();

    tilePositions_.clear();
    doorIds_.clear();

    QJsonArray tilesArray = json["tiles"].toArray();
    for (const QJsonValue& value : tilesArray) {
        QJsonObject tileObj = value.toObject();
        MapPos pos;
        pos.x = tileObj["x"].toInt();
        pos.y = tileObj["y"].toInt();
        pos.z = tileObj["z"].toInt();
        tilePositions_.append(pos);

        if (tileObj.contains("doorId")) {
            doorIds_[pos] = static_cast<quint8>(tileObj["doorId"].toInt());
        }
    }

    emit houseChanged();
}

QString House::getHouseInfo() const {
    QString info;
    info += QString("House ID: %1\n").arg(id_);
    info += QString("Name: %1\n").arg(name_);
    info += QString("Owner: %1\n").arg(owner_.isEmpty() ? "None" : owner_);
    info += QString("Rent: %1 gold\n").arg(rent_);
    info += QString("Town ID: %1\n").arg(townId_);
    info += QString("Guild Hall: %1\n").arg(isGuildHall_ ? "Yes" : "No");
    info += QString("Tiles: %1\n").arg(tilePositions_.size());
    info += QString("Doors: %1\n").arg(doorIds_.size());
    info += QString("Entry: [%1, %2, %3]\n").arg(entryPosition_.x).arg(entryPosition_.y).arg(entryPosition_.z);
    info += QString("Exit: [%1, %2, %3]").arg(exitPosition_.x).arg(exitPosition_.y).arg(exitPosition_.z);
    return info;
}

bool House::hasValidExit(Map* map) const {
    if (!map) return false;

    Tile* exitTile = map->getTile(exitPosition_);
    return exitTile != nullptr;
}

#include "House.moc"
