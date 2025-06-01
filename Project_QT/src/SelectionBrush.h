#ifndef SELECTIONBRUSH_H
#define SELECTIONBRUSH_H

#include "Brush.h"
#include "Position.h"
#include <QObject>
#include <QSet>
#include <QPointF>
#include <QUndoStack>

// Forward declarations
class Map;
class Selection;
class SelectionCommands;
class ClipboardManager;
class QKeyEvent;
class QMouseEvent;

/**
 * @brief Enhanced selection brush for Task 79
 * 
 * Complete implementation of selection tool with:
 * - Rectangle selection with rubber band
 * - Single tile/item selection
 * - Copy/Cut/Paste operations
 * - Transformation commands
 * - Delete operations
 * - Mouse interaction state machine
 */
class SelectionBrush : public Brush
{
    Q_OBJECT

public:
    enum SelectionMode {
        SINGLE_SELECT,      // Select single tiles/items
        RECTANGLE_SELECT,   // Rectangle selection
        POLYGON_SELECT,     // Polygon selection (future)
        LASSO_SELECT       // Lasso selection (future)
    };

    enum MouseState {
        IDLE,               // No mouse interaction
        DRAGGING,           // Dragging to create selection
        MOVING_SELECTION,   // Moving selected items
        RESIZING_SELECTION  // Resizing selection (future)
    };

    explicit SelectionBrush(QObject* parent = nullptr);
    ~SelectionBrush() override;

    // Brush interface implementation
    QString getName() const override;
    QString getDescription() const override;
    QIcon getIcon() const override;
    BrushType getType() const override;
    
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override;

    // Selection mode management
    void setSelectionMode(SelectionMode mode);
    SelectionMode getSelectionMode() const;

    // Selection operations
    void selectSingle(Map* map, const MapPos& position, bool addToSelection = false);
    void selectRectangle(Map* map, const MapPos& startPos, const MapPos& endPos, bool addToSelection = false);
    void clearSelection();

    // Clipboard operations
    void copySelection();
    void cutSelection();
    void pasteSelection(Map* map, const MapPos& targetPosition);
    void deleteSelection();

    // Transformation operations
    void moveSelection(const QPointF& offset);
    void rotateSelection(int degrees);
    void flipSelectionHorizontal();
    void flipSelectionVertical();

    // Mouse interaction
    void handleMousePress(Map* map, const QPointF& position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void handleMouseMove(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers);
    void handleMouseRelease(Map* map, const QPointF& position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    void handleMouseDoubleClick(Map* map, const QPointF& position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

    // Keyboard interaction
    void handleKeyPress(QKeyEvent* event);
    void handleKeyRelease(QKeyEvent* event);

    // State queries
    bool hasSelection() const;
    bool isSelecting() const;
    bool isMoving() const;
    int getSelectedTileCount() const;
    int getSelectedItemCount() const;

    // Integration points
    void setSelection(Selection* selection);
    Selection* getSelection() const;
    void setClipboardManager(ClipboardManager* clipboardManager);
    ClipboardManager* getClipboardManager() const;
    void setUndoStack(QUndoStack* undoStack);
    QUndoStack* getUndoStack() const;

signals:
    void selectionChanged();
    void selectionStarted();
    void selectionFinished();
    void selectionModeChanged(SelectionMode mode);
    void mouseStateChanged(MouseState state);
    void operationCompleted(const QString& operation, int tileCount, int itemCount);
    void operationFailed(const QString& operation, const QString& error);

private slots:
    void onSelectionChanged();

private:
    // Mouse interaction helpers
    void startDragging(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers);
    void updateDragging(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers);
    void finishDragging(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers);
    
    void startMoving(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers);
    void updateMoving(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers);
    void finishMoving(Map* map, const QPointF& position, Qt::KeyboardModifiers modifiers);

    // Selection helpers
    bool isPositionInSelection(const MapPos& position) const;
    MapPos screenToMapPosition(const QPointF& screenPos) const;
    QPointF mapToScreenPosition(const MapPos& mapPos) const;

    // Command execution helpers
    void executeCommand(QUndoCommand* command);
    void executeSelectionCommand(QUndoCommand* command, const QString& operationName);

    // State management
    void setState(MouseState newState);
    void updateCursor();
    void updateVisualFeedback();

    // Data members
    SelectionMode selectionMode_;
    MouseState mouseState_;
    
    // Mouse interaction state
    QPointF dragStartPosition_;
    QPointF dragCurrentPosition_;
    QPointF lastMousePosition_;
    bool isDragging_;
    bool isMoving_;
    Qt::KeyboardModifiers currentModifiers_;
    
    // Integration objects
    Selection* selection_;
    ClipboardManager* clipboardManager_;
    QUndoStack* undoStack_;
    Map* currentMap_;
    
    // Selection state
    QSet<MapPos> currentSelection_;
    MapPos selectionStartPos_;
    MapPos selectionEndPos_;
    
    // Visual feedback
    bool showRubberBand_;
    bool showMovePreview_;
    QPointF moveOffset_;
    
    // Settings
    bool autoSelectAfterPaste_;
    bool confirmDestructiveOperations_;
    bool enableAutomagicBordering_;
};

#endif // SELECTIONBRUSH_H
