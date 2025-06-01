#ifndef MAPVIEWINPUTHANDLER_H
#define MAPVIEWINPUTHANDLER_H

#include <QObject>
#include <QPointF>
#include <Qt> // For Qt::MouseButton

// Forward declarations
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class QFocusEvent;
class QInputEvent; // For updateModifierKeys

class MapView;
class Brush;
class BrushManager;
class Map;
class QUndoStack;
class QUndoCommand; // Added for currentDrawingCommand_

class MapViewInputHandler : public QObject {
    Q_OBJECT

public:
    // Enhanced interaction modes (state machine from wxwidgets canvas.cpp)
    enum class InteractionMode {
        Idle,                    // No active interaction
        Drawing,                 // Single-click drawing
        DraggingDraw,           // Shift+drag drawing (brush->canDrag())
        ReplaceDragging,        // Alt+drag for ground replacement
        SelectingBox,           // Rectangle selection
        DraggingSelection,      // Dragging selected area
        PanningView,            // Camera panning
        Pasting,                // Pasting clipboard content
        DrawingRect,            // Drawing rectangular areas
        DrawingLine,            // Drawing line areas
        BoundBoxSelection,      // Bounding box selection mode
        ScreenDragging,         // Screen/view dragging
        ContextMenu,            // Right-click context menu active
        BrushPreview           // Showing brush preview/hover
    };

    // Brush interaction states (from wxwidgets)
    enum class BrushState {
        Idle,
        StartDraw,
        ContinueDraw,
        EndDraw,
        StartDrag,
        ContinueDrag,
        EndDrag
    };
    Q_ENUM(InteractionMode) // Make enum usable in Qt's meta-object system

    explicit MapViewInputHandler(MapView* mapView,
                                 BrushManager* brushManager,
                                 Map* map, // Or your map data class
                                 QUndoStack* undoStack,
                                 QObject *parent = nullptr);
    ~MapViewInputHandler();

    void handleMousePressEvent(QMouseEvent* event, const QPointF& mapPosition);
    void handleMouseMoveEvent(QMouseEvent* event, const QPointF& mapPosition);
    void handleMouseReleaseEvent(QMouseEvent* event, const QPointF& mapPosition);
    void handleKeyPressEvent(QKeyEvent* event);
    void handleKeyReleaseEvent(QKeyEvent* event);
    void handleWheelEvent(QWheelEvent* event, const QPointF& mapPosition);

    // Enhanced key handling methods
    void handleModeSpecificKeys(QKeyEvent* event);
    void handleToolShortcuts(QKeyEvent* event);
    void handleViewShortcuts(QKeyEvent* event);
    void handleFloorNavigationKeys(QKeyEvent* event);
    void handleHotkeyKeys(QKeyEvent* event);
    void handleDrawingModeKeys(QKeyEvent* event);
    void handleSelectionModeKeys(QKeyEvent* event);
    void handlePanningModeKeys(QKeyEvent* event);
    void handlePastingModeKeys(QKeyEvent* event);
    void handleFocusOutEvent(QFocusEvent* event);

    // Enhanced mouse event handlers for wxWidgets compatibility
    void handleMouseActionClick(QMouseEvent* event, const QPointF& mapPosition);
    void handleMouseActionRelease(QMouseEvent* event, const QPointF& mapPosition);
    void handleMouseCameraClick(QMouseEvent* event, const QPointF& mapPosition);
    void handleMouseCameraRelease(QMouseEvent* event, const QPointF& mapPosition);
    void handleMousePropertiesClick(QMouseEvent* event, const QPointF& mapPosition);
    void handleMousePropertiesRelease(QMouseEvent* event, const QPointF& mapPosition);

    // Task 74: Waypoint interaction handling
    bool handleWaypointInteraction(QMouseEvent* event, const QPointF& mapPosition);

private:
    void updateModifierKeys(QInputEvent* event); // Helper to update modifier states

    // Enhanced state-specific handlers (Task 34 requirement)
    void startDrawing(const QPointF& mapPos, QMouseEvent* event);
    void continueDrawing(const QPointF& mapPos, QMouseEvent* event);
    void finishDrawing(const QPointF& mapPos, QMouseEvent* event);

    void startPanning(QMouseEvent* event);
    void continuePanning(QMouseEvent* event);
    void finishPanning(QMouseEvent* event);

    void startSelectionBox(const QPointF& mapPos, QMouseEvent* event);
    void updateSelectionBox(const QPointF& mapPos, QMouseEvent* event);
    void finishSelectionBox(const QPointF& mapPos, QMouseEvent* event);

    // Single-click selection handling (Task 61)
    void handleSingleClickSelection(const QPointF& mapPos, QMouseEvent* event, bool toggleMode);

