#ifndef ENHANCEDMAPVIEW_H
#define ENHANCEDMAPVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTimer>
#include <QElapsedTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "MapDrawingPrimitives.h"

// Forward declarations
class Map;
class Brush;
class Waypoint;
class House;
class Spawn;
class MapScene;
class MapOverlayRenderer;

// Task 72: Enhanced MapView with complete OpenGL replacement and advanced rendering
class EnhancedMapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit EnhancedMapView(QWidget* parent = nullptr);
    ~EnhancedMapView() override;

    // Map integration
    void setMap(Map* map);
    Map* getMap() const { return map_; }
    
    // Scene management
    void setMapScene(MapScene* scene);
    MapScene* getMapScene() const { return mapScene_; }
    
    // Drawing primitives
    void setDrawingPrimitives(MapDrawingPrimitives* primitives);
    MapDrawingPrimitives* getDrawingPrimitives() const { return drawingPrimitives_; }
    
    // Overlay renderer
    void setOverlayRenderer(MapOverlayRenderer* renderer);
    MapOverlayRenderer* getOverlayRenderer() const { return overlayRenderer_; }
    
    // Task 72: Enhanced rendering features
    void setRenderingMode(RenderingMode mode);
    RenderingMode getRenderingMode() const { return renderingMode_; }
    
    void setAntialiasing(bool enabled);
    bool isAntialiasingEnabled() const { return antialiasingEnabled_; }
    
    void setTextAntialiasing(bool enabled);
    bool isTextAntialiasingEnabled() const { return textAntialiasingEnabled_; }
    
    void setSmoothPixmapTransform(bool enabled);
    bool isSmoothPixmapTransformEnabled() const { return smoothPixmapTransformEnabled_; }
    
    // Task 72: Advanced view features
    void setAnimationsEnabled(bool enabled);
    bool areAnimationsEnabled() const { return animationsEnabled_; }
    
    void setTooltipsEnabled(bool enabled);
    bool areTooltipsEnabled() const { return tooltipsEnabled_; }
    
    void setHoverEffectsEnabled(bool enabled);
    bool areHoverEffectsEnabled() const { return hoverEffectsEnabled_; }
    
    // Task 72: Floor and view management
    void setCurrentFloor(int floor);
    int getCurrentFloor() const { return currentFloor_; }
    
    void setFloorFading(bool enabled);
    bool isFloorFadingEnabled() const { return floorFadingEnabled_; }
    
    void setFloorFadeAlpha(qreal alpha);
    qreal getFloorFadeAlpha() const { return floorFadeAlpha_; }
    
    // Task 72: Brush and tool integration
    void setBrush(Brush* brush);
    Brush* getBrush() const { return currentBrush_; }
    
    void setBrushSize(int size);
    int getBrushSize() const { return brushSize_; }
    
    void setBrushShape(BrushShape shape);
    BrushShape getBrushShape() const { return brushShape_; }
    
    void setBrushPreviewEnabled(bool enabled);
    bool isBrushPreviewEnabled() const { return brushPreviewEnabled_; }
    
    // Task 72: Selection and interaction
    void setSelectionMode(SelectionMode mode);
    SelectionMode getSelectionMode() const { return selectionMode_; }
    
    void setSelectionArea(const QRectF& area);
    QRectF getSelectionArea() const { return selectionArea_; }
    
    void clearSelection();
    bool hasSelection() const { return !selectionArea_.isEmpty(); }
    
    // Task 72: Navigation and centering
    void centerOnPosition(const QPointF& position, bool animated = true);
    void centerOnTile(int x, int y, int z = -1, bool animated = true);
    void centerOnWaypoint(Waypoint* waypoint, bool animated = true);
    void centerOnHouse(House* house, bool animated = true);
    void centerOnSpawn(Spawn* spawn, bool animated = true);
    
    // Task 72: Zoom and scale management
    void setZoomLevel(qreal zoom, bool animated = true);
    qreal getZoomLevel() const { return zoomLevel_; }
    
    void zoomIn(bool animated = true);
    void zoomOut(bool animated = true);
    void zoomToFit(bool animated = true);
    void zoomToSelection(bool animated = true);
    
    void setMinZoom(qreal minZoom);
    qreal getMinZoom() const { return minZoom_; }
    
    void setMaxZoom(qreal maxZoom);
    qreal getMaxZoom() const { return maxZoom_; }
    
    // Task 72: Tooltip and hover management
    void addTooltip(const QPointF& position, const QString& text, const QColor& color = QColor(255, 255, 255));
    void removeTooltip(const QPointF& position);
    void clearTooltips();
    
    void setHoverPosition(const QPointF& position);
    void clearHover();
    
    // Task 72: Performance and optimization
    void setUpdateMode(ViewportUpdateMode mode);
    ViewportUpdateMode getUpdateMode() const { return updateMode_; }
    
    void setOptimizationFlags(OptimizationFlags flags);
    OptimizationFlags getOptimizationFlags() const { return optimizationFlags_; }
    
    void invalidateCache();
    void updateViewport();

