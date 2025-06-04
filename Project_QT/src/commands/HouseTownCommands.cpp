#include "HouseTownCommands.h"
#include "Map.h"
#include "Tile.h"
#include <QDebug>

// CreateHouseCommand implementation
CreateHouseCommand::CreateHouseCommand(Map* map, House* house, QUndoCommand* parent)
    : QUndoCommand("Create House", parent),
      map_(map),
      house_(house),
      houseCreated_(false) {
}

CreateHouseCommand::~CreateHouseCommand() {
    if (!houseCreated_ && house_) {
        delete house_;
    }
}

void CreateHouseCommand::undo() {
    if (map_ && house_ && houseCreated_) {
        map_->removeHouse(house_);
        houseCreated_ = false;
        qDebug() << "CreateHouseCommand: Undid house creation";
    }
}

void CreateHouseCommand::redo() {
    if (map_ && house_ && !houseCreated_) {
        map_->addHouse(house_);
        houseCreated_ = true;
        qDebug() << "CreateHouseCommand: Created house";
    }
}

int CreateHouseCommand::id() const {
    return COMMAND_ID;
}

bool CreateHouseCommand::mergeWith(const QUndoCommand* other) {
    Q_UNUSED(other);
    return false; // House creation commands should not be merged
}

// DeleteHouseCommand implementation
DeleteHouseCommand::DeleteHouseCommand(Map* map, House* house, QUndoCommand* parent)
    : QUndoCommand("Delete House", parent),
      map_(map),
      house_(house),
      houseDeleted_(false) {
    // Backup house data for restoration
    if (house_) {
        houseData_["id"] = house_->getID();
        houseData_["name"] = house_->getName();
        // TODO: Add more house properties as needed
    }
}

DeleteHouseCommand::~DeleteHouseCommand() {
    if (houseDeleted_ && house_) {
        delete house_;
    }
}

void DeleteHouseCommand::undo() {
    if (map_ && house_ && houseDeleted_) {
        map_->addHouse(house_);
        houseDeleted_ = false;
        qDebug() << "DeleteHouseCommand: Restored house";
    }
}

void DeleteHouseCommand::redo() {
    if (map_ && house_ && !houseDeleted_) {
        map_->removeHouse(house_);
        houseDeleted_ = true;
        qDebug() << "DeleteHouseCommand: Deleted house";
    }
}

int DeleteHouseCommand::id() const {
    return COMMAND_ID;
}

bool DeleteHouseCommand::mergeWith(const QUndoCommand* other) {
    Q_UNUSED(other);
    return false; // House deletion commands should not be merged
}

// ModifyHouseCommand implementation
ModifyHouseCommand::ModifyHouseCommand(House* house, const QString& property, 
                                     const QVariant& newValue, const QVariant& oldValue,
                                     Map* map, QUndoCommand* parent)
    : QUndoCommand(QString("Modify House %1").arg(property), parent),
      house_(house),
      map_(map),
      property_(property),
      oldValue_(oldValue),
      newValue_(newValue) {
}

void ModifyHouseCommand::undo() {
    if (house_ && isValidProperty(property_)) {
        applyProperty(oldValue_);
        updateMap();
        qDebug() << "ModifyHouseCommand: Undid property" << property_;
    }
}

void ModifyHouseCommand::redo() {
    if (house_ && isValidProperty(property_)) {
        applyProperty(newValue_);
        updateMap();
        qDebug() << "ModifyHouseCommand: Applied property" << property_;
    }
}

int ModifyHouseCommand::id() const {
    return COMMAND_ID;
}

bool ModifyHouseCommand::mergeWith(const QUndoCommand* other) {
    const ModifyHouseCommand* otherCommand = dynamic_cast<const ModifyHouseCommand*>(other);
    if (!otherCommand || otherCommand->house_ != house_ || otherCommand->property_ != property_) {
        return false;
    }
    
    // Merge by updating the new value
    newValue_ = otherCommand->newValue_;
    return true;
}

