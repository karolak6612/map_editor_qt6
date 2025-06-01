// MapViewInputHandlerTileLogic.cpp - Tile/Area identification methods for Task 34
// This file contains the comprehensive tile area calculation and coordinate conversion logic

#include "MapViewInputHandler.h"
#include "MapView.h"
#include "BrushManager.h"
#include "Brush.h"
#include <QDebug>
#include <QtMath>

// Tile/Area identification methods (Task 34 requirement)

QList<QPointF> MapViewInputHandler::getAffectedTiles(const QPointF& primaryMapPos, Brush* currentBrush) const {
    if (!currentBrush) {
        return QList<QPointF>() << primaryMapPos;
    }
    
    int brushSize = currentBrush->getBrushSize();
    Brush::BrushShape shape = currentBrush->getBrushShape();
    
    if (brushSize <= 1) {
        // Single tile
        return QList<QPointF>() << snapToTileGrid(primaryMapPos);
    }
    
    // Multi-tile brush
    QPointF centerTile = snapToTileGrid(primaryMapPos);
    
    if (shape == Brush::BrushShape::Circle) {
        return getTilesInCircle(centerTile, brushSize);
    } else {
        // Square brush
        int halfSize = brushSize / 2;
        QPointF topLeft = centerTile - QPointF(halfSize, halfSize);
        QPointF bottomRight = centerTile + QPointF(halfSize, halfSize);
        return getTilesInRectangle(topLeft, bottomRight);
    }
}

QList<QPointF> MapViewInputHandler::getTilesInArea(const QPointF& startPos, const QPointF& endPos, Brush::BrushShape shape) const {
    if (shape == Brush::BrushShape::Circle) {
        // Calculate radius from distance
        QPointF center = (startPos + endPos) / 2.0;
        qreal radius = QLineF(startPos, endPos).length() / 2.0;
        return getTilesInCircle(center, qRound(radius));
    } else {
        // Rectangle
        return getTilesInRectangle(startPos, endPos);
    }
}

QList<QPointF> MapViewInputHandler::getTilesInRectangle(const QPointF& startPos, const QPointF& endPos) const {
    QList<QPointF> tiles;
    
    // Normalize rectangle
    QRectF rect = QRectF(startPos, endPos).normalized();
    
    // Snap to tile grid
    QPointF topLeft = snapToTileGrid(rect.topLeft());
    QPointF bottomRight = snapToTileGrid(rect.bottomRight());
    
    // Generate all tiles in rectangle
    for (int x = qRound(topLeft.x()); x <= qRound(bottomRight.x()); ++x) {
        for (int y = qRound(topLeft.y()); y <= qRound(bottomRight.y()); ++y) {
            QPointF tile(x, y);
            if (isValidTilePosition(tile)) {
                tiles.append(tile);
            }
        }
    }
    
    return tiles;
}

QList<QPointF> MapViewInputHandler::getTilesInCircle(const QPointF& centerPos, int radius) const {
    QList<QPointF> tiles;
    
    if (radius <= 0) {
        QPointF centerTile = snapToTileGrid(centerPos);
        if (isValidTilePosition(centerTile)) {
            tiles.append(centerTile);
        }
        return tiles;
    }
    
    QPointF center = snapToTileGrid(centerPos);
    int centerX = qRound(center.x());
    int centerY = qRound(center.y());
    
    // Use circle equation: (x-cx)² + (y-cy)² <= r²
    int radiusSquared = radius * radius;
    
    for (int x = centerX - radius; x <= centerX + radius; ++x) {
        for (int y = centerY - radius; y <= centerY + radius; ++y) {
            int dx = x - centerX;
            int dy = y - centerY;
            
            if (dx * dx + dy * dy <= radiusSquared) {
                QPointF tile(x, y);
                if (isValidTilePosition(tile)) {
                    tiles.append(tile);
                }
            }
        }
    }
    
    return tiles;
}

