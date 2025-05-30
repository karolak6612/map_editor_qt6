#include "MapView.h"
#include "MapViewInputHandler.h"
#include "Brush.h" // Include Brush.h for Brush type
#include <QGraphicsScene>
#include <QScrollBar>
#include <QDebug>
#include <QtMath> // For qBound
#include <QKeyEvent>

// --- Start of Placeholder Implementations ---
void MapView::pasteSelection(const QPointF& mapPos) { qDebug() << "MapView::pasteSelection at" << mapPos << "(placeholder)"; }
void MapView::clearSelection() { qDebug() << "MapView::clearSelection (placeholder)"; }
void MapView::toggleSelectionAt(const QPointF& mapPos) { qDebug() << "MapView::toggleSelectionAt" << mapPos << "(placeholder)"; }
bool MapView::isOnSelection(const QPointF& mapPos) const { qDebug() << "MapView::isOnSelection at" << mapPos << "(placeholder)"; return false; }
void MapView::selectObjectAt(const QPointF& mapPos) { qDebug() << "MapView::selectObjectAt" << mapPos << "(placeholder)"; }
void MapView::updateMoveSelectionFeedback(const QPointF& delta) { qDebug() << "MapView::updateMoveSelectionFeedback by" << delta << "(placeholder)"; }
void MapView::finalizeMoveSelection(const QPointF& delta) { qDebug() << "MapView::finalizeMoveSelection by" << delta << "(placeholder)"; }
void MapView::updateSelectionRectFeedback(const QPointF& startMapPos, const QPointF& currentMapPos) { qDebug() << "MapView::updateSelectionRectFeedback from" << startMapPos << "to" << currentMapPos << "(placeholder)"; }
void MapView::finalizeSelectionRect(const QPointF& startMapPos, const QPointF& endMapPos, Qt::KeyboardModifiers modifiers) { qDebug() << "MapView::finalizeSelectionRect from" << startMapPos << "to" << endMapPos << "Modifiers:" << modifiers << "(placeholder)"; }

// Drawing related placeholders are removed as they are now delegated to brushes.

void MapView::updateStatusBarWithMapPos(const QPointF& mapPos) { qDebug() << "MapView::updateStatusBarWithMapPos: Tile(" << mapPos.x() << "," << mapPos.y() << ") Floor:" << currentFloor_ << "Zoom:" << zoomLevel_; }
void MapView::updateZoomStatus() { qDebug() << "MapView::updateZoomStatus: Zoom:" << zoomLevel_ << "(placeholder)"; }
void MapView::updateFloorStatus() { qDebug() << "MapView::updateFloorStatus: Floor:" << currentFloor_ << "(placeholder)"; }

void MapView::increaseBrushSize_placeholder() { qDebug() << "MapView::increaseBrushSize_placeholder"; }
void MapView::decreaseBrushSize_placeholder() { qDebug() << "MapView::decreaseBrushSize_placeholder"; }
void MapView::updateFloorMenu_placeholder() { qDebug() << "MapView::updateFloorMenu_placeholder for floor" << currentFloor_; }

void MapView::showPropertiesDialogFor(const QPointF& mapPos) { qDebug() << "MapView::showPropertiesDialogFor map" << mapPos << "(placeholder)"; }
void MapView::switchToSelectionMode() { 
    qDebug() << "MapView::switchToSelectionMode"; 
    currentEditorMode_ = EditorMode::Selection; 
    // Potentially signal UI or editor manager
}
void MapView::setCurrentEditorMode(EditorMode mode) {
    qDebug() << "MapView::setCurrentEditorMode to" << (mode == EditorMode::Selection ? "Selection" : "Drawing");
    currentEditorMode_ = mode;
    // Potentially signal UI or editor manager
}
void MapView::endPasting() { qDebug() << "MapView::endPasting (placeholder)"; }
void MapView::showContextMenuAt(const QPoint& screenPos) { qDebug() << "MapView::showContextMenuAt screen" << screenPos << "(placeholder)"; }
void MapView::resetActionQueueTimer_placeholder() { qDebug() << "MapView::resetActionQueueTimer_placeholder"; }
// --- End of Placeholder Implementations ---

MapView::MapView(QWidget *parent) : QGraphicsView(parent),
    currentEditorMode_(EditorMode::Selection),
    currentBrush_(nullptr),
    zoomLevel_(1.0),
    currentFloor_(GROUND_LAYER),
    isPanning_(false),
    switchMouseButtons_(false), 
    doubleClickProperties_(true)
{
    setScene(new QGraphicsScene(this));
    setMouseTracking(true);
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    setFocusPolicy(Qt::StrongFocus);

    inputHandler_ = new MapViewInputHandler(this); // `this` is passed to MapViewInputHandler
    updateAndRefreshMapCoordinates(QPoint(width()/2, height()/2));
    updateZoomStatus();
    updateFloorStatus();
}

MapView::~MapView() {
    delete inputHandler_;
    // currentBrush_ is not owned by MapView, so no delete here.
}