void ModifyHouseCommand::applyProperty(const QVariant& value) {
    if (!house_) return;
    
    if (property_ == "name") {
        house_->setName(value.toString());
    } else if (property_ == "id") {
        house_->setID(value.toUInt());
    }
    // TODO: Add more properties as needed
}

void ModifyHouseCommand::updateMap() {
    if (map_) {
        map_->updateHouse(house_);
    }
}

bool ModifyHouseCommand::isValidProperty(const QString& property) const {
    static const QStringList validProperties = {"name", "id"};
    return validProperties.contains(property);
}

// AddHouseTileCommand implementation
AddHouseTileCommand::AddHouseTileCommand(House* house, const MapPos& position, 
                                        Map* map, QUndoCommand* parent)
    : QUndoCommand("Add House Tile", parent),
      house_(house),
      map_(map),
      position_(position) {
}

void AddHouseTileCommand::undo() {
    if (house_) {
        house_->removeTile(position_);
        updateTileFlags(false);
        updateMap();
        qDebug() << "AddHouseTileCommand: Removed tile from house";
    }
}

void AddHouseTileCommand::redo() {
    if (house_) {
        house_->addTile(position_);
        updateTileFlags(true);
        updateMap();
        qDebug() << "AddHouseTileCommand: Added tile to house";
    }
}

int AddHouseTileCommand::id() const {
    return COMMAND_ID;
}

bool AddHouseTileCommand::mergeWith(const QUndoCommand* other) {
    Q_UNUSED(other);
    return false; // Tile addition commands should not be merged
}

void AddHouseTileCommand::updateTileFlags(bool hasHouse) {
    if (map_) {
        Tile* tile = map_->getTile(position_.x, position_.y, position_.z);
        if (tile) {
            tile->setHouseFlag(hasHouse);
        }
    }
}

void AddHouseTileCommand::updateMap() {
    if (map_) {
        map_->updateHouse(house_);
    }
}

// RemoveHouseTileCommand implementation
RemoveHouseTileCommand::RemoveHouseTileCommand(House* house, const MapPos& position, 
                                              Map* map, QUndoCommand* parent)
    : QUndoCommand("Remove House Tile", parent),
      house_(house),
      map_(map),
      position_(position) {
}

void RemoveHouseTileCommand::undo() {
    if (house_) {
        house_->addTile(position_);
        updateTileFlags(true);
        updateMap();
        qDebug() << "RemoveHouseTileCommand: Restored tile to house";
    }
}

void RemoveHouseTileCommand::redo() {
    if (house_) {
        house_->removeTile(position_);
        updateTileFlags(false);
        updateMap();
        qDebug() << "RemoveHouseTileCommand: Removed tile from house";
    }
}

int RemoveHouseTileCommand::id() const {
    return COMMAND_ID;
}

bool RemoveHouseTileCommand::mergeWith(const QUndoCommand* other) {
    Q_UNUSED(other);
    return false; // Tile removal commands should not be merged
}

void RemoveHouseTileCommand::updateTileFlags(bool hasHouse) {
    if (map_) {
        Tile* tile = map_->getTile(position_.x, position_.y, position_.z);
        if (tile) {
            tile->setHouseFlag(hasHouse);
        }
    }
}

void RemoveHouseTileCommand::updateMap() {
    if (map_) {
        map_->updateHouse(house_);
    }
}

// CreateTownCommand implementation
CreateTownCommand::CreateTownCommand(Map* map, Town* town, QUndoCommand* parent)
    : QUndoCommand("Create Town", parent),
      map_(map),
      town_(town),
      townCreated_(false) {
}

CreateTownCommand::~CreateTownCommand() {
    if (!townCreated_ && town_) {
        delete town_;
    }
}

void CreateTownCommand::undo() {
    if (map_ && town_ && townCreated_) {
        map_->removeTown(town_);
        townCreated_ = false;
        qDebug() << "CreateTownCommand: Undid town creation";
    }
}

