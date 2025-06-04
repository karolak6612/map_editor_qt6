#ifndef SETGROUNDITEMCOMMAND_H
#define SETGROUNDITEMCOMMAND_H

#include <QUndoCommand>
#include <QPointF> // For tile coordinates
#include <QString> // For command text

// Forward declarations
class Map;   // Assuming 'Map' is the class managing tiles
class Item;  // Assuming 'Item' is the class for map items

class SetGroundItemCommand : public QUndoCommand {
public:
    SetGroundItemCommand(Map* map,
                         const QPointF& tilePos,
                         quint16 newGroundItemId, // 0 means remove ground
                         QUndoCommand* parent = nullptr);
    ~SetGroundItemCommand() override;

    void undo() override;
    void redo() override;

    // Optional: for merging commands if needed, though not used by GroundBrush directly for now
    // bool mergeWith(const QUndoCommand *other) override;
    // int id() const override;

private:
    Map* map_;
    QPointF tilePos_;
    quint16 newGroundItemId_; // The ID of the new ground item to place. 0 means remove.

    quint16 oldGroundItemId_ = 0; // ID of the ground item that was there before
    bool    hadOldGround_ = false;  // True if there was any ground item before this command
    bool    firstRedo_ = true;      // Task 016: Flag to capture old state only once
    // Item* oldGroundItem_ = nullptr; // Alternative: store the Item pointer. Simpler to store ID for now.
};

#endif // SETGROUNDITEMCOMMAND_H
