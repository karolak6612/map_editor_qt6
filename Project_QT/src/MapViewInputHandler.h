#ifndef MAPVIEWINPUTHANDLER_H
#define MAPVIEWINPUTHANDLER_H

#include <QPointF>
#include <QMouseEvent>

// Forward declaration (MapView.h includes this already, but good for clarity)
class MapView; 

// Include MapView.h for MapView::EditorMode and other necessary declarations
#include "MapView.h" 

class MapViewInputHandler {
public:
    explicit MapViewInputHandler(MapView* mapView);

    // Left button actions
    void handleMousePress(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos);
    void handleMouseMove(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos);
    void handleMouseRelease(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos);

    // Properties click/release (typically right mouse, or middle if switched)
    void handlePropertiesClick(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos);
    void handlePropertiesRelease(QMouseEvent *event, MapView::EditorMode mode, const QPointF& mapPos, const QPoint& screenPos);
    
    // Note: Camera click/release (panning) is currently handled directly in MapView for simplicity,
    // as it directly manipulates scrollbars and cursor. If it becomes more complex, it could move here.

    void resetInputStates();

private:
    MapView* mapView_; // Pointer to the MapView instance

    // State variables for input handling
    bool isSelecting_;          // True if currently drag-selecting a rectangle
    bool isDraggingSelection_;  // True if dragging an existing selection
    bool isDrawing_;            // True if actively drawing (e.g. brush stroke)
    bool isDraggingDraw_;       // True if drag-drawing (e.g. line, rectangle brush)

    QPointF dragStartMapPos_;      // Logical map coordinates for the start of a drag operation
    QPoint  clickOriginScreenPos_; // Screen position of a properties click, for context menu
};

#endif // MAPVIEWINPUTHANDLER_H