QList<QPointF> MapViewInputHandler::getTilesInLine(const QPointF& startPos, const QPointF& endPos) const {
    QList<QPointF> tiles;
    
    QPointF start = snapToTileGrid(startPos);
    QPointF end = snapToTileGrid(endPos);
    
    // Bresenham's line algorithm for tile coordinates
    int x0 = qRound(start.x());
    int y0 = qRound(start.y());
    int x1 = qRound(end.x());
    int y1 = qRound(end.y());
    
    int dx = qAbs(x1 - x0);
    int dy = qAbs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    int x = x0;
    int y = y0;
    
    while (true) {
        QPointF tile(x, y);
        if (isValidTilePosition(tile)) {
            tiles.append(tile);
        }
        
        if (x == x1 && y == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
    
    return tiles;
}

// Coordinate conversion and validation

QPointF MapViewInputHandler::screenToMapTile(const QPoint& screenPos) const {
    if (!mapView_) return QPointF();
    
    // Convert screen coordinates to map coordinates
    QPointF mapPos = mapView_->screenToMap(screenPos);
    
    // Snap to tile grid
    return snapToTileGrid(mapPos);
}

QPoint MapViewInputHandler::mapTileToScreen(const QPointF& mapTilePos) const {
    if (!mapView_) return QPoint();
    
    // Convert map tile coordinates to screen coordinates
    return mapView_->mapToScreen(mapTilePos);
}

bool MapViewInputHandler::isValidTilePosition(const QPointF& mapPos) const {
    if (!map_) return false;
    
    // Check if position is within map bounds
    int x = qRound(mapPos.x());
    int y = qRound(mapPos.y());
    
    return (x >= 0 && y >= 0 && x < map_->getWidth() && y < map_->getHeight());
}

QPointF MapViewInputHandler::snapToTileGrid(const QPointF& mapPos) const {
    // Snap to nearest tile center
    return QPointF(qRound(mapPos.x()), qRound(mapPos.y()));
}

// State machine management

void MapViewInputHandler::transitionToMode(InteractionMode newMode) {
    if (currentMode_ == newMode) return;
    
    if (!canTransitionTo(newMode)) {
        qWarning() << "MapViewInputHandler::transitionToMode: Invalid transition from" 
                   << static_cast<int>(currentMode_) << "to" << static_cast<int>(newMode);
        return;
    }
    
    previousMode_ = currentMode_;
    currentMode_ = newMode;
    
    qDebug() << "MapViewInputHandler: Transitioned from mode" << static_cast<int>(previousMode_) 
             << "to mode" << static_cast<int>(newMode);
}

void MapViewInputHandler::updateBrushState(BrushState newState) {
    if (currentBrushState_ == newState) return;
    
    currentBrushState_ = newState;
    
    qDebug() << "MapViewInputHandler: Updated brush state to" << static_cast<int>(newState);
}

bool MapViewInputHandler::canTransitionTo(InteractionMode newMode) const {
    // Define valid state transitions
    switch (currentMode_) {
        case InteractionMode::Idle:
            return true; // Can transition to any mode from idle
            
        case InteractionMode::Drawing:
            return (newMode == InteractionMode::Idle || 
                    newMode == InteractionMode::DraggingDraw ||
                    newMode == InteractionMode::BrushPreview);
            
        case InteractionMode::DraggingDraw:
            return (newMode == InteractionMode::Idle || 
                    newMode == InteractionMode::Drawing);
            
        case InteractionMode::SelectingBox:
            return (newMode == InteractionMode::Idle || 
                    newMode == InteractionMode::DraggingSelection);
            
        case InteractionMode::PanningView:
            return (newMode == InteractionMode::Idle);
            
        case InteractionMode::BrushPreview:
            return true; // Preview can transition to any mode
            
        default:
            return (newMode == InteractionMode::Idle); // Default: can only go to idle
    }
}

void MapViewInputHandler::resetInteractionState() {
    previousMode_ = currentMode_;
    currentMode_ = InteractionMode::Idle;
    currentBrushState_ = BrushState::Idle;
    
    // Reset all state flags
    isDragging_ = false;
    isDrawing_ = false;
    isDraggingDraw_ = false;
    isReplaceDragging_ = false;
    isScreenDragging_ = false;
    isBoundBoxSelection_ = false;
    isContextMenuActive_ = false;
    isBrushPreviewActive_ = false;
    
    // Clear drawing state
    currentDrawingTiles_.clear();
    previewTiles_.clear();
    currentDrawingArea_ = QRectF();
    
    // Clear positions
    dragStartMapPos_ = QPointF();
    dragCurrentMapPos_ = QPointF();
    dragEndMapPos_ = QPointF();
    
    qDebug() << "MapViewInputHandler: Reset interaction state";
}

// Visual feedback and preview

void MapViewInputHandler::updateBrushPreview(const QPointF& mapPos) {
    if (!mapView_ || !brushManager_) return;
    
    Brush* currentBrush = brushManager_->getCurrentBrush();
    if (!currentBrush) {
        clearBrushPreview();
        return;
    }
    
    brushPreviewPos_ = snapToTileGrid(mapPos);
    brushPreviewSize_ = currentBrush->getBrushSize();
    isBrushPreviewActive_ = true;
    
    // Get preview tiles
    previewTiles_ = getAffectedTiles(brushPreviewPos_, currentBrush);
    
    // Update visual feedback (this would trigger a repaint with preview overlay)
    mapView_->update();
}

void MapViewInputHandler::clearBrushPreview() {
    if (!isBrushPreviewActive_) return;
    
    isBrushPreviewActive_ = false;
    previewTiles_.clear();
    brushPreviewPos_ = QPointF();
    
    if (mapView_) {
        mapView_->update();
    }
}

void MapViewInputHandler::updateSelectionFeedback(const QRectF& area) {
    selectionFeedbackRect_ = area;
    
    if (mapView_) {
        mapView_->update();
    }
}

void MapViewInputHandler::clearSelectionFeedback() {
    selectionFeedbackRect_ = QRectF();
    
    if (mapView_) {
        mapView_->update();
    }
}
