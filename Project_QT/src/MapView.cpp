#include "MapView.h"
#include "MapViewInputHandler.h"
#include <QGraphicsScene>
#include <QScrollBar>
#include <QDebug>
#include <QtMath> // For qBound
#include <QKeyEvent> // Included for QKeyEvent

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

void MapView::startDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers) { qDebug() << "MapView::startDrawing at" << mapPos << "Modifiers:" << modifiers << "(placeholder)"; }
void MapView::continueDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers) { qDebug() << "MapView::continueDrawing at" << mapPos << "Modifiers:" << modifiers << "(placeholder)"; }
void MapView::finalizeDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers) { qDebug() << "MapView::finalizeDrawing at" << mapPos << "Modifiers:" << modifiers << "(placeholder)"; }
void MapView::updateDragDrawFeedback(const QPointF& startMapPos, const QPointF& currentMapPos) { qDebug() << "MapView::updateDragDrawFeedback from" << startMapPos << "to" << currentMapPos << "(placeholder)"; }
void MapView::finalizeDragDraw(const QPointF& startMapPos, const QPointF& endMapPos, Qt::KeyboardModifiers modifiers) { qDebug() << "MapView::finalizeDragDraw from" << startMapPos << "to" << endMapPos << "Modifiers:" << modifiers << "(placeholder)"; }

void MapView::updateStatusBarWithMapPos(const QPointF& mapPos) { qDebug() << "MapView::updateStatusBarWithMapPos: Tile(" << mapPos.x() << "," << mapPos.y() << ") Floor:" << currentFloor_ << "Zoom:" << zoomLevel_; }
void MapView::updateZoomStatus() { qDebug() << "MapView::updateZoomStatus: Zoom:" << zoomLevel_ << "(placeholder)"; }
void MapView::updateFloorStatus() { qDebug() << "MapView::updateFloorStatus: Floor:" << currentFloor_ << "(placeholder)"; }

void MapView::increaseBrushSize_placeholder() { qDebug() << "MapView::increaseBrushSize_placeholder"; }
void MapView::decreaseBrushSize_placeholder() { qDebug() << "MapView::decreaseBrushSize_placeholder"; }
void MapView::updateFloorMenu_placeholder() { qDebug() << "MapView::updateFloorMenu_placeholder for floor" << currentFloor_; }

void MapView::showPropertiesDialogFor(const QPointF& mapPos) { qDebug() << "MapView::showPropertiesDialogFor map" << mapPos << "(placeholder)"; }
void MapView::switchToSelectionMode() { 
    qDebug() << "MapView::switchToSelectionMode (placeholder)"; 
    currentEditorMode_ = EditorMode::Selection; 
}
void MapView::endPasting() { qDebug() << "MapView::endPasting (placeholder)"; }
void MapView::showContextMenuAt(const QPoint& screenPos) { qDebug() << "MapView::showContextMenuAt screen" << screenPos << "(placeholder)"; }
void MapView::resetActionQueueTimer_placeholder() { qDebug() << "MapView::resetActionQueueTimer_placeholder"; }
// --- End of Placeholder Implementations ---

MapView::MapView(QWidget *parent) : QGraphicsView(parent),
    zoomLevel_(1.0),
    currentFloor_(GROUND_LAYER),
    isPanning_(false),
    switchMouseButtons_(false), 
    doubleClickProperties_(true)
{
    setScene(new QGraphicsScene(this));
    setMouseTracking(true);
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    setFocusPolicy(Qt::StrongFocus); // Ensure view can receive key events

    inputHandler_ = new MapViewInputHandler(this);
    updateAndRefreshMapCoordinates(QPoint(width()/2, height()/2));
    updateZoomStatus();
    updateFloorStatus();
}

MapView::~MapView() {
    delete inputHandler_;
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
    if (scene()) { // Ensure scene is not null
        lastMapPos_ = screenToMap(screenPos);
        updateStatusBarWithMapPos(lastMapPos_);
    }
    if (viewport()) { // Ensure viewport is not null
        viewport()->update();
    }
}

void MapView::mousePressEvent(QMouseEvent *event) {
    lastMousePos_ = event->pos();
    QPointF mapPos = screenToMap(lastMousePos_);

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
        setCursor(Qt::ArrowCursor); 
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
            updateAndRefreshMapCoordinates(event->position().toPoint()); // Still update coords for status bar
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
    // In future, delegate to an inputHandler_ or specific game logic handler here
    // e.g., inputHandler_->handleKeyPress(event);
    QGraphicsView::keyPressEvent(event); // Call base for default behavior (e.g. scrolling with arrow keys if not handled)
}

void MapView::keyReleaseEvent(QKeyEvent *event) {
    qDebug() << "MapView::keyReleaseEvent, key:" << event->key() << "text:" << event->text() << "modifiers:" << event->modifiers();
    // e.g., inputHandler_->handleKeyRelease(event);
    QGraphicsView::keyReleaseEvent(event);
}
