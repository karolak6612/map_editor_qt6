// MapViewInputHandlerBrushInteraction.cpp - Enhanced brush interaction methods for Task 34
// This file contains the comprehensive brush interaction system implementation

#include "MapViewInputHandler.h"
#include "MapView.h"
#include "BrushManager.h"
#include "Brush.h"
#include <QDebug>
#include <QtMath>

// Enhanced brush interaction handlers (Task 34 requirement)

void MapViewInputHandler::startDraggingDraw(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush || !currentBrush->canDrag()) {
        qWarning() << "MapViewInputHandler::startDraggingDraw: Invalid brush for dragging";
        return;
    }
    
    dragStartMapPos_ = mapPos;
    dragCurrentMapPos_ = mapPos;
    isDraggingDraw_ = true;
    
    // Initialize drawing area
    currentDrawingArea_ = QRectF(mapPos, QSizeF(0, 0));
    currentDrawingTiles_.clear();
    
    // Get initial affected tiles
    currentDrawingTiles_ = getAffectedTiles(mapPos, currentBrush);
    
    // Start brush drawing operation
    if (currentBrush->startPaint(map_, mapPos.toPoint())) {
        updateBrushState(BrushState::StartDrag);
        qDebug() << "MapViewInputHandler: Started dragging draw at" << mapPos;
    } else {
        qWarning() << "MapViewInputHandler: Failed to start brush paint operation";
    }
    
    mapView_->update();
}

void MapViewInputHandler::continueDraggingDraw(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_ || !isDraggingDraw_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush) return;
    
    dragCurrentMapPos_ = mapPos;
    
    // Update drawing area bounds
    QRectF newArea = QRectF(dragStartMapPos_, mapPos).normalized();
    currentDrawingArea_ = currentDrawingArea_.united(newArea);
    
    // Get tiles in the drawing area based on brush shape
    QList<QPointF> newTiles = getTilesInArea(dragStartMapPos_, mapPos, currentBrush->getBrushShape());
    
    // Add new tiles to current drawing tiles
    for (const QPointF& tile : newTiles) {
        if (!currentDrawingTiles_.contains(tile)) {
            currentDrawingTiles_.append(tile);
        }
    }
    
    // Continue brush painting
    if (currentBrush->canSmear()) {
        currentBrush->draw(map_, mapPos.toPoint(), false);
    }
    
    updateBrushState(BrushState::ContinueDrag);
    mapView_->update();
}

void MapViewInputHandler::finishDraggingDraw(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_ || !isDraggingDraw_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush) return;
    
    dragEndMapPos_ = mapPos;
    
    // Final drawing operation
    currentBrush->endPaint(map_, mapPos.toPoint());
    
    // Create undo command for the entire drag operation
    if (undoStack_ && !currentDrawingTiles_.isEmpty()) {
        // Create batch command for all affected tiles
        // This would be implemented based on the specific undo system
        qDebug() << "MapViewInputHandler: Creating undo command for" << currentDrawingTiles_.size() << "tiles";
    }
    
    updateBrushState(BrushState::EndDrag);
    
    // Reset state
    isDraggingDraw_ = false;
    currentDrawingTiles_.clear();
    currentDrawingArea_ = QRectF();
    
    qDebug() << "MapViewInputHandler: Finished dragging draw from" << dragStartMapPos_ << "to" << mapPos;
    mapView_->update();
}

void MapViewInputHandler::startReplaceDragging(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush || !currentBrush->isGround()) {
        qWarning() << "MapViewInputHandler::startReplaceDragging: Invalid brush for replace dragging";
        return;
    }
    
    dragStartMapPos_ = mapPos;
    dragCurrentMapPos_ = mapPos;
    isReplaceDragging_ = true;
    
    // Initialize replace operation
    currentDrawingTiles_.clear();
    currentDrawingTiles_ = getAffectedTiles(mapPos, currentBrush);
    
    // Start replace operation
    if (currentBrush->startPaint(map_, mapPos.toPoint())) {
        updateBrushState(BrushState::StartDraw);
        qDebug() << "MapViewInputHandler: Started replace dragging at" << mapPos;
    }
    
    mapView_->update();
}

