#include "WallBrush.h"
#include "PlaceWallCommand.h" // For creating wall placement/removal commands
#include "Map.h"              // For Map* type hint
#include "Tile.h"             // For Tile* type hint (though not directly used here)
#include "QUndoCommand.h"     // For QUndoCommand* return type
#include <QMouseEvent>        // For QMouseEvent type
#include <QDebug>
#include <QObject>            // For tr()

WallBrush::WallBrush(QObject* parent)
    : TerrainBrush(parent),
      currentWallItemId_(0) {
    setSpecificName(tr("Wall Brush"));
    // Optionally, set a default lookId_ or currentWallItemId_ if there's a common default wall
    // e.g., setCurrentWallItemId(SOME_DEFAULT_WALL_ID);
    //      setLookID(SOME_DEFAULT_WALL_ID);
}

WallBrush::~WallBrush() {
}

// --- Overrides from Brush/TerrainBrush ---
Brush::Type WallBrush::type() const {
    return Brush::Type::Wall;
}

bool WallBrush::isWall() const {
    // This brush is specifically for walls.
    return true;
}

bool WallBrush::canSmear() const {
    // wxWallBrush returned false, meaning it typically doesn't "smear" like terrain.
    // Actions usually happen on distinct clicks or drags that define segments.
    return false;
}

bool WallBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(tilePos);      // Not used in this very basic check for now
    Q_UNUSED(drawingContext); // Not used in this very basic check for now

    if (!map) {
        qWarning() << "WallBrush::canDraw: Map pointer is null.";
        return false;
    }
    // For placing a wall (applyBrush): canDraw is true if an item ID is configured.
    // For removing a wall (removeBrush): canDraw is generally always true.
    // This method is usually checked before an "apply" action.
    // If currentWallItemId_ is 0, this brush isn't configured to place a specific wall.
    return currentWallItemId_ != 0;
}

QUndoCommand* WallBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    if (!map) {
        qWarning() << "WallBrush::applyBrush: Map pointer is null.";
        return nullptr;
    }
    if (currentWallItemId_ == 0) {
        qWarning() << "WallBrush::applyBrush: No currentWallItemId_ set. Brush not configured to place a wall.";
        return nullptr;
    }
    qDebug() << "WallBrush: Applying wall ID" << currentWallItemId_ << "at" << tilePos;
    return new PlaceWallCommand(map, tilePos, currentWallItemId_, parentCommand);
}

QUndoCommand* WallBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    if (!map) {
        qWarning() << "WallBrush::removeBrush: Map pointer is null.";
        return nullptr;
    }
    // Passing 0 to PlaceWallCommand signifies removal of walls of this type.
    // The command needs to be smart enough to identify what "type" of wall to remove,
    // or if it should remove any wall item.
    // For now, PlaceWallCommand with ID 0 is assumed to clear all wall items.
    qDebug() << "WallBrush: Removing wall at" << tilePos;
    return new PlaceWallCommand(map, tilePos, 0, parentCommand); // 0 signifies wall removal
}

int WallBrush::getBrushSize() const {
    // Walls are typically placed one tile at a time by default (size 0).
    // Larger "wall brush sizes" for drawing lines/rectangles of walls would be handled by
    // MapViewInputHandler's getAffectedTiles and iterating calls to applyBrush.
    return 0;
}

Brush::BrushShape WallBrush::getBrushShape() const {
    // Shape is not particularly relevant if size is 0.
    return Brush::BrushShape::Square;
}

QUndoCommand* WallBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                         Map* map, QUndoStack* undoStack,
                                         bool shiftPressed, bool ctrlPressed, bool altPressed,
                                         QUndoCommand* parentCommand) {
    Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (ctrlPressed) {
        // Ctrl+Click with Wall Brush attempts to remove a wall.
        // No canDraw check here for removal, as removeBrush might be able to act even if currentWallItemId_ is 0.
        return removeBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
    } else {
        // Normal click attempts to place a wall.
        if (!canDraw(map, mapPos, nullptr /*drawingContext*/)) {
            return nullptr;
        }
        return applyBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
    }
}

QUndoCommand* WallBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                     Map* map, QUndoStack* undoStack,
                                     bool shiftPressed, bool ctrlPressed, bool altPressed,
                                     QUndoCommand* parentCommand) {
    Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (event->buttons() & Qt::LeftButton) { // If dragging with left button
        if (ctrlPressed) { // Erasing on drag with Ctrl
             return removeBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
        } else { // Drawing on drag
            if (!canDraw(map, mapPos, nullptr /*drawingContext*/)) {
                return nullptr;
            }
            return applyBrush(map, mapPos, nullptr /*drawingContext*/, parentCommand);
        }
    }
    return nullptr;
}

QUndoCommand* WallBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                        Map* map, QUndoStack* undoStack,
                                        bool shiftPressed, bool ctrlPressed, bool altPressed,
                                        QUndoCommand* parentCommand) {
    Q_UNUSED(mapPos); Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(map); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(ctrlPressed); Q_UNUSED(altPressed); Q_UNUSED(parentCommand);
    // Most wall drawing actions happen on press or move (for dragging lines).
    // Release typically doesn't do anything further for simple wall brushes.
    return nullptr;
}

void WallBrush::cancel() {
    qDebug() << "WallBrush::cancel called";
    // Reset any internal state specific to WallBrush if it had a multi-step operation.
    // For this basic version, no specific state to reset.
}

// --- WallBrush specific methods ---
void WallBrush::setCurrentWallItemId(quint16 itemId) {
    currentWallItemId_ = itemId;
    // Potentially update lookId_ from TerrainBrush if the brush icon should change:
    // if (itemId != 0) setLookID(itemId);
    // else setLookID(DEFAULT_WALL_BRUSH_ICON_ID_OR_SIMILAR);
}

quint16 WallBrush::getCurrentWallItemId() const {
    return currentWallItemId_;
}
