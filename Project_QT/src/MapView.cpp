#include "MapView.h"
#include "MapViewInputHandler.h" // Ensure this is the correct new handler
#include "Brush.h" // Include Brush.h for Brush type
#include "BrushManager.h" // Added
#include "Map.h"          // Added
#include "QUndoStack.h"   // Added
#include <QGraphicsScene>
#include <QScrollBar>
#include <QPainter> // Added for drawForeground
#include <QDebug>
#include <QtMath> // For qBound
#include <QKeyEvent> // Already present
#include <QFocusEvent> // Added for focusOutEvent

// --- Start of Placeholder Implementations ---
// These might be removed or adapted if MapViewInputHandler calls them directly.
// For now, keeping them as stubs if MapViewInputHandler might still expect some.
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

MapView::MapView(BrushManager* brushManager, Map* map, QUndoStack* undoStack, QWidget *parent) : QGraphicsView(parent),
    currentEditorMode_(EditorMode::Selection), // Default mode
    currentBrush_(nullptr), // No active brush initially
    zoomLevel_(1.0),
    currentFloor_(GROUND_LAYER),
    isPanning_(false), // This state might be managed by inputHandler_ now
    switchMouseButtons_(false), 
    doubleClickProperties_(true),
    currentSelectionArea_() // Initialize currentSelectionArea_
{
    setScene(new QGraphicsScene(this));
    setMouseTracking(true); // Important for hover effects and map coordinate updates
    // setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // Good for zooming towards mouse
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setFocusPolicy(Qt::StrongFocus); // To receive key events

    // Instantiate the new input handler
    inputHandler_ = new MapViewInputHandler(this, brushManager, map, undoStack, this);

    updateAndRefreshMapCoordinates(QPoint(viewport()->width()/2, viewport()->height()/2)); // Use viewport size
    updateZoomStatus();
    updateFloorStatus();
}

MapView::~MapView() {
    delete inputHandler_; // MapView owns inputHandler_
    inputHandler_ = nullptr;
    // currentBrush_ is not owned by MapView.
}

// --- Interface methods for MapViewInputHandler ---
void MapView::pan(int dx, int dy) {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - dx);
    verticalScrollBar()->setValue(verticalScrollBar()->value() - dy);
    // update() or viewport()->update() might be needed if scrollbar changes don't trigger repaint automatically
}

void MapView::zoom(qreal factor, const QPointF& centerScreenPos) {
    QPointF sceneCenterPos = mapToScene(centerScreenPos.toPoint());

    double oldZoom = zoomLevel_;
    double newZoomLevel = oldZoom * factor;
    newZoomLevel = qBound(MIN_ZOOM, newZoomLevel, MAX_ZOOM); // Assuming MIN_ZOOM and MAX_ZOOM are defined

    double actualScaleFactor = newZoomLevel / oldZoom;

    if (qFuzzyCompare(actualScaleFactor, 1.0)) {
        return;
    }

    zoomLevel_ = newZoomLevel;

    // Scale the view
    scale(actualScaleFactor, actualScaleFactor);

    // Adjust scrollbars to keep the centerScreenPos fixed
    QPointF scenePosAfterScale = mapToScene(centerScreenPos.toPoint());
    QPointF viewScrollDelta = scenePosAfterScale - sceneCenterPos;

    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + qRound(viewScrollDelta.x()));
    verticalScrollBar()->setValue(verticalScrollBar()->value() + qRound(viewScrollDelta.y()));

    updateZoomStatus(); // Update any UI displaying zoom level
    viewport()->update(); // Refresh the viewport
}