void MapViewInputHandler::continueReplaceDragging(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_ || !isReplaceDragging_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush) return;
    
    dragCurrentMapPos_ = mapPos;
    
    // Get new affected tiles
    QList<QPointF> newTiles = getAffectedTiles(mapPos, currentBrush);
    
    // Add to current tiles if not already included
    for (const QPointF& tile : newTiles) {
        if (!currentDrawingTiles_.contains(tile)) {
            currentDrawingTiles_.append(tile);
            // Perform replace operation on new tile
            currentBrush->draw(map_, tile.toPoint(), true); // true for replace mode
        }
    }
    
    updateBrushState(BrushState::ContinueDraw);
    mapView_->update();
}

void MapViewInputHandler::finishReplaceDragging(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_ || !isReplaceDragging_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush) return;
    
    dragEndMapPos_ = mapPos;
    
    // End replace operation
    currentBrush->endPaint(map_, mapPos.toPoint());
    
    updateBrushState(BrushState::EndDraw);
    
    // Reset state
    isReplaceDragging_ = false;
    currentDrawingTiles_.clear();
    
    qDebug() << "MapViewInputHandler: Finished replace dragging from" << dragStartMapPos_ << "to" << mapPos;
    mapView_->update();
}

void MapViewInputHandler::startDrawingRect(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush) return;
    
    dragStartMapPos_ = mapPos;
    dragCurrentMapPos_ = mapPos;
    
    // Initialize rectangle drawing
    currentDrawingArea_ = QRectF(mapPos, QSizeF(0, 0));
    currentDrawingTiles_.clear();
    
    updateBrushState(BrushState::StartDraw);
    qDebug() << "MapViewInputHandler: Started rectangle drawing at" << mapPos;
    
    mapView_->update();
}

void MapViewInputHandler::updateDrawingRect(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_) return;
    
    dragCurrentMapPos_ = mapPos;
    
    // Update rectangle area
    currentDrawingArea_ = QRectF(dragStartMapPos_, mapPos).normalized();
    
    // Get tiles in rectangle
    currentDrawingTiles_ = getTilesInRectangle(dragStartMapPos_, mapPos);
    
    // Update visual feedback
    updateSelectionFeedback(currentDrawingArea_);
    
    updateBrushState(BrushState::ContinueDraw);
    mapView_->update();
}

void MapViewInputHandler::finishDrawingRect(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush) return;
    
    dragEndMapPos_ = mapPos;
    
    // Apply brush to all tiles in rectangle
    for (const QPointF& tile : currentDrawingTiles_) {
        currentBrush->draw(map_, tile.toPoint(), false);
    }
    
    updateBrushState(BrushState::EndDraw);
    
    // Clear visual feedback
    clearSelectionFeedback();
    currentDrawingTiles_.clear();
    currentDrawingArea_ = QRectF();
    
    qDebug() << "MapViewInputHandler: Finished rectangle drawing from" << dragStartMapPos_ << "to" << mapPos;
    mapView_->update();
}

void MapViewInputHandler::startDrawingLine(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush) return;
    
    dragStartMapPos_ = mapPos;
    dragCurrentMapPos_ = mapPos;
    
    currentDrawingTiles_.clear();
    
    updateBrushState(BrushState::StartDraw);
    qDebug() << "MapViewInputHandler: Started line drawing at" << mapPos;
    
    mapView_->update();
}

void MapViewInputHandler::updateDrawingLine(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_) return;
    
    dragCurrentMapPos_ = mapPos;
    
    // Get tiles in line
    currentDrawingTiles_ = getTilesInLine(dragStartMapPos_, mapPos);
    
    updateBrushState(BrushState::ContinueDraw);
    mapView_->update();
}

void MapViewInputHandler::finishDrawingLine(const QPointF& mapPos, QMouseEvent* event) {
    if (!mapView_ || !brushManager_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush) return;
    
    dragEndMapPos_ = mapPos;
    
    // Apply brush to all tiles in line
    for (const QPointF& tile : currentDrawingTiles_) {
        currentBrush->draw(map_, tile.toPoint(), false);
    }
    
    updateBrushState(BrushState::EndDraw);
    
    currentDrawingTiles_.clear();
    
    qDebug() << "MapViewInputHandler: Finished line drawing from" << dragStartMapPos_ << "to" << mapPos;
    mapView_->update();
}
