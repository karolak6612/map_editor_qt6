#include "MapView.h"
#include "MapViewInputHandler.h" // Ensure this is the correct new handler
#include "Brush.h" // Include Brush.h for Brush type
#include "BrushManager.h" // Added
#include "Map.h"          // Added
#include "QUndoStack.h"   // Added
#include "MapDrawingPrimitives.h" // Task 65: Drawing primitives
#include "WaypointBrush.h" // Task 74: Waypoint brush
#include "Waypoint.h"      // Task 74: Waypoint
#include "WaypointItem.h"  // Task 74: Waypoint item
#include "DrawingOptions.h" // Task 75: Drawing options
#include "MapViewEnhancements.h" // Task 80: Enhanced MapView features
#include <QGraphicsScene>
#include <QScrollBar>
#include <QPainter> // Added for drawForeground
#include <QDebug>
#include <QtMath> // For qBound
#include <QKeyEvent> // Already present
#include <QFocusEvent> // Added for focusOutEvent
#include <QToolTip> // Task 74: For waypoint tooltips

// --- Selection Implementation (Task 61) ---
void MapView::pasteSelection(const QPointF& mapPos) {
    if (!map_ || !map_->getSelection()) {
        qDebug() << "MapView::pasteSelection: No map or selection available";
        return;
    }

    // TODO: Implement paste functionality when copy/paste system is available
    qDebug() << "MapView::pasteSelection at" << mapPos << "(not yet implemented)";
}

void MapView::clearSelection() {
    if (!map_ || !map_->getSelection()) {
        return;
    }

    Selection* selection = map_->getSelection();
    selection->start(Selection::NONE);
    selection->clear();
    selection->finish(Selection::NONE);

    // Update visual representation
    updateSelectionVisuals();
    update();
}

void MapView::toggleSelectionAt(const QPointF& mapPos) {
    if (!map_ || !map_->getSelection()) {
        return;
    }

    MapPos tilePos = mapToTilePos(mapPos);
    Selection* selection = map_->getSelection();

    selection->start(Selection::NONE);

    if (selection->isSelected(tilePos)) {
        selection->removeTile(tilePos);
    } else {
        selection->addTile(tilePos);
    }

    selection->finish(Selection::NONE);

    // Update visual representation
    updateSelectionVisuals();
    update();
}

bool MapView::isOnSelection(const QPointF& mapPos) const {
    if (!map_ || !map_->getSelection()) {
        return false;
    }

    MapPos tilePos = mapToTilePos(mapPos);
    return map_->getSelection()->isSelected(tilePos);
}

void MapView::selectObjectAt(const QPointF& mapPos) {
    if (!map_ || !map_->getSelection()) {
        return;
    }

    MapPos tilePos = mapToTilePos(mapPos);
    Tile* tile = map_->getTile(tilePos.x, tilePos.y, tilePos.z);

    if (!tile) {
        clearSelection();
        return;
    }

    Selection* selection = map_->getSelection();
    selection->start(Selection::NONE);
    selection->clear();

    // Select the tile and its top item (wxwidgets behavior)
    selection->add(tile);

    selection->finish(Selection::NONE);

    // Update visual representation
    updateSelectionVisuals();
    update();
}

void MapView::updateMoveSelectionFeedback(const QPointF& delta) {
    // Update visual feedback for moving selection
    selectionMoveOffset_ = delta;
    update();
}

void MapView::finalizeMoveSelection(const QPointF& delta) {
    if (!map_ || !map_->getSelection()) {
        return;
    }

    // TODO: Implement actual selection moving when move system is available
    selectionMoveOffset_ = QPointF(0, 0);
    update();

    qDebug() << "MapView::finalizeMoveSelection by" << delta << "(move system not yet implemented)";
}

void MapView::updateSelectionRectFeedback(const QPointF& startMapPos, const QPointF& currentMapPos) {
    // Update the selection rectangle for visual feedback
    currentSelectionArea_ = QRectF(startMapPos, currentMapPos).normalized();
    update();
}
void MapView::finalizeSelectionRect(const QPointF& startMapPos, const QPointF& endMapPos, Qt::KeyboardModifiers modifiers) {
    if (!map_ || !map_->getSelection()) {
        return;
    }

    QRectF selectionRect = QRectF(startMapPos, endMapPos).normalized();
    Selection* selection = map_->getSelection();

    // Determine selection mode based on modifiers (wxwidgets behavior)
    bool addToSelection = (modifiers & Qt::ShiftModifier) && (modifiers & Qt::ControlModifier);
    bool newSelection = (modifiers & Qt::ShiftModifier) && !(modifiers & Qt::ControlModifier);
    bool toggleSelection = (modifiers & Qt::ControlModifier) && !(modifiers & Qt::ShiftModifier);

    selection->start(Selection::NONE);

    if (newSelection) {
        // Shift only: Clear existing selection and select new area
        selection->clear();
    }

    // Convert rectangle to tile coordinates
    MapPos startTile = mapToTilePos(selectionRect.topLeft());
    MapPos endTile = mapToTilePos(selectionRect.bottomRight());

    // Ensure proper bounds
    int minX = qMax(0, qMin(startTile.x, endTile.x));
    int maxX = qMin(map_->getWidth() - 1, qMax(startTile.x, endTile.x));
    int minY = qMax(0, qMin(startTile.y, endTile.y));
    int maxY = qMin(map_->getHeight() - 1, qMax(startTile.y, endTile.y));

    // Select tiles in the rectangle
    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            MapPos tilePos(x, y, currentFloor_);
            Tile* tile = map_->getTile(x, y, currentFloor_);

            if (!tile) continue;

            if (toggleSelection) {
                // Ctrl only: Toggle each tile
                if (selection->isSelected(tilePos)) {
                    selection->removeTile(tilePos);
                } else {
                    selection->addTile(tilePos);
                }
            } else {
                // Shift+Ctrl or Shift only: Add to selection
                selection->addTile(tilePos);
            }
        }
    }

    selection->finish(Selection::NONE);

    // Clear the visual selection rectangle
    currentSelectionArea_ = QRectF();

    // Update visual representation
    updateSelectionVisuals();
    update();
}

