#include "MapViewEnhancements.h"
#include "MapView.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QApplication>
#include <QDebug>
#include <QGraphicsView>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <QtMath>

// --- MapViewZoomSystem Implementation ---

MapViewZoomSystem::MapViewZoomSystem(MapView* mapView, QObject* parent)
    : QObject(parent)
    , mapView_(mapView)
    , currentZoom_(1.0)
    , minZoom_(0.125)
    , maxZoom_(25.0)
    , zoomFactor_(1.25)
    , animationEnabled_(true)
    , animationDuration_(200)
    , zoomAnimation_(nullptr)
{
    Q_ASSERT(mapView_);
    
    // Create zoom animation
    zoomAnimation_ = new QPropertyAnimation(this);
    zoomAnimation_->setDuration(animationDuration_);
    zoomAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    
    connect(zoomAnimation_, &QPropertyAnimation::valueChanged,
            this, &MapViewZoomSystem::onZoomAnimationValueChanged);
    connect(zoomAnimation_, &QPropertyAnimation::finished,
            this, &MapViewZoomSystem::onZoomAnimationFinished);
}

MapViewZoomSystem::~MapViewZoomSystem() = default;

void MapViewZoomSystem::zoomIn(const QPointF& centerPoint) {
    QPointF center = centerPoint.isNull() ? getViewCenter() : centerPoint;
    double newZoom = currentZoom_ * zoomFactor_;
    performZoom(newZoom, center);
}

void MapViewZoomSystem::zoomOut(const QPointF& centerPoint) {
    QPointF center = centerPoint.isNull() ? getViewCenter() : centerPoint;
    double newZoom = currentZoom_ / zoomFactor_;
    performZoom(newZoom, center);
}

void MapViewZoomSystem::zoomToLevel(double level, const QPointF& centerPoint) {
    QPointF center = centerPoint.isNull() ? getViewCenter() : centerPoint;
    performZoom(level, center);
}

void MapViewZoomSystem::zoomToFit(const QRectF& rect) {
    if (!mapView_ || rect.isEmpty()) {
        return;
    }
    
    QSize viewSize = mapView_->viewport()->size();
    double scaleX = viewSize.width() / rect.width();
    double scaleY = viewSize.height() / rect.height();
    double newZoom = qMin(scaleX, scaleY) * 0.9; // 90% to leave some margin
    
    performZoom(newZoom, rect.center(), true);
}

void MapViewZoomSystem::resetZoom() {
    performZoom(1.0, getViewCenter());
}

void MapViewZoomSystem::setZoomRange(double minZoom, double maxZoom) {
    minZoom_ = qMax(0.01, minZoom);
    maxZoom_ = qMin(100.0, maxZoom);
    
    // Ensure current zoom is within range
    if (currentZoom_ < minZoom_) {
        performZoom(minZoom_, getViewCenter(), false);
    } else if (currentZoom_ > maxZoom_) {
        performZoom(maxZoom_, getViewCenter(), false);
    }
}

void MapViewZoomSystem::handleWheelEvent(QWheelEvent* event) {
    if (!mapView_ || !event) {
        return;
    }
    
    // Calculate zoom direction
    double delta = event->angleDelta().y();
    if (qAbs(delta) < 1) {
        return;
    }
    
    // Calculate zoom factor based on wheel delta
    double factor = qPow(zoomFactor_, delta / 120.0);
    double newZoom = currentZoom_ * factor;
    
    // Get mouse position for zoom center
    QPointF mousePos = event->position();
    QPointF scenePos = mapView_->mapToScene(mousePos.toPoint());
    
    performZoom(newZoom, scenePos, false); // No animation for wheel zoom
    event->accept();
}

void MapViewZoomSystem::performZoom(double newZoom, const QPointF& centerPoint, bool animated) {
    // Constrain zoom level
    newZoom = qBound(minZoom_, newZoom, maxZoom_);
    
    if (qFuzzyCompare(newZoom, currentZoom_)) {
        return;
    }
    
    double oldZoom = currentZoom_;
    zoomCenter_ = centerPoint;
    
    if (animated && animationEnabled_) {
        emit zoomAnimationStarted();
        zoomAnimation_->setStartValue(oldZoom);
        zoomAnimation_->setEndValue(newZoom);
        zoomAnimation_->start();
    } else {
        currentZoom_ = newZoom;
        
        // Apply zoom to view
        if (mapView_) {
            QTransform transform;
            transform.scale(currentZoom_, currentZoom_);
            mapView_->setTransform(transform);
            
            // Center on zoom point
            mapView_->centerOn(zoomCenter_);
        }
        
        emit zoomChanged(currentZoom_, oldZoom);
    }
}