MapView::EditorMode MapView::getCurrentEditorMode() const {
    return currentEditorMode_;
}

void MapView::setActiveBrush(Brush* brush) {
    currentBrush_ = brush;
    qDebug() << "MapView::setActiveBrush to:" << (brush ? brush->name() : "nullptr");
    // If a brush is set, typically switch to Drawing mode.
    // This logic might be better handled by an Editor class or UI controller.
    if (brush) {
        setCurrentEditorMode(EditorMode::Drawing);
    } else {
        // Or switch to selection mode if no brush? Depends on desired UX.
        // setCurrentEditorMode(EditorMode::Selection); 
    }
}

Brush* MapView::getActiveBrush() const {
    return currentBrush_;
}

void MapView::setSwitchMouseButtons(bool switched) {
    switchMouseButtons_ = switched;
    qDebug() << "MapView::switchMouseButtons set to:" << switched_;
}

QPointF MapView::screenToMap(const QPoint& screenPos) const {
    QPointF scenePos = mapToScene(screenPos);
    double logicalX = scenePos.x();
    double logicalY = scenePos.y();
    logicalX += (GROUND_LAYER - currentFloor_) * TILE_SIZE;
    logicalY += (GROUND_LAYER - currentFloor_) * TILE_SIZE;
    return QPointF(logicalX / TILE_SIZE, logicalY / TILE_SIZE);
}

QPoint MapView::mapToScreen(const QPointF& mapTilePos) const {
    double sceneX = mapTilePos.x() * TILE_SIZE;
    double sceneY = mapTilePos.y() * TILE_SIZE;
    sceneX -= (GROUND_LAYER - currentFloor_) * TILE_SIZE;
    sceneY -= (GROUND_LAYER - currentFloor_) * TILE_SIZE;
    return mapFromScene(QPointF(sceneX, sceneY));
}

void MapView::changeFloor(int newFloor) {
    int oldFloor = currentFloor_;
    currentFloor_ = qBound(0, newFloor, MAP_MAX_LAYERS - 1);
    if (currentFloor_ != oldFloor) {
        updateFloorStatus();
        updateAndRefreshMapCoordinates(lastMousePos_);
        scene()->invalidate(sceneRect(), QGraphicsScene::AllLayers);
        updateFloorMenu_placeholder();
    }
}

void MapView::updateAndRefreshMapCoordinates(const QPoint& screenPos) {
    lastMousePos_ = screenPos;
    if (scene()) { 
        lastMapPos_ = screenToMap(screenPos);
        updateStatusBarWithMapPos(lastMapPos_);
    }
    if (viewport()) { 
        viewport()->update();
    }
}

void MapView::mousePressEvent(QMouseEvent *event) {
    lastMousePos_ = event->pos();
    QPointF mapPos = screenToMap(lastMousePos_);

    // Delegate all press events (except unhandled ones) to inputHandler_
    // The inputHandler will then decide based on mode (Selection/Drawing)
    // and for Drawing, it will use the activeBrush.
    // Panning is also initiated here for now.

    if (event->button() == Qt::LeftButton) {
        inputHandler_->handleMousePress(event, currentEditorMode_, mapPos, lastMousePos_);
    }
    else if (event->button() == Qt::MiddleButton) {
        if (switchMouseButtons_) { 
            inputHandler_->handlePropertiesClick(event, currentEditorMode_, mapPos, lastMousePos_);
        } else { 
            isPanning_ = true;
            dragStartScreenPos_ = lastMousePos_;
            setCursor(Qt::ClosedHandCursor);
        }
    }
    else if (event->button() == Qt::RightButton) {
        if (switchMouseButtons_) { 
            isPanning_ = true;
            dragStartScreenPos_ = lastMousePos_;
            setCursor(Qt::ClosedHandCursor);
        } else { 
            inputHandler_->handlePropertiesClick(event, currentEditorMode_, mapPos, lastMousePos_);
        }
    } else {
        QGraphicsView::mousePressEvent(event); 
        return;
    }
    updateAndRefreshMapCoordinates(lastMousePos_); 
    event->accept();
}

void MapView::mouseMoveEvent(QMouseEvent *event) {
    QPoint currentScreenPos = event->pos();
    QPointF currentMapPos = screenToMap(currentScreenPos); 

    if (isPanning_) {
        QPoint delta = currentScreenPos - dragStartScreenPos_;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        dragStartScreenPos_ = currentScreenPos;
    } else {
        // Always delegate to input handler if not panning.
        // It will check editor mode and active brush.
        inputHandler_->handleMouseMove(event, currentEditorMode_, currentMapPos, currentScreenPos);
    }
    
    updateAndRefreshMapCoordinates(currentScreenPos); 
    event->accept(); 
}

