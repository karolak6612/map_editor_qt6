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

    if (groundItemIdToPlace == 0) {
        qWarning() << "GroundBrush::applyBrush: currentGroundItemId_ is 0. Brush may not be configured to place a specific ground. No action taken.";
        return nullptr;
    }

    qDebug() << "GroundBrush::applyBrush: Attempting to place ground ID" << groundItemIdToPlace << "at" << tilePos;
    return new SetGroundItemCommand(map, tilePos, groundItemIdToPlace, parentCommand);
}

QUndoCommand* GroundBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);

    if (!map) {
        qWarning() << "GroundBrush::removeBrush: Map pointer is null.";
        return nullptr;
    }
    qDebug() << "GroundBrush::removeBrush: Attempting to remove ground at" << tilePos;
    return new SetGroundItemCommand(map, tilePos, 0, parentCommand); // 0 signifies ground removal
}

// Brush geometry methods
int GroundBrush::getBrushSize() const {
    return 0;
}

Brush::BrushShape GroundBrush::getBrushShape() const {
    return Brush::BrushShape::Square;
}

// --- Methods for managing the ground item ID ---
void GroundBrush::setCurrentGroundItemId(quint16 itemId) {
    currentGroundItemId_ = itemId;
    qDebug() << "GroundBrush: Set currentGroundItemId to" << currentGroundItemId_;
}

quint16 GroundBrush::getCurrentGroundItemId() const {
    return currentGroundItemId_;
}

// Optional border support
bool GroundBrush::hasOptionalBorder() const {
    // Placeholder implementation.
    // Actual logic would depend on how a ground brush type is defined
    // to support optional borders (e.g., loaded from XML attribute,
    // or based on its name/ID).
    // For now, let's assume no ground brush supports it by default,
    // unless a specific brush type overrides this or sets a member flag.
    // Example: return m_supportsOptionalBorder;
    // Example: if (name().contains("mountain", Qt::CaseInsensitive)) return true;
    return false;
}

// Cancel operation
void GroundBrush::cancel() {
    qDebug() << "GroundBrush::cancel called";
}

// Mouse event handlers
QUndoCommand* GroundBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                         Map* map, QUndoStack* undoStack,
                                         bool shiftPressed, bool ctrlPressed, bool altPressed,
                                         QUndoCommand* parentCommand) {
    Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (!canDraw(map, mapPos, nullptr )) {
        return nullptr;
    }

    if (ctrlPressed) {
        return removeBrush(map, mapPos, nullptr , parentCommand);
    } else {
        return applyBrush(map, mapPos, nullptr , parentCommand);
    }
}

QUndoCommand* GroundBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                        Map* map, QUndoStack* undoStack,
                                        bool shiftPressed, bool ctrlPressed, bool altPressed,
                                        QUndoCommand* parentCommand) {
    Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (event->buttons() & Qt::LeftButton) {
        if (!canDraw(map, mapPos, nullptr )) {
            return nullptr;
        }
        if (ctrlPressed) {
            return removeBrush(map, mapPos, nullptr , parentCommand);
        } else {
            return applyBrush(map, mapPos, nullptr , parentCommand);
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
    return nullptr;
}

// Convenience method
bool GroundBrush::isGround() const {
    return true;
}
