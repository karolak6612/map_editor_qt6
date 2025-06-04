#ifndef SELECTIONVISUALFEEDBACK_H
#define SELECTIONVISUALFEEDBACK_H

#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsEffect>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QRect>
#include <QRectF>
#include <QPointF>
#include <QPolygonF>
#include <QTimer>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QVariantAnimation>

#include "MapPos.h"

// Forward declarations
class MapView;
class MapScene;
class Selection;

/**
 * @brief Task 95: Selection Visual Feedback System for advanced selection visualization
 * 
 * Complete visual feedback system with wxwidgets compatibility:
 * - Selection outlines with animated borders
 * - Transformation handles and pivot points
 * - Ghost/preview rendering during transformations
 * - Rubber band selection with proper styling
 * - Real-time visual updates during interactions
 * - Performance-optimized rendering for large selections
 * - Customizable visual styles and themes
 */

/**
 * @brief Visual feedback rendering mode
 */
enum class FeedbackRenderingMode {
    IMMEDIATE,      // Immediate rendering without optimization
    OPTIMIZED,      // Optimized rendering with LOD and culling
    CACHED,         // Cached rendering with dirty region tracking
    ANIMATED        // Animated rendering with smooth transitions
};

/**
 * @brief Selection outline style
 */
struct SelectionOutlineStyle {
    QPen outlinePen;
    QBrush fillBrush;
    double opacity;
    bool animated;
    int animationDuration;
    bool dashed;
    double dashOffset;
    
    SelectionOutlineStyle() : opacity(0.7), animated(true), animationDuration(1000), dashed(true), dashOffset(0.0) {
        outlinePen = QPen(QColor(255, 255, 0), 2, Qt::DashLine);
        fillBrush = QBrush(QColor(255, 255, 0, 50));
    }
};

/**
 * @brief Transformation handle style
 */
struct TransformationHandleStyle {
    QBrush handleBrush;
    QPen handlePen;
    QBrush hoverBrush;
    QPen hoverPen;
    double handleSize;
    bool showRotationHandles;
    bool showScaleHandles;
    bool showMoveHandle;
    
    TransformationHandleStyle() : handleSize(8.0), showRotationHandles(true), showScaleHandles(true), showMoveHandle(true) {
        handleBrush = QBrush(QColor(255, 255, 255));
        handlePen = QPen(QColor(0, 0, 0), 1);
        hoverBrush = QBrush(QColor(255, 255, 0));
        hoverPen = QPen(QColor(0, 0, 0), 2);
    }
};

/**
 * @brief Ghost rendering style for transformation preview
 */
struct GhostRenderingStyle {
    double opacity;
    QColor tintColor;
    bool showItems;
    bool showTiles;
    bool showGrid;
    bool animated;
    
    GhostRenderingStyle() : opacity(0.5), tintColor(QColor(255, 255, 255, 128)),
                           showItems(true), showTiles(true), showGrid(false), animated(true) {}
};

/**
 * @brief Rubber band selection style
 */
struct RubberBandStyle {
    QPen borderPen;
    QBrush fillBrush;
    double opacity;
    bool animated;
    
    RubberBandStyle() : opacity(0.3), animated(false) {
        borderPen = QPen(QColor(0, 120, 215), 1, Qt::SolidLine);
        fillBrush = QBrush(QColor(0, 120, 215, 50));
    }
};

/**
 * @brief Selection visual feedback graphics item
 */
class SelectionFeedbackItem : public QGraphicsItem
{
public:
    explicit SelectionFeedbackItem(QGraphicsItem* parent = nullptr);
    ~SelectionFeedbackItem() override;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;

    // Selection data
    void setSelectedPositions(const QSet<MapPos>& positions);
    QSet<MapPos> getSelectedPositions() const { return selectedPositions_; }
    void clearSelection();

    // Visual styles
    void setOutlineStyle(const SelectionOutlineStyle& style);
    void setHandleStyle(const TransformationHandleStyle& style);
    void setGhostStyle(const GhostRenderingStyle& style);
    void setRubberBandStyle(const RubberBandStyle& style);