void MapView::mouseReleaseEvent(QMouseEvent *event) {
    lastMousePos_ = event->pos();
    QPointF mapPos = screenToMap(lastMousePos_);

    if (event->button() == Qt::LeftButton) {
        inputHandler_->handleMouseRelease(event, currentEditorMode_, mapPos, lastMousePos_);
        if (!isPanning_) setCursor(Qt::ArrowCursor); // Reset cursor if not panning (panning resets its own)
    }
    else if (event->button() == Qt::MiddleButton) {
        if (switchMouseButtons_) { 
            inputHandler_->handlePropertiesRelease(event, currentEditorMode_, mapPos, lastMousePos_);
        } else { 
            if (isPanning_) {
                isPanning_ = false;
                setCursor(Qt::ArrowCursor);
            }
        }
    }
    else if (event->button() == Qt::RightButton) {
        if (switchMouseButtons_) { 
            if (isPanning_) {
                isPanning_ = false;
                setCursor(Qt::ArrowCursor);
            }
        } else { 
            inputHandler_->handlePropertiesRelease(event, currentEditorMode_, mapPos, lastMousePos_);
        }
    } else {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }
    updateAndRefreshMapCoordinates(lastMousePos_); 
    event->accept();
}

void MapView::mouseDoubleClickEvent(QMouseEvent *event) {
    qDebug() << "MapView::mouseDoubleClickEvent, button:" << event->button();
    lastMousePos_ = event->pos();
    QPointF mapPos = screenToMap(lastMousePos_);

    if (event->button() == Qt::LeftButton) { 
        if (doubleClickProperties_) { 
            showPropertiesDialogFor(mapPos);
        }
        // It's possible a brush might want to handle double click too.
        // If so, this event could be passed to inputHandler and then to the brush.
    }
    updateAndRefreshMapCoordinates(lastMousePos_);
    event->accept();
}

void MapView::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        changeFloor(currentFloor_ - (event->angleDelta().y() > 0 ? 1 : -1));
    } else if (event->modifiers() & Qt::AltModifier) {
        if (event->angleDelta().y() > 0) { decreaseBrushSize_placeholder(); } 
        else { increaseBrushSize_placeholder(); }
    } else { 
        QPointF mousePosInView = event->position();
        QPointF mousePosInScene_BeforeZoom = mapToScene(mousePosInView.toPoint());
        double oldZoom = zoomLevel_;
        double newZoomFactorRequest = (event->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
        double newZoomLevel = oldZoom * newZoomFactorRequest;
        newZoomLevel = qBound(MIN_ZOOM, newZoomLevel, MAX_ZOOM);
        double actualScaleFactor = newZoomLevel / oldZoom;

        if (qFuzzyCompare(actualScaleFactor, 1.0)) { 
            updateAndRefreshMapCoordinates(event->position().toPoint()); 
            event->accept(); return; 
        }
        zoomLevel_ = newZoomLevel;
        scale(actualScaleFactor, actualScaleFactor);
        QPointF mousePosInView_AfterZoom = mapFromScene(mousePosInScene_BeforeZoom);
        QPointF viewScrollDelta = mousePosInView_AfterZoom - mousePosInView;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + qRound(viewScrollDelta.x()));
        verticalScrollBar()->setValue(verticalScrollBar()->value() + qRound(viewScrollDelta.y()));
        updateZoomStatus();
    }
    updateAndRefreshMapCoordinates(event->position().toPoint());
    event->accept();
}

void MapView::enterEvent(QEnterEvent *event) {
    qDebug() << "MapView::enterEvent";
    if (!event->buttons()) {
        if (inputHandler_) { inputHandler_->resetInputStates(); }
        isPanning_ = false;
        if (cursor().shape() == Qt::ClosedHandCursor || cursor().shape() == Qt::OpenHandCursor) {
             setCursor(Qt::ArrowCursor);
        }
    }
    viewport()->update();
    QGraphicsView::enterEvent(event);
}

void MapView::leaveEvent(QEvent *event) {
    qDebug() << "MapView::leaveEvent";
    updateAndRefreshMapCoordinates(lastMousePos_); 
    viewport()->update();
    QGraphicsView::leaveEvent(event);
}

void MapView::keyPressEvent(QKeyEvent *event) {
    qDebug() << "MapView::keyPressEvent, key:" << event->key() << "text:" << event->text() << "modifiers:" << event->modifiers();
    QGraphicsView::keyPressEvent(event);
}

void MapView::keyReleaseEvent(QKeyEvent *event) {
    qDebug() << "MapView::keyReleaseEvent, key:" << event->key() << "text:" << event->text() << "modifiers:" << event->modifiers();
    QGraphicsView::keyReleaseEvent(event);
}

void MapView::drawBackground(QPainter* painter, const QRectF& rect) {
    // Call base class implementation first (important for scene backgroundBrush, etc.)
    QGraphicsView::drawBackground(painter, rect);

    // Draw a custom placeholder background color over the default scene background
    // This ensures a visible placeholder even if scene background is transparent or not set.
    painter->fillRect(rect, QColor(30, 30, 30)); // Dark gray placeholder background
}