    // Enhanced brush interaction handlers
    void startDraggingDraw(const QPointF& mapPos, QMouseEvent* event);
    void continueDraggingDraw(const QPointF& mapPos, QMouseEvent* event);
    void finishDraggingDraw(const QPointF& mapPos, QMouseEvent* event);

    void startReplaceDragging(const QPointF& mapPos, QMouseEvent* event);
    void continueReplaceDragging(const QPointF& mapPos, QMouseEvent* event);
    void finishReplaceDragging(const QPointF& mapPos, QMouseEvent* event);

    void startDrawingRect(const QPointF& mapPos, QMouseEvent* event);
    void updateDrawingRect(const QPointF& mapPos, QMouseEvent* event);
    void finishDrawingRect(const QPointF& mapPos, QMouseEvent* event);

    void startDrawingLine(const QPointF& mapPos, QMouseEvent* event);
    void updateDrawingLine(const QPointF& mapPos, QMouseEvent* event);
    void finishDrawingLine(const QPointF& mapPos, QMouseEvent* event);

    // Tile/Area identification methods (Task 34 requirement)
    QList<QPointF> getAffectedTiles(const QPointF& primaryMapPos, Brush* currentBrush) const;
    QList<QPointF> getTilesInArea(const QPointF& startPos, const QPointF& endPos, Brush::BrushShape shape) const;
    QList<QPointF> getTilesInRectangle(const QPointF& startPos, const QPointF& endPos) const;
    QList<QPointF> getTilesInCircle(const QPointF& centerPos, int radius) const;
    QList<QPointF> getTilesInLine(const QPointF& startPos, const QPointF& endPos) const;

    // Coordinate conversion and validation
    QPointF screenToMapTile(const QPoint& screenPos) const;
    QPoint mapTileToScreen(const QPointF& mapTilePos) const;
    bool isValidTilePosition(const QPointF& mapPos) const;
    QPointF snapToTileGrid(const QPointF& mapPos) const;

    // State machine management
    void transitionToMode(InteractionMode newMode);
    void updateBrushState(BrushState newState);
    bool canTransitionTo(InteractionMode newMode) const;
    void resetInteractionState();

    // Visual feedback and preview
    void updateBrushPreview(const QPointF& mapPos);
    void clearBrushPreview();
    void updateSelectionFeedback(const QRectF& area);
    void clearSelectionFeedback();

    // Member variables
    MapView* mapView_;                 // Non-owning pointer to the MapView
    BrushManager* brushManager_;     // Non-owning pointer
    Map* map_;                       // Non-owning pointer to map data
    QUndoStack* undoStack_;          // Non-owning pointer

    // Enhanced state machine variables (from wxwidgets canvas.cpp)
    InteractionMode currentMode_ = InteractionMode::Idle;
    InteractionMode previousMode_ = InteractionMode::Idle;
    BrushState currentBrushState_ = BrushState::Idle;

    Qt::MouseButton pressedButton_ = Qt::NoButton;
    QPointF dragStartMapPos_;        // For drawing or selection box start in map coordinates
    QPointF dragCurrentMapPos_;      // Current drag position
    QPointF dragEndMapPos_;          // End drag position
    QPoint lastMouseScreenPos_;      // For calculating panning delta in screen coordinates

    // Enhanced modifier key tracking
    bool shiftModifierActive_ = false;
    bool ctrlModifierActive_ = false;
    bool altModifierActive_ = false;
    bool metaModifierActive_ = false;

    // Enhanced state flags (from wxwidgets)
    bool isDragging_ = false;           // General dragging state
    bool isDrawing_ = false;            // Drawing state
    bool isDraggingDraw_ = false;       // Shift+drag drawing
    bool isReplaceDragging_ = false;    // Alt+drag ground replacement
    bool isScreenDragging_ = false;     // Screen/view dragging
    bool isBoundBoxSelection_ = false;  // Bounding box selection
    bool isContextMenuActive_ = false;  // Right-click context menu
    bool isBrushPreviewActive_ = false; // Brush preview/hover

    // Drawing area tracking
    QRectF currentDrawingArea_;         // Current drawing area bounds
    QList<QPointF> currentDrawingTiles_; // Tiles affected in current operation
    QList<QPointF> previewTiles_;       // Tiles for brush preview

    // Command batching and undo integration
    QUndoCommand* currentDrawingCommand_ = nullptr;  // Main command for current operation
    QUndoCommand* currentBatchCommand_ = nullptr;    // Batch command for multi-tile operations
    QList<QUndoCommand*> pendingCommands_;           // Commands waiting to be executed

    // Visual feedback state
    QRectF selectionFeedbackRect_;      // Selection rectangle for visual feedback
    QPointF brushPreviewPos_;           // Current brush preview position
    int brushPreviewSize_ = 1;          // Current brush preview size
};

#endif // MAPVIEWINPUTHANDLER_H
