#ifndef PLACEDOORCOMMAND_H
#define PLACEDOORCOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QString>

// Forward declarations
class Map;
class Item;

/**
 * @brief Command for placing doors on the map
 * 
 * This command handles placing door items on tiles in an undoable manner.
 * Doors are typically placed on wall tiles.
 */
class PlaceDoorCommand : public QUndoCommand
{
public:
    enum DoorType {
        NormalDoor,
        LockedDoor,
        QuestDoor,
        MagicDoor,
        LevelDoor
    };

    PlaceDoorCommand(Map* map,
                    const QPointF& tilePos,
                    quint16 doorItemId,
                    DoorType doorType = NormalDoor,
                    quint8 doorId = 0,
                    QUndoCommand* parent = nullptr);
    ~PlaceDoorCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    quint16 doorItemId_;
    DoorType doorType_;
    quint8 doorId_;
    Item* doorInstance_;
    bool tilePreviouslyExisted_;
    quint16 previousDoorItemId_;
    bool hadPreviousDoor_;
};

/**
 * @brief Command for removing doors from the map
 */
class RemoveDoorCommand : public QUndoCommand
{
public:
    RemoveDoorCommand(Map* map,
                     const QPointF& tilePos,
                     Item* door = nullptr,
                     QUndoCommand* parent = nullptr);
    ~RemoveDoorCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    Item* doorInstance_;
    quint16 doorItemId_;
    quint8 doorId_;
    bool doorWasRemoved_;
};

#endif // PLACEDOORCOMMAND_H
