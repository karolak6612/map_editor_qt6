#include "MapViewZoomHandler.h"
#include "MapView.h"
#include <QGraphicsView>
#include <QScrollBar>
#include <QDebug>
#include <QtMath>

MapViewZoomHandler::MapViewZoomHandler(MapView* mapView, QObject *parent)
    : QObject(parent)
    , mapView_(mapView)
    , currentZoom_(DEFAULT_ZOOM)
    , minZoom_(MIN_ZOOM)
    , maxZoom_(MAX_ZOOM)
    , animatedZoom_(false)
    , wheelZoomFactor_(1.15)
    , invertWheelZoom_(false)
{
}

MapViewZoomHandler::~MapViewZoomHandler()
{
}

void MapViewZoomHandler::zoom(qreal factor, const QPointF& centerScreenPos)
{
    if (!mapView_) return;

    QPointF sceneCenterPos = mapView_->mapToScene(centerScreenPos.toPoint());

    double oldZoom = currentZoom_;
    double newZoomLevel = oldZoom * factor;
    newZoomLevel = constrainZoom(newZoomLevel);

    double actualScaleFactor = newZoomLevel / oldZoom;

    if (qFuzzyCompare(actualScaleFactor, 1.0)) {
        return;
    }

    currentZoom_ = newZoomLevel;

    // Apply zoom to the view
    applyZoomToView(actualScaleFactor, centerScreenPos);

    // Update scroll bars to keep the center point fixed
    updateScrollBarsAfterZoom(centerScreenPos, sceneCenterPos);

    // Emit signals and update status
    emitZoomChanged();
}

void MapViewZoomHandler::zoomIn()
{
    QPointF centerPos = getViewportCenter();
    zoom(ZOOM_IN_FACTOR, centerPos);
}

void MapViewZoomHandler::zoomOut()
{
    QPointF centerPos = getViewportCenter();
    zoom(ZOOM_OUT_FACTOR, centerPos);
}

void MapViewZoomHandler::resetZoom()
{
    QPointF centerPos = getViewportCenter();
    double currentZoom = currentZoom_;
    double targetZoom = DEFAULT_ZOOM;
    zoom(targetZoom / currentZoom, centerPos);
}

void MapViewZoomHandler::zoomToLevel(double level, const QPointF& centerPoint)
{
    if (!mapView_) return;

    level = constrainZoom(level);
    
    QPointF zoomCenter = centerPoint.isNull() ? getViewportCenter() : centerPoint;
    double factor = level / currentZoom_;
    
    zoom(factor, zoomCenter);
}

void MapViewZoomHandler::zoomToFit(const QRectF& rect)
{
    if (!mapView_ || rect.isEmpty()) return;

    // Calculate the zoom level needed to fit the rectangle in the viewport
    QRect viewportRect = mapView_->viewport()->rect();
    
    double scaleX = viewportRect.width() / rect.width();
    double scaleY = viewportRect.height() / rect.height();
    
    // Use the smaller scale to ensure the entire rectangle fits
    double targetZoom = qMin(scaleX, scaleY) * 0.9; // 90% to leave some margin
    targetZoom = constrainZoom(targetZoom);
    
    // Center on the rectangle
    QPointF rectCenter = rect.center();
    mapView_->centerOn(rectCenter);
    
    // Apply the zoom
    zoomToLevel(targetZoom);
}

void MapViewZoomHandler::handleWheelZoom(QWheelEvent* event)
{
    if (!mapView_ || !event) return;

    // Calculate zoom factor based on wheel delta
    double numDegrees = event->angleDelta().y() / 8.0;
    double numSteps = numDegrees / 15.0;
    
    double factor = qPow(wheelZoomFactor_, numSteps);
    
    if (invertWheelZoom_) {
        factor = 1.0 / factor;
    }

    // Zoom towards the mouse position
    QPointF mousePos = event->position();
    zoom(factor, mousePos);

    event->accept();
}

void MapViewZoomHandler::setCurrentZoom(double zoom)
{
    zoom = constrainZoom(zoom);
    if (!qFuzzyCompare(currentZoom_, zoom)) {
        currentZoom_ = zoom;
        emitZoomChanged();
    }
}

double MapViewZoomHandler::constrainZoom(double zoom) const
{
    return qBound(minZoom_, zoom, maxZoom_);
}

void MapViewZoomHandler::setZoomConstraints(double minZoom, double maxZoom)
{
    if (minZoom > 0 && maxZoom > minZoom) {
        minZoom_ = minZoom;
        maxZoom_ = maxZoom;
        
        // Constrain current zoom if necessary
        double newZoom = constrainZoom(currentZoom_);
        if (!qFuzzyCompare(currentZoom_, newZoom)) {
            setCurrentZoom(newZoom);
        }
        
        emit zoomConstraintsChanged(minZoom_, maxZoom_);
    }
}

void MapViewZoomHandler::centerOnMap()
{
    if (!mapView_ || !mapView_->scene()) return;
    
    QRectF sceneRect = mapView_->scene()->sceneRect();
    mapView_->centerOn(sceneRect.center());
}

void MapViewZoomHandler::centerOnPoint(const QPointF& point)
{
    if (!mapView_) return;
    
    mapView_->centerOn(point);
}

void MapViewZoomHandler::onZoomAnimationFinished()
{
    // Placeholder for future animation support
    qDebug() << "MapViewZoomHandler::onZoomAnimationFinished";
}

// Private helper methods
void MapViewZoomHandler::applyZoomToView(double scaleFactor, const QPointF& centerScreenPos)
{
    if (!mapView_) return;

    // Scale the view
    mapView_->scale(scaleFactor, scaleFactor);
}

void MapViewZoomHandler::updateScrollBarsAfterZoom(const QPointF& centerScreenPos, const QPointF& sceneCenterPos)
{
    if (!mapView_) return;

    // Adjust scrollbars to keep the centerScreenPos fixed
    QPointF scenePosAfterScale = mapView_->mapToScene(centerScreenPos.toPoint());
    QPointF viewScrollDelta = scenePosAfterScale - sceneCenterPos;

    QScrollBar* hScrollBar = mapView_->horizontalScrollBar();
    QScrollBar* vScrollBar = mapView_->verticalScrollBar();

    if (hScrollBar) {
        hScrollBar->setValue(hScrollBar->value() + qRound(viewScrollDelta.x()));
    }
    
    if (vScrollBar) {
        vScrollBar->setValue(vScrollBar->value() + qRound(viewScrollDelta.y()));
    }
}

QPointF MapViewZoomHandler::getViewportCenter() const
{
    if (!mapView_ || !mapView_->viewport()) {
        return QPointF();
    }
    
    return QPointF(mapView_->viewport()->rect().center());
}

void MapViewZoomHandler::emitZoomChanged()
{
    emit zoomChanged(currentZoom_);
    emit zoomLevelChanged(currentZoom_);
    
    // Update MapView's zoom status if it has an update method
    // This maintains compatibility with existing MapView interface
    if (mapView_) {
        // Trigger viewport update
        if (mapView_->viewport()) {
            mapView_->viewport()->update();
        }
    }
}


