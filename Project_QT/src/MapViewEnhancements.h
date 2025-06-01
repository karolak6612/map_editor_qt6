#ifndef MAPVIEWENHANCEMENTS_H
#define MAPVIEWENHANCEMENTS_H

#include <QObject>
#include <QTimer>
#include <QPointF>
#include <QRectF>
#include <QColor>
#include <QFont>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QEasingCurve>

// Forward declarations
class MapView;
class QPainter;
class QMouseEvent;
class QWheelEvent;

/**
 * @brief Enhanced zoom system for MapView with smooth animations and advanced features
 * 
 * Task 80: Complete zoom functionality including:
 * - Smooth zoom animations with easing curves
 * - Zoom to cursor position with pixel-perfect accuracy
 * - Zoom level constraints and validation
 * - Zoom status display and feedback
 * - Keyboard zoom shortcuts and mouse wheel support
 */
class MapViewZoomSystem : public QObject
{
    Q_OBJECT

public:
    explicit MapViewZoomSystem(MapView* mapView, QObject* parent = nullptr);
    ~MapViewZoomSystem() override;

    // Zoom operations
    void zoomIn(const QPointF& centerPoint = QPointF());
    void zoomOut(const QPointF& centerPoint = QPointF());
    void zoomToLevel(double level, const QPointF& centerPoint = QPointF());
    void zoomToFit(const QRectF& rect);
    void resetZoom();

    // Zoom properties
    double getCurrentZoom() const { return currentZoom_; }
    double getMinZoom() const { return minZoom_; }
    double getMaxZoom() const { return maxZoom_; }
    void setZoomRange(double minZoom, double maxZoom);

    // Zoom animation
    void setAnimationEnabled(bool enabled) { animationEnabled_ = enabled; }
    bool isAnimationEnabled() const { return animationEnabled_; }
    void setAnimationDuration(int duration) { animationDuration_ = duration; }
    int getAnimationDuration() const { return animationDuration_; }

    // Mouse wheel handling
    void handleWheelEvent(QWheelEvent* event);

    // Zoom factors
    void setZoomFactor(double factor) { zoomFactor_ = factor; }
    double getZoomFactor() const { return zoomFactor_; }

signals:
    void zoomChanged(double newZoom, double oldZoom);
    void zoomAnimationStarted();
    void zoomAnimationFinished();

private slots:
    void onZoomAnimationValueChanged(const QVariant& value);
    void onZoomAnimationFinished();

private:
    void performZoom(double newZoom, const QPointF& centerPoint, bool animated = true);
    void updateZoomConstraints();
    QPointF getViewCenter() const;

    MapView* mapView_;
    double currentZoom_;
    double minZoom_;
    double maxZoom_;
    double zoomFactor_;
    bool animationEnabled_;
    int animationDuration_;
    QPropertyAnimation* zoomAnimation_;
    QPointF zoomCenter_;
};

/**
 * @brief Enhanced grid display system with customizable appearance and performance
 * 
 * Task 80: Complete grid functionality including:
 * - Multiple grid types (tile grid, coordinate grid, custom grid)
 * - Customizable grid appearance (color, line width, style)
 * - Performance-optimized rendering for large maps
 * - Grid visibility based on zoom level
 * - Grid snapping for precise positioning
 */
class MapViewGridSystem : public QObject
{
    Q_OBJECT

public:
    enum GridType {
        TILE_GRID,          // Standard tile grid
        COORDINATE_GRID,    // Coordinate-based grid
        CUSTOM_GRID         // Custom spacing grid
    };

    enum GridStyle {
        SOLID_LINES,        // Solid grid lines
        DASHED_LINES,       // Dashed grid lines
        DOTTED_LINES        // Dotted grid lines
    };

    explicit MapViewGridSystem(MapView* mapView, QObject* parent = nullptr);
    ~MapViewGridSystem() override;

    // Grid visibility
    void setGridVisible(bool visible);
    bool isGridVisible() const { return gridVisible_; }

    // Grid type and style
    void setGridType(GridType type);
    GridType getGridType() const { return gridType_; }
    void setGridStyle(GridStyle style);
    GridStyle getGridStyle() const { return gridStyle_; }

    // Grid appearance
    void setGridColor(const QColor& color);
    QColor getGridColor() const { return gridColor_; }
    void setGridLineWidth(int width);
    int getGridLineWidth() const { return gridLineWidth_; }
    void setGridSpacing(int spacing);
    int getGridSpacing() const { return gridSpacing_; }

    // Grid visibility thresholds
    void setMinZoomForGrid(double minZoom);
    double getMinZoomForGrid() const { return minZoomForGrid_; }
    void setMaxZoomForGrid(double maxZoom);
    double getMaxZoomForGrid() const { return maxZoomForGrid_; }

    // Grid snapping
    void setSnapToGrid(bool snap);
    bool isSnapToGrid() const { return snapToGrid_; }
    QPointF snapToGrid(const QPointF& point) const;

    // Rendering
    void renderGrid(QPainter* painter, const QRectF& viewRect, double zoom);

signals:
    void gridVisibilityChanged(bool visible);
    void gridAppearanceChanged();

private:
    void updateGridVisibility(double zoom);
    void renderTileGrid(QPainter* painter, const QRectF& viewRect, double zoom);
    void renderCoordinateGrid(QPainter* painter, const QRectF& viewRect, double zoom);
    void renderCustomGrid(QPainter* painter, const QRectF& viewRect, double zoom);

