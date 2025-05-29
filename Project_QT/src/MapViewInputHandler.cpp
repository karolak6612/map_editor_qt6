#include "MapViewInputHandler.h"
#include "MapView.h" // To call methods on MapView
#include <QDebug>
#include <QGuiApplication> // For Qt::ShiftModifier, Qt::ControlModifier

MapViewInputHandler::MapViewInputHandler(MapView* mapView)
    : mapView_(mapView),
      isSelecting_(false),
      isDraggingSelection_(false),
      isDrawing_(false),
      isDraggingDraw_(false)
      // dragStartMapPos_ and clickOriginScreenPos_ are default initialized
{
    if (!mapView_) {
        qWarning() << "MapViewInputHandler initialized with a null MapView instance!";
    }
}

// --- Left Mouse Button Logic ---
void MapViewInputHandler::handleMousePress(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos) {
    // This is for Left Button actions
    bool isPasting = false; // Placeholder: would be mapView_->isPasting();

    if (mode == MapView::EditorMode::Selection) {
        if (isPasting) {
            mapView_->pasteSelection(mapPos);
            isDraggingSelection_ = true; // Allow dragging the pasted selection
            dragStartMapPos_ = mapPos;
        } else if (event->modifiers() & Qt::ShiftModifier) {
            isSelecting_ = true;
            dragStartMapPos_ = mapPos;
            // Original logic often clears selection unless Ctrl is also held for adding to selection.
            // If Ctrl+Shift is for adding, this clear is correct.
            mapView_->clearSelection(); 
        } else if (event->modifiers() & Qt::ControlModifier) {
            mapView_->toggleSelectionAt(mapPos); // Toggle selection of item at mapPos
            // Typically, after toggle, no drag operation starts immediately.
            isDraggingSelection_ = false; // Ensure not dragging after a toggle
        } else {
            // Standard click without modifiers
            if (mapView_->isOnSelection(mapPos)) {
                isDraggingSelection_ = true; // Start dragging the existing selection
                dragStartMapPos_ = mapPos;
            } else {
                mapView_->clearSelection();
                mapView_->selectObjectAt(mapPos); // Select item at mapPos
                // After selecting, check if an object was actually selected to allow dragging
                if (mapView_->isOnSelection(mapPos)) { 
                    isDraggingSelection_ = true;
                    dragStartMapPos_ = mapPos;
                } else {
                     // If nothing was selected, start a new selection rectangle
                    isSelecting_ = true;
                    dragStartMapPos_ = mapPos;
                }
            }
        }
    } else { // Drawing Mode
        // Brush* brush = mapView_->getCurrentBrush(); // Placeholder
        bool brushCanDrag = false; // Placeholder: brush && brush->canDrag();

        if ((event->modifiers() & Qt::ShiftModifier) && brushCanDrag) {
            isDraggingDraw_ = true;
            dragStartMapPos_ = mapPos;
            // mapView_->startDragDrawing(mapPos, event->modifiers()); // Or similar
        } else {
            isDrawing_ = true;
            mapView_->startDrawing(mapPos, event->modifiers());
        }
    }
}

void MapViewInputHandler::handleMouseMove(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos) {
    // This is for Left Button related move actions
    if (mode == MapView::EditorMode::Selection) {
        if (isDraggingSelection_) {
            mapView_->updateMoveSelectionFeedback(mapPos - dragStartMapPos_);
        } else if (isSelecting_) {
            mapView_->updateSelectionRectFeedback(dragStartMapPos_, mapPos);
        }
    } else { // Drawing Mode
        if (isDrawing_) {
            mapView_->continueDrawing(mapPos, event->modifiers());
        } else if (isDraggingDraw_) {
            mapView_->updateDragDrawFeedback(dragStartMapPos_, mapPos);
        }
    }
}

