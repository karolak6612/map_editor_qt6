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
#include <QScrollBar>   // For scroll bar access
#include <QSize>        // For QSize

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

    // Task 74: Check for waypoint interactions first
    if (handleWaypointInteraction(event, mapPosition)) {
        return; // Waypoint interaction handled
    }

    // Enhanced mouse button handling with wxWidgets compatibility
    if (pressedButton_ == Qt::LeftButton) {
        if (mapView_->getSwitchMouseButtons()) {
            handleMouseCameraClick(event, mapPosition);
        } else {
            handleMouseActionClick(event, mapPosition);
        }
    } else if (pressedButton_ == Qt::MiddleButton) {
        handleMouseCameraClick(event, mapPosition);
    } else if (pressedButton_ == Qt::RightButton) {
        if (mapView_->getSwitchMouseButtons()) {
            handleMousePropertiesClick(event, mapPosition);
        } else {
            handleMousePropertiesClick(event, mapPosition);
        }
    }
    mapView_->update(); // Request repaint for visual feedback
}

void MapViewInputHandler::handleMouseActionClick(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_ || !brushManager_) return;

    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (currentBrush) { // Drawing mode
        // Enhanced state machine logic from wxwidgets
        isDragging_ = true;
        isDrawing_ = true;
        dragStartMapPos_ = mapPosition;
        dragCurrentMapPos_ = mapPosition;

        // Determine drawing mode based on modifiers and brush capabilities
        if (shiftModifierActive_ && currentBrush->canDrag()) {
            isDraggingDraw_ = true;
            transitionToMode(InteractionMode::DraggingDraw);
            updateBrushState(BrushState::StartDrag);
            startDraggingDraw(mapPosition, event);
        } else if (altModifierActive_ && currentBrush->isGround()) {
            isReplaceDragging_ = true;
            transitionToMode(InteractionMode::ReplaceDragging);
            updateBrushState(BrushState::StartDraw);
            startReplaceDragging(mapPosition, event);
        } else if (ctrlModifierActive_) {
            // Rectangle drawing mode
            transitionToMode(InteractionMode::DrawingRect);
            updateBrushState(BrushState::StartDraw);
            startDrawingRect(mapPosition, event);
        } else {
            // Normal drawing mode
            transitionToMode(InteractionMode::Drawing);
            updateBrushState(BrushState::StartDraw);

            // Check if brush supports continuous drawing or single-click only
            if (currentBrush->getBrushSize() == 0 && !currentBrush->oneSizeFitsAll()) {
                isDrawing_ = true;
            } else {
                isDrawing_ = currentBrush->canSmear();
            }

            startDrawing(mapPosition, event);
        }

        qDebug() << "MapViewInputHandler: Started drawing mode" << static_cast<int>(currentMode_)
                 << "with brush" << currentBrush->name();

    } else { // Selection mode (no active brush)
        // Implement wxwidgets selection behavior (Task 61)
        if (shiftModifierActive_) {
            // Shift: Start box selection (clear existing unless Ctrl is also held)
            transitionToMode(InteractionMode::SelectingBox);
            isBoundBoxSelection_ = true;
            qDebug() << "MapViewInputHandler: Starting box selection (Shift)";
            startSelectionBox(mapPosition, event);
        } else if (ctrlModifierActive_) {
            // Ctrl: Toggle individual item/tile selection
            handleSingleClickSelection(mapPosition, event, true); // toggle mode
        } else {
            // No modifiers: Check if clicking on existing selection for dragging
            if (mapView_->isOnSelection(mapPosition)) {
                // Clicking on existing selection - start dragging
                transitionToMode(InteractionMode::DraggingSelection);
                qDebug() << "MapViewInputHandler: Starting selection drag";
            } else {
                // Clicking on empty space or unselected tile - select it
                handleSingleClickSelection(mapPosition, event, false); // replace mode
            }
        }
    }
}

