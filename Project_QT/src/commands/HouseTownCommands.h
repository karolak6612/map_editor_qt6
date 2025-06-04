#ifndef HOUSETOWNCOMMANDS_H
#define HOUSETOWNCOMMANDS_H

#include <QUndoCommand>
#include <QVariant>
#include <QString>
#include <QMap>
#include <QList>

#include "MapPos.h"
#include "House.h"
#include "Town.h"

// Forward declarations
class Map;
class Tile;

/**
 * @brief Task 94: Houses and Towns Command System for undo/redo functionality
 * 
 * Complete undo/redo system for house and town operations:
 * - House creation, deletion, and modification commands
 * - Town creation, deletion, and modification commands
 * - House tile assignment and removal commands
 * - Town-house linking commands
 * - Batch operations for multiple houses/towns
 * - Map integration for automatic updates
 * - Tile flag management commands
 * - Efficient memory usage and command merging
 */

/**
 * @brief Create House Command
 */
class CreateHouseCommand : public QUndoCommand
{
public:
    explicit CreateHouseCommand(Map* map, House* house, QUndoCommand* parent = nullptr);
    ~CreateHouseCommand() override;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    House* getHouse() const { return house_; }
    Map* getMap() const { return map_; }

private:
    Map* map_;
    House* house_;
    bool houseCreated_;
    
    static const int COMMAND_ID = 2001;
};

/**
 * @brief Delete House Command
 */
class DeleteHouseCommand : public QUndoCommand
{
public:
    explicit DeleteHouseCommand(Map* map, House* house, QUndoCommand* parent = nullptr);
    ~DeleteHouseCommand() override;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    House* getHouse() const { return house_; }
    Map* getMap() const { return map_; }

private:
    Map* map_;
    House* house_;
    bool houseDeleted_;
    QVariantMap houseData_; // Backup of house data for restoration
    
    static const int COMMAND_ID = 2002;
};

/**
 * @brief Modify House Command
 */
class ModifyHouseCommand : public QUndoCommand
{
public:
    explicit ModifyHouseCommand(House* house, const QString& property, 
                               const QVariant& newValue, const QVariant& oldValue,
                               Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~ModifyHouseCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    House* getHouse() const { return house_; }
    QString getProperty() const { return property_; }
    QVariant getOldValue() const { return oldValue_; }
    QVariant getNewValue() const { return newValue_; }

private:
    void applyProperty(const QVariant& value);
    void updateMap();
    bool isValidProperty(const QString& property) const;

private:
    House* house_;
    Map* map_;
    QString property_;
    QVariant oldValue_;
    QVariant newValue_;
    
    static const int COMMAND_ID = 2003;
};

/**
 * @brief Add House Tile Command
 */
class AddHouseTileCommand : public QUndoCommand
{
public:
    explicit AddHouseTileCommand(House* house, const MapPos& position, 
                                Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~AddHouseTileCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    House* getHouse() const { return house_; }
    MapPos getPosition() const { return position_; }

private:
    void updateTileFlags(bool hasHouse);
    void updateMap();

private:
    House* house_;
    Map* map_;
    MapPos position_;
    
    static const int COMMAND_ID = 2004;
};

/**
 * @brief Remove House Tile Command
 */
class RemoveHouseTileCommand : public QUndoCommand
{
public:
    explicit RemoveHouseTileCommand(House* house, const MapPos& position, 
                                   Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~RemoveHouseTileCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    House* getHouse() const { return house_; }
    MapPos getPosition() const { return position_; }

private:
    void updateTileFlags(bool hasHouse);
    void updateMap();

private:
    House* house_;
    Map* map_;
    MapPos position_;
    
    static const int COMMAND_ID = 2005;
};

/**
 * @brief Create Town Command
 */
class CreateTownCommand : public QUndoCommand
{
public:
    explicit CreateTownCommand(Map* map, Town* town, QUndoCommand* parent = nullptr);
    ~CreateTownCommand() override;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Town* getTown() const { return town_; }
    Map* getMap() const { return map_; }

private:
    Map* map_;
    Town* town_;
    bool townCreated_;
    
    static const int COMMAND_ID = 2006;
};

/**
 * @brief Delete Town Command
 */
class DeleteTownCommand : public QUndoCommand
{
public:
    explicit DeleteTownCommand(Map* map, Town* town, QUndoCommand* parent = nullptr);
    ~DeleteTownCommand() override;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Town* getTown() const { return town_; }
    Map* getMap() const { return map_; }

private:
    Map* map_;
    Town* town_;
    bool townDeleted_;
    QVariantMap townData_; // Backup of town data for restoration
    
