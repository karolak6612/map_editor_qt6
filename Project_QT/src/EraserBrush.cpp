#include "EraserBrush.h"
#include "Map.h"
#include "Tile.h"
#include "EraseCommand.h"
#include <QDebug>
#include <QMouseEvent>
#include <QUndoCommand>

EraserBrush::EraserBrush(QObject *parent) : Brush(parent) {
}

// Brush type identification
Brush::Type EraserBrush::type() const {
    return Type::Eraser;
}

QString EraserBrush::name() const {
    return QStringLiteral("Eraser Brush");
}

bool EraserBrush::isEraser() const {
    return true;
}

EraserBrush* EraserBrush::asEraser() {
    return this;
}

const EraserBrush* EraserBrush::asEraser() const {
    return this;
}

// Brush properties
int EraserBrush::getBrushSize() const {
    return brushSize_;
}

Brush::BrushShape EraserBrush::getBrushShape() const {
    return brushShape_;
}

bool EraserBrush::needBorders() const {
    return true; // Matching wxWidgets EraserBrush behavior
}

bool EraserBrush::canDrag() const {
    return true; // Matching wxWidgets EraserBrush behavior
}

bool EraserBrush::canSmear() const {
    return true; // Allow continuous erasing on mouse move
}

int EraserBrush::getLookID() const {
    return 0; // TODO: Define EDITOR_SPRITE_ERASER constant
}

// Core brush action interface (stubs for now)
bool EraserBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(map)
    Q_UNUSED(tilePos)
    Q_UNUSED(drawingContext)
    return true; // Eraser can always "draw" (erase)
}

QUndoCommand* EraserBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);

    if (!map) {
        return nullptr;
    }

    // Check if there's a tile at this position
    Tile* tile = map->getTile(tilePos);
    if (!tile || tile->isEmpty()) {
        qDebug() << "EraserBrush::applyBrush - No tile or empty tile at" << tilePos;
        return nullptr;
    }

    // Create and return erase command
    EraseCommand* command = new EraseCommand(
        map,
        tilePos,
        EraseCommand::EraseAll, // Default to erasing all items
        parentCommand
    );

    qDebug() << "EraserBrush::applyBrush creating EraseCommand at" << tilePos;
    return command;
}

QUndoCommand* EraserBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(map);
    Q_UNUSED(tilePos);
    Q_UNUSED(drawingContext);
    Q_UNUSED(parentCommand);

    // For EraserBrush, removeBrush doesn't make much sense conceptually
    // since erasing is the primary action. We could implement selective undo
    // or just return nullptr to indicate no action.
    qDebug() << "EraserBrush::removeBrush at" << tilePos << "- no action (eraser doesn't 'remove')";
    return nullptr;
}

// Mouse event handlers with proper signatures
QUndoCommand* EraserBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                          Map* map, QUndoStack* undoStack,
                                          bool shiftPressed, bool ctrlPressed, bool altPressed,
                                          QUndoCommand* parentCommand) {
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)

    qDebug() << "EraserBrush::mousePressEvent at" << mapPos
             << "button:" << event->button()
             << "shift:" << shiftPressed
             << "ctrl:" << ctrlPressed
             << "alt:" << altPressed;

    // Placeholder: Identify target tile(s) based on brush size and shape
    qDebug() << "Target tile(s): [" << static_cast<int>(mapPos.x()) << "," << static_cast<int>(mapPos.y()) << "]";
    qDebug() << "Brush size:" << getBrushSize() << "shape:" << static_cast<int>(getBrushShape());

    // TODO: Create actual erase command when Map/Tile system is ready
    return applyBrush(map, mapPos, nullptr, parentCommand);
}

QUndoCommand* EraserBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                         Map* map, QUndoStack* undoStack,
                                         bool shiftPressed, bool ctrlPressed, bool altPressed,
                                         QUndoCommand* parentCommand) {
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)

    if (event->buttons() != Qt::NoButton && canSmear()) {
        qDebug() << "EraserBrush::mouseMoveEvent at" << mapPos
                 << "shift:" << shiftPressed
                 << "ctrl:" << ctrlPressed
                 << "alt:" << altPressed;

        // Placeholder: Continuous erasing
        qDebug() << "Continuous erasing at tile: [" << static_cast<int>(mapPos.x()) << "," << static_cast<int>(mapPos.y()) << "]";

        // TODO: Create actual erase command when Map/Tile system is ready
        return applyBrush(map, mapPos, nullptr, parentCommand);
    }

    return nullptr;
}

QUndoCommand* EraserBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                            Map* map, QUndoStack* undoStack,
                                            bool shiftPressed, bool ctrlPressed, bool altPressed,
                                            QUndoCommand* parentCommand) {
    Q_UNUSED(map)
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)

    qDebug() << "EraserBrush::mouseReleaseEvent at" << mapPos
             << "button:" << event->button()
             << "shift:" << shiftPressed
             << "ctrl:" << ctrlPressed
             << "alt:" << altPressed;

    // Placeholder: Finalize erasing operation
    qDebug() << "Finalizing erase operation";

    return nullptr; // No additional command needed on release for eraser
}

void EraserBrush::cancel() {
    qDebug() << "EraserBrush::cancel - Canceling ongoing erase operation";
    // TODO: Implement cancellation logic when needed
}