// Helper methods for selection (Task 61)
MapPos MapView::mapToTilePos(const QPointF& mapPos) const {
    // Convert map coordinates to tile position
    int tileX = static_cast<int>(qFloor(mapPos.x()));
    int tileY = static_cast<int>(qFloor(mapPos.y()));
    return MapPos(tileX, tileY, currentFloor_);
}

void MapView::updateSelectionVisuals() {
    // Update visual representation of selection
    // This will be called whenever selection changes
    if (!map_ || !map_->getSelection()) {
        return;
    }

    // Force a repaint to show selection changes
    update();
}

// Drawing related placeholders are removed as they are now delegated to brushes.

void MapView::updateStatusBarWithMapPos(const QPointF& mapPos) {
    qDebug() << "MapView::updateStatusBarWithMapPos: Tile(" << mapPos.x() << "," << mapPos.y() << ") Floor:" << currentFloor_ << "Zoom:" << zoomLevel_;

    // Emit signal for MainWindow to update status bar
    emit statusUpdateRequested("coordinates", QVariantMap{
        {"x", mapPos.x()},
        {"y", mapPos.y()},
        {"z", currentFloor_}
    });
}

void MapView::updateZoomStatus() {
    qDebug() << "MapView::updateZoomStatus: Zoom:" << zoomLevel_ << "(placeholder)";

    // Emit signal for MainWindow to update status bar
    emit statusUpdateRequested("zoom", QVariantMap{
        {"level", zoomLevel_}
    });
}

void MapView::updateFloorStatus() {
    qDebug() << "MapView::updateFloorStatus: Floor:" << currentFloor_ << "(placeholder)";

    // Emit signal for MainWindow to update status bar
    emit statusUpdateRequested("floor", QVariantMap{
        {"layer", currentFloor_}
    });
}

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
    currentSelectionArea_(), // Initialize currentSelectionArea_
    waypointBrush_(nullptr), // Task 74: Initialize waypoint brush
    waypointsVisible_(true), // Task 74: Waypoints visible by default
    selectedWaypoint_(nullptr), // Task 74: No waypoint selected initially
    waypointTooltip_(nullptr), // Task 74: No tooltip initially
    tooltipTimer_(nullptr), // Task 74: No tooltip timer initially
    lastTooltipPosition_(), // Task 74: Initialize tooltip position
    drawingOptions_(), // Task 75: Initialize drawing options with defaults
    // Task 80: Initialize enhancement systems
    zoomSystem_(nullptr),
    gridSystem_(nullptr),
    mouseTracker_(nullptr),
    drawingFeedback_(nullptr),
    // Task 85: Initialize tile locking visual system
    showLockedTiles_(true),
    lockedTileOverlayColor_(QColor(255, 0, 0, 100))
{
    setScene(new QGraphicsScene(this));
    setMouseTracking(true); // Important for hover effects and map coordinate updates
    // setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // Good for zooming towards mouse
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setFocusPolicy(Qt::StrongFocus); // To receive key events

    // Instantiate the new input handler
    inputHandler_ = new MapViewInputHandler(this, brushManager, map, undoStack, this);

    // Task 65: Initialize drawing primitives and overlay renderer
    drawingPrimitives_ = new MapDrawingPrimitives();
    overlayRenderer_ = new MapOverlayRenderer(drawingPrimitives_);

    // Task 75: Set default drawing options
    drawingOptions_.setDefaultMode();

    // Task 80: Initialize enhancement systems
    zoomSystem_ = new MapViewZoomSystem(this, this);
    gridSystem_ = new MapViewGridSystem(this, this);
    mouseTracker_ = new MapViewMouseTracker(this, this);
    drawingFeedback_ = new MapViewDrawingFeedback(this, this);

    // Connect enhancement system signals
    connect(zoomSystem_, &MapViewZoomSystem::zoomChanged, this, &MapView::updateZoomStatus);
    connect(mouseTracker_, &MapViewMouseTracker::mousePositionChanged,
            this, [this](const QPointF& screenPos, const QPointF& mapPos) {
                updateStatusBarWithMapPos(mapPos);
            });

    updateAndRefreshMapCoordinates(QPoint(viewport()->width()/2, viewport()->height()/2)); // Use viewport size
    updateZoomStatus();
    updateFloorStatus();
}