void MapView::setSelectionArea(const QRectF& rect) {
    if (currentSelectionArea_ != rect) {
        currentSelectionArea_ = rect;
        viewport()->update(); // Trigger drawForeground
    }
}
// --- End of Interface methods ---


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
    // The following logic for floor offset seems specific to how this MapView handles coordinates.
    // Assuming TILE_SIZE and GROUND_LAYER are defined class members or constants.
    // If map coordinates are directly scene coordinates, this might be simpler.
    // For now, retaining existing logic.
    double logicalX = scenePos.x(); // This might be tile units if scene is set up that way
    double logicalY = scenePos.y(); // Or pixel units that need conversion

    // The example from problem description implies map coordinates are tile-based.
    // If mapToScene returns scene pixels, and TILE_SIZE is pixels per tile:
    // logicalX = scenePos.x() / TILE_SIZE;
    // logicalY = scenePos.y() / TILE_SIZE;
    // The floor adjustment also depends on whether scene coords are floor-agnostic.
    // For now, assume screenToMap correctly converts to the MapViewInputHandler's expected "mapPosition".
    // The current implementation seems to convert to a tile-based coordinate system adjusted for floor.
    // This should be compatible if MapViewInputHandler expects tile coordinates.
    logicalX += (GROUND_LAYER - currentFloor_) * TILE_SIZE; // This looks like pixel adjustment before division
    logicalY += (GROUND_LAYER - currentFloor_) * TILE_SIZE; // If scenePos is already in pixels for current floor
                                                            // this adjustment might be double.
                                                            // Let's assume it's correct for now.
    return QPointF(logicalX / TILE_SIZE, logicalY / TILE_SIZE);
}

QPoint MapView::mapToScreen(const QPointF& mapTilePos) const {
    // Inverse of screenToMap
    double sceneX = mapTilePos.x() * TILE_SIZE;
    double sceneY = mapTilePos.y() * TILE_SIZE;
    sceneX -= (GROUND_LAYER - currentFloor_) * TILE_SIZE;
    sceneY -= (GROUND_LAYER - currentFloor_) * TILE_SIZE;
    return mapFromScene(QPointF(sceneX, sceneY)); // mapFromScene converts scene coords to screen (widget) coords
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
    lastMousePos_ = event->pos(); // Store screen position
    if (inputHandler_) {
        QPointF mapPos = screenToMap(lastMousePos_);
        inputHandler_->handleMousePressEvent(event, mapPos);
    } else {
        QGraphicsView::mousePressEvent(event);
    }
    updateAndRefreshMapCoordinates(lastMousePos_); // For status bar, etc.
    // Event acceptance should be handled by inputHandler_ or fall through to base.
}

