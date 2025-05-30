#include "MapViewInputHandler.h"
#include "MapView.h"
#include "BrushManager.h"
#include "Brush.h"
#include "Map.h" // Assuming Map.h is the correct header for your map data class
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QFocusEvent>
#include <QCursor>
#include <QGuiApplication> // For QGuiApplication::queryKeyboardModifiers()
#include <QDebug>

MapViewInputHandler::MapViewInputHandler(MapView* mapView,
                                         BrushManager* brushManager,
                                         Map* map,
                                         QUndoStack* undoStack,
                                         QObject *parent)
    : QObject(parent),
      mapView_(mapView),
      brushManager_(brushManager),
      map_(map),
      undoStack_(undoStack),
      currentMode_(InteractionMode::Idle),
      pressedButton_(Qt::NoButton),
      shiftModifierActive_(false),
      ctrlModifierActive_(false),
      altModifierActive_(false) {
    Q_ASSERT(mapView_);
    Q_ASSERT(brushManager_);
    Q_ASSERT(map_);
    Q_ASSERT(undoStack_);
}

MapViewInputHandler::~MapViewInputHandler() {
    // Non-owning pointers, so no explicit deletion here
}

void MapViewInputHandler::updateModifierKeys(QInputEvent* event) {
    if (!event) { // Should not happen if called from an event handler
        qWarning("MapViewInputHandler::updateModifierKeys called with null event");
        return;
    }
    Qt::KeyboardModifiers modifiers = event->modifiers();
    shiftModifierActive_ = modifiers.testFlag(Qt::ShiftModifier);
    ctrlModifierActive_ = modifiers.testFlag(Qt::ControlModifier);
    altModifierActive_ = modifiers.testFlag(Qt::AltModifier);
}

// Overload for situations where QInputEvent is not directly available (e.g. key release from polling)
// Not strictly needed if all calls pass QInputEvent, but can be a utility.
void MapViewInputHandler::updateModifierKeys() {
    Qt::KeyboardModifiers modifiers = QGuiApplication::queryKeyboardModifiers();
    shiftModifierActive_ = modifiers.testFlag(Qt::ShiftModifier);
    ctrlModifierActive_ = modifiers.testFlag(Qt::ControlModifier);
    altModifierActive_ = modifiers.testFlag(Qt::AltModifier);
}


void MapViewInputHandler::handleMousePressEvent(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_ || !brushManager_) return;
    updateModifierKeys(event);
    pressedButton_ = event->button();

    if (pressedButton_ == Qt::LeftButton) {
        Brush* currentBrush = brushManager_->getCurrentBrush();
        // Consider if ctrl or shift are active, they might modify brush behavior or select a different tool
        // For now, simple: if brush is active, draw. Otherwise, select.
        if (currentBrush != nullptr  && !(event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
            currentMode_ = InteractionMode::Drawing;
            startDrawing(mapPosition, event);
        } else { // No brush, or Ctrl/Shift is pressed (could be for selection tool)
            currentMode_ = InteractionMode::SelectingBox;
            startSelectionBox(mapPosition, event);
        }
    } else if (pressedButton_ == Qt::MiddleButton) {
        currentMode_ = InteractionMode::PanningView;
        startPanning(event);
    } else if (pressedButton_ == Qt::RightButton) {
        // Placeholder for context menu or alternative brush action (e.g., erase if Ctrl is held)
        if (ctrlModifierActive_) {
            // Example: Could be an erase mode or secondary brush function
            // For now, let's treat it like a potential pan or do nothing specific
            // currentMode_ = InteractionMode::Erasing; // If such mode exists
            // startErasing(mapPosition, event);
             qDebug() << "Right mouse button + Ctrl pressed at" << mapPosition;
        } else {
             qDebug() << "Right mouse button pressed at" << mapPosition;
            // Potentially open context menu here: mapView_->showContextMenu(event->globalPos(), mapPosition);
        }
    }
    mapView_->update(); // Request repaint for visual feedback (e.g. cursor change, start of drawing)
}

void MapViewInputHandler::handleMouseMoveEvent(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_) return;
    updateModifierKeys(event);

    switch (currentMode_) {
        case InteractionMode::Drawing:
            continueDrawing(mapPosition, event);
            break;
        case InteractionMode::PanningView:
            continuePanning(event);
            break;
        case InteractionMode::SelectingBox:
            updateSelectionBox(mapPosition, event);
            break;
        case InteractionMode::Idle:
            // mapView_->updateHoverInfo(mapPosition); // Example: update status bar, highlight tile under cursor
            // This can also be handled by MapView itself via setMouseTracking(true)
            break;
        default:
            break;
    }
}

