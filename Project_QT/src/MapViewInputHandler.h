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
class Map; // Replace with actual map data class if different
class QUndoStack;

class MapViewInputHandler : public QObject {
    Q_OBJECT

public:
    enum class InteractionMode {
        Idle,
        Drawing,
        SelectingBox,
        DraggingSelection, // Placeholder, full implementation later
        PanningView,
        Pasting           // Placeholder, full implementation later
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
    void handleFocusOutEvent(QFocusEvent* event);


private:
    void updateModifierKeys(QInputEvent* event); // Helper to update modifier states

    // State-specific handlers
    void startDrawing(const QPointF& mapPos, QMouseEvent* event);
    void continueDrawing(const QPointF& mapPos, QMouseEvent* event);
    void finishDrawing(const QPointF& mapPos, QMouseEvent* event);

    void startPanning(QMouseEvent* event);
    void continuePanning(QMouseEvent* event);
    void finishPanning(QMouseEvent* event);

    void startSelectionBox(const QPointF& mapPos, QMouseEvent* event);
    void updateSelectionBox(const QPointF& mapPos, QMouseEvent* event);
    void finishSelectionBox(const QPointF& mapPos, QMouseEvent* event);

    // Member variables
    MapView* mapView_;                 // Non-owning pointer to the MapView
    BrushManager* brushManager_;     // Non-owning pointer
    Map* map_;                       // Non-owning pointer to map data
    QUndoStack* undoStack_;          // Non-owning pointer

    InteractionMode currentMode_ = InteractionMode::Idle;
    Qt::MouseButton pressedButton_ = Qt::NoButton;
    QPointF dragStartMapPos_;        // For drawing or selection box start in map coordinates
    QPoint lastMouseScreenPos_;      // For calculating panning delta in screen coordinates

    bool shiftModifierActive_ = false;
    bool ctrlModifierActive_ = false;
    bool altModifierActive_ = false;
};

#endif // MAPVIEWINPUTHANDLER_H