void MapViewInputHandler::handleMouseCameraClick(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_) return;

    currentMode_ = InteractionMode::PanningView;

    // Store middle mouse button click position for camera controls
    lastMouseScreenPos_ = event->pos();

    if (ctrlModifierActive_) {
        // Ctrl+Middle: Reset zoom to 1.0 and center on cursor (matching wxWidgets)
        QSize viewSize = mapView_->viewport()->size();
        QPoint cursorPos = event->pos();

        // Calculate scroll adjustment to center on cursor
        int scrollX = static_cast<int>(-viewSize.width() * (1.0 - mapView_->getZoomLevel()) *
                                     (qMax(cursorPos.x(), 1) / double(viewSize.width())));
        int scrollY = static_cast<int>(-viewSize.height() * (1.0 - mapView_->getZoomLevel()) *
                                     (qMax(cursorPos.y(), 1) / double(viewSize.height())));

        mapView_->pan(scrollX, scrollY);
        mapView_->zoom(1.0 / mapView_->getZoomLevel(), event->position()); // Reset to zoom 1.0
    } else {
        // Normal middle mouse: Start screen dragging
        startPanning(event);
    }
}

void MapViewInputHandler::handleMousePropertiesClick(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_) return;

    // Right-click typically opens properties or context menu
    qDebug() << "Properties click at map position:" << mapPosition;
    // This could trigger a properties dialog or context menu
    // mapView_->showPropertiesDialogFor(mapPosition);
}

void MapViewInputHandler::handleMouseMoveEvent(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_) return;
    updateModifierKeys(event);

    // Update current drag position
    dragCurrentMapPos_ = mapPosition;

    switch (currentMode_) {
        case InteractionMode::Drawing:
            continueDrawing(mapPosition, event);
            break;
        case InteractionMode::DraggingDraw:
            continueDraggingDraw(mapPosition, event);
            break;
        case InteractionMode::ReplaceDragging:
            continueReplaceDragging(mapPosition, event);
            break;
        case InteractionMode::DrawingRect:
            updateDrawingRect(mapPosition, event);
            break;
        case InteractionMode::DrawingLine:
            updateDrawingLine(mapPosition, event);
            break;
        case InteractionMode::PanningView:
        case InteractionMode::ScreenDragging:
            continuePanning(event);
            break;
        case InteractionMode::SelectingBox:
        case InteractionMode::BoundBoxSelection:
            updateSelectionBox(mapPosition, event);
            break;
        case InteractionMode::DraggingSelection:
            // Handle selection dragging
            if (mapView_->isOnSelection(mapPosition)) {
                mapView_->updateMoveSelectionFeedback(mapPosition - dragStartMapPos_);
            }
            break;
        case InteractionMode::Idle:
        case InteractionMode::BrushPreview:
            // Update brush preview on hover
            updateBrushPreview(mapPosition);
            // mapView_->updateHoverInfo(mapPosition); // Update status bar, highlight tile under cursor
            break;
        case InteractionMode::ContextMenu:
            // Context menu is active, no movement handling needed
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
        // Enhanced mouse button release handling with wxWidgets compatibility
        if (event->button() == Qt::LeftButton) {
            if (mapView_->getSwitchMouseButtons()) {
                handleMouseCameraRelease(event, mapPosition);
            } else {
                handleMouseActionRelease(event, mapPosition);
            }
        } else if (event->button() == Qt::MiddleButton) {
            handleMouseCameraRelease(event, mapPosition);
        } else if (event->button() == Qt::RightButton) {
            if (mapView_->getSwitchMouseButtons()) {
                handleMousePropertiesRelease(event, mapPosition);
            } else {
                handleMousePropertiesRelease(event, mapPosition);
            }
        }

        // Enhanced state reset after calling finish handlers
        resetInteractionState();
        pressedButton_ = Qt::NoButton;

        mapView_->update(); // General update after action is finished
    }
}

void MapViewInputHandler::handleMouseActionRelease(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_) return;

    InteractionMode modeEnded = currentMode_;

    // Update final drag position
    dragEndMapPos_ = mapPosition;

    switch (modeEnded) {
        case InteractionMode::Drawing:
            finishDrawing(mapPosition, event);
            break;
        case InteractionMode::DraggingDraw:
            finishDraggingDraw(mapPosition, event);
            break;
        case InteractionMode::ReplaceDragging:
            finishReplaceDragging(mapPosition, event);
            break;
        case InteractionMode::DrawingRect:
            finishDrawingRect(mapPosition, event);
            break;
        case InteractionMode::DrawingLine:
            finishDrawingLine(mapPosition, event);
            break;
        case InteractionMode::SelectingBox:
        case InteractionMode::BoundBoxSelection:
            finishSelectionBox(mapPosition, event);
            break;
        case InteractionMode::DraggingSelection:
            // Finalize selection move
            if (mapView_->isOnSelection(mapPosition)) {
                mapView_->finalizeMoveSelection(mapPosition - dragStartMapPos_);
            }
            break;
        case InteractionMode::PastingSelection:
            // Finalize paste operation
            mapView_->pasteSelection(mapPosition);
            break;
        default:
            break;
    }
}

