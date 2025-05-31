#ifndef SETTILECOLORCOMMAND_H
#define SETTILECOLORCOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QColor>

// Forward declarations
class Map; // Assuming 'Map' is the class managing tiles
class Tile; // Assuming 'Tile' is the class for individual map tiles

class SetTileColorCommand : public QUndoCommand {
public:
    SetTileColorCommand(Map* map, const QPointF& tilePos, const QColor& newColor, QUndoCommand* parent = nullptr);
    ~SetTileColorCommand() override;

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QPointF tilePos_; // Using QPointF for map coordinates as used in Brush/MapViewInputHandler
    QColor oldColor_;
    QColor newColor_;
    bool firstRedo_ = true; // To capture oldColor_ only on the first redo
};

#endif // SETTILECOLORCOMMAND_H