void MapViewZoomSystem::onZoomAnimationValueChanged(const QVariant& value) {
    double zoom = value.toDouble();
    double oldZoom = currentZoom_;
    currentZoom_ = zoom;
    
    if (mapView_) {
        QTransform transform;
        transform.scale(currentZoom_, currentZoom_);
        mapView_->setTransform(transform);
        mapView_->centerOn(zoomCenter_);
    }
    
    emit zoomChanged(currentZoom_, oldZoom);
}

void MapViewZoomSystem::onZoomAnimationFinished() {
    emit zoomAnimationFinished();
}

QPointF MapViewZoomSystem::getViewCenter() const {
    if (!mapView_) {
        return QPointF();
    }
    
    QRect viewRect = mapView_->viewport()->rect();
    return mapView_->mapToScene(viewRect.center());
}

// --- MapViewGridSystem Implementation ---

MapViewGridSystem::MapViewGridSystem(MapView* mapView, QObject* parent)
    : QObject(parent)
    , mapView_(mapView)
    , gridVisible_(true)
    , gridType_(TILE_GRID)
    , gridStyle_(SOLID_LINES)
    , gridColor_(QColor(255, 255, 255, 128))
    , gridLineWidth_(1)
    , gridSpacing_(32)
    , minZoomForGrid_(0.25)
    , maxZoomForGrid_(10.0)
    , snapToGrid_(false)
{
    Q_ASSERT(mapView_);
}

MapViewGridSystem::~MapViewGridSystem() = default;

