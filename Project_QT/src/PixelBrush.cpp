#include "PixelBrush.h"
#include "SetTileColorCommand.h" // The command this brush creates
#include "Map.h"                 // For interacting with the map
#include "Tile.h"                // For interacting with tiles // Assuming Tile.h exists and is needed for SetTileColorCommand context
#include "MapView.h"             // For context if needed
#include <QMouseEvent>
#include <QUndoStack> // For QUndoCommand context
#include <QDebug>

PixelBrush::PixelBrush(QObject *parent) : Brush(parent), drawColor_(Qt::black) {}

PixelBrush::PixelBrush(const QColor& color, QObject *parent) : Brush(parent), drawColor_(color) {}

PixelBrush::~PixelBrush() {}

QString PixelBrush::name() const {
    return "Pixel Brush";
}

int PixelBrush::getBrushSize() const {
    return 0;
}

BrushShape PixelBrush::getBrushShape() const {
    return BrushShape::Square;
}

Brush::Type PixelBrush::type() const {
    return Brush::Type::Pixel;
}

int PixelBrush::getLookID() const {
    return 0; // Pixel brush doesn't have a specific look ID
}

bool PixelBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(drawingContext);
    if (!map) return false;
    // PixelBrush can draw on any tile, as it just changes a debug property.
    // A more complex brush would check: map->getTile(tilePos) != nullptr, etc.
    return true;
}

QUndoCommand* PixelBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    if (!map) return nullptr;
    qDebug() << "PixelBrush: Applying brush at" << tilePos << "with color" << drawColor_;
    // The SetTileColorCommand will interact with the Tile object via the Map pointer.
    return new SetTileColorCommand(map, tilePos, drawColor_, parentCommand);
}

QUndoCommand* PixelBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    if (!map) return nullptr;
    QColor defaultColor = Qt::transparent; // Or a specific "erased" color like Qt::black or a color from the tile's original state
    qDebug() << "PixelBrush: Removing brush (setting to transparent) at" << tilePos;
    return new SetTileColorCommand(map, tilePos, defaultColor, parentCommand);
}

QUndoCommand* PixelBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                        Map* map, QUndoStack* undoStack,
                                        bool shiftPressed, bool ctrlPressed, bool altPressed,
                                        QUndoCommand* parentCommand) {
    Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (!canDraw(map, mapPos, nullptr /*drawingContext*/)) {
        return nullptr;
    }

    if (ctrlPressed) { // Ctrl + Click to erase with this brush
        return removeBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
    } else {
        return applyBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
    }
}

QUndoCommand* PixelBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                       Map* map, QUndoStack* undoStack,
                                       bool shiftPressed, bool ctrlPressed, bool altPressed,
                                       QUndoCommand* parentCommand) {
    Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    // Only apply if left button is pressed (MapViewInputHandler should manage this by only calling if dragging)
    if (event->buttons() & Qt::LeftButton) {
        if (!canDraw(map, mapPos, nullptr /*drawingContext*/)) {
            return nullptr;
        }
        if (ctrlPressed) { // Erasing on drag with Ctrl
            return removeBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
        } else { // Drawing on drag
            return applyBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
        }
    }
    return nullptr;
}

QUndoCommand* PixelBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                          Map* map, QUndoStack* undoStack,
                                          bool shiftPressed, bool ctrlPressed, bool altPressed,
                                          QUndoCommand* parentCommand) {
    Q_UNUSED(mapPos); Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(map); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(ctrlPressed); Q_UNUSED(altPressed); Q_UNUSED(parentCommand);
    // qDebug() << "PixelBrush: Mouse release at" << mapPos; // Reduced verbosity
    return nullptr;
}

void PixelBrush::cancel() {
    qDebug() << "PixelBrush: Cancelled.";
    // No specific state to reset for this simple brush.
}