    // Rendering control
    void setRenderingMode(FeedbackRenderingMode mode);
    FeedbackRenderingMode getRenderingMode() const { return renderingMode_; }
    void setVisible(bool visible);
    void setOpacity(qreal opacity);

    // Animation control
    void startAnimation();
    void stopAnimation();
    void pauseAnimation();
    void resumeAnimation();
    bool isAnimated() const { return animated_; }

    // Transformation feedback
    void showTransformationPreview(const QSet<MapPos>& originalPositions, const QSet<MapPos>& newPositions);
    void hideTransformationPreview();
    void updateTransformationProgress(double progress);

    // Handle management
    void showTransformationHandles(const QRect& bounds);
    void hideTransformationHandles();
    void updateHandlePositions(const QRect& bounds);
    int getHoveredHandle(const QPointF& position) const;

private:
    void updateBoundingRect();
    void updateShape();
    void drawSelectionOutline(QPainter* painter);
    void drawTransformationHandles(QPainter* painter);
    void drawGhostPreview(QPainter* painter);
    void drawRubberBand(QPainter* painter);

private:
    QSet<MapPos> selectedPositions_;
    QSet<MapPos> ghostPositions_;
    QRectF boundingRect_;
    QPainterPath shape_;
    
    SelectionOutlineStyle outlineStyle_;
    TransformationHandleStyle handleStyle_;
    GhostRenderingStyle ghostStyle_;
    RubberBandStyle rubberBandStyle_;
    
    FeedbackRenderingMode renderingMode_;
    bool animated_;
    bool showHandles_;
    bool showGhost_;
    bool showRubberBand_;
    
    QRect handleBounds_;
    QList<QRectF> handleRects_;
    int hoveredHandle_;
    
    QPropertyAnimation* dashAnimation_;
    QPropertyAnimation* opacityAnimation_;
    double animationProgress_;
};

/**
 * @brief Main Selection Visual Feedback System
 */
class SelectionVisualFeedback : public QObject
{
    Q_OBJECT

public:
    explicit SelectionVisualFeedback(QObject* parent = nullptr);
    ~SelectionVisualFeedback() override;

    // Component setup
    void setMapView(MapView* mapView);
    void setMapScene(MapScene* scene);
    void setSelection(Selection* selection);
    MapView* getMapView() const { return mapView_; }
    MapScene* getMapScene() const { return scene_; }
    Selection* getSelection() const { return selection_; }

    // Visual feedback control
    void enableFeedback(bool enabled);
    bool isFeedbackEnabled() const { return feedbackEnabled_; }
    void setRenderingMode(FeedbackRenderingMode mode);
    FeedbackRenderingMode getRenderingMode() const { return renderingMode_; }

    // Selection visualization
    void showSelectionOutline(bool show);
    bool isSelectionOutlineVisible() const { return showOutline_; }
    void updateSelectionOutline();
    void animateSelectionOutline(bool animate);

    // Transformation visualization
    void showTransformationHandles(bool show);
    bool areTransformationHandlesVisible() const { return showHandles_; }
    void updateTransformationHandles();
    void showTransformationPivot(bool show, const MapPos& pivot = MapPos());
    void showTransformationPreview(const QSet<MapPos>& newPositions);
    void hideTransformationPreview();

    // Rubber band selection
    void showRubberBand(const QRectF& rect);
    void updateRubberBand(const QRectF& rect);
    void hideRubberBand();
    bool isRubberBandVisible() const { return showRubberBand_; }

    // Ghost rendering for transformations
    void enableGhostRendering(bool enabled);
    bool isGhostRenderingEnabled() const { return ghostRenderingEnabled_; }
    void showGhostSelection(const QSet<MapPos>& positions);
    void updateGhostSelection(const QSet<MapPos>& positions);
    void hideGhostSelection();

    // Visual styles
    void setSelectionOutlineStyle(const SelectionOutlineStyle& style);
    SelectionOutlineStyle getSelectionOutlineStyle() const { return outlineStyle_; }
    void setTransformationHandleStyle(const TransformationHandleStyle& style);
    TransformationHandleStyle getTransformationHandleStyle() const { return handleStyle_; }
    void setGhostRenderingStyle(const GhostRenderingStyle& style);
    GhostRenderingStyle getGhostRenderingStyle() const { return ghostStyle_; }
    void setRubberBandStyle(const RubberBandStyle& style);
    RubberBandStyle getRubberBandStyle() const { return rubberBandStyle_; }

