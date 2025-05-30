#include "GroundBrush.h"
#include "Map.h"            // For Map* type hint, though not directly used in stubs
#include "QUndoCommand.h"   // For QUndoCommand* return type
#include <QDebug>
#include <QMouseEvent>      // For event->buttons() and QMouseEvent type
#include <QObject>          // For tr()

// Constructor
GroundBrush::GroundBrush(QObject *parent)
    : TerrainBrush(parent) { // Call base class constructor
    setSpecificName(tr("Ground Brush")); // Set the specific name for this brush type
    // setLookID(SOME_DEFAULT_GROUND_ITEM_ID); // Optionally set a default lookId_
}

// Destructor
GroundBrush::~GroundBrush() {
    // Perform any specific cleanup for GroundBrush if needed
}

// Type identification
Brush::Type GroundBrush::type() const {
    return Brush::Type::Ground;
}

// Core action method stubs
bool GroundBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(tilePos);      // Not used in this very basic check
    Q_UNUSED(drawingContext); // Not used in this very basic check

    if (!map) {
        qWarning() << "GroundBrush::canDraw: Map pointer is null.";
        return false;
    }

    // Basic check: A ground brush can attempt to draw if there's a map.
    // More advanced checks could involve:
    // - if currentGroundItemId_ is valid (not 0) for an apply operation.
    // - if the tile at tilePos is not obstructed for ground placement by other items.
    // - ground_equivalent checks with neighbors (deferred).
    // - if drawingContext provides a specific item, check if it's a valid ground type.

    // This method is more about "can this TYPE of brush operate here in general".
    // The specific configured item ID (currentGroundItemId_) is checked in applyBrush.
    // For removeBrush, it can generally always attempt to act.
    return true;
}

#include "SetGroundItemCommand.h" // Added include

QUndoCommand* GroundBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext); // Not used in this basic implementation

    if (!map) {
        qWarning() << "GroundBrush::applyBrush: Map pointer is null.";
        return nullptr;
    }

    quint16 groundItemIdToPlace = getCurrentGroundItemId();
    // currentGroundItemId_ is a member, accessible via getCurrentGroundItemId()

    if (groundItemIdToPlace == 0) {
        // If currentGroundItemId_ is 0, it could mean "erase ground" or "brush not configured".
        // For applyBrush, it's more likely it means the brush is not configured to place anything specific.
        // The removeBrush method should be used for explicit erasure.
        // So, if groundItemIdToPlace is 0 here, it implies the brush is not properly set up to *place* a ground.
        qWarning() << "GroundBrush::applyBrush: currentGroundItemId_ is 0. Brush may not be configured to place a specific ground. No action taken.";
        return nullptr;
    }

    qDebug() << "GroundBrush::applyBrush: Attempting to place ground ID" << groundItemIdToPlace << "at" << tilePos;
    return new SetGroundItemCommand(map, tilePos, groundItemIdToPlace, parentCommand);
}

QUndoCommand* GroundBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext); // Not typically used for a generic ground removal

    if (!map) {
        qWarning() << "GroundBrush::removeBrush: Map pointer is null.";
        return nullptr;
    }

    // To remove ground, we pass an item ID of 0 to SetGroundItemCommand.
    // The command's redo() will see newGroundItemId_ == 0 and call map->removeGround().
    // Its undo() will restore whatever was there before (captured by its first redo).
    qDebug() << "GroundBrush::removeBrush: Attempting to remove ground at" << tilePos;
    return new SetGroundItemCommand(map, tilePos, 0, parentCommand); // 0 signifies ground removal
}

// Brush geometry methods
int GroundBrush::getBrushSize() const {
    // This should ideally be retrieved from a BrushManager or global settings.
    // For now, returning a default size 0 (single tile).
    // qDebug() << "GroundBrush::getBrushSize() - returning placeholder 0";
    return 0;
}

Brush::BrushShape GroundBrush::getBrushShape() const {
    // This should also come from global settings or BrushManager.
    // qDebug() << "GroundBrush::getBrushShape() - returning placeholder Square";
    return Brush::BrushShape::Square;
}

// --- Methods for managing the ground item ID ---
void GroundBrush::setCurrentGroundItemId(quint16 itemId) {
    currentGroundItemId_ = itemId;
    // Optionally, if the brush's lookId should reflect the item, update it here.
    // This might involve looking up item properties if lookId_ is e.g. a client sprite ID.
    // For now, just setting the ID.
    qDebug() << "GroundBrush: Set currentGroundItemId to" << currentGroundItemId_;
}

quint16 GroundBrush::getCurrentGroundItemId() const {
    return currentGroundItemId_;
}

// Cancel operation
void GroundBrush::cancel() {
    qDebug() << "GroundBrush::cancel called";
    // Reset any internal state if GroundBrush had a multi-step operation.
}

// Mouse event handlers
QUndoCommand* GroundBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                         Map* map, QUndoStack* undoStack,
                                         bool shiftPressed, bool ctrlPressed, bool altPressed,
                                         QUndoCommand* parentCommand) {
    Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(undoStack); // undoStack is for MapViewInputHandler
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (!canDraw(map, mapPos, nullptr /* drawingContext */)) {
        return nullptr;
    }

    if (ctrlPressed) { // Ctrl + Click to erase with this ground brush
        return removeBrush(map, mapPos, nullptr /* drawingContext */, parentCommand);
    } else {
        return applyBrush(map, mapPos, nullptr /* drawingContext */, parentCommand);
    }
}

QUndoCommand* GroundBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                        Map* map, QUndoStack* undoStack,
                                        bool shiftPressed, bool ctrlPressed, bool altPressed,
                                        QUndoCommand* parentCommand) {
    Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (event->buttons() & Qt::LeftButton) { // Only draw if left button is held (dragging)
        if (!canDraw(map, mapPos, nullptr /* drawingContext */)) {
            return nullptr;
        }
        if (ctrlPressed) {
            return removeBrush(map, mapPos, nullptr /* drawingContext */, parentCommand);
        } else {
            return applyBrush(map, mapPos, nullptr /* drawingContext */, parentCommand);
        }
    }
    return nullptr;
}

QUndoCommand* GroundBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                           Map* map, QUndoStack* undoStack,
                                           bool shiftPressed, bool ctrlPressed, bool altPressed,
                                           QUndoCommand* parentCommand) {
    Q_UNUSED(mapPos); Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(map); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(ctrlPressed); Q_UNUSED(altPressed); Q_UNUSED(parentCommand);
    // For simple ground brushes, actions usually happen on press or move.
    // Release might finalize a complex operation, but not for this basic version.
    // qDebug() << "GroundBrush::mouseReleaseEvent at" << mapPos;
    return nullptr;
}

// Convenience method
bool GroundBrush::isGround() const {
    return true; // This is, indeed, a GroundBrush.
}
