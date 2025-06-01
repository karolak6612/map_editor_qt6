#include "MapDrawingPrimitives.h"
#include "Brush.h"
#include <QDebug>
#include <QtMath>

// MapDrawingPrimitives implementation
MapDrawingPrimitives::MapDrawingPrimitives(const DrawingOptions& options)
    : options_(options) {
}

void MapDrawingPrimitives::setDrawingOptions(const DrawingOptions& options) {
    options_ = options;
}

void MapDrawingPrimitives::setupPainter(QPainter* painter, bool useAntialiasing) const {
    if (!painter) return;
    
    painter->save();
    
    if (useAntialiasing && options_.useAntialiasing) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    }
    
    // Set global alpha if specified
    if (options_.globalAlpha < 255) {
        painter->setOpacity(options_.globalAlpha / 255.0);
    }
}

void MapDrawingPrimitives::restorePainter(QPainter* painter) const {
    if (painter) {
        painter->restore();
    }
}

QPen MapDrawingPrimitives::createPen(const QColor& color, int width, Qt::PenStyle style) const {
    QPen pen(color, width, style);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    return pen;
}

QBrush MapDrawingPrimitives::createBrush(const QColor& color, Qt::BrushStyle style) const {
    return QBrush(color, style);
}

// Basic shape drawing
void MapDrawingPrimitives::drawRect(QPainter* painter, const QRectF& rect, const QColor& color, 
                                   int borderWidth, Qt::PenStyle borderStyle) {
    if (!painter || rect.isEmpty()) return;
    
    setupPainter(painter);
    
    QPen pen = createPen(color, borderWidth, borderStyle);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawFilledRect(QPainter* painter, const QRectF& rect, const QColor& fillColor, 
                                         const QColor& borderColor, int borderWidth) {
    if (!painter || rect.isEmpty()) return;
    
    setupPainter(painter);
    
    // Fill the rectangle
    QBrush brush = createBrush(fillColor);
    painter->setBrush(brush);
    
    if (borderColor.isValid() && borderWidth > 0) {
        QPen pen = createPen(borderColor, borderWidth);
        painter->setPen(pen);
    } else {
        painter->setPen(Qt::NoPen);
    }
    
    painter->drawRect(rect);
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawLine(QPainter* painter, const QPointF& start, const QPointF& end, 
                                   const QColor& color, int width, Qt::PenStyle style) {
    if (!painter) return;
    
    setupPainter(painter);
    
    QPen pen = createPen(color, width, style);
    painter->setPen(pen);
    painter->drawLine(start, end);
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawEllipse(QPainter* painter, const QRectF& rect, const QColor& color, 
                                      int borderWidth, Qt::PenStyle borderStyle) {
    if (!painter || rect.isEmpty()) return;
    
    setupPainter(painter);
    
    QPen pen = createPen(color, borderWidth, borderStyle);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(rect);
    
    restorePainter(painter);
}

void MapDrawingPrimitives::drawFilledEllipse(QPainter* painter, const QRectF& rect, const QColor& fillColor, 
                                            const QColor& borderColor, int borderWidth) {
    if (!painter || rect.isEmpty()) return;
    
    setupPainter(painter);
    
    // Fill the ellipse
    QBrush brush = createBrush(fillColor);
    painter->setBrush(brush);
    
    if (borderColor.isValid() && borderWidth > 0) {
        QPen pen = createPen(borderColor, borderWidth);
        painter->setPen(pen);
    } else {
        painter->setPen(Qt::NoPen);
    }
    
    painter->drawEllipse(rect);
    
    restorePainter(painter);
}

// Grid drawing
void MapDrawingPrimitives::drawGrid(QPainter* painter, const QRectF& viewRect, int currentFloor) {
    if (!painter || !options_.showGrid) return;
    
    drawGridLines(painter, viewRect, options_.gridColor, options_.gridLineWidth, options_.tileSize);
}

void MapDrawingPrimitives::drawGridLines(QPainter* painter, const QRectF& viewRect, 
                                        const QColor& color, int lineWidth, int spacing) {
    if (!painter || spacing <= 0) return;
    
    setupPainter(painter, false); // No antialiasing for grid lines
    
    QPen pen = createPen(color, lineWidth);
    painter->setPen(pen);
    
    // Draw vertical lines
    int startX = static_cast<int>(viewRect.left() / spacing) * spacing;
    for (int x = startX; x <= viewRect.right(); x += spacing) {
        painter->drawLine(x, viewRect.top(), x, viewRect.bottom());
    }
    
    // Draw horizontal lines
    int startY = static_cast<int>(viewRect.top() / spacing) * spacing;
    for (int y = startY; y <= viewRect.bottom(); y += spacing) {
        painter->drawLine(viewRect.left(), y, viewRect.right(), y);
    }
    
    restorePainter(painter);
}

// Selection drawing
void MapDrawingPrimitives::drawSelectionBox(QPainter* painter, const QRectF& selectionRect, int currentFloor) {
    if (!painter || selectionRect.isEmpty()) return;
    
    // Apply floor offset
    QRectF adjustedRect = selectionRect;
    if (options_.show3DEffect) {
        QPointF offset = getFloorOffset(currentFloor, currentFloor);
        adjustedRect.translate(offset);
    }
    
    // Draw filled selection area
    drawFilledRect(painter, adjustedRect, options_.selectionColor, 
                  options_.selectionBorderColor, options_.selectionBorderWidth);
}

void MapDrawingPrimitives::drawSelectionArea(QPainter* painter, const QVector<QRectF>& tiles, int currentFloor) {
    if (!painter || tiles.isEmpty()) return;
    
    for (const QRectF& tile : tiles) {
        drawSelectionBox(painter, tile, currentFloor);
    }
}

void MapDrawingPrimitives::drawSelectionBorder(QPainter* painter, const QRectF& rect, int currentFloor) {
    if (!painter || rect.isEmpty()) return;
    
    // Apply floor offset
    QRectF adjustedRect = rect;
    if (options_.show3DEffect) {
        QPointF offset = getFloorOffset(currentFloor, currentFloor);
        adjustedRect.translate(offset);
    }
    
    drawRect(painter, adjustedRect, options_.selectionBorderColor, 
             options_.selectionBorderWidth, options_.selectionBorderStyle);
}

// Brush preview drawing
void MapDrawingPrimitives::drawBrushPreview(QPainter* painter, const QPointF& centerPos, Brush* brush, 
                                           int brushSize, BrushShape shape, BrushValidity validity, 
                                           int currentFloor) {
    if (!painter || !options_.showBrushPreview) return;
    
    QColor brushColor = getBrushColor(brush, validity);
    
    switch (shape) {
        case BrushShape::Square:
            drawSquareBrushPreview(painter, centerPos, brushSize, brushColor, currentFloor);
            break;
        case BrushShape::Circle:
            drawCircleBrushPreview(painter, centerPos, brushSize, brushColor, currentFloor);
            break;
    }
}

void MapDrawingPrimitives::drawSquareBrushPreview(QPainter* painter, const QPointF& centerPos, int brushSize, 
                                                 const QColor& color, int currentFloor) {
    if (!painter) return;
    
    QVector<QPointF> tiles = getSquareTiles(centerPos, brushSize);
    
    for (const QPointF& tilePos : tiles) {
        QRectF tileRect = getTileRect(tilePos, currentFloor);
        drawFilledRect(painter, tileRect, color, color.darker(150), 1);
    }
}

void MapDrawingPrimitives::drawCircleBrushPreview(QPainter* painter, const QPointF& centerPos, int brushSize, 
                                                 const QColor& color, int currentFloor) {
    if (!painter) return;
    
    QVector<QPointF> tiles = getCircleTiles(centerPos, brushSize);
    
    for (const QPointF& tilePos : tiles) {
        QRectF tileRect = getTileRect(tilePos, currentFloor);
        drawFilledRect(painter, tileRect, color, color.darker(150), 1);
    }
}

void MapDrawingPrimitives::drawBrushTile(QPainter* painter, const QPointF& tilePos, const QColor& color, 
                                        BrushValidity validity, int currentFloor) {
    if (!painter) return;
    
    QRectF tileRect = getTileRect(tilePos, currentFloor);
    QColor finalColor = color;
    
    // Modify color based on validity
    switch (validity) {
        case BrushValidity::Valid:
            finalColor = options_.validBrushColor;
            break;
        case BrushValidity::Invalid:
            finalColor = options_.invalidBrushColor;
            break;
        case BrushValidity::Neutral:
        default:
            // Use provided color
            break;
    }
    
    drawFilledRect(painter, tileRect, finalColor, finalColor.darker(150), 1);
}

// Drag drawing preview
void MapDrawingPrimitives::drawDragPreview(QPainter* painter, const QRectF& dragArea, Brush* brush, 
                                          BrushShape shape, int currentFloor) {
    if (!painter || dragArea.isEmpty()) return;
    
    QColor brushColor = getBrushColor(brush, BrushValidity::Neutral);
    
    switch (shape) {
        case BrushShape::Square:
            drawRectangleDragPreview(painter, dragArea, brushColor, currentFloor);
            break;
        case BrushShape::Circle:
            drawCircleDragPreview(painter, dragArea, brushColor, currentFloor);
            break;
    }
}

void MapDrawingPrimitives::drawRectangleDragPreview(QPainter* painter, const QRectF& dragArea, 
                                                   const QColor& color, int currentFloor) {
    if (!painter || dragArea.isEmpty()) return;
    
    // Apply floor offset
    QRectF adjustedArea = dragArea;
    if (options_.show3DEffect) {
        QPointF offset = getFloorOffset(currentFloor, currentFloor);
        adjustedArea.translate(offset);
    }
    
    drawFilledRect(painter, adjustedArea, color, color.darker(150), options_.brushPreviewBorderWidth);
}

void MapDrawingPrimitives::drawCircleDragPreview(QPainter* painter, const QRectF& dragArea, 
                                                const QColor& color, int currentFloor) {
    if (!painter || dragArea.isEmpty()) return;
    
    // Apply floor offset
    QRectF adjustedArea = dragArea;
    if (options_.show3DEffect) {
        QPointF offset = getFloorOffset(currentFloor, currentFloor);
        adjustedArea.translate(offset);
    }
    
    drawFilledEllipse(painter, adjustedArea, color, color.darker(150), options_.brushPreviewBorderWidth);
}

// Special drawing
void MapDrawingPrimitives::drawIngameBox(QPainter* painter, const QRectF& viewRect, int currentFloor) {
    if (!painter || !options_.showIngameBox) return;
    
    // Draw ingame boundary box (typically for showing playable area)
    QRectF ingameRect = viewRect; // This would be calculated based on actual ingame boundaries
    drawRect(painter, ingameRect, options_.ingameBoxColor, 2, Qt::DashDotLine);
}

void MapDrawingPrimitives::drawFloorIndicator(QPainter* painter, const QPointF& pos, int floor, int currentFloor) {
    if (!painter || !options_.show3DEffect) return;

    // Draw a small indicator showing the floor level
    QPointF adjustedPos = pos + getFloorOffset(floor, currentFloor);
    QRectF indicatorRect(adjustedPos.x() - 2, adjustedPos.y() - 2, 4, 4);

    QColor floorColor = QColor::fromHsv((floor * 30) % 360, 200, 200, 180);
    drawFilledEllipse(painter, indicatorRect, floorColor);
}

// Task 72: Enhanced drawing methods (OpenGL replacement)
void MapDrawingPrimitives::drawTooltips(QPainter* painter, const QList<MapTooltip>& tooltips, int currentFloor) {
    if (!painter || tooltips.isEmpty()) {
        return;
    }

    setupPainter(painter);

    for (const MapTooltip& tooltip : tooltips) {
        drawSingleTooltip(painter, tooltip, currentFloor);
    }

    restorePainter(painter);
}

void MapDrawingPrimitives::drawSingleTooltip(QPainter* painter, const MapTooltip& tooltip, int currentFloor) {
    if (tooltip.text.isEmpty()) {
        return;
    }

    // Calculate text metrics
    QFont font("Arial", 10);
    QFontMetrics metrics(font);

    QStringList lines = tooltip.text.split('\n');
    qreal maxWidth = 0;
    qreal totalHeight = 0;

    // Process lines and handle word wrapping
    QStringList processedLines;
    for (const QString& line : lines) {
        if (line.length() <= tooltip.maxCharsPerLine) {
            processedLines.append(line);
            maxWidth = qMax(maxWidth, (qreal)metrics.horizontalAdvance(line));
            totalHeight += metrics.height();
        } else {
            // Word wrap long lines
            QStringList words = line.split(' ');
            QString currentLine;

            for (const QString& word : words) {
                QString testLine = currentLine.isEmpty() ? word : currentLine + " " + word;
                if (testLine.length() <= tooltip.maxCharsPerLine) {
                    currentLine = testLine;
                } else {
                    if (!currentLine.isEmpty()) {
                        processedLines.append(currentLine);
                        maxWidth = qMax(maxWidth, (qreal)metrics.horizontalAdvance(currentLine));
                        totalHeight += metrics.height();
                    }
                    currentLine = word;
                }
            }

            if (!currentLine.isEmpty()) {
                processedLines.append(currentLine);
                maxWidth = qMax(maxWidth, (qreal)metrics.horizontalAdvance(currentLine));
                totalHeight += metrics.height();
            }
        }
    }

    // Add ellipsis if needed
    if (tooltip.ellipsis && processedLines.size() > 5) {
        processedLines = processedLines.mid(0, 5);
        processedLines.last() += "...";
    }

    // Calculate tooltip dimensions
    qreal padding = 8;
    qreal width = maxWidth + 2 * padding;
    qreal height = totalHeight + 2 * padding;

    // Calculate position (above the tile with arrow pointing down)
    QPointF adjustedPos = tooltip.position + getFloorOffset(0, currentFloor);
    qreal centerX = adjustedPos.x() + options_.tileSize / 2;
    qreal topY = adjustedPos.y() - height - 10; // 10px gap above tile

    // Create tooltip shape (speech bubble)
    QPainterPath tooltipPath;
    QRectF tooltipRect(centerX - width/2, topY, width, height);

    // Rounded rectangle for main body
    tooltipPath.addRoundedRect(tooltipRect, 5, 5);

    // Arrow pointing down to tile
    QPointF arrowTip(centerX, adjustedPos.y() - 2);
    QPointF arrowLeft(centerX - 8, topY + height);
    QPointF arrowRight(centerX + 8, topY + height);

    tooltipPath.moveTo(arrowLeft);
    tooltipPath.lineTo(arrowTip);
    tooltipPath.lineTo(arrowRight);
    tooltipPath.closeSubpath();

    // Draw tooltip background
    painter->fillPath(tooltipPath, QBrush(tooltip.color));
    painter->setPen(QPen(QColor(0, 0, 0), 1));
    painter->drawPath(tooltipPath);

    // Draw text
    painter->setPen(QColor(0, 0, 0));
    painter->setFont(font);

    qreal textY = tooltipRect.top() + padding + metrics.ascent();
    for (const QString& line : processedLines) {
        qreal textX = tooltipRect.left() + padding;
        painter->drawText(QPointF(textX, textY), line);
        textY += metrics.height();
    }
}

void MapDrawingPrimitives::drawHoverIndicator(QPainter* painter, const QPointF& position, int currentFloor) {
    if (!painter) return;

    setupPainter(painter);

    QRectF rect = getTileRect(position, currentFloor);

    // Draw subtle hover highlight
    QColor hoverColor(255, 255, 255, 64);
    painter->fillRect(rect, hoverColor);

    // Draw border
    painter->setPen(QPen(QColor(255, 255, 255, 128), 1));
    painter->drawRect(rect);

    restorePainter(painter);
}

void MapDrawingPrimitives::drawBrushIndicator(QPainter* painter, const QPointF& position, Brush* brush,
                                             const QColor& color, int currentFloor) {
    if (!painter || !brush) {
        return;
    }

    setupPainter(painter);

    QPointF adjustedPos = position + getFloorOffset(0, currentFloor);
    QPointF center(adjustedPos.x() + options_.tileSize / 2, adjustedPos.y() + options_.tileSize / 2);

    // Draw brush indicator shape (from wxwidgets map_drawer)
    // 7----0----1
    // |         |
    // 6--5  3--2
    //     \/
    //     4

    QVector<QPointF> vertices = {
        QPointF(center.x(), center.y() - 20),        // 0
        QPointF(center.x() + 15, center.y() - 20),   // 1
        QPointF(center.x() + 15, center.y() - 5),    // 2
        QPointF(center.x() + 5, center.y() - 5),     // 3
        QPointF(center.x(), center.y()),             // 4 (tip)
        QPointF(center.x() - 5, center.y() - 5),     // 5
        QPointF(center.x() - 15, center.y() - 5),    // 6
        QPointF(center.x() - 15, center.y() - 20)    // 7
    };

    // Draw background circle
    QRectF circleRect(center.x() - options_.tileSize/2, center.y() - options_.tileSize/2,
                     options_.tileSize, options_.tileSize);
    painter->fillEllipse(circleRect, QColor(0, 0, 0, 80));

    // Draw indicator polygon
    QPolygonF polygon(vertices);
    painter->fillPath(QPainterPath().addPolygon(polygon), QBrush(color));

    // Draw border
    painter->setPen(QPen(QColor(0, 0, 0, 180), 1));
    painter->drawPolygon(polygon);

    restorePainter(painter);
}

// Task 76: Enhanced brush indicator shape drawing
void MapDrawingPrimitives::drawBrushIndicatorShape(QPainter* painter, const QPointF& center,
                                                  Brush* brush, const QColor& color) {
    if (!painter || !brush) return;

    // Draw brush indicator shape (from wxwidgets map_drawer)
    // 7----0----1
    // |         |
    // 6--5  3--2
    //     \/
    //     4

    QVector<QPointF> vertices = {
        QPointF(center.x(), center.y() - 20),        // 0
        QPointF(center.x() + 15, center.y() - 20),   // 1
        QPointF(center.x() + 15, center.y() - 5),    // 2
        QPointF(center.x() + 5, center.y() - 5),     // 3
        QPointF(center.x(), center.y()),             // 4 (tip)
        QPointF(center.x() - 5, center.y() - 5),     // 5
        QPointF(center.x() - 15, center.y() - 5),    // 6
        QPointF(center.x() - 15, center.y() - 20)    // 7
    };

    QPolygonF polygon(vertices);

    // Fill the indicator with gradient for better visibility
    QLinearGradient gradient(center.x() - 15, center.y() - 20, center.x() + 15, center.y());
    gradient.setColorAt(0.0, color.lighter(120));
    gradient.setColorAt(1.0, color);

    painter->setBrush(QBrush(gradient));
    painter->setPen(QPen(color.darker(150), 2));
    painter->drawPolygon(polygon);
}

// Task 76: Draw brush size indicator
void MapDrawingPrimitives::drawBrushSizeIndicator(QPainter* painter, const QPointF& center,
                                                 Brush* brush, const QColor& color) {
    if (!painter || !brush) return;

    int size = brush->getSize();
    if (size <= 1) return;

    // Draw size indicator as circles around the main indicator
    qreal radius = 10 + (size * 3);
    QColor sizeColor = color;
    sizeColor.setAlpha(100);

    painter->setPen(QPen(sizeColor, 1, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(center, radius, radius);

    // Draw size text
    painter->setPen(QPen(color.darker(150), 1));
    painter->setFont(QFont("Arial", 8, QFont::Bold));
    QRectF textRect(center.x() + 20, center.y() - 25, 20, 15);
    painter->drawText(textRect, Qt::AlignCenter, QString("%1x%1").arg(size));
}

// Task 76: Draw brush type indicator
void MapDrawingPrimitives::drawBrushTypeIndicator(QPainter* painter, const QPointF& center,
                                                 Brush* brush, const QColor& color) {
    if (!painter || !brush) return;

    QString typeIndicator;
    QColor typeColor = color.darker(150);

    // Get brush type indicator
    if (brush->getName().contains("RAW", Qt::CaseInsensitive)) {
        typeIndicator = "R";
        typeColor = QColor(255, 100, 100);
    } else if (brush->getName().contains("Border", Qt::CaseInsensitive)) {
        typeIndicator = "B";
        typeColor = QColor(100, 255, 100);
    } else if (brush->getName().contains("Wall", Qt::CaseInsensitive)) {
        typeIndicator = "W";
        typeColor = QColor(100, 100, 255);
    } else if (brush->getName().contains("Door", Qt::CaseInsensitive)) {
        typeIndicator = "D";
        typeColor = QColor(255, 255, 100);
    } else if (brush->getName().contains("House", Qt::CaseInsensitive)) {
        typeIndicator = "H";
        typeColor = QColor(255, 100, 255);
    } else if (brush->getName().contains("Waypoint", Qt::CaseInsensitive)) {
        typeIndicator = "P";
        typeColor = QColor(100, 255, 255);
    } else {
        typeIndicator = "?";
    }

    if (!typeIndicator.isEmpty()) {
        painter->setPen(QPen(typeColor, 1));
        painter->setFont(QFont("Arial", 10, QFont::Bold));
        QRectF typeRect(center.x() - 25, center.y() - 25, 15, 15);
        painter->drawText(typeRect, Qt::AlignCenter, typeIndicator);
    }
}

// Utility methods
QColor MapDrawingPrimitives::getBrushColor(Brush* brush, BrushValidity validity) const {
    // Return color based on validity first
    switch (validity) {
        case BrushValidity::Valid:
            return options_.validBrushColor;
        case BrushValidity::Invalid:
            return options_.invalidBrushColor;
        case BrushValidity::Neutral:
        default:
            break;
    }
    
    // Return color based on brush type (from wxwidgets logic)
    if (!brush) {
        return options_.neutralBrushColor;
    }
    
    // This would need to be implemented based on actual Brush class methods
    // For now, return default colors based on brush type names
    QString brushName = brush->getName().toLower();
    
    if (brushName.contains("house")) {
        return options_.houseBrushColor;
    } else if (brushName.contains("flag")) {
        return options_.flagBrushColor;
    } else if (brushName.contains("spawn")) {
        return options_.spawnBrushColor;
    } else if (brushName.contains("eraser")) {
        return options_.eraserBrushColor;
    }
    
    return options_.neutralBrushColor;
}

QRectF MapDrawingPrimitives::getTileRect(const QPointF& tilePos, int currentFloor) const {
    QPointF scenePos = mapToScene(tilePos, currentFloor);
    return QRectF(scenePos.x(), scenePos.y(), options_.tileSize, options_.tileSize);
}

QPointF MapDrawingPrimitives::getFloorOffset(int floor, int currentFloor) const {
    if (!options_.show3DEffect) {
        return QPointF(0, 0);
    }
    
    int floorDiff = currentFloor - floor;
    return QPointF(floorDiff * options_.floorOffset, floorDiff * options_.floorOffset);
}

QVector<QPointF> MapDrawingPrimitives::getCircleTiles(const QPointF& center, int radius) const {
    QVector<QPointF> tiles;
    
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            double distance = qSqrt(x * x + y * y);
            if (distance <= radius + 0.005) { // Small epsilon for floating point comparison
                tiles.append(QPointF(center.x() + x, center.y() + y));
            }
        }
    }
    
    return tiles;
}

QVector<QPointF> MapDrawingPrimitives::getSquareTiles(const QPointF& center, int size) const {
    QVector<QPointF> tiles;
    
    for (int y = -size; y <= size; ++y) {
        for (int x = -size; x <= size; ++x) {
            tiles.append(QPointF(center.x() + x, center.y() + y));
        }
    }
    
    return tiles;
}

// Coordinate conversion helpers
QRectF MapDrawingPrimitives::mapToScene(const QRectF& mapRect, int currentFloor) const {
    QPointF topLeft = mapToScene(mapRect.topLeft(), currentFloor);
    QSizeF size(mapRect.width() * options_.tileSize, mapRect.height() * options_.tileSize);
    return QRectF(topLeft, size);
}

QPointF MapDrawingPrimitives::mapToScene(const QPointF& mapPos, int currentFloor) const {
    QPointF scenePos(mapPos.x() * options_.tileSize, mapPos.y() * options_.tileSize);
    
    if (options_.show3DEffect) {
        scenePos += getFloorOffset(currentFloor, currentFloor);
    }
    
    return scenePos;
}

QRectF MapDrawingPrimitives::sceneToMap(const QRectF& sceneRect, int currentFloor) const {
    QPointF topLeft = sceneToMap(sceneRect.topLeft(), currentFloor);
    QSizeF size(sceneRect.width() / options_.tileSize, sceneRect.height() / options_.tileSize);
    return QRectF(topLeft, size);
}

QPointF MapDrawingPrimitives::sceneToMap(const QPointF& scenePos, int currentFloor) const {
    QPointF adjustedPos = scenePos;
    
    if (options_.show3DEffect) {
        adjustedPos -= getFloorOffset(currentFloor, currentFloor);
    }
    
    return QPointF(adjustedPos.x() / options_.tileSize, adjustedPos.y() / options_.tileSize);
}

// MapOverlayRenderer implementation
MapOverlayRenderer::MapOverlayRenderer(MapDrawingPrimitives* primitives)
    : primitives_(primitives) {
}

void MapOverlayRenderer::setBrushPreviewState(const QPointF& position, Brush* brush, int size,
                                             BrushShape shape, BrushValidity validity) {
    brushPreviewPos_ = position;
    brushPreviewBrush_ = brush;
    brushPreviewSize_ = size;
    brushPreviewShape_ = shape;
    brushPreviewValidity_ = validity;
    hasBrushPreview_ = true;
}

void MapOverlayRenderer::clearBrushPreview() {
    hasBrushPreview_ = false;
    brushPreviewBrush_ = nullptr;
}

void MapOverlayRenderer::setSelectionArea(const QRectF& area) {
    selectionArea_ = area;
    hasSelection_ = !area.isEmpty();
}

void MapOverlayRenderer::setSelectionTiles(const QVector<QRectF>& tiles) {
    selectionTiles_ = tiles;
    hasSelection_ = !tiles.isEmpty();
}

void MapOverlayRenderer::clearSelection() {
    selectionArea_ = QRectF();
    selectionTiles_.clear();
    hasSelection_ = false;
}

void MapOverlayRenderer::setDragArea(const QRectF& area, Brush* brush, BrushShape shape) {
    dragArea_ = area;
    dragBrush_ = brush;
    dragShape_ = shape;
    hasDragArea_ = !area.isEmpty();
}

void MapOverlayRenderer::clearDragArea() {
    dragArea_ = QRectF();
    dragBrush_ = nullptr;
    hasDragArea_ = false;
}

void MapOverlayRenderer::renderOverlays(QPainter* painter, const QRectF& viewRect, int currentFloor) {
    if (!painter || !primitives_) return;

    // Draw grid first (background)
    if (showGrid_) {
        primitives_->drawGrid(painter, viewRect, currentFloor);
    }

    // Draw selection area
    if (showSelectionBox_ && hasSelection_) {
        if (!selectionArea_.isEmpty()) {
            primitives_->drawSelectionBox(painter, selectionArea_, currentFloor);
        }

        if (!selectionTiles_.isEmpty()) {
            primitives_->drawSelectionArea(painter, selectionTiles_, currentFloor);
        }
    }

    // Draw drag area preview
    if (hasDragArea_ && dragBrush_) {
        primitives_->drawDragPreview(painter, dragArea_, dragBrush_, dragShape_, currentFloor);
    }

    // Draw brush preview (on top)
    if (showBrushPreview_ && hasBrushPreview_ && brushPreviewBrush_) {
        primitives_->drawBrushPreview(painter, brushPreviewPos_, brushPreviewBrush_,
                                     brushPreviewSize_, brushPreviewShape_,
                                     brushPreviewValidity_, currentFloor);
    }

    // Draw ingame box if enabled
    primitives_->drawIngameBox(painter, viewRect, currentFloor);
}
