#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent> // Added for keyboard events
#include <QPointF>
#include <QEnterEvent>
#include <QDebug>

// Forward declaration
class MapViewInputHandler;

// Constants
const int GROUND_LAYER = 7;
const int TILE_SIZE = 32;
const int MAP_MAX_LAYERS = 16;
const double MIN_ZOOM = 0.125;
const double MAX_ZOOM = 25.0;

class MapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit MapView(QWidget *parent = nullptr);
    ~MapView() override;

    enum class EditorMode {
        Selection,
        Drawing
    };
    EditorMode currentEditorMode_ = EditorMode::Selection;

    // Configuration
    void setSwitchMouseButtons(bool switched);
    bool getSwitchMouseButtons() const { return switchMouseButtons_; }

    // Public getters
    double getZoomLevel() const { return zoomLevel_; }
    int getCurrentFloor() const { return currentFloor_; }
    EditorMode getCurrentEditorMode() const { return currentEditorMode_; }


    QPointF screenToMap(const QPoint& screenPos) const;
    QPoint mapToScreen(const QPointF& mapTilePos) const;

    // Placeholder methods
    void pasteSelection(const QPointF& mapPos);
    void clearSelection();
    void toggleSelectionAt(const QPointF& mapPos);
    bool isOnSelection(const QPointF& mapPos) const;
    void selectObjectAt(const QPointF& mapPos);
    void updateMoveSelectionFeedback(const QPointF& delta);
    void finalizeMoveSelection(const QPointF& delta);
    void updateSelectionRectFeedback(const QPointF& startMapPos, const QPointF& currentMapPos);
    void finalizeSelectionRect(const QPointF& startMapPos, const QPointF& endMapPos, Qt::KeyboardModifiers modifiers);

    void startDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers);
    void continueDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers);
    void finalizeDrawing(const QPointF& mapPos, Qt::KeyboardModifiers modifiers);
    void updateDragDrawFeedback(const QPointF& startMapPos, const QPointF& currentMapPos);
    void finalizeDragDraw(const QPointF& startMapPos, const QPointF& endMapPos, Qt::KeyboardModifiers modifiers);

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
    void keyPressEvent(QKeyEvent *event) override;   // Added
    void keyReleaseEvent(QKeyEvent *event) override; // Added

private:
    void changeFloor(int newFloor);
    void updateAndRefreshMapCoordinates(const QPoint& screenPos);

    double zoomLevel_;
    int currentFloor_;
    bool isPanning_;
    QPoint lastMousePos_;
    QPointF lastMapPos_;
    QPoint dragStartScreenPos_;

    bool switchMouseButtons_ = false;
    bool doubleClickProperties_ = true;

    MapViewInputHandler* inputHandler_;
};

#endif // MAPVIEW_H