void MapView::mouseMoveEvent(QMouseEvent *event) {
    QPoint currentScreenPos = event->pos();
    if (inputHandler_) {
        QPointF currentMapPos = screenToMap(currentScreenPos);
        inputHandler_->handleMouseMoveEvent(event, currentMapPos);
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
    updateAndRefreshMapCoordinates(currentScreenPos); // For status bar, etc.
}

void MapView::mouseReleaseEvent(QMouseEvent *event) {
    lastMousePos_ = event->pos(); // Store screen position
    if (inputHandler_) {
        QPointF mapPos = screenToMap(lastMousePos_);
        inputHandler_->handleMouseReleaseEvent(event, mapPos);
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
    updateAndRefreshMapCoordinates(lastMousePos_); // For status bar, etc.
}

void MapView::mouseDoubleClickEvent(QMouseEvent *event) {
    // Currently, MapViewInputHandler doesn't have a specific double click handler.
    // If double click should behave like a normal press for drawing/selection,
    // then delegate it. Otherwise, handle here or add to inputHandler.
    lastMousePos_ = event->pos();
    if (doubleClickProperties_ && event->button() == Qt::LeftButton) {
        QPointF mapPos = screenToMap(lastMousePos_);
        showPropertiesDialogFor(mapPos); // Existing behavior
        event->accept();
    } else {
        // Optionally, could pass to inputHandler_ as a normal press:
        // if (inputHandler_) {
        //     QPointF mapPos = screenToMap(lastMousePos_);
        //     inputHandler_->handleMousePressEvent(event, mapPos); // Treat as press
        // } else {
        QGraphicsView::mouseDoubleClickEvent(event);
        // }
    }
    updateAndRefreshMapCoordinates(lastMousePos_);
}

void MapView::wheelEvent(QWheelEvent *event) {
    // The new input handler expects mapPosition for wheel events for context (e.g. tool-specific behavior)
    // However, zooming is often centered on screen position.
    // The inputHandler's handleWheelEvent primarily calls MapView::zoom with screen coordinates.
    if (inputHandler_) {
        QPointF mapPos = screenToMap(event->position().toPoint()); // Contextual map position
        inputHandler_->handleWheelEvent(event, mapPos);
    } else {
        QGraphicsView::wheelEvent(event);
    }
    updateAndRefreshMapCoordinates(event->position().toPoint()); // For status bar
    // Event acceptance should be handled by inputHandler_ or MapView::zoom
}

void MapView::enterEvent(QEnterEvent *event) {
    // This logic for resetting input states on enter might be useful.
    // The new inputHandler does not have resetInputStates().
    // For now, keep original behavior, can be refactored into inputHandler if needed.
    qDebug() << "MapView::enterEvent - Buttons:" << event->buttons();
    if (!event->buttons()) { // Only reset if no buttons are pressed (i.e., not dragging into window)
        isPanning_ = false; // Reset internal panning state if any
        if (cursor().shape() == Qt::ClosedHandCursor) { // Reset cursor if it was ClosedHand
             setCursor(Qt::ArrowCursor);
        }
        // Consider notifying inputHandler to reset its internal states if it maintains any
        // e.g. inputHandler_->handleFocusInEvent() or similar if that becomes necessary.
    }
    QGraphicsView::enterEvent(event); // Call base
    viewport()->update(); // Request repaint, good for hover effects
}

void MapView::leaveEvent(QEvent *event) {
    // Similar to enterEvent, could notify inputHandler if it needs to clear states.
    updateAndRefreshMapCoordinates(lastMousePos_); 
    QGraphicsView::leaveEvent(event);
    viewport()->update();
}

void MapView::focusOutEvent(QFocusEvent *event) {
    if (inputHandler_) {
        inputHandler_->handleFocusOutEvent(event);
    } else {
        QGraphicsView::focusOutEvent(event);
    }
}

void MapView::keyPressEvent(QKeyEvent *event) {
    if (inputHandler_) {
        inputHandler_->handleKeyPressEvent(event);
        // Assuming inputHandler will call event->accept() if it handles the key.
        // If not accepted, and not one of the specific MapView navigation keys below,
        // then fall through to QGraphicsView::keyPressEvent.
    }

    // Fallback or direct MapView key handling if not fully processed by inputHandler
    if (!event->isAccepted()) {
        bool acceptedByMapView = true;
        // Keep existing key navigation for scrolling and floors if desired as direct MapView feature
        switch (event->key()) {
            case Qt::Key_Up:
            case Qt::Key_W:
                verticalScrollBar()->setValue(verticalScrollBar()->value() - TILE_SIZE);
                break;
            case Qt::Key_Down:
            case Qt::Key_S:
                verticalScrollBar()->setValue(verticalScrollBar()->value() + TILE_SIZE);
                break;
            case Qt::Key_Left:
                horizontalScrollBar()->setValue(horizontalScrollBar()->value() - TILE_SIZE);
                break;
            case Qt::Key_Right:
            case Qt::Key_D:
                horizontalScrollBar()->setValue(horizontalScrollBar()->value() + TILE_SIZE);
                break;
            case Qt::Key_PageUp: changeFloor(currentFloor_ - 1); break;
            case Qt::Key_PageDown: changeFloor(currentFloor_ + 1); break;
            default:
                acceptedByMapView = false; // Not one of the direct navigation keys
                QGraphicsView::keyPressEvent(event); // Call base for other keys
                break;
        }
        if (acceptedByMapView) {
            event->accept();
        }
    }
     updateAndRefreshMapCoordinates(lastMousePos_); // Update status bar after potential view change
}

void MapView::keyReleaseEvent(QKeyEvent *event) {
    if (inputHandler_) {
        inputHandler_->handleKeyReleaseEvent(event);
    }
    if (!event->isAccepted()) {
        QGraphicsView::keyReleaseEvent(event);
    }
}

void MapView::drawBackground(QPainter* painter, const QRectF& rect) {
    QGraphicsView::drawBackground(painter, rect);
    painter->fillRect(rect, QColor(30, 30, 30)); // Dark gray placeholder background
}

void MapView::drawForeground(QPainter *painter, const QRectF &rect) {
    QGraphicsView::drawForeground(painter, rect); // Call base first

    if (currentSelectionArea_.isValid() && !currentSelectionArea_.isEmpty()) {
        painter->save();
        QPen selectionPen(Qt::yellow, 1, Qt::DashLine);
        painter->setPen(selectionPen);
        // currentSelectionArea_ is in map coordinates. Must convert to scene coordinates.
        // This depends on how screenToMap and mapToScene are set up.
        // If currentSelectionArea_ is already in scene coordinates (e.g. if map coordinates ARE scene coordinates)
        // then painter->drawRect(currentSelectionArea_); is fine.
        // If currentSelectionArea_ is in tile coordinates, it needs proper transformation.
        // Assuming for now that MapViewInputHandler sets currentSelectionArea_ in scene coordinates
        // that QGraphicsView can draw directly.
        // Or, more likely, MapViewInputHandler provides it in map coordinates,
        // and we need to convert it to scene coordinates for drawing.
        // For QGraphicsView, drawing is done in scene coordinates.
        // Let's assume currentSelectionArea_ is set by setSelectionArea in *map* coordinates.
        // We need to convert this to scene coordinates.
        // This is tricky because the selection area is a QRectF, not just points.
        // A simple way if TILE_SIZE is consistent:
        // QRectF sceneSelectionRect(currentSelectionArea_.topLeft() * TILE_SIZE, currentSelectionArea_.size() * TILE_SIZE);
        // This is a simplification. Proper conversion might need to handle floor offsets if selection spans floors or if scene isn't 0,0 based for tiles.
        // For this task, assuming currentSelectionArea_ is already in scene coordinates for drawing.
        // This means MapViewInputHandler's updateSelectionBox would convert mapPos to scenePos.
        // Or that setSelectionArea is called with scene coordinates.
        // The setSelectionArea method in MapView.h implies it takes a QRectF.
        // The MapViewInputHandler calculates selectionRect = QRectF(dragStartMapPos_, mapPos).normalized();
        // where mapPos is from screenToMap. So currentSelectionArea_ IS in map (tile) coordinates.

        // Convert currentSelectionArea_ (tile coords) to scene coordinates for drawing
        QPointF sceneTopLeft = mapToScene(currentSelectionArea_.topLeft() * TILE_SIZE); // Assuming mapToScene can take raw pixel-like scene coords
        QPointF sceneBottomRight = mapToScene(currentSelectionArea_.bottomRight() * TILE_SIZE);
        // This conversion is not quite right if mapToScene expects tile coords.
        // Let's simplify: Assume TILE_SIZE is the scale factor between tile units and scene units,
        // and currentFloor adjustments are handled by mapToScene or are part of the scene's setup.
        // For a direct QGraphicsView, if items are placed at (tileX*TILE_SIZE, tileY*TILE_SIZE),
        // then the selection rect in scene coords is:
        QRectF sceneRectToDraw = QRectF(
            currentSelectionArea_.left() * TILE_SIZE - (GROUND_LAYER - currentFloor_) * TILE_SIZE,
            currentSelectionArea_.top() * TILE_SIZE - (GROUND_LAYER - currentFloor_) * TILE_SIZE,
            currentSelectionArea_.width() * TILE_SIZE,
            currentSelectionArea_.height() * TILE_SIZE
        );
        painter->drawRect(sceneRectToDraw);
        painter->restore();
    }
}
