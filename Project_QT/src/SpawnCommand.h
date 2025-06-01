#ifndef SPAWNCOMMAND_H
#define SPAWNCOMMAND_H

#include <QUndoCommand>
#include "Map.h"

// Forward declarations
class Spawn;
class Tile;

/**
 * Command for placing a spawn on a tile
 */
class PlaceSpawnCommand : public QUndoCommand {
public:
    PlaceSpawnCommand(Map* map, const MapPos& position, Spawn* spawn, QUndoCommand* parent = nullptr);
    ~PlaceSpawnCommand();

    void undo() override;
    void redo() override;

private:
    Map* map_;
    MapPos position_;
    Spawn* spawn_;
    Spawn* previousSpawn_;
    bool spawnWasCreated_;
};

/**
 * Command for removing a spawn from a tile
 */
class RemoveSpawnCommand : public QUndoCommand {
public:
    RemoveSpawnCommand(Map* map, const MapPos& position, QUndoCommand* parent = nullptr);
    ~RemoveSpawnCommand();

    void undo() override;
    void redo() override;

private:
    Map* map_;
    MapPos position_;
    Spawn* removedSpawn_;
};

/**
 * Command for editing spawn properties
 */
class EditSpawnCommand : public QUndoCommand {
public:
    EditSpawnCommand(Spawn* spawn, const QStringList& newCreatureNames, 
                     int newRadius, int newInterval, int newMaxCreatures,
                     QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Spawn* spawn_;
    
    // New values
    QStringList newCreatureNames_;
    int newRadius_;
    int newInterval_;
    int newMaxCreatures_;
    
    // Old values
    QStringList oldCreatureNames_;
    int oldRadius_;
    int oldInterval_;
    int oldMaxCreatures_;
};

#endif // SPAWNCOMMAND_H