void MapViewInputHandler::handleMouseReleaseEvent(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_) return;

    // It's important to get modifier keys state from the event itself, not query global state,
    // as the global state might have changed if a key was released slightly before the mouse button.
    updateModifierKeys(event);

    if (event->button() == pressedButton_) { // Ensure we're releasing the button that initiated the action
        InteractionMode modeEnded = currentMode_; // Store current mode before resetting
        currentMode_ = InteractionMode::Idle;    // Reset mode first
        pressedButton_ = Qt::NoButton;

        switch (modeEnded) {
            case InteractionMode::Drawing:
                finishDrawing(mapPosition, event);
                break;
            case InteractionMode::PanningView:
                finishPanning(event);
                break;
            case InteractionMode::SelectingBox:
                finishSelectionBox(mapPosition, event);
                break;
            default:
                // This case should ideally not be reached if pressedButton_ was set.
                break;
        }
        // General update might be needed if the action modified the view or model
        mapView_->update();
    }
}

void MapViewInputHandler::handleKeyPressEvent(QKeyEvent* event) {
    if (!mapView_ || !undoStack_) return;
    updateModifierKeys(event); // Update general modifier state

    if (event->key() == Qt::Key_Escape && currentMode_ != InteractionMode::Idle) {
        qDebug() << "Escape pressed, cancelling current operation: " << static_cast<int>(currentMode_);

        // Specific cancellation logic for each mode
        if(currentMode_ == InteractionMode::Drawing) {
             Brush* brush = brushManager_ ? brushManager_->getCurrentBrush() : nullptr;
             if(brush) brush->cancel(); // Assuming Brush has a cancel method
        } else if (currentMode_ == InteractionMode::SelectingBox) {
             mapView_->setSelectionArea(QRectF()); // Clear visual selection box
             // Any selection command in progress should be cancelled/discarded
        } else if (currentMode_ == InteractionMode::PanningView) {
            // Panning usually doesn't have a "cancellable" state beyond stopping.
            // Ensure cursor is reset.
            mapView_->unsetCursor();
        }
        // Add other modes as they come

        currentMode_ = InteractionMode::Idle;
        pressedButton_ = Qt::NoButton; // Reset pressed button state
        mapView_->update(); // Refresh view
        event->accept(); // Indicate event was handled
        return;
    }

    // Handle Undo/Redo
    if (event->matches(QKeySequence::Undo)) {
        undoStack_->undo();
        mapView_->update();
        event->accept();
    } else if (event->matches(QKeySequence::Redo)) {
        undoStack_->redo();
        mapView_->update();
        event->accept();
    }

    // Other shortcuts can be added here
    // e.g., brush selection, tool activation
    // if (event->key() == Qt::Key_B) { brushManager_->selectNextBrush(); }

    if(!event->isAccepted()) {
        // Forward to current brush if it wants to handle key events
        Brush* currentBrush = brushManager_ ? brushManager_->getCurrentBrush() : nullptr;
        if (currentBrush && currentMode_ == InteractionMode::Drawing) { // Or other relevant modes
            // currentBrush->keyPressEvent(event, mapView_);
        }
    }
    // No general mapView_->update() here unless a key press directly changes visual state handled here
}

void MapViewInputHandler::handleKeyReleaseEvent(QKeyEvent* event) {
    if (!mapView_) return;
    updateModifierKeys(event); // Update general modifier state

    // If a key release changes behavior (e.g. releasing Shift for straight lines stops constraining)
    // this is where you might trigger an update or change state.
    // Example: If drawing and shift was active for line drawing, and now released.
    // if (currentMode_ == InteractionMode::Drawing && event->key() == Qt::Key_Shift) {
    //     Brush* brush = brushManager_ ? brushManager_->getCurrentBrush() : nullptr;
    //     if (brush) {
    //         // Notify brush that shift is no longer pressed, it might change its preview
    //         // brush->modifierChanged(shiftModifierActive_, ctrlModifierActive_, altModifierActive_);
    //     }
    // }

    // Forward to current brush if it wants to handle key events
     Brush* currentBrush = brushManager_ ? brushManager_->getCurrentBrush() : nullptr;
     if (currentBrush && currentMode_ == InteractionMode::Drawing) { // Or other relevant modes
         // currentBrush->keyReleaseEvent(event, mapView_);
     }
}

void MapViewInputHandler::handleWheelEvent(QWheelEvent* event, const QPointF& mapPosition) {
    if (!mapView_) return;
    updateModifierKeys(event);

    qreal delta = event->angleDelta().y();
    qreal zoomFactorMultiplier = 1.1; // How much to zoom in/out per step

    if (delta > 0) { // Zoom in
        mapView_->zoom(zoomFactorMultiplier, event->position()); // event->position() is screen coords for zoom center
    } else if (delta < 0) { // Zoom out
        mapView_->zoom(1.0 / zoomFactorMultiplier, event->position());
    }
    event->accept(); // Indicate event was handled
    // mapView_->update() should be called by mapView_->zoom()
}