void CreateTownCommand::redo() {
    if (map_ && town_ && !townCreated_) {
        map_->addTown(town_);
        townCreated_ = true;
        qDebug() << "CreateTownCommand: Created town";
    }
}

int CreateTownCommand::id() const {
    return COMMAND_ID;
}

bool CreateTownCommand::mergeWith(const QUndoCommand* other) {
    Q_UNUSED(other);
    return false; // Town creation commands should not be merged
}

// DeleteTownCommand implementation
DeleteTownCommand::DeleteTownCommand(Map* map, Town* town, QUndoCommand* parent)
    : QUndoCommand("Delete Town", parent),
      map_(map),
      town_(town),
      townDeleted_(false) {
    // Backup town data for restoration
    if (town_) {
        townData_["id"] = town_->getID();
        townData_["name"] = town_->getName();
        // TODO: Add more town properties as needed
    }
}

DeleteTownCommand::~DeleteTownCommand() {
    if (townDeleted_ && town_) {
        delete town_;
    }
}

void DeleteTownCommand::undo() {
    if (map_ && town_ && townDeleted_) {
        map_->addTown(town_);
        townDeleted_ = false;
        qDebug() << "DeleteTownCommand: Restored town";
    }
}

void DeleteTownCommand::redo() {
    if (map_ && town_ && !townDeleted_) {
        map_->removeTown(town_);
        townDeleted_ = true;
        qDebug() << "DeleteTownCommand: Deleted town";
    }
}

int DeleteTownCommand::id() const {
    return COMMAND_ID;
}

bool DeleteTownCommand::mergeWith(const QUndoCommand* other) {
    Q_UNUSED(other);
    return false; // Town deletion commands should not be merged
}

// ModifyTownCommand implementation
ModifyTownCommand::ModifyTownCommand(Town* town, const QString& property,
                                   const QVariant& newValue, const QVariant& oldValue,
                                   Map* map, QUndoCommand* parent)
    : QUndoCommand(QString("Modify Town %1").arg(property), parent),
      town_(town),
      map_(map),
      property_(property),
      oldValue_(oldValue),
      newValue_(newValue) {
}

void ModifyTownCommand::undo() {
    if (town_ && isValidProperty(property_)) {
        applyProperty(oldValue_);
        updateMap();
        qDebug() << "ModifyTownCommand: Undid property" << property_;
    }
}

void ModifyTownCommand::redo() {
    if (town_ && isValidProperty(property_)) {
        applyProperty(newValue_);
        updateMap();
        qDebug() << "ModifyTownCommand: Applied property" << property_;
    }
}

int ModifyTownCommand::id() const {
    return COMMAND_ID;
}

bool ModifyTownCommand::mergeWith(const QUndoCommand* other) {
    const ModifyTownCommand* otherCommand = dynamic_cast<const ModifyTownCommand*>(other);
    if (!otherCommand || otherCommand->town_ != town_ || otherCommand->property_ != property_) {
        return false;
    }

    // Merge by updating the new value
    newValue_ = otherCommand->newValue_;
    return true;
}

void ModifyTownCommand::applyProperty(const QVariant& value) {
    if (!town_) return;

    if (property_ == "name") {
        town_->setName(value.toString());
    } else if (property_ == "id") {
        town_->setID(value.toUInt());
    }
    // TODO: Add more properties as needed
}

void ModifyTownCommand::updateMap() {
    if (map_) {
        map_->updateTown(town_);
    }
}

bool ModifyTownCommand::isValidProperty(const QString& property) const {
    static const QStringList validProperties = {"name", "id"};
    return validProperties.contains(property);
}

// LinkTownHouseCommand implementation
LinkTownHouseCommand::LinkTownHouseCommand(Town* town, House* house,
                                         Map* map, QUndoCommand* parent)
    : QUndoCommand("Link Town House", parent),
      town_(town),
      house_(house),
      map_(map) {
}

