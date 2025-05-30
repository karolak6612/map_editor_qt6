#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPointF>
#include <QRectF>   // Added for drawBackground
#include <QPainter> // Added for drawBackground
#include <QEnterEvent>
#include <QFocusEvent> // Added for focusOutEvent
#include <QDebug>

// Forward declarations
class MapViewInputHandler;
class Brush; // Added forward declaration
class BrushManager;
class Map;
class QUndoStack;

// Constants
const int GROUND_LAYER = 7;
const int TILE_SIZE = 32;
const int MAP_MAX_LAYERS = 16;
const double MIN_ZOOM = 0.125;
const double MAX_ZOOM = 25.0;

class MapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit MapView(BrushManager* brushManager, Map* map, QUndoStack* undoStack, QWidget *parent = nullptr);
    ~MapView() override;

    enum class EditorMode {
        Selection,
        Drawing // Renamed from DrawingMode for consistency if it was different
    };
    
    // Editor Mode and Active Brush
    EditorMode getCurrentEditorMode() const; // Getter for currentEditorMode_
    void setCurrentEditorMode(EditorMode mode); // Setter if needed, or manage internally
    
    void setActiveBrush(Brush* brush);
    Brush* getActiveBrush() const;

    // Configuration
    void setSwitchMouseButtons(bool switched);
    bool getSwitchMouseButtons() const { return switchMouseButtons_; }

    // Public getters
    double getZoomLevel() const { return zoomLevel_; }
    int getCurrentFloor() const { return currentFloor_; }

    QPointF screenToMap(const QPoint& screenPos) const;
    QPoint mapToScreen(const QPointF& mapTilePos) const;

    // Public interface for MapViewInputHandler
    void pan(int dx, int dy);
    void zoom(qreal factor, const QPointF& centerScreenPos); // Changed center to screen pos for consistency with wheelEvent
    void setSelectionArea(const QRectF& rect);

    // Placeholder methods (many are called by MapViewInputHandler via mapView_ pointer)
    void pasteSelection(const QPointF& mapPos);
    void clearSelection();
    void toggleSelectionAt(const QPointF& mapPos);
    bool isOnSelection(const QPointF& mapPos) const;
    void selectObjectAt(const QPointF& mapPos);
    void updateMoveSelectionFeedback(const QPointF& delta);
    void finalizeMoveSelection(const QPointF& delta);
    void updateSelectionRectFeedback(const QPointF& startMapPos, const QPointF& currentMapPos);
    void finalizeSelectionRect(const QPointF& startMapPos, const QPointF& endMapPos, Qt::KeyboardModifiers modifiers);

    // These drawing related placeholders will be removed from MapView.h/cpp
    // as brushes will handle their own drawing logic via mouse events.
    // void startDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers); // To be removed
    // void continueDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers); // To be removed
    // void finalizeDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers); // To be removed
    // void updateDragDrawFeedback(const QPointF& startMapPos, const QPointF& currentMapPos); // To be removed
    // void finalizeDragDraw(const QPointF& startMapPos, const QPointF& endMapPos, Qt::KeyboardModifiers modifiers); // To be removed

    void updateStatusBarWithMapPos(const QPointF& mapPos);
    void updateZoomStatus();
    void updateFloorStatus();

    void increaseBrushSize_placeholder();
    void decreaseBrushSize_placeholder();
    void updateFloorMenu_placeholder();

    void showPropertiesDialogFor(const QPointF& mapPos);
    void switchToSelectionMode();
    void endPasting();
    void showContextMenuAt(const QPoint& screenPos);
    void resetActionQueueTimer_placeholder();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override; // Added

    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override; // Added
private:
    void changeFloor(int newFloor);
    void updateAndRefreshMapCoordinates(const QPoint& screenPos);

    EditorMode currentEditorMode_ = EditorMode::Selection; // Keep private, use getter/setter
    Brush* currentBrush_ = nullptr;       // Active brush

    double zoomLevel_;
    int currentFloor_;
    bool isPanning_;
    QPoint lastMousePos_;
    QPointF lastMapPos_;
    QPoint dragStartScreenPos_;

    bool switchMouseButtons_ = false;
    bool doubleClickProperties_ = true;

    MapViewInputHandler* inputHandler_;
    QRectF currentSelectionArea_; // Added for drawing selection
};

#endif // MAPVIEW_H