void MapViewGridSystem::setGridVisible(bool visible) {
    if (gridVisible_ != visible) {
        gridVisible_ = visible;
        emit gridVisibilityChanged(visible);
        
        if (mapView_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewGridSystem::setGridType(GridType type) {
    if (gridType_ != type) {
        gridType_ = type;
        emit gridAppearanceChanged();
        
        if (mapView_ && gridVisible_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewGridSystem::setGridStyle(GridStyle style) {
    if (gridStyle_ != style) {
        gridStyle_ = style;
        emit gridAppearanceChanged();
        
        if (mapView_ && gridVisible_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewGridSystem::setGridColor(const QColor& color) {
    if (gridColor_ != color) {
        gridColor_ = color;
        emit gridAppearanceChanged();
        
        if (mapView_ && gridVisible_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewGridSystem::setGridLineWidth(int width) {
    width = qMax(1, width);
    if (gridLineWidth_ != width) {
        gridLineWidth_ = width;
        emit gridAppearanceChanged();
        
        if (mapView_ && gridVisible_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewGridSystem::setGridSpacing(int spacing) {
    spacing = qMax(1, spacing);
    if (gridSpacing_ != spacing) {
        gridSpacing_ = spacing;
        emit gridAppearanceChanged();
        
        if (mapView_ && gridVisible_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewGridSystem::setMinZoomForGrid(double minZoom) {
    minZoomForGrid_ = qMax(0.01, minZoom);
}

void MapViewGridSystem::setMaxZoomForGrid(double maxZoom) {
    maxZoomForGrid_ = qMin(100.0, maxZoom);
}

void MapViewGridSystem::setSnapToGrid(bool snap) {
    snapToGrid_ = snap;
}

QPointF MapViewGridSystem::snapToGrid(const QPointF& point) const {
    if (!snapToGrid_) {
        return point;
    }
    
    double spacing = gridSpacing_;
    double x = qRound(point.x() / spacing) * spacing;
    double y = qRound(point.y() / spacing) * spacing;
    
    return QPointF(x, y);
}

void MapViewGridSystem::renderGrid(QPainter* painter, const QRectF& viewRect, double zoom) {
    if (!painter || !gridVisible_) {
        return;
    }
    
    // Check zoom level constraints
    if (zoom < minZoomForGrid_ || zoom > maxZoomForGrid_) {
        return;
    }
    
    // Setup painter
    painter->save();
    
    QPen gridPen(gridColor_, gridLineWidth_);
    switch (gridStyle_) {
        case SOLID_LINES:
            gridPen.setStyle(Qt::SolidLine);
            break;
        case DASHED_LINES:
            gridPen.setStyle(Qt::DashLine);
            break;
        case DOTTED_LINES:
            gridPen.setStyle(Qt::DotLine);
            break;
    }
    painter->setPen(gridPen);
    
    // Render appropriate grid type
    switch (gridType_) {
        case TILE_GRID:
            renderTileGrid(painter, viewRect, zoom);
            break;
        case COORDINATE_GRID:
            renderCoordinateGrid(painter, viewRect, zoom);
            break;
        case CUSTOM_GRID:
            renderCustomGrid(painter, viewRect, zoom);
            break;
    }
    
    painter->restore();
}

void MapViewGridSystem::renderTileGrid(QPainter* painter, const QRectF& viewRect, double zoom) {
    double spacing = gridSpacing_ * zoom;
    
    // Calculate grid bounds
    int startX = static_cast<int>(viewRect.left() / spacing) * spacing;
    int startY = static_cast<int>(viewRect.top() / spacing) * spacing;
    int endX = static_cast<int>(viewRect.right() / spacing + 1) * spacing;
    int endY = static_cast<int>(viewRect.bottom() / spacing + 1) * spacing;
    
    // Draw vertical lines
    for (int x = startX; x <= endX; x += spacing) {
        painter->drawLine(x, viewRect.top(), x, viewRect.bottom());
    }
    
    // Draw horizontal lines
    for (int y = startY; y <= endY; y += spacing) {
        painter->drawLine(viewRect.left(), y, viewRect.right(), y);
    }
}

void MapViewGridSystem::renderCoordinateGrid(QPainter* painter, const QRectF& viewRect, double zoom) {
    // Similar to tile grid but with coordinate labels
    renderTileGrid(painter, viewRect, zoom);
    
    // Add coordinate labels if zoom is high enough
    if (zoom > 2.0) {
        painter->save();
        painter->setPen(QPen(gridColor_, 1));
        painter->setFont(QFont("Arial", 8));
        
        double spacing = gridSpacing_ * zoom;
        int startX = static_cast<int>(viewRect.left() / spacing) * spacing;
        int startY = static_cast<int>(viewRect.top() / spacing) * spacing;
        
        for (int x = startX; x <= viewRect.right(); x += spacing) {
            for (int y = startY; y <= viewRect.bottom(); y += spacing) {
                QString coord = QString("(%1,%2)").arg(x / gridSpacing_).arg(y / gridSpacing_);
                painter->drawText(x + 2, y + 12, coord);
            }
        }
        
        painter->restore();
    }
}

void MapViewGridSystem::renderCustomGrid(QPainter* painter, const QRectF& viewRect, double zoom) {
    // Custom grid with user-defined spacing
    double spacing = gridSpacing_ * zoom;
    
    if (spacing < 5) { // Don't draw if too dense
        return;
    }
    
    renderTileGrid(painter, viewRect, zoom);
}

// --- MapViewMouseTracker Implementation ---

MapViewMouseTracker::MapViewMouseTracker(MapView* mapView, QObject* parent)
    : QObject(parent)
    , mapView_(mapView)
    , mouseTrackingEnabled_(true)
    , hoverEffectsEnabled_(true)
    , mouseOverMap_(false)
    , hoverHighlightColor_(QColor(255, 255, 0, 128))
    , hoverTimer_(nullptr)
    , hoverDelay_(500)
{
    Q_ASSERT(mapView_);

    // Create hover timer
    hoverTimer_ = new QTimer(this);
    hoverTimer_->setSingleShot(true);
    connect(hoverTimer_, &QTimer::timeout, this, &MapViewMouseTracker::onHoverTimer);
}

MapViewMouseTracker::~MapViewMouseTracker() = default;

void MapViewMouseTracker::setMouseTrackingEnabled(bool enabled) {
    if (mouseTrackingEnabled_ != enabled) {
        mouseTrackingEnabled_ = enabled;

        if (mapView_) {
            mapView_->setMouseTracking(enabled);
        }
    }
}

void MapViewMouseTracker::setHoverEffectsEnabled(bool enabled) {
    if (hoverEffectsEnabled_ != enabled) {
        hoverEffectsEnabled_ = enabled;

        if (!enabled) {
            stopHoverTimer();
        }
    }
}

void MapViewMouseTracker::setHoverHighlightColor(const QColor& color) {
    hoverHighlightColor_ = color;
}

void MapViewMouseTracker::handleMouseMoveEvent(QMouseEvent* event) {
    if (!mouseTrackingEnabled_ || !event) {
        return;
    }

    QPointF screenPos = event->position();
    updateMousePosition(screenPos);

    if (hoverEffectsEnabled_) {
        updateHoverEffects();
        startHoverTimer();
    }
}

void MapViewMouseTracker::handleMouseEnterEvent(QMouseEvent* event) {
    Q_UNUSED(event)

    mouseOverMap_ = true;
    emit mouseEnteredMap();
}

void MapViewMouseTracker::handleMouseLeaveEvent(QMouseEvent* event) {
    Q_UNUSED(event)

    mouseOverMap_ = false;
    stopHoverTimer();

    if (!lastHoveredTile_.isNull()) {
        emit tileUnhovered(lastHoveredTile_);
        lastHoveredTile_ = QPointF();
    }

    emit mouseLeftMap();
}

QPointF MapViewMouseTracker::screenToMap(const QPointF& screenPos) const {
    if (!mapView_) {
        return QPointF();
    }

    return mapView_->mapToScene(screenPos.toPoint());
}

QPointF MapViewMouseTracker::mapToScreen(const QPointF& mapPos) const {
    if (!mapView_) {
        return QPointF();
    }

    return mapView_->mapFromScene(mapPos);
}

void MapViewMouseTracker::updateMousePosition(const QPointF& screenPos) {
    currentMousePos_ = screenPos;
    currentMapPos_ = screenToMap(screenPos);

    emit mousePositionChanged(currentMousePos_, currentMapPos_);
}

void MapViewMouseTracker::updateHoverEffects() {
    if (!hoverEffectsEnabled_) {
        return;
    }

    // Calculate tile position
    QPointF tilePos = QPointF(
        qFloor(currentMapPos_.x() / 32.0) * 32.0,
        qFloor(currentMapPos_.y() / 32.0) * 32.0
    );

    // Check if we're hovering over a new tile
    if (tilePos != lastHoveredTile_) {
        if (!lastHoveredTile_.isNull()) {
            emit tileUnhovered(lastHoveredTile_);
        }

        lastHoveredTile_ = tilePos;
        emit tileHovered(tilePos);
    }
}

void MapViewMouseTracker::startHoverTimer() {
    if (hoverTimer_) {
        hoverTimer_->start(hoverDelay_);
    }
}

void MapViewMouseTracker::stopHoverTimer() {
    if (hoverTimer_) {
        hoverTimer_->stop();
    }
}

void MapViewMouseTracker::onHoverTimer() {
    // Hover timer expired - could trigger tooltips or other delayed effects
    qDebug() << "MapViewMouseTracker: Hover timer expired at" << currentMapPos_;
}

// --- MapViewDrawingFeedback Implementation ---

MapViewDrawingFeedback::MapViewDrawingFeedback(MapView* mapView, QObject* parent)
    : QObject(parent)
    , mapView_(mapView)
    , feedbackEnabled_(true)
    , hasBrushPreview_(false)
    , brushPreviewSize_(1)
    , brushPreviewColor_(QColor(255, 255, 255, 128))
    , hasSelectionBox_(false)
    , hasDrawingIndicator_(false)
    , hasStatusOverlay_(false)
    , statusOverlayFont_("Arial", 10)
{
    Q_ASSERT(mapView_);
}

MapViewDrawingFeedback::~MapViewDrawingFeedback() = default;

void MapViewDrawingFeedback::setFeedbackEnabled(bool enabled) {
    if (feedbackEnabled_ != enabled) {
        feedbackEnabled_ = enabled;
        emit feedbackChanged();

        if (mapView_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewDrawingFeedback::setBrushPreview(const QPointF& position, int size, const QColor& color) {
    brushPreviewPos_ = position;
    brushPreviewSize_ = qMax(1, size);
    brushPreviewColor_ = color;
    hasBrushPreview_ = true;

    emit feedbackChanged();

    if (mapView_) {
        mapView_->viewport()->update();
    }
}

void MapViewDrawingFeedback::clearBrushPreview() {
    if (hasBrushPreview_) {
        hasBrushPreview_ = false;
        emit feedbackChanged();

        if (mapView_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewDrawingFeedback::setSelectionBox(const QRectF& rect) {
    selectionBoxRect_ = rect;
    hasSelectionBox_ = true;

    emit feedbackChanged();

    if (mapView_) {
        mapView_->viewport()->update();
    }
}

void MapViewDrawingFeedback::clearSelectionBox() {
    if (hasSelectionBox_) {
        hasSelectionBox_ = false;
        emit feedbackChanged();

        if (mapView_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewDrawingFeedback::setDrawingIndicator(const QPointF& position, const QString& text) {
    drawingIndicatorPos_ = position;
    drawingIndicatorText_ = text;
    hasDrawingIndicator_ = true;

    emit feedbackChanged();

    if (mapView_) {
        mapView_->viewport()->update();
    }
}

void MapViewDrawingFeedback::clearDrawingIndicator() {
    if (hasDrawingIndicator_) {
        hasDrawingIndicator_ = false;
        emit feedbackChanged();

        if (mapView_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewDrawingFeedback::setStatusOverlay(const QString& text, const QPointF& position) {
    statusOverlayText_ = text;
    statusOverlayPos_ = position;
    hasStatusOverlay_ = true;

    emit feedbackChanged();

    if (mapView_) {
        mapView_->viewport()->update();
    }
}

void MapViewDrawingFeedback::clearStatusOverlay() {
    if (hasStatusOverlay_) {
        hasStatusOverlay_ = false;
        emit feedbackChanged();

        if (mapView_) {
            mapView_->viewport()->update();
        }
    }
}

void MapViewDrawingFeedback::renderFeedback(QPainter* painter, const QRectF& viewRect) {
    if (!painter || !feedbackEnabled_) {
        return;
    }

    painter->save();

    // Render all feedback elements
    if (hasBrushPreview_) {
        renderBrushPreview(painter);
    }

    if (hasSelectionBox_) {
        renderSelectionBox(painter);
    }

    if (hasDrawingIndicator_) {
        renderDrawingIndicator(painter);
    }

    if (hasStatusOverlay_) {
        renderStatusOverlay(painter);
    }

    painter->restore();
}

void MapViewDrawingFeedback::renderBrushPreview(QPainter* painter) {
    painter->save();

    // Draw brush preview circle/square
    QPen previewPen(brushPreviewColor_, 2);
    previewPen.setStyle(Qt::DashLine);
    painter->setPen(previewPen);
    painter->setBrush(QBrush(brushPreviewColor_));

    QPointF screenPos = mapView_->mapFromScene(brushPreviewPos_);
    double radius = brushPreviewSize_ * 16; // Half tile size

    painter->drawEllipse(screenPos, radius, radius);

    painter->restore();
}

void MapViewDrawingFeedback::renderSelectionBox(QPainter* painter) {
    painter->save();

    QPen selectionPen(QColor(255, 255, 255, 200), 2);
    selectionPen.setStyle(Qt::DashLine);
    painter->setPen(selectionPen);
    painter->setBrush(QBrush(QColor(255, 255, 255, 50)));

    QRectF screenRect = mapView_->mapRectFromScene(selectionBoxRect_);
    painter->drawRect(screenRect);

    painter->restore();
}

void MapViewDrawingFeedback::renderDrawingIndicator(QPainter* painter) {
    painter->save();

    painter->setPen(QPen(QColor(255, 255, 255), 1));
    painter->setFont(QFont("Arial", 12, QFont::Bold));

    QPointF screenPos = mapView_->mapFromScene(drawingIndicatorPos_);
    painter->drawText(screenPos.toPoint() + QPoint(10, -10), drawingIndicatorText_);

    painter->restore();
}

void MapViewDrawingFeedback::renderStatusOverlay(QPainter* painter) {
    painter->save();

    painter->setPen(QPen(QColor(255, 255, 255), 1));
    painter->setFont(statusOverlayFont_);

    QPointF pos = statusOverlayPos_.isNull() ?
                  QPointF(10, mapView_->height() - 20) :
                  mapView_->mapFromScene(statusOverlayPos_);

    // Draw background
    QFontMetrics fm(statusOverlayFont_);
    QRect textRect = fm.boundingRect(statusOverlayText_);
    textRect.moveTopLeft(pos.toPoint());
    textRect.adjust(-5, -2, 5, 2);

    painter->fillRect(textRect, QColor(0, 0, 0, 128));
    painter->drawText(pos.toPoint(), statusOverlayText_);

    painter->restore();
}
