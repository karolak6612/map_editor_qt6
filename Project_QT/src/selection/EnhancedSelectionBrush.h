#ifndef ENHANCEDSELECTIONBRUSH_H
#define ENHANCEDSELECTIONBRUSH_H

#include "SelectionBrush.h"
#include "MapPos.h"
#include <QObject>
#include <QSet>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QUndoStack>
#include <QTimer>
#include <QElapsedTimer>
#include <QVariantMap>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QGraphicsItem>

// Forward declarations
class Map;
class Selection;
class MapView;
class MapScene;
class Tile;
class Item;
class SelectionTransformationEngine;
class SelectionVisualFeedback;

/**
 * @brief Task 95: Enhanced SelectionBrush with complete transformation logic and modifiers
 * 
 * Complete implementation of selection tool with full wxwidgets compatibility:
 * - Complete transformation logic (move, rotate, flip) with proper item handling
 * - All modifier key interactions for complex selections
 * - Visual feedback during selection and transformation
 * - Robust map data changes with undo/redo support
 * - Item flags and properties handling during transformations
 * - Perfect replication of wxwidgets map_display behavior
 * - Advanced selection modes and interaction states
 */

/**
 * @brief Selection interaction state for complex mouse handling
 */
enum class SelectionInteractionState {
    IDLE,                    // No active interaction
    SELECTING_NEW,           // Creating new selection
    SELECTING_RECTANGLE,     // Rectangle selection in progress
    SELECTING_POLYGON,       // Polygon selection in progress
    SELECTING_LASSO,         // Lasso selection in progress
    MOVING_SELECTION,        // Moving existing selection
    ROTATING_SELECTION,      // Rotating selection around pivot
    SCALING_SELECTION,       // Scaling selection
    TRANSFORMING_SELECTION,  // General transformation in progress
    DRAGGING_HANDLE,         // Dragging transformation handle
    CONTEXT_MENU_PENDING     // Right-click context menu pending
};

/**
 * @brief Selection modifier behavior (wxwidgets compatibility)
 */
enum class SelectionModifierBehavior {
    REPLACE,        // Replace current selection (no modifiers)
    ADD,           // Add to selection (Ctrl)
    SUBTRACT,      // Subtract from selection (Ctrl+Shift)
    INTERSECT,     // Intersect with selection (Alt)
    TOGGLE         // Toggle selection state (Shift)
};

/**
 * @brief Transformation pivot point for rotations and scaling
 */
struct TransformationPivot {
    MapPos position;
    QPointF screenPosition;
    bool isCustom;
    bool isVisible;
    
    TransformationPivot() : isCustom(false), isVisible(false) {}
    TransformationPivot(const MapPos& pos) : position(pos), isCustom(true), isVisible(true) {}
};

/**
 * @brief Enhanced SelectionBrush with complete transformation capabilities
 */
class EnhancedSelectionBrush : public SelectionBrush
{
    Q_OBJECT

public:
    explicit EnhancedSelectionBrush(QObject* parent = nullptr);
    ~EnhancedSelectionBrush() override;

    // Component setup
    void setMapView(MapView* mapView);
    void setMapScene(MapScene* scene);
    MapView* getMapView() const { return mapView_; }
    MapScene* getMapScene() const { return scene_; }