signals:
    // Task 72: Enhanced interaction signals
    void mapClicked(const QPointF& position, Qt::MouseButton button);
    void mapDoubleClicked(const QPointF& position, Qt::MouseButton button);
    void mapRightClicked(const QPointF& position);
    void mapDragged(const QPointF& from, const QPointF& to);
    
    void tileClicked(int x, int y, int z, Qt::MouseButton button);
    void tileDoubleClicked(int x, int y, int z, Qt::MouseButton button);
    void tileRightClicked(int x, int y, int z);
    void tileHovered(int x, int y, int z);
    void tileLeft();
    
    void brushApplied(const QPointF& position, Brush* brush);
    void brushDragged(const QRectF& area, Brush* brush);
    
    void selectionChanged(const QRectF& area);
    void selectionCleared();
    
    void floorChanged(int floor);
    void zoomChanged(qreal zoom);
    void viewportChanged(const QRectF& viewport);
    
    // Task 72: Animation and effect signals
    void animationStarted();
    void animationFinished();
    void tooltipRequested(const QPointF& position);
    void contextMenuRequested(const QPointF& position);

public slots:
    // Task 72: Map change handlers
    void onMapChanged();
    void onMapSizeChanged();
    void onMapDataChanged(const QRectF& area);
    
    // Task 72: Floor change handlers
    void onFloorUp();
    void onFloorDown();
    void onFloorChanged(int floor);
    
    // Task 72: Zoom handlers
    void onZoomIn();
    void onZoomOut();
    void onZoomToFit();
    void onZoomToSelection();
    void onZoomChanged(qreal zoom);
    
    // Task 72: Tool handlers
    void onBrushChanged(Brush* brush);
    void onBrushSizeChanged(int size);
    void onBrushShapeChanged(BrushShape shape);
    
    // Task 72: Selection handlers
    void onSelectionModeChanged(SelectionMode mode);
    void onSelectAll();
    void onClearSelection();
    
    // Task 72: Animation handlers
    void onAnimationFinished();
    void onCenteringFinished();
    void onZoomingFinished();

protected:
    // Task 72: Enhanced event handling
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void leaveEvent(QEvent* event) override;
    
    // Task 72: Custom drawing
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

private slots:
    // Task 72: Internal update handlers
    void onUpdateTimer();
    void onAnimationTimer();
    void onTooltipTimer();
    void onHoverTimer();

private:
    // Task 72: Initialization and setup
    void setupView();
    void setupAnimations();
    void setupTimers();
    void connectSignals();
    
    // Task 72: Coordinate conversion
    QPointF viewToMap(const QPointF& viewPos) const;
    QPointF mapToView(const QPointF& mapPos) const;
    QRectF viewToMap(const QRectF& viewRect) const;
    QRectF mapToView(const QRectF& mapRect) const;
    
    // Task 72: Drawing helpers
    void drawEnhancedOverlays(QPainter* painter, const QRectF& rect);
    void drawTooltips(QPainter* painter, const QRectF& rect);
    void drawHoverEffects(QPainter* painter, const QRectF& rect);
    void drawBrushPreview(QPainter* painter, const QRectF& rect);
    void drawSelectionOverlay(QPainter* painter, const QRectF& rect);
    void drawFloorIndicators(QPainter* painter, const QRectF& rect);
    
    // Task 72: Animation helpers
    void animateToPosition(const QPointF& position, int duration = 500);
    void animateToZoom(qreal zoom, int duration = 300);
    void animateFloorChange(int fromFloor, int toFloor, int duration = 200);
    
    // Task 72: Performance helpers
    void updateCachedData();
    void optimizeRendering();
    bool shouldSkipDrawing(const QRectF& itemRect, const QRectF& viewRect) const;
    
    // Core components
    Map* map_;
    MapScene* mapScene_;
    MapDrawingPrimitives* drawingPrimitives_;
    MapOverlayRenderer* overlayRenderer_;
    
    // Task 72: Rendering configuration
    RenderingMode renderingMode_;
    bool antialiasingEnabled_;
    bool textAntialiasingEnabled_;
    bool smoothPixmapTransformEnabled_;
    
    // Task 72: View state
    int currentFloor_;
    qreal zoomLevel_;
    qreal minZoom_;
    qreal maxZoom_;
    bool floorFadingEnabled_;
    qreal floorFadeAlpha_;
    
    // Task 72: Tool state
    Brush* currentBrush_;
    int brushSize_;
    BrushShape brushShape_;
    bool brushPreviewEnabled_;
    SelectionMode selectionMode_;
    QRectF selectionArea_;
    
    // Task 72: Interaction state
    bool animationsEnabled_;
    bool tooltipsEnabled_;
    bool hoverEffectsEnabled_;
    QPointF hoverPosition_;
    QList<MapTooltip> tooltips_;
    
    // Task 72: Performance settings
    ViewportUpdateMode updateMode_;
    OptimizationFlags optimizationFlags_;
    
    // Task 72: Animation system
    QPropertyAnimation* centerAnimation_;
    QPropertyAnimation* zoomAnimation_;
    QPropertyAnimation* floorAnimation_;
    QParallelAnimationGroup* animationGroup_;
    
    // Task 72: Timers
    QTimer* updateTimer_;
    QTimer* animationTimer_;
    QTimer* tooltipTimer_;
    QTimer* hoverTimer_;
    
    // Task 72: Mouse interaction
    bool isDragging_;
    bool isSelecting_;
    QPointF lastMousePos_;
    QPointF dragStartPos_;
    Qt::MouseButton dragButton_;
    
    // Task 72: Performance tracking
    QElapsedTimer frameTimer_;
    int frameCount_;
    qreal averageFPS_;
};

#endif // ENHANCEDMAPVIEW_H