    MapView* mapView_;
    bool gridVisible_;
    GridType gridType_;
    GridStyle gridStyle_;
    QColor gridColor_;
    int gridLineWidth_;
    int gridSpacing_;
    double minZoomForGrid_;
    double maxZoomForGrid_;
    bool snapToGrid_;
};

/**
 * @brief Enhanced mouse tracking system with hover effects and cursor feedback
 * 
 * Task 80: Complete mouse tracking including:
 * - Precise mouse position tracking with sub-pixel accuracy
 * - Hover effects for tiles and items
 * - Cursor feedback for different tools and modes
 * - Mouse capture and release handling
 * - Coordinate display and status updates
 */
class MapViewMouseTracker : public QObject
{
    Q_OBJECT

public:
    explicit MapViewMouseTracker(MapView* mapView, QObject* parent = nullptr);
    ~MapViewMouseTracker() override;

    // Mouse tracking
    void setMouseTrackingEnabled(bool enabled);
    bool isMouseTrackingEnabled() const { return mouseTrackingEnabled_; }

    // Current mouse state
    QPointF getCurrentMousePosition() const { return currentMousePos_; }
    QPointF getCurrentMapPosition() const { return currentMapPos_; }
    bool isMouseOverMap() const { return mouseOverMap_; }

    // Hover effects
    void setHoverEffectsEnabled(bool enabled);
    bool isHoverEffectsEnabled() const { return hoverEffectsEnabled_; }
    void setHoverHighlightColor(const QColor& color);
    QColor getHoverHighlightColor() const { return hoverHighlightColor_; }

    // Mouse event handling
    void handleMouseMoveEvent(QMouseEvent* event);
    void handleMouseEnterEvent(QMouseEvent* event);
    void handleMouseLeaveEvent(QMouseEvent* event);

    // Coordinate conversion
    QPointF screenToMap(const QPointF& screenPos) const;
    QPointF mapToScreen(const QPointF& mapPos) const;

signals:
    void mousePositionChanged(const QPointF& screenPos, const QPointF& mapPos);
    void mouseEnteredMap();
    void mouseLeftMap();
    void tileHovered(const QPointF& tilePos);
    void tileUnhovered(const QPointF& tilePos);

private slots:
    void onHoverTimer();

private:
    void updateMousePosition(const QPointF& screenPos);
    void updateHoverEffects();
    void startHoverTimer();
    void stopHoverTimer();

    MapView* mapView_;
    bool mouseTrackingEnabled_;
    bool hoverEffectsEnabled_;
    QPointF currentMousePos_;
    QPointF currentMapPos_;
    QPointF lastHoveredTile_;
    bool mouseOverMap_;
    QColor hoverHighlightColor_;
    QTimer* hoverTimer_;
    int hoverDelay_;
};

/**
 * @brief Enhanced drawing feedback system with visual indicators and previews
 * 
 * Task 80: Complete drawing feedback including:
 * - Brush preview with accurate size and shape
 * - Drawing indicators for active operations
 * - Visual feedback for tool states
 * - Performance-optimized overlay rendering
 * - Customizable feedback appearance
 */
class MapViewDrawingFeedback : public QObject
{
    Q_OBJECT

public:
    enum FeedbackType {
        BRUSH_PREVIEW,      // Brush preview indicator
        SELECTION_BOX,      // Selection rectangle
        DRAG_INDICATOR,     // Drag operation indicator
        TOOL_CURSOR,        // Tool-specific cursor
        STATUS_OVERLAY      // Status information overlay
    };

    explicit MapViewDrawingFeedback(MapView* mapView, QObject* parent = nullptr);
    ~MapViewDrawingFeedback() override;

    // Feedback visibility
    void setFeedbackEnabled(bool enabled);
    bool isFeedbackEnabled() const { return feedbackEnabled_; }

    // Brush preview
    void setBrushPreview(const QPointF& position, int size, const QColor& color);
    void clearBrushPreview();
    bool hasBrushPreview() const { return hasBrushPreview_; }

    // Selection feedback
    void setSelectionBox(const QRectF& rect);
    void clearSelectionBox();
    bool hasSelectionBox() const { return hasSelectionBox_; }

    // Drawing indicators
    void setDrawingIndicator(const QPointF& position, const QString& text);
    void clearDrawingIndicator();
    bool hasDrawingIndicator() const { return hasDrawingIndicator_; }

    // Status overlay
    void setStatusOverlay(const QString& text, const QPointF& position = QPointF());
    void clearStatusOverlay();
    bool hasStatusOverlay() const { return hasStatusOverlay_; }

    // Rendering
    void renderFeedback(QPainter* painter, const QRectF& viewRect);

signals:
    void feedbackChanged();

private:
    void renderBrushPreview(QPainter* painter);
    void renderSelectionBox(QPainter* painter);
    void renderDrawingIndicator(QPainter* painter);
    void renderStatusOverlay(QPainter* painter);

    MapView* mapView_;
    bool feedbackEnabled_;

    // Brush preview
    bool hasBrushPreview_;
    QPointF brushPreviewPos_;
    int brushPreviewSize_;
    QColor brushPreviewColor_;

    // Selection box
    bool hasSelectionBox_;
    QRectF selectionBoxRect_;

    // Drawing indicator
    bool hasDrawingIndicator_;
    QPointF drawingIndicatorPos_;
    QString drawingIndicatorText_;

    // Status overlay
    bool hasStatusOverlay_;
    QString statusOverlayText_;
    QPointF statusOverlayPos_;
    QFont statusOverlayFont_;
};

#endif // MAPVIEWENHANCEMENTS_H