    // Enhanced brush interface
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) override;
    QString getName() const override { return "Enhanced Selection"; }
    QString getDescription() const override { return "Advanced selection tool with full transformation support"; }

    // Interaction state management
    SelectionInteractionState getInteractionState() const { return interactionState_; }
    void setInteractionState(SelectionInteractionState state);
    bool isInteractionActive() const;
    void cancelCurrentInteraction();

    // Enhanced mouse event handling (wxwidgets compatibility)
    void handleMousePress(Map* map, const QPointF& position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers) override;
    void handleMouseMove(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers) override;
    void handleMouseRelease(Map* map, const QPointF& position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers) override;
    void handleMouseDoubleClick(Map* map, const QPointF& position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

    // Enhanced keyboard event handling
    void handleKeyPress(QKeyEvent* event);
    void handleKeyRelease(QKeyEvent* event);
    bool handleShortcut(QKeySequence sequence);

    // Selection operations with modifier support
    void selectWithModifiers(Map* map, const MapPos& position, Qt::KeyboardModifiers modifiers);
    void selectRectangleWithModifiers(Map* map, const MapPos& startPos, const MapPos& endPos, Qt::KeyboardModifiers modifiers);
    void selectPolygonWithModifiers(Map* map, const QList<MapPos>& points, Qt::KeyboardModifiers modifiers);
    void selectLassoWithModifiers(Map* map, const QList<QPointF>& path, Qt::KeyboardModifiers modifiers);

    // Advanced transformation operations
    void moveSelectionToPosition(Map* map, const MapPos& targetPosition);
    void moveSelectionByOffset(Map* map, const QPointF& offset);
    void rotateSelectionAroundPivot(Map* map, double degrees, const MapPos& pivot);
    void rotateSelectionAroundCenter(Map* map, double degrees);
    void flipSelectionHorizontalAroundPivot(Map* map, const MapPos& pivot);
    void flipSelectionVerticalAroundPivot(Map* map, const MapPos& pivot);
    void scaleSelection(Map* map, double scaleX, double scaleY, const MapPos& pivot);

    // Transformation pivot management
    void setTransformationPivot(const MapPos& pivot);
    void setTransformationPivotToCenter();
    void setTransformationPivotToSelection();
    TransformationPivot getTransformationPivot() const { return transformationPivot_; }
    void clearTransformationPivot();

    // Visual feedback control
    void enableVisualFeedback(bool enabled);
    bool isVisualFeedbackEnabled() const { return visualFeedbackEnabled_; }
    void setGhostingEnabled(bool enabled);
    bool isGhostingEnabled() const { return ghostingEnabled_; }
    void setSelectionOutlineEnabled(bool enabled);
    bool isSelectionOutlineEnabled() const { return selectionOutlineEnabled_; }

    // Advanced selection modes
    void setSelectionMode(SelectionMode mode);
    SelectionMode getSelectionMode() const;
    void setSelectionType(SelectionType type);
    SelectionType getSelectionType() const;

    // Item and tile handling during transformations
    void setPreserveItemProperties(bool preserve);
    bool isPreserveItemProperties() const { return preserveItemProperties_; }
    void setAutoRotateItems(bool autoRotate);
    bool isAutoRotateItems() const { return autoRotateItems_; }
    void setMergeOnPaste(bool merge);
    bool isMergeOnPaste() const { return mergeOnPaste_; }

    // Performance and optimization
    void setUpdateThrottling(bool enabled);
    bool isUpdateThrottling() const { return updateThrottling_; }
    void setMaxSelectionSize(int maxTiles);
    int getMaxSelectionSize() const { return maxSelectionSize_; }

    // Statistics and monitoring
    QVariantMap getStatistics() const;
    void resetStatistics();
    int getSelectedTileCount() const;
    int getSelectedItemCount() const;
    QRect getSelectionBounds() const;

public slots:
    // Transformation slots
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void rotateClockwise();
    void rotateCounterClockwise();
    void flipHorizontal();
    void flipVertical();
    void resetTransformation();

    // Selection management slots
    void selectAll();
    void selectNone();
    void invertSelection();
    void selectSimilar();
    void expandSelection();
    void contractSelection();

    // Visual update slots
    void updateVisualFeedback();
    void refreshSelectionDisplay();
    void updateTransformationHandles();

signals:
    void interactionStateChanged(SelectionInteractionState newState, SelectionInteractionState oldState);
    void transformationStarted();
    void transformationProgress(double progress);
    void transformationCompleted();
    void transformationCancelled();
    void selectionBoundsChanged(const QRect& bounds);
    void pivotChanged(const TransformationPivot& pivot);

private slots:
    void onUpdateTimer();
    void onThrottleTimer();

private:
    // Modifier behavior analysis
    SelectionModifierBehavior analyzeModifierBehavior(Qt::KeyboardModifiers modifiers) const;
    bool isAddToSelectionModifier(Qt::KeyboardModifiers modifiers) const;
    bool isSubtractFromSelectionModifier(Qt::KeyboardModifiers modifiers) const;
    bool isToggleSelectionModifier(Qt::KeyboardModifiers modifiers) const;
    bool isIntersectSelectionModifier(Qt::KeyboardModifiers modifiers) const;

    // Interaction state management
    void enterInteractionState(SelectionInteractionState newState);
    void exitInteractionState(SelectionInteractionState oldState);
    void updateInteractionCursor();
    void updateInteractionFeedback();

    // Selection operations implementation
    void performSelection(Map* map, const QSet<MapPos>& positions, SelectionModifierBehavior behavior);
    void performRectangleSelection(Map* map, const MapPos& startPos, const MapPos& endPos, SelectionModifierBehavior behavior);
    void performPolygonSelection(Map* map, const QList<MapPos>& points, SelectionModifierBehavior behavior);
    void performLassoSelection(Map* map, const QList<QPointF>& path, SelectionModifierBehavior behavior);

    // Transformation implementation
    void executeTransformation(Map* map, const QString& transformationType, const QVariantMap& parameters);
    void prepareTransformation(Map* map);
    void applyTransformation(Map* map, const QVariantMap& parameters);
    void finalizeTransformation(Map* map);
    void cancelTransformation(Map* map);

    // Item transformation logic
    void transformItem(Item* item, const QVariantMap& transformation);
    void rotateItem(Item* item, double degrees);
    void flipItemHorizontal(Item* item);
    void flipItemVertical(Item* item);
    void updateItemOrientation(Item* item, int newOrientation);
    bool shouldRotateItemType(quint16 itemType) const;

    // Visual feedback implementation
    void drawSelectionOutline(QPainter* painter, const QRect& bounds);
    void drawTransformationHandles(QPainter* painter, const QRect& bounds);
    void drawTransformationPivot(QPainter* painter, const TransformationPivot& pivot);
    void drawGhostSelection(QPainter* painter, const QSet<MapPos>& positions);
    void drawRubberBand(QPainter* painter, const QRectF& rect);

    // Utility methods
    QSet<MapPos> getRectanglePositions(const MapPos& startPos, const MapPos& endPos) const;
    QSet<MapPos> getPolygonPositions(const QList<MapPos>& points) const;
    QSet<MapPos> getLassoPositions(const QList<QPointF>& path) const;
    QRect calculateSelectionBounds(const QSet<MapPos>& positions) const;
    MapPos calculateSelectionCenter(const QSet<MapPos>& positions) const;
    bool isPositionInLasso(const MapPos& position, const QList<QPointF>& path) const;

    // Performance optimization
    void throttleUpdates();
    void optimizeSelection();
    bool shouldThrottleUpdate() const;
    void updateStatistics();

private:
    // Core components
    MapView* mapView_;
    MapScene* scene_;
    SelectionTransformationEngine* transformationEngine_;
    SelectionVisualFeedback* visualFeedback_;

    // Interaction state
    SelectionInteractionState interactionState_;
    Qt::KeyboardModifiers currentModifiers_;
    QPointF lastMousePosition_;
    QPointF dragStartPosition_;
    QPointF dragCurrentPosition_;
    bool isDragging_;
    bool isTransforming_;

    // Transformation state
    TransformationPivot transformationPivot_;
    QVariantMap currentTransformation_;
    QSet<MapPos> transformationSelection_;
    QMap<MapPos, QVariantMap> originalTileStates_;

    // Visual feedback settings
    bool visualFeedbackEnabled_;
    bool ghostingEnabled_;
    bool selectionOutlineEnabled_;
    bool showTransformationHandles_;
    bool showTransformationPivot_;

    // Behavior settings
    bool preserveItemProperties_;
    bool autoRotateItems_;
    bool mergeOnPaste_;
    bool updateThrottling_;
    int maxSelectionSize_;

    // Performance tracking
    QTimer* updateTimer_;
    QTimer* throttleTimer_;
    QElapsedTimer lastUpdateTime_;
    mutable QVariantMap statistics_;
    int operationCount_;
    qint64 totalUpdateTime_;

    // Constants
    static const int DEFAULT_UPDATE_INTERVAL = 16; // ~60 FPS
    static const int DEFAULT_THROTTLE_DELAY = 100; // milliseconds
    static const int DEFAULT_MAX_SELECTION_SIZE = 10000; // tiles
    static const double DEFAULT_ROTATION_STEP = 90.0; // degrees
    static const double DEFAULT_MOVE_STEP = 1.0; // tiles
};

#endif // ENHANCEDSELECTIONBRUSH_H
