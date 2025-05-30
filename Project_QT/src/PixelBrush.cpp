#include "PixelBrush.h"
#include "SetTileColorCommand.h" // The command this brush creates
#include "Map.h"                 // For interacting with the map
#include "Tile.h"                // For interacting with tiles
#include "MapView.h"             // For context if needed
#include <QMouseEvent>
#include <QUndoStack>
#include <QDebug>

PixelBrush::PixelBrush(QObject *parent) : Brush(parent) {}

PixelBrush::~PixelBrush() {}

QString PixelBrush::name() const {
    return "Pixel Brush";
}

int PixelBrush::getBrushSize() const {
    // For a pixel brush, size is 0 (acts on a single tile identified by mapPos)
    // The MapViewInputHandler::getAffectedTiles will use this.
    // If this brush were to draw an area itself, it would use this size.
    return 0;
}

BrushShape PixelBrush::getBrushShape() const {
    return BrushShape::Square; // Shape doesn't matter much for size 0
}

QUndoCommand* PixelBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                        Map* map, QUndoStack* undoStack,
                                        bool shiftPressed, bool ctrlPressed, bool altPressed,
                                        QUndoCommand* parentCommand) {
    if (!map) return nullptr;
    qDebug() << "PixelBrush: Mouse press at" << mapPos;

    // Create and return a command to change the tile color.
    // MapViewInputHandler will push this to its currentDrawingCommand_ or directly to undoStack_.
    // If parentCommand is supplied by MapViewInputHandler (for macro), SetTileColorCommand should use it.
    SetTileColorCommand* cmd = new SetTileColorCommand(map, mapPos, drawColor_, parentCommand);
    //setText is handled in command's redo
    return cmd;
}

QUndoCommand* PixelBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                       Map* map, QUndoStack* undoStack,
                                       bool shiftPressed, bool ctrlPressed, bool altPressed,
                                       QUndoCommand* parentCommand) {
    if (!map) return nullptr;
    qDebug() << "PixelBrush: Mouse move at" << mapPos;

    // If drawing on move (smearing), create a command.
    // This assumes MapViewInputHandler calls this for each tile in a multi-tile brush footprint during a drag.
    SetTileColorCommand* cmd = new SetTileColorCommand(map, mapPos, drawColor_, parentCommand);
    return cmd;
}

QUndoCommand* PixelBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                          Map* map, QUndoStack* undoStack,
                                          bool shiftPressed, bool ctrlPressed, bool altPressed,
                                          QUndoCommand* parentCommand) {
    if (!map) return nullptr;
    qDebug() << "PixelBrush: Mouse release at" << mapPos;
    // Typically, for simple brushes, release might not do anything if press/move handled it.
    // Or it could finalize an operation. For PixelBrush, action is on press/move.
    return nullptr;
}

void PixelBrush::cancel() {
    qDebug() << "PixelBrush: Cancelled.";
    // Reset any internal brush state if it had any from a press/drag
}
