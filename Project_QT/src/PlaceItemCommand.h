#ifndef PLACEITEMCOMMAND_H
#define PLACEITEMCOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QString>

// Forward declarations
class Map;
class Item;

/**
 * @brief Command for placing generic items on the map
 * 
 * This command handles placing any type of item on tiles in an undoable manner.
 * Used by RAWBrush and other generic item placement operations.
 */
class PlaceItemCommand : public QUndoCommand
{
public:
    PlaceItemCommand(Map* map,
                    const QPointF& tilePos,
                    quint16 itemId,
                    quint8 count = 1,
                    QUndoCommand* parent = nullptr);
    ~PlaceItemCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    quint16 itemId_;
    quint8 count_;
    Item* itemInstance_;
    bool tilePreviouslyExisted_;
};

/**
 * @brief Command for removing generic items from the map
 */
class RemoveItemCommand : public QUndoCommand
{
public:
    RemoveItemCommand(Map* map,
                     const QPointF& tilePos,
                     quint16 itemId = 0, // 0 means remove any item
                     Item* specificItem = nullptr,
                     QUndoCommand* parent = nullptr);
    ~RemoveItemCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    quint16 itemId_;
    Item* itemInstance_;
    quint8 itemCount_;
    bool itemWasRemoved_;
    int itemStackPosition_;
};

#endif // PLACEITEMCOMMAND_H