void MapViewInputHandler::handleMouseCameraRelease(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_) return;

    if (currentMode_ == InteractionMode::PanningView) {
        finishPanning(event);

        // Check for click-to-center behavior (matching wxWidgets)
        QPoint releasePos = event->pos();
        QPoint pressPos = lastMouseScreenPos_;

        // If mouse didn't move much, treat as a click to center view
        if (qAbs(releasePos.x() - pressPos.x()) <= 3 && qAbs(releasePos.y() - pressPos.y()) <= 3) {
            if (!ctrlModifierActive_) { // Only if Ctrl wasn't held (Ctrl+click resets zoom)
                QSize viewSize = mapView_->viewport()->size();
                QPoint cursorPos = event->pos();

                // Center view on cursor position
                int scrollX = static_cast<int>(mapView_->getZoomLevel() * (2 * cursorPos.x() - viewSize.width()));
                int scrollY = static_cast<int>(mapView_->getZoomLevel() * (2 * cursorPos.y() - viewSize.height()));

                mapView_->pan(scrollX, scrollY);
            }
        }
    }
}

void MapViewInputHandler::handleMousePropertiesRelease(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_) return;

    // Handle properties release - could show context menu or properties dialog
    qDebug() << "Properties release at map position:" << mapPosition;
    // mapView_->showContextMenuAt(event->globalPosition().toPoint());
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
        return;
    } else if (event->matches(QKeySequence::Redo)) {
        undoStack_->redo();
        mapView_->update();
        event->accept();
        return;
    }

    // Handle floor navigation keys first (high priority)
    handleFloorNavigationKeys(event);
    if (event->isAccepted()) return;

    // Handle hotkey system (F1-F12 and number keys)
    handleHotkeyKeys(event);
    if (event->isAccepted()) return;

    // Enhanced key handling for mode-specific behaviors
    handleModeSpecificKeys(event);

    // Handle tool and brush shortcuts
    handleToolShortcuts(event);

    // Handle view shortcuts
    handleViewShortcuts(event);

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

// --- Enhanced Key Handling Methods ---

void MapViewInputHandler::handleModeSpecificKeys(QKeyEvent* event) {
    if (!mapView_ || event->isAccepted()) return;

    switch (currentMode_) {
        case InteractionMode::Drawing:
        case InteractionMode::DraggingDraw:
            handleDrawingModeKeys(event);
            break;
        case InteractionMode::SelectingBox:
        case InteractionMode::DraggingSelection:
            handleSelectionModeKeys(event);
            break;
        case InteractionMode::PanningView:
            handlePanningModeKeys(event);
            break;
        case InteractionMode::Pasting:
            handlePastingModeKeys(event);
            break;
        case InteractionMode::Idle:
        default:
            // No mode-specific keys in idle mode
            break;
    }
}