void MapViewInputHandler::handleFocusOutEvent(QFocusEvent* event) {
    if (!mapView_) return;
    qDebug() << "MapViewInputHandler: Focus Out event received.";
    
    // If an operation was in progress, it's often safest to cancel it
    if (currentMode_ != InteractionMode::Idle) {
        qDebug() << "Focus lost during an operation, cancelling mode:" << static_cast<int>(currentMode_);

        if(currentMode_ == InteractionMode::Drawing) {
             Brush* brush = brushManager_ ? brushManager_->getCurrentBrush() : nullptr;
             if(brush) brush->cancel(); // Assuming Brush has a cancel method
        } else if (currentMode_ == InteractionMode::SelectingBox) {
             mapView_->setSelectionArea(QRectF()); // Clear visual selection box
        } else if (currentMode_ == InteractionMode::PanningView) {
            // Panning might just stop, ensure cursor is reset.
            mapView_->unsetCursor();
        }
        // Add other modes as they are developed

        currentMode_ = InteractionMode::Idle;
        pressedButton_ = Qt::NoButton;
        mapView_->update(); // Refresh view to remove any visual artifacts
    }

    // Reset all modifier key states as they are unreliable when the widget/window doesn't have focus
    shiftModifierActive_ = false;
    ctrlModifierActive_ = false;
    altModifierActive_ = false;
}


// --- Helper Methods Implementation ---

void MapViewInputHandler::startDrawing(const QPointF& mapPos, QMouseEvent* event) {
    if (!brushManager_ || !mapView_ || !map_ || !undoStack_) return;
    Brush* brush = brushManager_->getCurrentBrush();
    if (brush) {
        // Pass all relevant state to the brush's event handler
        brush->mousePress(mapPos, event, mapView_, map_, undoStack_,
                          shiftModifierActive_, ctrlModifierActive_, altModifierActive_);
    }
    dragStartMapPos_ = mapPos; // Used by some brushes for line/rect drawing, or by this handler
    mapView_->update(); // Ensure view updates if brush made changes or shows preview
}

void MapViewInputHandler::continueDrawing(const QPointF& mapPos, QMouseEvent* event) {
    if (!brushManager_ || !mapView_ || !map_ || !undoStack_) return;
    Brush* brush = brushManager_->getCurrentBrush();
    if (brush) {
        brush->mouseMove(mapPos, event, mapView_, map_, undoStack_,
                         shiftModifierActive_, ctrlModifierActive_, altModifierActive_);
    }
    mapView_->update(); // Brush might be continuously drawing or updating a preview
}

void MapViewInputHandler::finishDrawing(const QPointF& mapPos, QMouseEvent* event) {
    if (!brushManager_ || !mapView_ || !map_ || !undoStack_) return;
    Brush* brush = brushManager_->getCurrentBrush();
    if (brush) {
        brush->mouseRelease(mapPos, event, mapView_, map_, undoStack_,
                            shiftModifierActive_, ctrlModifierActive_, altModifierActive_);
    }
    mapView_->update(); // Final update for the drawing operation
}

void MapViewInputHandler::startPanning(QMouseEvent* event) {
    if (!mapView_) return;
    lastMouseScreenPos_ = event->pos(); // QMouseEvent::pos() is widget-local screen coordinates
    mapView_->setCursor(Qt::ClosedHandCursor);
}

void MapViewInputHandler::continuePanning(QMouseEvent* event) {
    if (!mapView_) return;
    QPoint currentScreenPos = event->pos();
    QPoint delta = currentScreenPos - lastMouseScreenPos_;
    mapView_->pan(delta.x(), delta.y());
    lastMouseScreenPos_ = currentScreenPos;
    // mapView_->update() is typically called by pan() if changes occur
}

void MapViewInputHandler::finishPanning(QMouseEvent* event) {
    if (!mapView_) return;
    mapView_->unsetCursor(); // Reset cursor to default
}

void MapViewInputHandler::startSelectionBox(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_) return;
    dragStartMapPos_ = mapPos;
    // Initialize selection area in MapView for visual feedback
    mapView_->setSelectionArea(QRectF(dragStartMapPos_, dragStartMapPos_));
    mapView_->update();
}

void MapViewInputHandler::updateSelectionBox(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_) return;
    QRectF selectionRect = QRectF(dragStartMapPos_, mapPos).normalized();
    mapView_->setSelectionArea(selectionRect); // Update visual selection area
    mapView_->update();
}

void MapViewInputHandler::finishSelectionBox(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !map_ || !undoStack_) return;
    QRectF selectionRect = QRectF(dragStartMapPos_, mapPos).normalized();

    mapView_->setSelectionArea(QRectF()); // Clear the temporary visual selection box from MapView

    // The actual selection logic should be handled by the Map class or a selection manager.
    // This might involve creating a QUndoCommand for the selection change.
    if (map_) { // Ensure map_ is valid
        map_->selectItemsInRect(selectionRect, shiftModifierActive_, ctrlModifierActive_, undoStack_);
    }

    mapView_->update(); // Update MapView to reflect any changes in selection state
}
