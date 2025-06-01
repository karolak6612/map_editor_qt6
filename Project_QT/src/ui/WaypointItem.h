#ifndef WAYPOINTITEM_H
#define WAYPOINTITEM_H

#include <QGraphicsObject>
#include <QRectF>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QTimer>
#include <QColor>
#include "../Map.h"

// Forward declarations
class Waypoint;
class Map;

/**
 * @brief WaypointItem - QGraphicsItem for visualizing waypoints on the map
 * 
 * This class provides visual representation of waypoints including markers,
 * radius visualization, labels, and interactive features for waypoint editing.
 */
class WaypointItem : public QGraphicsObject {
    Q_OBJECT

public:
    enum WaypointStyle {
        Marker,          // Simple marker icon
        Circle,          // Colored circle
        Icon,            // Custom icon based on waypoint type
        RadiusArea,      // Show waypoint radius as circle
        LabeledMarker    // Marker with text label
    };

    enum InteractionMode {
        ReadOnly,        // Display only
        Selectable,      // Can be selected
        Editable,        // Can be moved and edited
        Draggable        // Can be dragged to new position
    };

    explicit WaypointItem(Waypoint* waypoint, Map* mapContext, QGraphicsItem* parent = nullptr);
    ~WaypointItem() override;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    // Waypoint access
    Waypoint* getWaypoint() const;
    void setWaypoint(Waypoint* waypoint);

    // Visual properties
    void setWaypointStyle(WaypointStyle style);
    WaypointStyle getWaypointStyle() const;

    void setInteractionMode(InteractionMode mode);
    InteractionMode getInteractionMode() const;

    void setMarkerSize(qreal size);
    qreal getMarkerSize() const;

    void setShowRadius(bool show);
    bool getShowRadius() const;

    void setShowLabel(bool show);
    bool getShowLabel() const;

    void setLabelFont(const QFont& font);
    QFont getLabelFont() const;

    // Visual appearance customization
    void setMarkerPen(const QPen& pen);
    QPen getMarkerPen() const;

    void setMarkerBrush(const QBrush& brush);
    QBrush getMarkerBrush() const;

    void setRadiusPen(const QPen& pen);
    QPen getRadiusPen() const;

    void setRadiusBrush(const QBrush& brush);
    QBrush getRadiusBrush() const;

    void setLabelColor(const QColor& color);
    QColor getLabelColor() const;

    // Animation and highlighting
    void setAnimationEnabled(bool enabled);
    bool isAnimationEnabled() const;

    void setHighlighted(bool highlighted);
    bool isHighlighted() const;

    // Position and geometry
    void updatePosition();
    void updateFromWaypoint();

    // Mouse interaction
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

    // Hover effects
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    // Context menu
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

public slots:
    void onWaypointChanged();
    void onAnimationTimer();

signals:
    void waypointClicked(Waypoint* waypoint);
    void waypointDoubleClicked(Waypoint* waypoint);
    void waypointRightClicked(Waypoint* waypoint, const QPoint& screenPos); // Task 74: Right-click signal
    void waypointMoved(Waypoint* waypoint, const MapPos& newPosition);
    void waypointContextMenu(Waypoint* waypoint, const QPoint& globalPos);
    void waypointSelectionChanged(Waypoint* waypoint, bool selected);

private:
    Waypoint* waypoint_;
    Map* mapContext_;
    
    // Visual properties
    WaypointStyle style_;
    InteractionMode interactionMode_;
    qreal markerSize_;
    bool showRadius_;
    bool showLabel_;
    QFont labelFont_;
    
    // Appearance
    QPen markerPen_;
    QBrush markerBrush_;
    QPen radiusPen_;
    QBrush radiusBrush_;
    QColor labelColor_;
    
    // State
    bool highlighted_;
    bool animationEnabled_;
    bool hovering_;
    bool dragging_;
    QPointF dragStartPos_;
    MapPos originalPosition_;
    
    // Animation
    QTimer* animationTimer_;
    qreal animationPhase_;
    
    // Cached geometry
    mutable QRectF cachedBoundingRect_;
    mutable bool boundingRectValid_;
    
    // Helper methods
    void updateBoundingRect() const;
    void invalidateBoundingRect();
    QRectF calculateBoundingRect() const;
    QPointF waypointToScenePos() const;
    MapPos scenePosToWaypoint(const QPointF& scenePos) const;
    
    // Drawing methods
    void drawMarker(QPainter* painter) const;
    void drawCircle(QPainter* painter) const;
    void drawIcon(QPainter* painter) const;
    void drawRadius(QPainter* painter) const;
    void drawLabel(QPainter* painter) const;
    void drawHighlight(QPainter* painter) const;
    
    // Style helpers
    QPen getEffectivePen(const QPen& basePen) const;
    QBrush getEffectiveBrush(const QBrush& baseBrush) const;
    QColor getEffectiveColor(const QColor& baseColor) const;
    
    // Icon helpers
    QPixmap getWaypointIcon() const;
    QPixmap createDefaultIcon() const;
    
    // Interaction helpers
    bool isPositionValid(const MapPos& position) const;
    void startDrag(const QPointF& startPos);
    void updateDrag(const QPointF& currentPos);
    void finishDrag(const QPointF& endPos);
    void cancelDrag();
};

#endif // WAYPOINTITEM_H