void LinkTownHouseCommand::undo() {
    if (town_ && house_) {
        town_->removeHouse(house_);
        house_->setTown(nullptr);
        updateMap();
        qDebug() << "LinkTownHouseCommand: Unlinked house from town";
    }
}

void LinkTownHouseCommand::redo() {
    if (town_ && house_) {
        town_->addHouse(house_);
        house_->setTown(town_);
        updateMap();
        qDebug() << "LinkTownHouseCommand: Linked house to town";
    }
}

int LinkTownHouseCommand::id() const {
    return COMMAND_ID;
}

bool LinkTownHouseCommand::mergeWith(const QUndoCommand* other) {
    Q_UNUSED(other);
    return false; // Link commands should not be merged
}

void LinkTownHouseCommand::updateMap() {
    if (map_) {
        map_->updateTown(town_);
        map_->updateHouse(house_);
    }
}

// UnlinkTownHouseCommand implementation
UnlinkTownHouseCommand::UnlinkTownHouseCommand(Town* town, House* house,
                                             Map* map, QUndoCommand* parent)
    : QUndoCommand("Unlink Town House", parent),
      town_(town),
      house_(house),
      map_(map) {
}

void UnlinkTownHouseCommand::undo() {
    if (town_ && house_) {
        town_->addHouse(house_);
        house_->setTown(town_);
        updateMap();
        qDebug() << "UnlinkTownHouseCommand: Restored link between house and town";
    }
}

void UnlinkTownHouseCommand::redo() {
    if (town_ && house_) {
        town_->removeHouse(house_);
        house_->setTown(nullptr);
        updateMap();
        qDebug() << "UnlinkTownHouseCommand: Unlinked house from town";
    }
}

int UnlinkTownHouseCommand::id() const {
    return COMMAND_ID;
}

bool UnlinkTownHouseCommand::mergeWith(const QUndoCommand* other) {
    Q_UNUSED(other);
    return false; // Unlink commands should not be merged
}

void UnlinkTownHouseCommand::updateMap() {
    if (map_) {
        map_->updateTown(town_);
        map_->updateHouse(house_);
    }
}

// BatchHouseOperationsCommand implementation
BatchHouseOperationsCommand::BatchHouseOperationsCommand(const QList<HouseOperation>& operations,
                                                        Map* map, QUndoCommand* parent)
    : QUndoCommand("Batch House Operations", parent),
      operations_(operations),
      map_(map) {
}

void BatchHouseOperationsCommand::undo() {
    applyOperations(false);
    updateMap();
    qDebug() << "BatchHouseOperationsCommand: Undid batch operations";
}

void BatchHouseOperationsCommand::redo() {
    applyOperations(true);
    updateMap();
    qDebug() << "BatchHouseOperationsCommand: Applied batch operations";
}

int BatchHouseOperationsCommand::id() const {
    return COMMAND_ID;
}

bool BatchHouseOperationsCommand::mergeWith(const QUndoCommand* other) {
    const BatchHouseOperationsCommand* otherCommand = dynamic_cast<const BatchHouseOperationsCommand*>(other);
    if (!otherCommand || otherCommand->map_ != map_) {
        return false;
    }

    // Merge operations
    operations_.append(otherCommand->operations_);
    return true;
}

void BatchHouseOperationsCommand::applyOperations(bool forward) {
    // TODO: Implement batch operation logic
    Q_UNUSED(forward);
    qDebug() << "BatchHouseOperationsCommand: Processing" << operations_.size() << "operations";
}

void BatchHouseOperationsCommand::updateMap() {
    if (map_) {
        map_->updateAllHouses();
    }
}

bool BatchHouseOperationsCommand::validateOperations(const QList<HouseOperation>& operations) const {
    for (const HouseOperation& op : operations) {
        if (!op.house) {
            return false;
        }
    }
    return true;
}

// BatchTownOperationsCommand implementation
BatchTownOperationsCommand::BatchTownOperationsCommand(const QList<TownOperation>& operations,
                                                      Map* map, QUndoCommand* parent)
    : QUndoCommand("Batch Town Operations", parent),
      operations_(operations),
      map_(map) {
}