    static const int COMMAND_ID = 2007;
};

/**
 * @brief Modify Town Command
 */
class ModifyTownCommand : public QUndoCommand
{
public:
    explicit ModifyTownCommand(Town* town, const QString& property, 
                              const QVariant& newValue, const QVariant& oldValue,
                              Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~ModifyTownCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Town* getTown() const { return town_; }
    QString getProperty() const { return property_; }
    QVariant getOldValue() const { return oldValue_; }
    QVariant getNewValue() const { return newValue_; }

private:
    void applyProperty(const QVariant& value);
    void updateMap();
    bool isValidProperty(const QString& property) const;

private:
    Town* town_;
    Map* map_;
    QString property_;
    QVariant oldValue_;
    QVariant newValue_;
    
    static const int COMMAND_ID = 2008;
};

/**
 * @brief Link Town House Command
 */
class LinkTownHouseCommand : public QUndoCommand
{
public:
    explicit LinkTownHouseCommand(Town* town, House* house, 
                                 Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~LinkTownHouseCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Town* getTown() const { return town_; }
    House* getHouse() const { return house_; }

private:
    void updateMap();

private:
    Town* town_;
    House* house_;
    Map* map_;
    
    static const int COMMAND_ID = 2009;
};

/**
 * @brief Unlink Town House Command
 */
class UnlinkTownHouseCommand : public QUndoCommand
{
public:
    explicit UnlinkTownHouseCommand(Town* town, House* house, 
                                   Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~UnlinkTownHouseCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Town* getTown() const { return town_; }
    House* getHouse() const { return house_; }

private:
    void updateMap();

private:
    Town* town_;
    House* house_;
    Map* map_;
    
    static const int COMMAND_ID = 2010;
};

/**
 * @brief Batch House Operations Command
 */
class BatchHouseOperationsCommand : public QUndoCommand
{
public:
    struct HouseOperation {
        enum Type {
            CREATE_HOUSE,
            DELETE_HOUSE,
            MODIFY_HOUSE,
            ADD_TILE,
            REMOVE_TILE
        };
        
        Type type;
        House* house;
        QString property;
        QVariant oldValue;
        QVariant newValue;
        MapPos position;
        
        HouseOperation(Type t = MODIFY_HOUSE, House* h = nullptr)
            : type(t), house(h) {}
    };

    explicit BatchHouseOperationsCommand(const QList<HouseOperation>& operations,
                                        Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~BatchHouseOperationsCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    QList<HouseOperation> getOperations() const { return operations_; }
    int getOperationCount() const { return operations_.size(); }

private:
    void applyOperations(bool forward);
    void updateMap();
    bool validateOperations(const QList<HouseOperation>& operations) const;

private:
    QList<HouseOperation> operations_;
    Map* map_;
    
    static const int COMMAND_ID = 2011;
};

/**
 * @brief Batch Town Operations Command
 */
class BatchTownOperationsCommand : public QUndoCommand
{
public:
    struct TownOperation {
        enum Type {
            CREATE_TOWN,
            DELETE_TOWN,
            MODIFY_TOWN,
            LINK_HOUSE,
            UNLINK_HOUSE
        };
        
        Type type;
        Town* town;
        House* house;
        QString property;
        QVariant oldValue;
        QVariant newValue;
        
        TownOperation(Type t = MODIFY_TOWN, Town* tn = nullptr, House* h = nullptr)
            : type(t), town(tn), house(h) {}
    };

    explicit BatchTownOperationsCommand(const QList<TownOperation>& operations,
                                       Map* map = nullptr, QUndoCommand* parent = nullptr);
    ~BatchTownOperationsCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    QList<TownOperation> getOperations() const { return operations_; }
    int getOperationCount() const { return operations_.size(); }

private:
    void applyOperations(bool forward);
    void updateMap();
    bool validateOperations(const QList<TownOperation>& operations) const;

private:
    QList<TownOperation> operations_;
    Map* map_;
    
    static const int COMMAND_ID = 2012;
};

/**
 * @brief Houses and Towns Command Factory
 */
class HouseTownCommandFactory
{
public:
    // House commands
    static CreateHouseCommand* createHouseCommand(Map* map, House* house);
    static DeleteHouseCommand* deleteHouseCommand(Map* map, House* house);
    static ModifyHouseCommand* modifyHouseCommand(House* house, const QString& property,
                                                 const QVariant& newValue, const QVariant& oldValue,
                                                 Map* map = nullptr);
    static AddHouseTileCommand* addHouseTileCommand(House* house, const MapPos& position, Map* map = nullptr);
    static RemoveHouseTileCommand* removeHouseTileCommand(House* house, const MapPos& position, Map* map = nullptr);
    
    // Town commands
    static CreateTownCommand* createTownCommand(Map* map, Town* town);
    static DeleteTownCommand* deleteTownCommand(Map* map, Town* town);
    static ModifyTownCommand* modifyTownCommand(Town* town, const QString& property,
                                               const QVariant& newValue, const QVariant& oldValue,
                                               Map* map = nullptr);
    static LinkTownHouseCommand* linkTownHouseCommand(Town* town, House* house, Map* map = nullptr);
    static UnlinkTownHouseCommand* unlinkTownHouseCommand(Town* town, House* house, Map* map = nullptr);
    
    // Batch commands
    static BatchHouseOperationsCommand* createBatchHouseCommand(const QList<BatchHouseOperationsCommand::HouseOperation>& operations,
                                                               Map* map = nullptr);
    static BatchTownOperationsCommand* createBatchTownCommand(const QList<BatchTownOperationsCommand::TownOperation>& operations,
                                                             Map* map = nullptr);

private:
    HouseTownCommandFactory() = default;
};

#endif // HOUSETOWNCOMMANDS_H