MapView::~MapView() {
    delete inputHandler_; // MapView owns inputHandler_
    inputHandler_ = nullptr;

    // Task 65: Clean up drawing primitives
    delete overlayRenderer_;
    delete drawingPrimitives_;
    overlayRenderer_ = nullptr;
    drawingPrimitives_ = nullptr;

    // Task 80: Clean up enhancement systems
    delete zoomSystem_;
    delete gridSystem_;
    delete mouseTracker_;
    delete drawingFeedback_;
    zoomSystem_ = nullptr;
    gridSystem_ = nullptr;
    mouseTracker_ = nullptr;
    drawingFeedback_ = nullptr;

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

void MapView::zoomIn() {
    if (zoomSystem_) {
        zoomSystem_->zoomIn();
    } else {
        // Fallback to original implementation
        QPointF centerPos = mapToScene(viewport()->rect().center());
        zoom(1.25, centerPos); // 25% zoom in
    }
}

void MapView::zoomOut() {
    if (zoomSystem_) {
        zoomSystem_->zoomOut();
    } else {
        // Fallback to original implementation
        QPointF centerPos = mapToScene(viewport()->rect().center());
        zoom(0.8, centerPos); // 20% zoom out
    }
}

void MapView::resetZoom() {
    if (zoomSystem_) {
        zoomSystem_->resetZoom();
    } else {
        // Fallback to original implementation
        QPointF centerPos = mapToScene(viewport()->rect().center());
        double currentZoom = zoomLevel_;
        double targetZoom = 1.0;
        zoom(targetZoom / currentZoom, centerPos);
    }
}

void MapView::centerOnMap() {
    if (scene()) {
        QRectF sceneRect = scene()->sceneRect();
        centerOn(sceneRect.center());
    }
}

// Task 80: Enhanced zoom operations
void MapView::zoomToLevel(double level, const QPointF& centerPoint) {
    if (zoomSystem_) {
        zoomSystem_->zoomToLevel(level, centerPoint);
    }
}

void MapView::zoomToFit(const QRectF& rect) {
    if (zoomSystem_) {
        zoomSystem_->zoomToFit(rect);
    }
}

void MapView::handleWheelZoom(QWheelEvent* event) {
    if (zoomSystem_) {
        zoomSystem_->handleWheelEvent(event);
    }
}

// Task 80: Enhanced grid operations
void MapView::setGridVisible(bool visible) {
    if (gridSystem_) {
        gridSystem_->setGridVisible(visible);
    }
}

bool MapView::isGridVisible() const {
    return gridSystem_ ? gridSystem_->isGridVisible() : false;
}

void MapView::setGridType(int type) {
    if (gridSystem_) {
        gridSystem_->setGridType(static_cast<MapViewGridSystem::GridType>(type));
    }
}

void MapView::setGridColor(const QColor& color) {
    if (gridSystem_) {
        gridSystem_->setGridColor(color);
    }
}

// Task 80: Enhanced mouse tracking
void MapView::setMouseTrackingEnabled(bool enabled) {
    if (mouseTracker_) {
        mouseTracker_->setMouseTrackingEnabled(enabled);
    }
}

void MapView::setHoverEffectsEnabled(bool enabled) {
    if (mouseTracker_) {
        mouseTracker_->setHoverEffectsEnabled(enabled);
    }
}

QPointF MapView::getCurrentMousePosition() const {
    return mouseTracker_ ? mouseTracker_->getCurrentMousePosition() : QPointF();
}

QPointF MapView::getCurrentMapPosition() const {
    return mouseTracker_ ? mouseTracker_->getCurrentMapPosition() : QPointF();
}

// Task 80: Enhanced drawing feedback
void MapView::setBrushPreview(const QPointF& position, int size, const QColor& color) {
    if (drawingFeedback_) {
        drawingFeedback_->setBrushPreview(position, size, color);
    }
}

void MapView::clearBrushPreview() {
    if (drawingFeedback_) {
        drawingFeedback_->clearBrushPreview();
    }
}

void MapView::setDrawingIndicator(const QPointF& position, const QString& text) {
    if (drawingFeedback_) {
        drawingFeedback_->setDrawingIndicator(position, text);
    }
}

void MapView::clearDrawingIndicator() {
    if (drawingFeedback_) {
        drawingFeedback_->clearDrawingIndicator();
    }
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
    qDebug() << "MapView::switchMouseButtons set to:" << switched;
}

QPointF MapView::screenToMap(const QPoint& screenPos) const {
    // Enhanced implementation matching wxWidgets ScreenToMap behavior
    QPointF scenePos = mapToScene(screenPos);

    // Apply content scale factor (equivalent to wxWidgets GetContentScaleFactor)
    double scaleFactor = devicePixelRatio();
    double adjustedX = scenePos.x() * scaleFactor;
    double adjustedY = scenePos.y() * scaleFactor;

    // Convert to map coordinates with zoom consideration
    double mapX, mapY;
    if (adjustedX < 0) {
        mapX = adjustedX / TILE_SIZE;
    } else {
        mapX = (adjustedX * zoomLevel_) / TILE_SIZE;
    }

    if (adjustedY < 0) {
        mapY = adjustedY / TILE_SIZE;
    } else {
        mapY = (adjustedY * zoomLevel_) / TILE_SIZE;
    }

    // Apply floor offset (matching wxWidgets logic)
    if (currentFloor_ <= GROUND_LAYER) {
        mapX += GROUND_LAYER - currentFloor_;
        mapY += GROUND_LAYER - currentFloor_;
    }

    return QPointF(mapX, mapY);
}

QPoint MapView::mapToScreen(const QPointF& mapTilePos) const {
    // Enhanced inverse of screenToMap matching wxWidgets behavior
    double mapX = mapTilePos.x();
    double mapY = mapTilePos.y();

    // Remove floor offset
    if (currentFloor_ <= GROUND_LAYER) {
        mapX -= GROUND_LAYER - currentFloor_;
        mapY -= GROUND_LAYER - currentFloor_;
    }

    // Convert to scene coordinates with zoom consideration
    double sceneX = mapX * TILE_SIZE / zoomLevel_;
    double sceneY = mapY * TILE_SIZE / zoomLevel_;

    // Apply content scale factor
    double scaleFactor = devicePixelRatio();
    sceneX /= scaleFactor;
    sceneY /= scaleFactor;

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
    lastMousePos_ = event->pos(); // Store screen position

    // Update state tracking variables (wxWidgets compatibility)
    lastClickScreenPos_ = lastMousePos_;
    lastClickMapPos_ = screenToMap(lastMousePos_);
    lastClickMapZ_ = currentFloor_;

    // Calculate absolute position (for wxWidgets compatibility)
    lastClickAbsPos_ = QPoint(
        static_cast<int>(lastMousePos_.x() * zoomLevel_) + horizontalScrollBar()->value(),
        static_cast<int>(lastMousePos_.y() * zoomLevel_) + verticalScrollBar()->value()
    );

    if (inputHandler_) {
        inputHandler_->handleMousePressEvent(event, lastClickMapPos_);
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

    // Task 74: Update waypoint tooltips on mouse move
    if (waypointsVisible_) {
        updateWaypointTooltip(currentScreenPos);
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
    // Task 80: Enhanced wheel event handling with zoom system integration
    if (event->modifiers() & Qt::ControlModifier && zoomSystem_) {
        // Use enhanced zoom system for Ctrl+wheel zoom
        zoomSystem_->handleWheelEvent(event);
    } else if (inputHandler_) {
        // The new input handler expects mapPosition for wheel events for context (e.g. tool-specific behavior)
        // However, zooming is often centered on screen position.
        // The inputHandler's handleWheelEvent primarily calls MapView::zoom with screen coordinates.
        QPointF mapPos = screenToMap(event->position().toPoint()); // Contextual map position
        inputHandler_->handleWheelEvent(event, mapPos);
    } else {
        QGraphicsView::wheelEvent(event);
    }
    updateAndRefreshMapCoordinates(event->position().toPoint()); // For status bar
    // Event acceptance should be handled by inputHandler_ or MapView::zoom
}

void MapView::enterEvent(QEnterEvent *event) {
    // Enhanced enter event handling matching wxWidgets OnGainMouse behavior
    qDebug() << "MapView::enterEvent - Buttons:" << event->buttons();

    if (!event->buttons()) { // Only reset if no buttons are pressed (i.e., not dragging into window)
        // Reset various state flags (matching wxWidgets behavior)
        isPanning_ = false;
        isScreenDragging_ = false;
        isDragging_ = false;
        isDrawing_ = false;

        if (cursor().shape() == Qt::ClosedHandCursor) { // Reset cursor if it was ClosedHand
             setCursor(Qt::ArrowCursor);
        }

        // Reset input handler state if needed
        if (inputHandler_) {
            // inputHandler_->resetStates(); // Could add this method if needed
        }
    }

    // Task 80: Notify mouse tracker of enter event
    if (mouseTracker_) {
        QMouseEvent mouseEvent(QEvent::MouseMove, event->position(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        mouseTracker_->handleMouseEnterEvent(&mouseEvent);
    }

    QGraphicsView::enterEvent(event); // Call base
    viewport()->update(); // Request repaint, good for hover effects
}

void MapView::leaveEvent(QEvent *event) {
    // Enhanced leave event handling matching wxWidgets OnLoseMouse behavior
    qDebug() << "MapView::leaveEvent";

    // Update coordinates one last time before leaving
    updateAndRefreshMapCoordinates(lastMousePos_);

    // Task 80: Notify mouse tracker of leave event
    if (mouseTracker_) {
        QMouseEvent mouseEvent(QEvent::MouseMove, QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        mouseTracker_->handleMouseLeaveEvent(&mouseEvent);
    }

    // Could reset some states or notify input handler
    // For now, keep minimal behavior to avoid interfering with ongoing operations

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
        Qt::KeyboardModifiers modifiers = event->modifiers();

        // Enhanced key navigation with zoom support
        switch (event->key()) {
            // Navigation keys (matching wxWidgets OnKeyDown behavior)
            case Qt::Key_Up:
            case Qt::Key_W:
                if (modifiers & Qt::ControlModifier) {
                    verticalScrollBar()->setValue(verticalScrollBar()->value() - TILE_SIZE * 10);
                } else {
                    int tiles = (zoomLevel_ == 1.0) ? 1 : 3;
                    verticalScrollBar()->setValue(verticalScrollBar()->value() - TILE_SIZE * tiles * zoomLevel_);
                }
                updateStatusBarWithMapPos(screenToMap(lastMousePos_));
                break;
            case Qt::Key_Down:
            case Qt::Key_S:
                if (modifiers & Qt::ControlModifier) {
                    // Ctrl+S is handled by menu action, don't interfere
                    acceptedByMapView = false;
                } else {
                    int tiles = (zoomLevel_ == 1.0) ? 1 : 3;
                    verticalScrollBar()->setValue(verticalScrollBar()->value() + TILE_SIZE * tiles * zoomLevel_);
                    updateStatusBarWithMapPos(screenToMap(lastMousePos_));
                }
                break;
            case Qt::Key_Left:
            case Qt::Key_A:
                if (modifiers & Qt::ControlModifier) {
                    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - TILE_SIZE * 10);
                } else {
                    int tiles = (zoomLevel_ == 1.0) ? 1 : 3;
                    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - TILE_SIZE * tiles * zoomLevel_);
                }
                updateStatusBarWithMapPos(screenToMap(lastMousePos_));
                break;
            case Qt::Key_Right:
            case Qt::Key_D:
                if (modifiers & Qt::ControlModifier) {
                    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + TILE_SIZE * 10);
                } else {
                    int tiles = (zoomLevel_ == 1.0) ? 1 : 3;
                    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + TILE_SIZE * tiles * zoomLevel_);
                }
                updateStatusBarWithMapPos(screenToMap(lastMousePos_));
                break;

            // Floor navigation
            case Qt::Key_PageUp:
                changeFloor(currentFloor_ - 1);
                break;
            case Qt::Key_PageDown:
                changeFloor(currentFloor_ + 1);
                break;

            // Zoom shortcuts (matching wxWidgets behavior)
            case Qt::Key_Plus:
            case Qt::Key_Equal: // For keyboards where + requires shift
                if (modifiers & Qt::ControlModifier) {
                    zoomIn();
                } else {
                    acceptedByMapView = false;
                }
                break;
            case Qt::Key_Minus:
                if (modifiers & Qt::ControlModifier) {
                    zoomOut();
                } else {
                    acceptedByMapView = false;
                }
                break;
            case Qt::Key_0:
                if (modifiers & Qt::ControlModifier) {
                    resetZoom();
                } else {
                    acceptedByMapView = false;
                }
                break;

            // Additional navigation shortcuts
            case Qt::Key_Home:
                if (modifiers & Qt::ControlModifier) {
                    centerOnMap();
                } else {
                    acceptedByMapView = false;
                }
                break;

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

    // Task 80: Render enhanced grid system
    if (gridSystem_) {
        double currentZoom = zoomSystem_ ? zoomSystem_->getCurrentZoom() : zoomLevel_;
        gridSystem_->renderGrid(painter, rect, currentZoom);
    }

    // Task 80: Render enhanced drawing feedback
    if (drawingFeedback_) {
        drawingFeedback_->renderFeedback(painter, rect);
    }

    // Task 65: Use overlay renderer for all drawing primitives
    if (overlayRenderer_) {
        // Update selection area in overlay renderer
        if (currentSelectionArea_.isValid() && !currentSelectionArea_.isEmpty()) {
            overlayRenderer_->setSelectionArea(currentSelectionArea_);
        } else {
            overlayRenderer_->clearSelection();
        }

        // Render all overlays (selection, brush preview, grid, etc.)
        overlayRenderer_->renderOverlays(painter, rect, currentFloor_);
    }
}

// Task 65: Drawing primitives integration methods
void MapView::setShowGrid(bool show) {
    if (overlayRenderer_) {
        overlayRenderer_->setShowGrid(show);
        viewport()->update();
    }
}

void MapView::setShowBrushPreview(bool show) {
    if (overlayRenderer_) {
        overlayRenderer_->setShowBrushPreview(show);
        viewport()->update();
    }
}

void MapView::setBrushPreviewState(const QPointF& position, Brush* brush, int size, bool isValid) {
    if (overlayRenderer_) {
        BrushValidity validity = isValid ? BrushValidity::Valid : BrushValidity::Invalid;
        BrushShape shape = BrushShape::Square; // Default to square, could be determined from brush properties

        overlayRenderer_->setBrushPreviewState(position, brush, size, shape, validity);
        viewport()->update();
    }
}

void MapView::clearBrushPreview() {
    if (overlayRenderer_) {
        overlayRenderer_->clearBrushPreview();
        viewport()->update();
    }
}

void MapView::updateBrushPreview(const QPointF& mousePos) {
    if (!overlayRenderer_ || !currentBrush_) {
        clearBrushPreview();
        return;
    }

    // Get brush size from brush manager or use default
    int brushSize = 1; // This would come from BrushManager or Brush properties

    // Determine brush shape from brush properties
    BrushShape shape = BrushShape::Square; // Default
    // This could be determined from brush properties:
    // if (currentBrush_->getShape() == "circle") shape = BrushShape::Circle;

    // Determine validity based on brush->canDraw() logic
    BrushValidity validity = BrushValidity::Neutral;
    // This would use actual brush validation:
    // if (currentBrush_->canDraw(map, mousePos)) validity = BrushValidity::Valid;
    // else validity = BrushValidity::Invalid;

    overlayRenderer_->setBrushPreviewState(mousePos, currentBrush_, brushSize, shape, validity);
    viewport()->update();
}

// Task 74: Enhanced waypoint interaction implementation
void MapView::setWaypointBrush(WaypointBrush* brush) {
    waypointBrush_ = brush;
    qDebug() << "MapView::setWaypointBrush to:" << (brush ? brush->getName() : "nullptr");
}

bool MapView::isWaypointToolActive() const {
    return waypointBrush_ != nullptr && currentBrush_ == waypointBrush_;
}

void MapView::placeWaypointAt(const QPointF& mapPos) {
    if (!waypointBrush_ || !map_) {
        qDebug() << "MapView::placeWaypointAt: No waypoint brush or map available";
        return;
    }

    // Convert map position to tile coordinates
    MapPos tilePos = mapToTilePos(mapPos);

    // Check if waypoint can be placed at this position
    QPointF tilePosF(tilePos.x, tilePos.y);
    if (!waypointBrush_->canDraw(map_, tilePosF)) {
        qDebug() << "MapView::placeWaypointAt: Cannot place waypoint at" << mapPos;
        return;
    }

    // Apply waypoint brush to place waypoint
    QUndoCommand* cmd = waypointBrush_->applyBrush(map_, tilePosF);
    if (cmd) {
        // TODO: Add to undo stack when available
        delete cmd; // For now, just delete the command
    }

    // Update waypoint visuals
    updateWaypointVisuals();

    qDebug() << "MapView::placeWaypointAt: Placed waypoint at" << mapPos;
}

void MapView::selectWaypoint(Waypoint* waypoint) {
    if (selectedWaypoint_ == waypoint) {
        return; // Already selected
    }

    // Deselect previous waypoint
    if (selectedWaypoint_) {
        highlightWaypoint(selectedWaypoint_, false);
    }

    selectedWaypoint_ = waypoint;

    // Highlight new waypoint
    if (selectedWaypoint_) {
        highlightWaypoint(selectedWaypoint_, true);
        qDebug() << "MapView::selectWaypoint:" << selectedWaypoint_->getName();
    } else {
        qDebug() << "MapView::selectWaypoint: Cleared selection";
    }

    viewport()->update();
}

void MapView::editWaypoint(Waypoint* waypoint) {
    if (!waypoint) {
        qDebug() << "MapView::editWaypoint: No waypoint provided";
        return;
    }

    // TODO: Open waypoint editing dialog
    qDebug() << "MapView::editWaypoint:" << waypoint->getName() << "(dialog not yet implemented)";

    // For now, just select the waypoint
    selectWaypoint(waypoint);
}

void MapView::deleteWaypoint(Waypoint* waypoint) {
    if (!waypoint || !map_) {
        qDebug() << "MapView::deleteWaypoint: No waypoint or map available";
        return;
    }

    // Remove from selection if selected
    if (selectedWaypoint_ == waypoint) {
        selectedWaypoint_ = nullptr;
    }

    // Remove waypoint item from scene
    if (waypointItems_.contains(waypoint)) {
        WaypointItem* item = waypointItems_.take(waypoint);
        if (item && scene()) {
            scene()->removeItem(item);
            delete item;
        }
    }

    // Remove waypoint from map
    map_->removeWaypoint(waypoint->getName());

    qDebug() << "MapView::deleteWaypoint:" << waypoint->getName();
    viewport()->update();
}

void MapView::updateWaypointVisuals() {
    if (!map_ || !scene()) {
        return;
    }

    // Get all waypoints from map
    QList<Waypoint*> waypoints = map_->getWaypoints();

    // Remove waypoint items that no longer exist
    QList<Waypoint*> toRemove;
    for (auto it = waypointItems_.begin(); it != waypointItems_.end(); ++it) {
        if (!waypoints.contains(it.key())) {
            toRemove.append(it.key());
        }
    }

    for (Waypoint* waypoint : toRemove) {
        WaypointItem* item = waypointItems_.take(waypoint);
        if (item) {
            scene()->removeItem(item);
            delete item;
        }
    }

    // Add or update waypoint items
    for (Waypoint* waypoint : waypoints) {
        if (!waypointItems_.contains(waypoint)) {
            // Create new waypoint item
            WaypointItem* item = new WaypointItem(waypoint, map_);
            waypointItems_[waypoint] = item;
            scene()->addItem(item);
        } else {
            // Update existing waypoint item
            WaypointItem* item = waypointItems_[waypoint];
            item->updateFromWaypoint();
        }
    }

    viewport()->update();
}

void MapView::refreshWaypointItems() {
    // Clear all waypoint items and recreate them
    for (auto it = waypointItems_.begin(); it != waypointItems_.end(); ++it) {
        WaypointItem* item = it.value();
        if (item && scene()) {
            scene()->removeItem(item);
            delete item;
        }
    }
    waypointItems_.clear();

    updateWaypointVisuals();
}

void MapView::setWaypointsVisible(bool visible) {
    if (waypointsVisible_ == visible) {
        return;
    }

    waypointsVisible_ = visible;

    // Update visibility of all waypoint items
    for (WaypointItem* item : waypointItems_.values()) {
        if (item) {
            item->setVisible(visible);
        }
    }

    viewport()->update();
    qDebug() << "MapView::setWaypointsVisible:" << visible;
}

// Task 74: Waypoint tooltips implementation
void MapView::showWaypointTooltip(Waypoint* waypoint, const QPoint& position) {
    if (!waypoint) {
        hideWaypointTooltip();
        return;
    }

    // Create tooltip text with waypoint information
    QString tooltipText = QString("Waypoint: %1").arg(waypoint->getName());

    if (!waypoint->getType().isEmpty() && waypoint->getType() != "generic") {
        tooltipText += QString("\nType: %1").arg(waypoint->getType());
    }

    if (waypoint->radius() > 1) {
        tooltipText += QString("\nRadius: %1").arg(waypoint->radius());
    }

    if (!waypoint->getScriptOrText().isEmpty()) {
        QString script = waypoint->getScriptOrText();
        if (script.length() > 50) {
            script = script.left(47) + "...";
        }
        tooltipText += QString("\nScript: %1").arg(script);
    }

    MapPos pos = waypoint->getPosition();
    tooltipText += QString("\nPosition: [%1, %2, %3]").arg(pos.x).arg(pos.y).arg(pos.z);

    // Show tooltip at the specified position
    QToolTip::showText(mapToGlobal(position), tooltipText, this);
    lastTooltipPosition_ = position;

    qDebug() << "MapView::showWaypointTooltip:" << waypoint->getName() << "at" << position;
}

void MapView::hideWaypointTooltip() {
    QToolTip::hideText();
    lastTooltipPosition_ = QPoint();
}

void MapView::updateWaypointTooltip(const QPoint& position) {
    if (!map_ || !scene()) {
        hideWaypointTooltip();
        return;
    }

    // Find waypoint at current position
    QPointF mapPos = screenToMap(position);
    Waypoint* waypointAtPos = nullptr;

    // Check all waypoints to see if mouse is over one
    QList<Waypoint*> waypoints = map_->getWaypoints();
    for (Waypoint* waypoint : waypoints) {
        if (!waypoint) continue;

        MapPos waypointPos = waypoint->getPosition();
        QPointF waypointMapPos(waypointPos.x, waypointPos.y);

        // Check if mouse is within waypoint area (considering radius)
        double distance = QPointF(mapPos - waypointMapPos).manhattanLength();
        double threshold = qMax(1.0, static_cast<double>(waypoint->radius()));

        if (distance <= threshold) {
            waypointAtPos = waypoint;
            break;
        }
    }

    if (waypointAtPos) {
        showWaypointTooltip(waypointAtPos, position);
    } else {
        hideWaypointTooltip();
    }
}

// Task 74: Waypoint centering implementation
void MapView::centerOnWaypoint(Waypoint* waypoint) {
    if (!waypoint) {
        qDebug() << "MapView::centerOnWaypoint: No waypoint provided";
        return;
    }

    MapPos waypointPos = waypoint->getPosition();
    QPointF mapPos(waypointPos.x, waypointPos.y);

    // Convert to scene coordinates
    QPointF scenePos = mapToScreen(mapPos);

    // Center the view on the waypoint
    centerOn(scenePos);

    // Update floor if necessary
    if (currentFloor_ != waypointPos.z) {
        changeFloor(waypointPos.z);
    }

    // Highlight the waypoint temporarily
    highlightWaypoint(waypoint, true);

    qDebug() << "MapView::centerOnWaypoint:" << waypoint->getName() << "at" << mapPos;
}

void MapView::highlightWaypoint(Waypoint* waypoint, bool highlight) {
    if (!waypoint || !waypointItems_.contains(waypoint)) {
        return;
    }

    WaypointItem* item = waypointItems_[waypoint];
    if (item) {
        item->setHighlighted(highlight);
        viewport()->update();
    }

    qDebug() << "MapView::highlightWaypoint:" << waypoint->getName() << "highlighted:" << highlight;
}

// Task 74: Helper methods for waypoint system
MapPos MapView::mapToTilePos(const QPointF& mapPos) const {
    return MapPos(static_cast<int>(mapPos.x()), static_cast<int>(mapPos.y()), currentFloor_);
}

QPointF MapView::screenToMap(const QPoint& screenPos) const {
    // Convert screen coordinates to scene coordinates
    QPointF scenePos = mapToScene(screenPos);

    // Convert scene coordinates to map coordinates
    // This assumes 1:1 mapping between scene and map coordinates
    return scenePos;
}

QPointF MapView::mapToScreen(const QPointF& mapPos) const {
    // Convert map coordinates to scene coordinates
    QPointF scenePos = mapPos;

    // Convert scene coordinates to screen coordinates
    return mapFromScene(scenePos);
}

// Task 75: View settings management implementation
void MapView::setDrawingOptions(const DrawingOptions& options) {
    DrawingOptions oldOptions = drawingOptions_;
    drawingOptions_ = options;

    // Check if options changed and update visuals if needed
    if (oldOptions != drawingOptions_) {
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setDrawingOptions: Updated drawing options";
    }
}

// Task 75: Individual view setting setters
void MapView::setShowGrid(bool show) {
    if (drawingOptions_.showGrid != show) {
        drawingOptions_.showGrid = show;
        viewport()->update();
        qDebug() << "MapView::setShowGrid:" << show;
    }
}

void MapView::setShowZones(bool show) {
    if (drawingOptions_.showZones != show) {
        drawingOptions_.showZones = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowZones:" << show;
    }
}

void MapView::setShowLights(bool show) {
    if (drawingOptions_.showLights != show) {
        drawingOptions_.showLights = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowLights:" << show;
    }
}

void MapView::setShowWaypoints(bool show) {
    if (drawingOptions_.showWaypoints != show) {
        drawingOptions_.showWaypoints = show;
        setWaypointsVisible(show); // Update waypoint visibility
        qDebug() << "MapView::setShowWaypoints:" << show;
    }
}

void MapView::setShowCreatures(bool show) {
    if (drawingOptions_.showCreatures != show) {
        drawingOptions_.showCreatures = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowCreatures:" << show;
    }
}

void MapView::setShowSpawns(bool show) {
    if (drawingOptions_.showSpawns != show) {
        drawingOptions_.showSpawns = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowSpawns:" << show;
    }
}

void MapView::setShowHouses(bool show) {
    if (drawingOptions_.showHouses != show) {
        drawingOptions_.showHouses = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowHouses:" << show;
    }
}

void MapView::setShowShade(bool show) {
    if (drawingOptions_.showShade != show) {
        drawingOptions_.showShade = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowShade:" << show;
    }
}

void MapView::setShowSpecialTiles(bool show) {
    if (drawingOptions_.showSpecialTiles != show) {
        drawingOptions_.showSpecialTiles = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowSpecialTiles:" << show;
    }
}

void MapView::setShowTechnicalItems(bool show) {
    if (drawingOptions_.showTechnicalItems != show) {
        drawingOptions_.showTechnicalItems = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowTechnicalItems:" << show;
    }
}

void MapView::setShowIngameBox(bool show) {
    if (drawingOptions_.showIngameBox != show) {
        drawingOptions_.showIngameBox = show;
        viewport()->update();
        qDebug() << "MapView::setShowIngameBox:" << show;
    }
}

void MapView::setShowWallHooks(bool show) {
    if (drawingOptions_.showWallHooks != show) {
        drawingOptions_.showWallHooks = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowWallHooks:" << show;
    }
}

void MapView::setShowTowns(bool show) {
    if (drawingOptions_.showTowns != show) {
        drawingOptions_.showTowns = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowTowns:" << show;
    }
}

void MapView::setShowBlocking(bool show) {
    if (drawingOptions_.showBlocking != show) {
        drawingOptions_.showBlocking = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowBlocking:" << show;
    }
}

void MapView::setHighlightItems(bool highlight) {
    if (drawingOptions_.highlightItems != highlight) {
        drawingOptions_.highlightItems = highlight;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setHighlightItems:" << highlight;
    }
}

void MapView::setHighlightLockedDoors(bool highlight) {
    if (drawingOptions_.highlightLockedDoors != highlight) {
        drawingOptions_.highlightLockedDoors = highlight;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setHighlightLockedDoors:" << highlight;
    }
}

void MapView::setShowExtra(bool show) {
    if (drawingOptions_.showExtra != show) {
        drawingOptions_.showExtra = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowExtra:" << show;
    }
}

void MapView::setShowAllFloors(bool show) {
    if (drawingOptions_.showAllFloors != show) {
        drawingOptions_.showAllFloors = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowAllFloors:" << show;
    }
}

void MapView::setGhostItems(bool ghost) {
    if (drawingOptions_.ghostItems != ghost) {
        drawingOptions_.ghostItems = ghost;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setGhostItems:" << ghost;
    }
}

void MapView::setGhostHigherFloors(bool ghost) {
    if (drawingOptions_.ghostHigherFloors != ghost) {
        drawingOptions_.ghostHigherFloors = ghost;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setGhostHigherFloors:" << ghost;
    }
}

void MapView::setTransparentFloors(bool transparent) {
    if (drawingOptions_.transparentFloors != transparent) {
        drawingOptions_.transparentFloors = transparent;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setTransparentFloors:" << transparent;
    }
}

void MapView::setTransparentItems(bool transparent) {
    if (drawingOptions_.transparentItems != transparent) {
        drawingOptions_.transparentItems = transparent;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setTransparentItems:" << transparent;
    }
}

void MapView::setShowPreview(bool show) {
    if (drawingOptions_.showPreview != show) {
        drawingOptions_.showPreview = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowPreview:" << show;
    }
}

void MapView::setShowTooltips(bool show) {
    if (drawingOptions_.showTooltips != show) {
        drawingOptions_.showTooltips = show;
        qDebug() << "MapView::setShowTooltips:" << show;
    }
}

void MapView::setShowAsMinimapColors(bool show) {
    if (drawingOptions_.showAsMinimapColors != show) {
        drawingOptions_.showAsMinimapColors = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowAsMinimapColors:" << show;
    }
}

void MapView::setShowOnlyColors(bool show) {
    if (drawingOptions_.showOnlyColors != show) {
        drawingOptions_.showOnlyColors = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowOnlyColors:" << show;
    }
}

void MapView::setShowOnlyModified(bool show) {
    if (drawingOptions_.showOnlyModified != show) {
        drawingOptions_.showOnlyModified = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setShowOnlyModified:" << show;
    }
}

void MapView::setAlwaysShowZones(bool show) {
    if (drawingOptions_.alwaysShowZones != show) {
        drawingOptions_.alwaysShowZones = show;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setAlwaysShowZones:" << show;
    }
}

void MapView::setIngameMode(bool ingame) {
    if (drawingOptions_.ingame != ingame) {
        drawingOptions_.ingame = ingame;
        if (ingame) {
            drawingOptions_.setIngameMode();
        } else {
            drawingOptions_.setDefaultMode();
        }
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setIngameMode:" << ingame;
    }
}

void MapView::setExperimentalFog(bool fog) {
    if (drawingOptions_.experimentalFog != fog) {
        drawingOptions_.experimentalFog = fog;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setExperimentalFog:" << fog;
    }
}

void MapView::setExtHouseShader(bool shader) {
    if (drawingOptions_.extHouseShader != shader) {
        drawingOptions_.extHouseShader = shader;
        updateAllTileItems();
        viewport()->update();
        qDebug() << "MapView::setExtHouseShader:" << shader;
    }
}

// Task 75: Convenience methods
void MapView::setDefaultViewSettings() {
    drawingOptions_.setDefaultMode();
    updateAllTileItems();
    viewport()->update();
    qDebug() << "MapView::setDefaultViewSettings: Applied default view settings";
}

void MapView::setIngameViewSettings() {
    drawingOptions_.setIngameMode();
    updateAllTileItems();
    viewport()->update();
    qDebug() << "MapView::setIngameViewSettings: Applied ingame view settings";
}

void MapView::updateAllTileItems() {
    if (!scene()) {
        return;
    }

    // Update all tile items in the scene to reflect new drawing options
    QList<QGraphicsItem*> items = scene()->items();
    for (QGraphicsItem* item : items) {
        // Check if item is a tile item and update it
        // This will be implemented when MapTileItem is enhanced to use DrawingOptions
        item->update();
    }

    qDebug() << "MapView::updateAllTileItems: Updated" << items.size() << "items";
}

// Task 85: Tile locking visual indication implementation
void MapView::setShowLockedTiles(bool show) {
    if (showLockedTiles_ != show) {
        showLockedTiles_ = show;
        updateLockedTileVisuals();
        viewport()->update();
        qDebug() << "MapView::setShowLockedTiles:" << show;
    }
}

bool MapView::isShowLockedTiles() const {
    return showLockedTiles_;
}

void MapView::setLockedTileOverlayColor(const QColor& color) {
    if (lockedTileOverlayColor_ != color) {
        lockedTileOverlayColor_ = color;
        if (showLockedTiles_) {
            updateLockedTileVisuals();
            viewport()->update();
        }
        qDebug() << "MapView::setLockedTileOverlayColor:" << color.name();
    }
}

QColor MapView::getLockedTileOverlayColor() const {
    return lockedTileOverlayColor_;
}

void MapView::updateLockedTileVisuals() {
    if (!map_ || !scene()) {
        return;
    }

    // Update all tile items to show/hide locked tile overlays
    QList<QGraphicsItem*> items = scene()->items();
    for (QGraphicsItem* item : items) {
        // Check if item is a tile item and update its locked visual state
        // This will be implemented when MapTileItem is enhanced to show lock overlays
        item->update();
    }

    qDebug() << "MapView::updateLockedTileVisuals: Updated locked tile visuals";
}