void BatchTownOperationsCommand::undo() {
    applyOperations(false);
    updateMap();
    qDebug() << "BatchTownOperationsCommand: Undid batch operations";
}

void BatchTownOperationsCommand::redo() {
    applyOperations(true);
    updateMap();
    qDebug() << "BatchTownOperationsCommand: Applied batch operations";
}

int BatchTownOperationsCommand::id() const {
    return COMMAND_ID;
}

bool BatchTownOperationsCommand::mergeWith(const QUndoCommand* other) {
    const BatchTownOperationsCommand* otherCommand = dynamic_cast<const BatchTownOperationsCommand*>(other);
    if (!otherCommand || otherCommand->map_ != map_) {
        return false;
    }

    // Merge operations
    operations_.append(otherCommand->operations_);
    return true;
}

void BatchTownOperationsCommand::applyOperations(bool forward) {
    // TODO: Implement batch operation logic
    Q_UNUSED(forward);
    qDebug() << "BatchTownOperationsCommand: Processing" << operations_.size() << "operations";
}

void BatchTownOperationsCommand::updateMap() {
    if (map_) {
        map_->updateAllTowns();
    }
}

bool BatchTownOperationsCommand::validateOperations(const QList<TownOperation>& operations) const {
    for (const TownOperation& op : operations) {
        if (!op.town) {
            return false;
        }
    }
    return true;
}

// HouseTownCommandFactory implementation
CreateHouseCommand* HouseTownCommandFactory::createHouseCommand(Map* map, House* house) {
    return new CreateHouseCommand(map, house);
}

DeleteHouseCommand* HouseTownCommandFactory::deleteHouseCommand(Map* map, House* house) {
    return new DeleteHouseCommand(map, house);
}

ModifyHouseCommand* HouseTownCommandFactory::modifyHouseCommand(House* house, const QString& property,
                                                               const QVariant& newValue, const QVariant& oldValue,
                                                               Map* map) {
    return new ModifyHouseCommand(house, property, newValue, oldValue, map);
}

AddHouseTileCommand* HouseTownCommandFactory::addHouseTileCommand(House* house, const MapPos& position, Map* map) {
    return new AddHouseTileCommand(house, position, map);
}

RemoveHouseTileCommand* HouseTownCommandFactory::removeHouseTileCommand(House* house, const MapPos& position, Map* map) {
    return new RemoveHouseTileCommand(house, position, map);
}

CreateTownCommand* HouseTownCommandFactory::createTownCommand(Map* map, Town* town) {
    return new CreateTownCommand(map, town);
}

DeleteTownCommand* HouseTownCommandFactory::deleteTownCommand(Map* map, Town* town) {
    return new DeleteTownCommand(map, town);
}

ModifyTownCommand* HouseTownCommandFactory::modifyTownCommand(Town* town, const QString& property,
                                                             const QVariant& newValue, const QVariant& oldValue,
                                                             Map* map) {
    return new ModifyTownCommand(town, property, newValue, oldValue, map);
}

LinkTownHouseCommand* HouseTownCommandFactory::linkTownHouseCommand(Town* town, House* house, Map* map) {
    return new LinkTownHouseCommand(town, house, map);
}

UnlinkTownHouseCommand* HouseTownCommandFactory::unlinkTownHouseCommand(Town* town, House* house, Map* map) {
    return new UnlinkTownHouseCommand(town, house, map);
}

BatchHouseOperationsCommand* HouseTownCommandFactory::createBatchHouseCommand(const QList<BatchHouseOperationsCommand::HouseOperation>& operations,
                                                                             Map* map) {
    return new BatchHouseOperationsCommand(operations, map);
}

BatchTownOperationsCommand* HouseTownCommandFactory::createBatchTownCommand(const QList<BatchTownOperationsCommand::TownOperation>& operations,
                                                                           Map* map) {
    return new BatchTownOperationsCommand(operations, map);
}