void MapViewInputHandler::handleToolShortcuts(QKeyEvent* event) {
    if (!mapView_ || !brushManager_ || event->isAccepted()) return;

    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Tool selection shortcuts (matching wxWidgets patterns)
    switch (event->key()) {
        case Qt::Key_B:
            if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
                // brushManager_->selectBrushTool();
                qDebug() << "Brush tool shortcut (B) - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_E:
            if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
                // brushManager_->selectEraserTool();
                qDebug() << "Eraser tool shortcut (E) - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_S:
            if (!(modifiers & Qt::ControlModifier)) { // Avoid conflict with Ctrl+S (Save)
                // brushManager_->selectSelectionTool();
                qDebug() << "Selection tool shortcut (S) - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_F:
            if (!(modifiers & Qt::ControlModifier)) { // Avoid conflict with Ctrl+F (Find)
                // brushManager_->selectFillTool();
                qDebug() << "Fill tool shortcut (F) - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_R:
            if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
                // brushManager_->rotateBrush();
                qDebug() << "Rotate brush shortcut (R) - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_BracketLeft: // [
            if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
                // brushManager_->decreaseBrushSize();
                qDebug() << "Decrease brush size shortcut ([) - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_BracketRight: // ]
            if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
                // brushManager_->increaseBrushSize();
                qDebug() << "Increase brush size shortcut (]) - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_J:
            if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
                // Jump to brush (from wxWidgets)
                qDebug() << "Jump to brush shortcut (J) - placeholder";
                event->accept();
            }
            break;
    }
}

void MapViewInputHandler::handleViewShortcuts(QKeyEvent* event) {
    if (!mapView_ || event->isAccepted()) return;

    Qt::KeyboardModifiers modifiers = event->modifiers();

    // View-related shortcuts
    switch (event->key()) {
        case Qt::Key_Space:
            if (modifiers & Qt::ControlModifier) {
                // Ctrl+Space: Fill doodad preview buffer (from wxWidgets)
                qDebug() << "Ctrl+Space - Fill doodad preview buffer - placeholder";
                event->accept();
            } else if (!(modifiers & Qt::AltModifier)) {
                // Space: Switch mode (from wxWidgets)
                qDebug() << "Space - Switch mode - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_Tab:
            if (modifiers & Qt::ShiftModifier) {
                // Shift+Tab: Cycle tab backwards
                qDebug() << "Shift+Tab - Cycle tab backwards - placeholder";
                event->accept();
            } else if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
                // Tab: Cycle tab forwards
                qDebug() << "Tab - Cycle tab forwards - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_G:
            if (modifiers & Qt::ControlModifier) {
                // Go to coordinates dialog
                qDebug() << "Ctrl+G - Go to coordinates - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_H:
            if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
                // Toggle grid display
                qDebug() << "H - Toggle grid - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_P:
            if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
                // Go to previous position (from wxWidgets)
                qDebug() << "P - Go to previous position - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_Q:
            if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
                // Show shade (from wxWidgets)
                qDebug() << "Q - Show shade - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_W:
            if (modifiers & Qt::ControlModifier) {
                // Show all floors (from wxWidgets)
                qDebug() << "Ctrl+W - Show all floors - placeholder";
                event->accept();
            }
            break;
    }
}

void MapViewInputHandler::handleDrawingModeKeys(QKeyEvent* event) {
    if (!mapView_ || event->isAccepted()) return;

    // Drawing mode specific keys
    switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            // Confirm current drawing operation
            qDebug() << "Enter - Confirm drawing operation - placeholder";
            currentMode_ = InteractionMode::Idle;
            event->accept();
            break;
        case Qt::Key_Shift:
            // Shift modifier for constrained drawing (handled by modifier tracking)
            break;
        default:
            break;
    }
}

void MapViewInputHandler::handleSelectionModeKeys(QKeyEvent* event) {
    if (!mapView_ || event->isAccepted()) return;

    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Selection mode specific keys
    switch (event->key()) {
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            // Delete selected items
            qDebug() << "Delete - Remove selected items - placeholder";
            event->accept();
            break;
        case Qt::Key_C:
            if (modifiers & Qt::ControlModifier) {
                // Copy selection
                qDebug() << "Ctrl+C - Copy selection - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_X:
            if (modifiers & Qt::ControlModifier) {
                // Cut selection
                qDebug() << "Ctrl+X - Cut selection - placeholder";
                event->accept();
            }
            break;
        case Qt::Key_V:
            if (modifiers & Qt::ControlModifier) {
                // Paste selection
                qDebug() << "Ctrl+V - Paste selection - placeholder";
                currentMode_ = InteractionMode::Pasting;
                event->accept();
            }
            break;
        case Qt::Key_A:
            if (modifiers & Qt::ControlModifier) {
                // Select all
                qDebug() << "Ctrl+A - Select all - placeholder";
                event->accept();
            }
            break;
        default:
            break;
    }
}

void MapViewInputHandler::handlePanningModeKeys(QKeyEvent* event) {
    if (!mapView_ || event->isAccepted()) return;

    // Panning mode specific keys
    switch (event->key()) {
        case Qt::Key_Space:
            // Space release will exit panning mode (handled in handleViewShortcuts)
            break;
        default:
            break;
    }
}

void MapViewInputHandler::handlePastingModeKeys(QKeyEvent* event) {
    if (!mapView_ || event->isAccepted()) return;

    // Pasting mode specific keys
    switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            // Confirm paste operation
            qDebug() << "Enter - Confirm paste operation - placeholder";
            currentMode_ = InteractionMode::Idle;
            event->accept();
            break;
        case Qt::Key_R:
            // Rotate pasted content
            qDebug() << "R - Rotate pasted content - placeholder";
            event->accept();
            break;
        default:
            break;
    }
}

void MapViewInputHandler::handleFloorNavigationKeys(QKeyEvent* event) {
    if (!mapView_ || event->isAccepted()) return;

    Qt::KeyboardModifiers modifiers = event->modifiers();
    int currentFloor = mapView_->getCurrentFloor();

    // Floor navigation keys (matching wxWidgets OnKeyDown behavior)
    switch (event->key()) {
        case Qt::Key_PageUp:
            // Page Up: Go up one floor
            mapView_->changeFloor(currentFloor - 1);
            qDebug() << "Page Up - Floor up to:" << (currentFloor - 1);
            event->accept();
            break;
        case Qt::Key_PageDown:
            // Page Down: Go down one floor
            mapView_->changeFloor(currentFloor + 1);
            qDebug() << "Page Down - Floor down to:" << (currentFloor + 1);
            event->accept();
            break;
        default:
            break;
    }
}

void MapViewInputHandler::handleHotkeyKeys(QKeyEvent* event) {
    if (!mapView_ || event->isAccepted()) return;

    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Hotkey system (matching wxWidgets hotkey behavior)
    int hotkeyIndex = -1;

    // F1-F12 keys for hotkeys
    if (event->key() >= Qt::Key_F1 && event->key() <= Qt::Key_F12) {
        hotkeyIndex = event->key() - Qt::Key_F1; // F1=0, F2=1, etc.
    }
    // Number keys 0-9 for hotkeys (when not modified)
    else if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9 &&
             !(modifiers & (Qt::ControlModifier | Qt::AltModifier))) {
        hotkeyIndex = (event->key() - Qt::Key_0 + 9) % 10; // 1=0, 2=1, ..., 9=8, 0=9
    }

    if (hotkeyIndex >= 0) {
        if (modifiers & Qt::ControlModifier) {
            // Ctrl+Hotkey: Set hotkey to current position/brush
            qDebug() << "Ctrl+Hotkey" << hotkeyIndex << "- Set hotkey - placeholder";
            event->accept();
        } else {
            // Hotkey: Use hotkey (go to position or select brush)
            qDebug() << "Hotkey" << hotkeyIndex << "- Use hotkey - placeholder";
            event->accept();
        }
    }

    // Additional navigation shortcuts from wxWidgets
    switch (event->key()) {
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            if (modifiers & Qt::ControlModifier) {
                // Ctrl+Plus: Zoom in (handled by menu action)
                // Don't accept here to let menu action handle it
            } else {
                // Plus: Go up one floor (numpad + behavior)
                int currentFloor = mapView_->getCurrentFloor();
                mapView_->changeFloor(currentFloor - 1);
                qDebug() << "Plus - Floor up to:" << (currentFloor - 1);
                event->accept();
            }
            break;
        case Qt::Key_Minus:
            if (modifiers & Qt::ControlModifier) {
                // Ctrl+Minus: Zoom out (handled by menu action)
                // Don't accept here to let menu action handle it
            } else {
                // Minus: Go down one floor (numpad - behavior)
                int currentFloor = mapView_->getCurrentFloor();
                mapView_->changeFloor(currentFloor + 1);
                qDebug() << "Minus - Floor down to:" << (currentFloor + 1);
                event->accept();
            }
            break;
    }
}

void MapViewInputHandler::handleWheelEvent(QWheelEvent* event, const QPointF& mapPosition) {
    if (!mapView_) return;
    updateModifierKeys(event);

    qreal delta = event->angleDelta().y();

    // Enhanced wheel handling matching wxWidgets behavior
    if (ctrlModifierActive_) {
        // Ctrl+Wheel: Change floor (matching wxWidgets OnWheel behavior)
        static double floorDiff = 0.0;
        floorDiff += delta;
        if (qAbs(floorDiff) >= 120.0) { // Standard wheel delta
            if (floorDiff < 0.0) {
                int newFloor = mapView_->getCurrentFloor() - 1;
                mapView_->changeFloor(newFloor);
            } else {
                int newFloor = mapView_->getCurrentFloor() + 1;
                mapView_->changeFloor(newFloor);
            }
            floorDiff = 0.0;
        }
        mapView_->updateAndRefreshMapCoordinates(event->position().toPoint());
    } else if (altModifierActive_) {
        // Alt+Wheel: Change brush size (matching wxWidgets OnWheel behavior)
        static double brushDiff = 0.0;
        brushDiff += delta;
        if (qAbs(brushDiff) >= 120.0) { // Standard wheel delta
            if (brushDiff < 0.0) {
                mapView_->increaseBrushSize_placeholder();
            } else {
                mapView_->decreaseBrushSize_placeholder();
            }
            brushDiff = 0.0;
        }
    } else {
        // Normal wheel: Zoom (enhanced to match wxWidgets zoom behavior)
        static double zoomDiff = 0.0;
        zoomDiff += delta;

        // Use zoom speed configuration (matching wxWidgets)
        double zoomSpeed = 0.1; // Default zoom speed, could be configurable
        double zoomChange = -delta * zoomSpeed / 640.0; // Matching wxWidgets calculation

        double currentZoom = mapView_->getZoomLevel();
        double newZoom = currentZoom + zoomChange;

        // Apply zoom limits
        if (newZoom < MIN_ZOOM) {
            zoomChange = MIN_ZOOM - currentZoom;
            newZoom = MIN_ZOOM;
        } else if (newZoom > MAX_ZOOM) {
            zoomChange = MAX_ZOOM - currentZoom;
            newZoom = MAX_ZOOM;
        }

        if (qAbs(zoomChange) > 0.001) { // Only zoom if change is significant
            double zoomFactor = newZoom / currentZoom;
            mapView_->zoom(zoomFactor, event->position());
        }
    }

    event->accept(); // Indicate event was handled
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

    // Store modifier state for selection logic (Task 61)
    shiftModifierActive_ = event->modifiers() & Qt::ShiftModifier;
    ctrlModifierActive_ = event->modifiers() & Qt::ControlModifier;

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

    // Use the stored modifier state and current event modifiers (Task 61)
    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Call MapView's finalizeSelectionRect with proper modifier handling
    mapView_->finalizeSelectionRect(dragStartMapPos_, mapPos, modifiers);

    // Clear the temporary visual selection box from MapView
    mapView_->setSelectionArea(QRectF());

    // Reset modifier state
    shiftModifierActive_ = false;
    ctrlModifierActive_ = false;

    mapView_->update(); // Update MapView to reflect any changes in selection state
}

// Single-click selection handling (Task 61)
void MapViewInputHandler::handleSingleClickSelection(const QPointF& mapPos, QMouseEvent* event, bool toggleMode) {
    if (!mapView_ || !map_) return;

    MapPos tilePos = MapPos(static_cast<int>(qFloor(mapPos.x())),
                           static_cast<int>(qFloor(mapPos.y())),
                           mapView_->getCurrentFloor());

    Tile* tile = map_->getTile(tilePos.x, tilePos.y, tilePos.z);
    Selection* selection = map_->getSelection();

    if (!selection) return;

    selection->start(Selection::NONE);

    if (!tile) {
        // Clicking on empty space - clear selection
        if (!toggleMode) {
            selection->clear();
        }
    } else {
        if (toggleMode) {
            // Ctrl+Click: Toggle selection of this tile/item
            if (selection->isSelected(tilePos)) {
                selection->removeTile(tilePos);
            } else {
                selection->addTile(tilePos);
            }
        } else {
            // Normal click: Replace selection with this tile/item
            selection->clear();
            selection->add(tile);
        }
    }

    selection->finish(Selection::NONE);

    // Update visual representation
    mapView_->updateSelectionVisuals();
    mapView_->update();
}

// Task 74: Waypoint interaction handling
bool MapViewInputHandler::handleWaypointInteraction(QMouseEvent* event, const QPointF& mapPosition) {
    if (!mapView_ || !map_) return false;

    // Check if waypoint tool is active
    if (!mapView_->isWaypointToolActive()) {
        return false; // Not handling waypoint interactions
    }

    if (event->button() == Qt::LeftButton) {
        // Left-click with waypoint tool: Place waypoint
        mapView_->placeWaypointAt(mapPosition);
        event->accept();
        return true;
    }

    return false; // Not handled
}