void MapViewInputHandler::handleMouseRelease(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos) {
    // This is for Left Button related release actions
    if (mode == MapView::EditorMode::Selection) {
        if (isDraggingSelection_) {
            mapView_->finalizeMoveSelection(mapPos - dragStartMapPos_);
            isDraggingSelection_ = false;
        } else if (isSelecting_) {
            mapView_->finalizeSelectionRect(dragStartMapPos_, mapPos, event->modifiers());
            isSelecting_ = false;
        }
    } else { // Drawing Mode
        if (isDrawing_) {
            mapView_->finalizeDrawing(mapPos, event->modifiers());
            isDrawing_ = false;
        } else if (isDraggingDraw_) {
            mapView_->finalizeDragDraw(dragStartMapPos_, mapPos, event->modifiers());
            isDraggingDraw_ = false;
        }
    }
}

// --- Properties (Right/Middle) Mouse Button Logic ---
void MapViewInputHandler::handlePropertiesClick(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos) {
    qDebug() << "MapViewInputHandler::handlePropertiesClick at screen" << screenPos << "map" << mapPos;
    mapView_->setFocus(); // Ensure view has focus

    // From MapCanvas::OnMousePropertiesClick
    // if (editor->GetMode() != EditorInterface::MODE_SELECTION) { editor->SetMode(EditorInterface::MODE_SELECTION); }
    if (mapView_->getCurrentEditorMode() != MapView::EditorMode::Selection) {
        mapView_->switchToSelectionMode(); // Placeholder
    }
    mapView_->endPasting(); // Placeholder

    // Reset selection states from other buttons
    isSelecting_ = false; 
    isDraggingSelection_ = false;
    isDrawing_ = false;
    isDraggingDraw_ = false;

    clickOriginScreenPos_ = screenPos; // Store for context menu

    if (event->modifiers() & Qt::ShiftModifier) {
        isSelecting_ = true; // Start bounding box selection with this button
        dragStartMapPos_ = mapPos;
        if (!(event->modifiers() & Qt::ControlModifier)) { // If not also holding Ctrl
            mapView_->clearSelection();
        }
    } else if (!(event->modifiers() & Qt::ControlModifier) && !mapView_->isOnSelection(mapPos)) {
        // If not Shift, not Ctrl, and not clicking on an existing selection, clear current selection
        // and select the object under the cursor.
        mapView_->clearSelection();
        mapView_->selectObjectAt(mapPos); // This might select something or not
    } else if (event->modifiers() & Qt::ControlModifier) {
        // If Ctrl is held, toggle selection of the item under cursor
        // This part was missing in the original snippet for properties click, but is common.
        // For now, let properties click with Ctrl pass through to context menu,
        // or it could do a toggle like left click with Ctrl.
        // Let's assume for now Ctrl+RightClick doesn't start a drag selection or clear,
        // it just modifies the selection state for the context menu.
        mapView_->toggleSelectionAt(mapPos);
    }
    // No isDraggingSelection_ = true; here, that's for left mouse.
    // Properties click itself doesn't start a drag, but context menu might operate on selection.
}

void MapViewInputHandler::handlePropertiesRelease(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos) {
    qDebug() << "MapViewInputHandler::handlePropertiesRelease at screen" << screenPos << "map" << mapPos;

    if (isSelecting_) { // If a bounding box selection was made with the properties button
        mapView_->finalizeSelectionRect(dragStartMapPos_, mapPos, event->modifiers());
        isSelecting_ = false;
    } else {
        // It was a regular click (not a drag for bounding box)
        // Check if the mouse moved significantly, if so, maybe don't show context menu (original didn't have this check here)
        // QPoint moveDelta = screenPos - clickOriginScreenPos_;
        // if (moveDelta.manhattanLength() < QApplication::startDragDistance()) {
             mapView_->showContextMenuAt(clickOriginScreenPos_); // Placeholder
        // }
    }
    
    mapView_->resetActionQueueTimer_placeholder(); // Placeholder

    // Ensure states are reset
    isDraggingSelection_ = false; 
    isSelecting_ = false;
}


void MapViewInputHandler::resetInputStates() {
    isSelecting_ = false;
    isDraggingSelection_ = false;
    isDrawing_ = false;
    isDraggingDraw_ = false;
    dragStartMapPos_ = QPointF(); // Reset drag start position
    clickOriginScreenPos_ = QPoint(); // Reset click origin
    qDebug() << "MapViewInputHandler::resetInputStates called";
}
