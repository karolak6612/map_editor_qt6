// MapDrawingPrimitives_Enhanced.cpp - Additional enhanced drawing methods for Task 72

#include "MapDrawingPrimitives.h"
#include "Brush.h"
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QFont>
#include <QFontMetrics>
#include <QtMath>

// Task 72: Additional enhanced drawing methods implementation

void MapDrawingPrimitives::drawHookIndicator(QPainter* painter, const QPointF& position, bool hookSouth, 
                                            bool hookEast, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QPointF adjustedPos = position + getFloorOffset(0, currentFloor);
    
    // Draw hook indicator (from wxwidgets map_drawer)
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(0, 0, 255, 200))); // Blue hook indicator
    
    if (hookSouth) {
        // Hook pointing south
        QPolygonF hookPolygon;
        hookPolygon << QPointF(adjustedPos.x() + 10, adjustedPos.y() + 10)
                   << QPointF(adjustedPos.x() + 20, adjustedPos.y() + 10)
                   << QPointF(adjustedPos.x() + 30, adjustedPos.y() + 20)
                   << QPointF(adjustedPos.x() + 20, adjustedPos.y() + 20);
        painter->drawPolygon(hookPolygon);
    } else if (hookEast) {
        // Hook pointing east
        QPolygonF hookPolygon;
        hookPolygon << QPointF(adjustedPos.x() + 20, adjustedPos.y() + 10)
                   << QPointF(adjustedPos.x() + 30, adjustedPos.y() + 20)
                   << QPointF(adjustedPos.x() + 30, adjustedPos.y() + 30)
                   << QPointF(adjustedPos.x() + 20, adjustedPos.y() + 20);
        painter->drawPolygon(hookPolygon);
    }
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawWaypointIndicator(QPainter* painter, const QPointF& position, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QPointF adjustedPos = position + getFloorOffset(0, currentFloor);
    QPointF center(adjustedPos.x() + options_.tileSize / 2, adjustedPos.y() + options_.tileSize / 2);
    
    // Draw waypoint flame (blue flame from wxwidgets)
    QLinearGradient flameGradient(center.x(), center.y() - 16, center.x(), center.y() + 16);
    flameGradient.setColorAt(0, QColor(100, 150, 255, 255)); // Light blue at top
    flameGradient.setColorAt(0.5, QColor(0, 100, 255, 255)); // Medium blue in middle
    flameGradient.setColorAt(1, QColor(0, 50, 200, 255));    // Dark blue at bottom
    
    // Create flame shape
    QPainterPath flamePath;
    flamePath.moveTo(center.x(), center.y() + 16);
    flamePath.quadTo(center.x() - 8, center.y() + 8, center.x() - 6, center.y() - 4);
    flamePath.quadTo(center.x() - 4, center.y() - 12, center.x(), center.y() - 16);
    flamePath.quadTo(center.x() + 4, center.y() - 12, center.x() + 6, center.y() - 4);
    flamePath.quadTo(center.x() + 8, center.y() + 8, center.x(), center.y() + 16);
    
    painter->fillPath(flamePath, QBrush(flameGradient));
    painter->setPen(QPen(QColor(0, 0, 150), 1));
    painter->drawPath(flamePath);
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawSpawnIndicator(QPainter* painter, const QPointF& position, bool selected, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QPointF adjustedPos = position + getFloorOffset(0, currentFloor);
    QPointF center(adjustedPos.x() + options_.tileSize / 2, adjustedPos.y() + options_.tileSize / 2);
    
    // Draw spawn flame (purple flame from wxwidgets)
    QColor flameColor = selected ? QColor(128, 128, 128, 255) : QColor(255, 0, 255, 255); // Gray if selected, purple otherwise
    
    QLinearGradient flameGradient(center.x(), center.y() - 16, center.x(), center.y() + 16);
    flameGradient.setColorAt(0, flameColor.lighter(150));
    flameGradient.setColorAt(0.5, flameColor);
    flameGradient.setColorAt(1, flameColor.darker(150));
    
    // Create flame shape
    QPainterPath flamePath;
    flamePath.moveTo(center.x(), center.y() + 16);
    flamePath.quadTo(center.x() - 8, center.y() + 8, center.x() - 6, center.y() - 4);
    flamePath.quadTo(center.x() - 4, center.y() - 12, center.x(), center.y() - 16);
    flamePath.quadTo(center.x() + 4, center.y() - 12, center.x() + 6, center.y() - 4);
    flamePath.quadTo(center.x() + 8, center.y() + 8, center.x(), center.y() + 16);
    
    painter->fillPath(flamePath, QBrush(flameGradient));
    painter->setPen(QPen(flameColor.darker(200), 1));
    painter->drawPath(flamePath);
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawHouseExitIndicator(QPainter* painter, const QPointF& position, bool current, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QPointF adjustedPos = position + getFloorOffset(0, currentFloor);
    QPointF center(adjustedPos.x() + options_.tileSize / 2, adjustedPos.y() + options_.tileSize / 2);
    
    // Draw house exit splash (blue splash from wxwidgets)
    QColor splashColor = current ? QColor(64, 255, 255, 255) : QColor(64, 64, 255, 255); // Cyan if current, blue otherwise
    
    // Create splash effect with multiple circles
    for (int i = 0; i < 3; ++i) {
        qreal radius = 4 + i * 3;
        qreal alpha = 255 - i * 60;
        QColor circleColor = splashColor;
        circleColor.setAlpha(alpha);
        
        QRectF circleRect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(circleColor));
        painter->drawEllipse(circleRect);
    }
    
    // Draw center dot
    QRectF centerDot(center.x() - 2, center.y() - 2, 4, 4);
    painter->setBrush(QBrush(splashColor));
    painter->drawEllipse(centerDot);
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawTownTempleIndicator(QPainter* painter, const QPointF& position, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QPointF adjustedPos = position + getFloorOffset(0, currentFloor);
    QPointF center(adjustedPos.x() + options_.tileSize / 2, adjustedPos.y() + options_.tileSize / 2);
    
    // Draw town temple flag (gray flag from wxwidgets)
    QColor flagColor(255, 255, 64, 170); // Yellow with transparency
    
    // Draw flag pole
    painter->setPen(QPen(QColor(100, 100, 100), 2));
    painter->drawLine(center.x(), center.y() - 16, center.x(), center.y() + 16);
    
    // Draw flag
    QPainterPath flagPath;
    flagPath.moveTo(center.x(), center.y() - 16);
    flagPath.lineTo(center.x() + 12, center.y() - 12);
    flagPath.lineTo(center.x() + 8, center.y() - 8);
    flagPath.lineTo(center.x(), center.y() - 4);
    flagPath.closeSubpath();
    
    painter->fillPath(flagPath, QBrush(flagColor));
    painter->setPen(QPen(QColor(200, 200, 0), 1));
    painter->drawPath(flagPath);
    
    restorePainter(painter);
}

// Task 72: Advanced brush preview drawing
void MapDrawingPrimitives::drawRawBrushPreview(QPainter* painter, const QPointF& position, quint16 itemId, 
                                              const QColor& color, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QRectF rect = getTileRect(position, currentFloor);
    
    // Draw raw brush preview with item ID
    painter->fillRect(rect, color);
    painter->setPen(QPen(color.darker(150), 2));
    painter->drawRect(rect);
    
    // Draw item ID text
    painter->setPen(QColor(255, 255, 255));
    painter->setFont(QFont("Arial", 8, QFont::Bold));
    painter->drawText(rect, Qt::AlignCenter, QString::number(itemId));
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawWaypointBrushPreview(QPainter* painter, const QPointF& position, Brush* brush,
                                                   BrushValidity validity, int currentFloor) {
    if (!painter || !brush) return;
    
    QColor color = (validity == BrushValidity::Valid) ? QColor(0, 255, 0, 128) : QColor(255, 0, 0, 128);
    
    // Draw waypoint brush indicator
    drawBrushIndicator(painter, position, brush, color, currentFloor);
}

void MapDrawingPrimitives::drawHouseBrushPreview(QPainter* painter, const QPointF& position, quint32 houseId,
                                                BrushValidity validity, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QRectF rect = getTileRect(position, currentFloor);
    QColor color = getBrushColor(nullptr, validity);
    
    // Draw house brush preview
    painter->fillRect(rect, color);
    painter->setPen(QPen(color.darker(150), 2));
    painter->drawRect(rect);
    
    // Draw house ID
    painter->setPen(QColor(255, 255, 255));
    painter->setFont(QFont("Arial", 8, QFont::Bold));
    painter->drawText(rect, Qt::AlignCenter, QString("H%1").arg(houseId));
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawSpawnBrushPreview(QPainter* painter, const QPointF& position, const QString& spawnName,
                                                BrushValidity validity, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QRectF rect = getTileRect(position, currentFloor);
    QColor color = getBrushColor(nullptr, validity);
    
    // Draw spawn brush preview
    painter->fillRect(rect, color);
    painter->setPen(QPen(color.darker(150), 2));
    painter->drawRect(rect);
    
    // Draw spawn name initial
    painter->setPen(QColor(255, 255, 255));
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    QString initial = spawnName.isEmpty() ? "S" : spawnName.left(1).toUpper();
    painter->drawText(rect, Qt::AlignCenter, initial);
    
    restorePainter(painter);
}

// Task 72: Enhanced selection drawing
void MapDrawingPrimitives::drawSelectionHighlight(QPainter* painter, const QRectF& area, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QRectF adjustedArea = mapToScene(area, currentFloor);
    
    // Draw selection highlight with animated effect
    QColor highlightColor = options_.selectionColor;
    highlightColor.setAlpha(100);
    
    painter->fillRect(adjustedArea, highlightColor);
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawSelectionBounds(QPainter* painter, const QRectF& bounds, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QRectF adjustedBounds = mapToScene(bounds, currentFloor);
    
    // Draw selection bounds with dashed border
    QPen borderPen(options_.selectionBorderColor, options_.selectionBorderWidth, options_.selectionBorderStyle);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(adjustedBounds);
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawSelectionHandles(QPainter* painter, const QRectF& bounds, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QRectF adjustedBounds = mapToScene(bounds, currentFloor);
    
    // Draw selection handles at corners and edges
    qreal handleSize = 6;
    QColor handleColor = options_.selectionBorderColor;
    
    QVector<QPointF> handlePositions = {
        adjustedBounds.topLeft(),
        QPointF(adjustedBounds.center().x(), adjustedBounds.top()),
        adjustedBounds.topRight(),
        QPointF(adjustedBounds.right(), adjustedBounds.center().y()),
        adjustedBounds.bottomRight(),
        QPointF(adjustedBounds.center().x(), adjustedBounds.bottom()),
        adjustedBounds.bottomLeft(),
        QPointF(adjustedBounds.left(), adjustedBounds.center().y())
    };
    
    painter->setPen(QPen(handleColor.darker(150), 1));
    painter->setBrush(QBrush(handleColor));
    
    for (const QPointF& pos : handlePositions) {
        QRectF handleRect(pos.x() - handleSize/2, pos.y() - handleSize/2, handleSize, handleSize);
        painter->drawRect(handleRect);
    }
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawDragPreviewAdvanced(QPainter* painter, const QRectF& area, Brush* brush,
                                                  BrushShape shape, qreal opacity, int currentFloor) {
    if (!painter) return;
    
    setupPainter(painter);
    
    // Set opacity for drag preview
    painter->setOpacity(opacity);
    
    QColor dragColor = getBrushColor(brush, BrushValidity::Neutral);
    dragColor.setAlpha(static_cast<int>(255 * opacity));
    
    if (shape == BrushShape::Circle) {
        drawCircleDragPreview(painter, area, dragColor, currentFloor);
    } else {
        drawRectangleDragPreview(painter, area, dragColor, currentFloor);
    }
    
    restorePainter(painter);
}
