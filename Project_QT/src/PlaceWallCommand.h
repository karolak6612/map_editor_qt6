#ifndef PLACEWALLCOMMAND_H
#define PLACEWALLCOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QList> // For storing multiple old items if necessary
#include <QString> // For command text

// Forward declarations
class Map;
class Tile;
class Item;

class PlaceWallCommand : public QUndoCommand {
public:
    PlaceWallCommand(Map* map,
                     const QPointF& tilePos,
                     quint16 wallItemId, // ID of the wall item to place
                     QUndoCommand* parent = nullptr);
    ~PlaceWallCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_;
    quint16 newWallItemId_;

    // Store information about the state before the command
    struct OldWallState {
        quint16 itemId;
        // Potentially other attributes if needed for perfect restoration
    };
    QList<OldWallState> oldWallStates_;
    bool tilePreviouslyExisted_ = false;
};

#endif // PLACEWALLCOMMAND_H
