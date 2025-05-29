#include "MapViewInputHandler.h"
#include "MapView.h"   // To call methods on MapView
#include "Brush.h"     // For Brush type
#include <QDebug>
#include <QMouseEvent> // Already included via MapView.h usually, but good for explicitness
#include <QGuiApplication> // For Qt::ShiftModifier, Qt::ControlModifier

MapViewInputHandler::MapViewInputHandler(MapView* mapView)
    : mapView_(mapView),
      isSelecting_(false),
      isDraggingSelection_(false),
      isDrawing_(false), // This state might become redundant if brushes manage their own drawing state
      isDraggingDraw_(false) // Same as above
{
    if (!mapView_) {
        qWarning() << "MapViewInputHandler initialized with a null MapView instance!";
    }
}

// --- Left Mouse Button Logic ---
void MapViewInputHandler::handleMousePress(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos) {
    if (mode == MapView::EditorMode::Drawing) {
        Brush* activeBrush = mapView_->getActiveBrush();
        if (activeBrush) {
            // Reset internal drawing states of input handler, brush will manage its own state.
            isDrawing_ = false; 
            isDraggingDraw_ = false;
            activeBrush->mousePressEvent(mapPos, event, mapView_);
        } else {
            qDebug() << "DrawingMode active, but no brush selected.";
        }
    } else if (mode == MapView::EditorMode::Selection) {
        // This is for Left Button actions in SelectionMode
        bool isPasting = false; // Placeholder: would be mapView_->isPasting();
        if (isPasting) {
            mapView_->pasteSelection(mapPos);
            isDraggingSelection_ = true; 
            dragStartMapPos_ = mapPos;
        } else if (event->modifiers() & Qt::ShiftModifier) {
            isSelecting_ = true;
            dragStartMapPos_ = mapPos;
            mapView_->clearSelection(); 
        } else if (event->modifiers() & Qt::ControlModifier) {
            mapView_->toggleSelectionAt(mapPos); 
            isDraggingSelection_ = false; 
        } else {
            if (mapView_->isOnSelection(mapPos)) {
                isDraggingSelection_ = true; 
                dragStartMapPos_ = mapPos;
            } else {
                mapView_->clearSelection();
                mapView_->selectObjectAt(mapPos); 
                if (mapView_->isOnSelection(mapPos)) { 
                    isDraggingSelection_ = true;
                    dragStartMapPos_ = mapPos;
                } else {
                    isSelecting_ = true;
                    dragStartMapPos_ = mapPos;
                }
            }
        }
    }
}

void MapViewInputHandler::handleMouseMove(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos) {
    if (mode == MapView::EditorMode::Drawing) {
        Brush* activeBrush = mapView_->getActiveBrush();
        if (activeBrush) {
            // The brush's mouseMoveEvent should check event->buttons() if it only wants to act on drag
            activeBrush->mouseMoveEvent(mapPos, event, mapView_);
        }
        // No specific isDrawing_ or isDraggingDraw_ check here anymore, brush handles its logic.
    } else if (mode == MapView::EditorMode::Selection) {
        // This is for Left Button related move actions in SelectionMode
        if (isDraggingSelection_) {
            mapView_->updateMoveSelectionFeedback(mapPos - dragStartMapPos_);
        } else if (isSelecting_) {
            mapView_->updateSelectionRectFeedback(dragStartMapPos_, mapPos);
        }
    }
}

void MapViewInputHandler::handleMouseRelease(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos) {
    if (mode == MapView::EditorMode::Drawing) {
        Brush* activeBrush = mapView_->getActiveBrush();
        if (activeBrush) {
            activeBrush->mouseReleaseEvent(mapPos, event, mapView_);
        }
        // Reset internal drawing states after brush action, if they were used by this handler
        isDrawing_ = false;
        isDraggingDraw_ = false;
    } else if (mode == MapView::EditorMode::Selection) {
        // This is for Left Button related release actions in SelectionMode
        if (isDraggingSelection_) {
            mapView_->finalizeMoveSelection(mapPos - dragStartMapPos_);
            isDraggingSelection_ = false;
        } else if (isSelecting_) {
            mapView_->finalizeSelectionRect(dragStartMapPos_, mapPos, event->modifiers());
            isSelecting_ = false;
        }
    }
}

// --- Properties (Right/Middle) Mouse Button Logic ---
void MapViewInputHandler::handlePropertiesClick(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos) {
    qDebug() << "MapViewInputHandler::handlePropertiesClick at screen" << screenPos << "map" << mapPos;
    mapView_->setFocus(); 

    if (mapView_->getCurrentEditorMode() != MapView::EditorMode::Selection) {
        mapView_->switchToSelectionMode(); 
    }
    mapView_->endPasting(); 

    // Reset all interaction states, including drawing ones, as properties click usually means changing context.
    resetInputStates(); // This now covers all boolean flags.

    clickOriginScreenPos_ = screenPos; 

    if (event->modifiers() & Qt::ShiftModifier) {
        isSelecting_ = true; 
        dragStartMapPos_ = mapPos;
        if (!(event->modifiers() & Qt::ControlModifier)) { 
            mapView_->clearSelection();
        }
    } else if (!(event->modifiers() & Qt::ControlModifier) && !mapView_->isOnSelection(mapPos)) {
        mapView_->clearSelection();
        mapView_->selectObjectAt(mapPos); 
    } else if (event->modifiers() & Qt::ControlModifier) {
        mapView_->toggleSelectionAt(mapPos);
    }
}

void MapViewInputHandler::handlePropertiesRelease(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos) {
    qDebug() << "MapViewInputHandler::handlePropertiesRelease at screen" << screenPos << "map" << mapPos;

    if (isSelecting_) { 
        mapView_->finalizeSelectionRect(dragStartMapPos_, mapPos, event->modifiers());
        // isSelecting_ will be reset by resetInputStates below or explicitly.
    } else {
        mapView_->showContextMenuAt(clickOriginScreenPos_); 
    }
    
    mapView_->resetActionQueueTimer_placeholder(); 
    resetInputStates(); // Reset all states after properties release action.
}


void MapViewInputHandler::resetInputStates() {
    isSelecting_ = false;
    isDraggingSelection_ = false;
    isDrawing_ = false;
    isDraggingDraw_ = false;
    dragStartMapPos_ = QPointF(); 
    clickOriginScreenPos_ = QPoint(); 
    qDebug() << "MapViewInputHandler::resetInputStates called";
}
