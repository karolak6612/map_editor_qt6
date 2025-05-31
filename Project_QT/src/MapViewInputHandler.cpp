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
#include <QUndoStack>   // Added
#include <QUndoCommand> // Added
#include <QtMath>       // For qSqrt

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
    dragStartMapPos_ = mapPosition; // Store for all press events

    if (pressedButton_ == Qt::LeftButton) {
        Brush* currentBrush = brushManager_->getCurrentBrush();
        if (currentBrush) { // Drawing mode
            currentMode_ = InteractionMode::Drawing; // Default drawing mode
            isDraggingDraw_ = currentBrush->canDrag() && shiftModifierActive_;
            isReplaceDragging_ = currentBrush->isGround() && altModifierActive_; // Assuming isGround() exists

            if (isDraggingDraw_) {
                currentMode_ = InteractionMode::DraggingDraw;
                qDebug() << "MapViewInputHandler: Starting DraggingDraw";
            }
            if (isReplaceDragging_) {
                // Additional setup for replace dragging might be needed here,
                // e.g., getting the tile/brush to replace from mapPosition.
                // For now, just setting the flag. The brush will handle the logic.
                qDebug() << "MapViewInputHandler: Starting ReplaceDragging";
            }
            startDrawing(mapPosition, event);

        } else { // Selection mode (no active brush)
            // TODO: Implement isPasting() check and finishPasting() call
            // For now, directly to selection:
            bool clearPreviousSelection = !ctrlModifierActive_; // Standard behavior: Ctrl to add to selection
            if (shiftModifierActive_ || ctrlModifierActive_) { // Shift or Ctrl for box selection modifications
                currentMode_ = InteractionMode::SelectingBox;
                // Pass clearPreviousSelection to startSelectionBox if it needs to handle it
                // map_->getSelection()->setClearPrevious(clearPreviousSelection); // Example
                qDebug() << "MapViewInputHandler: Starting SelectingBox (Shift/Ctrl). Clear prev:" << clearPreviousSelection;
                startSelectionBox(mapPosition, event);
            } else { // No modifiers, standard selection
                // TODO: Check if clicking on an existing selection to DraggingSelection
                // For now, default to starting a new selection box or single click selection.
                // map_->getSelection()->setClearPrevious(true); // Example
                qDebug() << "MapViewInputHandler: Starting SelectingBox (no modifier).";
                currentMode_ = InteractionMode::SelectingBox;
                startSelectionBox(mapPosition, event);
            }
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
        case InteractionMode::DraggingDraw: // DraggingDraw also calls continueDrawing
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
        InteractionMode modeEnded = currentMode_;

        // Reset mode and flags before calling finish handlers,
        // as finish handlers might set new states (e.g. after pasting).
        currentMode_ = InteractionMode::Idle;
        pressedButton_ = Qt::NoButton;
        isDraggingDraw_ = false;
        isReplaceDragging_ = false;
        // currentDrawingCommand_ handling will be done by drawing helpers if used.

        switch (modeEnded) {
            case InteractionMode::Drawing:
            case InteractionMode::DraggingDraw:
                finishDrawing(mapPosition, event);
                break;
            case InteractionMode::PanningView:
                finishPanning(event);
                break;
            case InteractionMode::SelectingBox:
                finishSelectionBox(mapPosition, event);
                break;
            // TODO: Handle InteractionMode::Pasting, InteractionMode::DraggingSelection
            default:
                break;
        }
        mapView_->update(); // General update after action is finished
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

QList<QPointF> MapViewInputHandler::getAffectedTiles(const QPointF& primaryMapPos, Brush* currentBrush) const {
    QList<QPointF> affectedTiles;
    if (!currentBrush) {
        qWarning("getAffectedTiles: No current brush provided.");
        affectedTiles.append(primaryMapPos);
        return affectedTiles;
    }

    int brushSize = currentBrush->getBrushSize();
    BrushShape brushShape = currentBrush->getBrushShape();

    if (brushShape == BrushShape::Square) {
        for (int y_offset = -brushSize; y_offset <= brushSize; ++y_offset) {
            for (int x_offset = -brushSize; x_offset <= brushSize; ++x_offset) {
                affectedTiles.append(QPointF(primaryMapPos.x() + x_offset, primaryMapPos.y() + y_offset));
            }
        }
    } else if (brushShape == BrushShape::Circle) {
        for (int y_offset = -brushSize; y_offset <= brushSize; ++y_offset) {
            for (int x_offset = -brushSize; x_offset <= brushSize; ++x_offset) {
                double distance = qSqrt(static_cast<double>(x_offset * x_offset + y_offset * y_offset));
                if (distance <= static_cast<double>(brushSize) + 0.5) { // +0.5 for better inclusivity of edge tiles
                    affectedTiles.append(QPointF(primaryMapPos.x() + x_offset, primaryMapPos.y() + y_offset));
                }
            }
        }
    } else {
        qWarning("getAffectedTiles: Unknown brush shape! Defaulting to primary tile.");
        affectedTiles.append(primaryMapPos);
    }

    if (affectedTiles.isEmpty()) { // Ensure at least the primary tile
        affectedTiles.append(primaryMapPos);
    }

    return affectedTiles;
}

void MapViewInputHandler::startDrawing(const QPointF& mapPos, QMouseEvent* event) {
    if (!brushManager_ || !mapView_ || !map_ || !undoStack_) return;
    currentDrawingCommand_ = nullptr; // Reset at the beginning of a new potential stroke.

    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (currentBrush) {
        QList<QPointF> tiles = getAffectedTiles(mapPos, currentBrush);
        for (const QPointF& tilePos : tiles) {
            QUndoCommand* cmd = currentBrush->mousePressEvent(tilePos, event, mapView_, map_, undoStack_,
                                                             shiftModifierActive_, ctrlModifierActive_, altModifierActive_,
                                                             nullptr); // parentCommand is null for press
            if (cmd) {
                if (!currentDrawingCommand_) {
                    currentDrawingCommand_ = cmd; // First command becomes the main command for the stroke
                } else {
                    // If multiple commands are generated by a single press (e.g. large brush),
                    // this simplified logic pushes subsequent ones directly.
                    // Ideally, the brush's mousePressEvent for a large brush would return ONE command
                    // that encompasses all initial changes. Or, a macro would be built here.
                    undoStack_->push(cmd);
                }
            }
        }
    }
    // dragStartMapPos_ is already set in handleMousePressEvent
    mapView_->update(); // Ensure view updates if brush made changes or shows preview
}

void MapViewInputHandler::continueDrawing(const QPointF& mapPos, QMouseEvent* event) {
    if (!brushManager_ || !mapView_ || !map_ || !undoStack_) return;
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (currentBrush) {
        QList<QPointF> tiles = getAffectedTiles(mapPos, currentBrush);
        for (const QPointF& tilePos : tiles) {
            if (!currentDrawingCommand_) {
                // If no command was started by press (e.g. press was on empty space, or brush needs drag to activate)
                // The first move event that generates a command will start currentDrawingCommand_.
                QUndoCommand* moveCmd = currentBrush->mouseMoveEvent(tilePos, event, mapView_, map_, undoStack_,
                                                                  shiftModifierActive_, ctrlModifierActive_, altModifierActive_,
                                                                  nullptr); // No parent yet
                if (moveCmd) {
                    currentDrawingCommand_ = moveCmd;
                }
            } else {
                // A command is in progress, pass it as parent.
                // The brush's mouseMoveEvent is expected to add its actions as children to currentDrawingCommand_
                // or otherwise modify it. It should ideally not return a new top-level command if a parent is given,
                // unless it wants to replace currentDrawingCommand_ (which is not standard QUndoCommand usage).
                currentBrush->mouseMoveEvent(tilePos, event, mapView_, map_, undoStack_,
                                             shiftModifierActive_, ctrlModifierActive_, altModifierActive_,
                                             currentDrawingCommand_);
            }
        }
    }
    mapView_->update(); // Brush might be continuously drawing or updating a preview
}

void MapViewInputHandler::finishDrawing(const QPointF& mapPos, QMouseEvent* event) {
    if (!brushManager_ || !mapView_ || !map_ || !undoStack_) return;
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (currentBrush) {
        QList<QPointF> tiles = getAffectedTiles(mapPos, currentBrush);
        for (const QPointF& tilePos : tiles) {
            if (!currentDrawingCommand_) {
                // If no command was started by press or move, try to create one on release.
                QUndoCommand* releaseCmd = currentBrush->mouseReleaseEvent(tilePos, event, mapView_, map_, undoStack_,
                                                                         shiftModifierActive_, ctrlModifierActive_, altModifierActive_,
                                                                         nullptr); // No parent yet
                if (releaseCmd) {
                    currentDrawingCommand_ = releaseCmd; // This becomes the command for the whole gesture.
                }
            } else {
                // A command is in progress, pass it as parent.
                currentBrush->mouseReleaseEvent(tilePos, event, mapView_, map_, undoStack_,
                                                shiftModifierActive_, ctrlModifierActive_, altModifierActive_,
                                                currentDrawingCommand_);
            }
        }
    }

    if (currentDrawingCommand_) {
        // Set a meaningful text for the aggregated command if it doesn't have one.
        // Brushes should ideally set their own text.
        if (currentDrawingCommand_->text().isEmpty()) {
             currentDrawingCommand_->setText(QObject::tr("Brush Stroke"));
        }
        undoStack_->push(currentDrawingCommand_);
    }

    // Reset drawing-specific flags (already done in handleMouseReleaseEvent, but good for clarity if called elsewhere)
    isDraggingDraw_ = false;
    isReplaceDragging_ = false;
    currentDrawingCommand_ = nullptr; // Reset for the next operation.
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