    // Performance and optimization
    void setOptimizationEnabled(bool enabled);
    bool isOptimizationEnabled() const { return optimizationEnabled_; }
    void setMaxRenderingDistance(double distance);
    double getMaxRenderingDistance() const { return maxRenderingDistance_; }
    void setCacheEnabled(bool enabled);
    bool isCacheEnabled() const { return cacheEnabled_; }

    // Animation control
    void setAnimationEnabled(bool enabled);
    bool isAnimationEnabled() const { return animationEnabled_; }
    void setAnimationDuration(int duration);
    int getAnimationDuration() const { return animationDuration_; }
    void setAnimationEasing(QEasingCurve::Type easing);
    QEasingCurve::Type getAnimationEasing() const { return animationEasing_; }

    // Handle interaction
    int getHandleAtPosition(const QPointF& position) const;
    bool isPositionOnHandle(const QPointF& position) const;
    void setHandleHover(int handleIndex, bool hover);
    void clearHandleHover();

public slots:
    void onSelectionChanged();
    void onMapViewChanged();
    void onZoomChanged();
    void updateVisualFeedback();
    void refreshDisplay();

signals:
    void handleHovered(int handleIndex);
    void handleClicked(int handleIndex);
    void visualFeedbackUpdated();

private slots:
    void onAnimationFinished();
    void onAnimationValueChanged(const QVariant& value);

private:
    // Graphics item management
    void createFeedbackItems();
    void destroyFeedbackItems();
    void updateFeedbackItems();
    void addFeedbackItemToScene();
    void removeFeedbackItemFromScene();

    // Rendering optimization
    void optimizeRendering();
    void updateLevelOfDetail();
    void cullInvisibleElements();
    void updateDirtyRegions();
    void invalidateCache();

    // Animation management
    void setupAnimations();
    void startSelectionAnimation();
    void stopSelectionAnimation();
    void updateAnimationProgress();

    // Coordinate conversion
    QPointF mapToScene(const MapPos& mapPos) const;
    QRectF mapToScene(const QRect& mapRect) const;
    MapPos sceneToMap(const QPointF& scenePos) const;
    QRect sceneToMap(const QRectF& sceneRect) const;

    // Utility methods
    QRect calculateSelectionBounds() const;
    QRect calculateHandleBounds() const;
    bool isPositionVisible(const MapPos& position) const;
    double calculateDistanceToView(const MapPos& position) const;

private:
    // Core components
    MapView* mapView_;
    MapScene* scene_;
    Selection* selection_;

    // Graphics items
    SelectionFeedbackItem* feedbackItem_;
    QGraphicsItem* rubberBandItem_;
    QGraphicsItem* ghostItem_;
    QList<QGraphicsItem*> handleItems_;

    // Visual settings
    SelectionOutlineStyle outlineStyle_;
    TransformationHandleStyle handleStyle_;
    GhostRenderingStyle ghostStyle_;
    RubberBandStyle rubberBandStyle_;

    // Rendering settings
    FeedbackRenderingMode renderingMode_;
    bool feedbackEnabled_;
    bool optimizationEnabled_;
    bool cacheEnabled_;
    double maxRenderingDistance_;

    // Display state
    bool showOutline_;
    bool showHandles_;
    bool showRubberBand_;
    bool ghostRenderingEnabled_;

    // Animation settings
    bool animationEnabled_;
    int animationDuration_;
    QEasingCurve::Type animationEasing_;

    // Animation objects
    QPropertyAnimation* outlineAnimation_;
    QPropertyAnimation* handleAnimation_;
    QSequentialAnimationGroup* animationGroup_;

    // State tracking
    QSet<MapPos> lastSelectedPositions_;
    QRect lastSelectionBounds_;
    QRectF lastRubberBandRect_;
    int hoveredHandle_;
    bool needsUpdate_;

    // Performance tracking
    QTimer* updateTimer_;
    qint64 lastUpdateTime_;
    int frameCount_;
    double averageFrameTime_;
};

#endif // SELECTIONVISUALFEEDBACK_H
