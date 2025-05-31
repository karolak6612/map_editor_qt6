#ifndef PLACEDECORATIONCOMMAND_H
#define PLACEDECORATIONCOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QString> // For command text

// Forward declarations
class Map;
class Tile;
class Item; // For the item being placed/removed

class PlaceDecorationCommand : public QUndoCommand {
public:
    PlaceDecorationCommand(Map* map,
                           const QPointF& tilePos,
                           quint16 decorationItemId, // ID of the decoration item.
                           QUndoCommand* parent = nullptr);
    ~PlaceDecorationCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    quint16 decorationItemId_; // The ID of the decoration item to place.

    // Store information about the state before/after the command
    Item* itemInstance_ = nullptr; // Stores the actual item instance created/manipulated by this command
                                   // On redo, this is the item added.
                                   // On undo, this item is removed from tile and this pointer is used to delete it if not re-added.
    bool tilePreviouslyExisted_ = false;
    // Note: If a decoration replaces another, old state would be more complex.
    // This command assumes adding a new decoration, and undo removes that specific one.
};

#endif // PLACEDECORATIONCOMMAND_H
