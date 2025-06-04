#ifndef SPAWNITEM_H
#define SPAWNITEM_H

#include <QGraphicsObject>
#include <QRectF>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QTimer>
#include <QColor>
#include "../Map.h"

// Forward declarations
class Spawn;
class Map;

/**
 * @brief Task 92: Enhanced SpawnItem - Finalizing Spawn Visualization & Attribute Reflection
 *
 * Complete spawn visualization implementation with full wxwidgets compatibility:
 * - Visual representation of spawn areas and radius (purple flame marker)
 * - Interactive spawn editing and selection with proper drawing flags
 * - Spawn markers and creature position indicators
 * - Context menu support for spawn operations
 * - Real-time updates when spawn attributes change
 * - Integration with SpawnBrush application and map updates
 * - Proper TileItem flag handling and borderization logic
 * - Dynamic visual updates reflecting all spawn attributes
 * - 1:1 compatibility with wxwidgets spawn visualization (SPRITE_SPAWN)
 */
class SpawnItem : public QGraphicsObject {
    Q_OBJECT

public:
    enum SpawnStyle {
        CreatureIcon,    // Show creature icon/sprite
        SpawnArea,       // Show spawn area as circle/rectangle
        SpawnMarker,     // Simple spawn marker
        DetailedView     // Show creature icon + area + details
    };

    enum InteractionMode {
        ReadOnly,        // Display only
        Selectable,      // Can be selected
        Editable,        // Can be moved and edited
        Draggable        // Can be dragged to new position
    };

    explicit SpawnItem(Spawn* spawn, Map* mapContext, QGraphicsItem* parent = nullptr);
    ~SpawnItem() override;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    // Spawn access
    Spawn* getSpawn() const;
    void setSpawn(Spawn* spawn);

    // Visual properties
    void setSpawnStyle(SpawnStyle style);
    SpawnStyle getSpawnStyle() const;

    void setInteractionMode(InteractionMode mode);
    InteractionMode getInteractionMode() const;

    void setIconSize(qreal size);
    qreal getIconSize() const;

    void setShowSpawnArea(bool show);
    bool getShowSpawnArea() const;

    void setShowCreatureCount(bool show);
    bool getShowCreatureCount() const;

    void setShowSpawnTime(bool show);
    bool getShowSpawnTime() const;

    // Visual appearance customization
    void setSpawnPen(const QPen& pen);
    QPen getSpawnPen() const;

    void setSpawnBrush(const QBrush& brush);
    QBrush getSpawnBrush() const;

    void setAreaPen(const QPen& pen);
    QPen getAreaPen() const;

    void setAreaBrush(const QBrush& brush);
    QBrush getAreaBrush() const;

    void setTextColor(const QColor& color);
    QColor getTextColor() const;

    void setTextFont(const QFont& font);
    QFont getTextFont() const;

    // Animation and highlighting
    void setAnimationEnabled(bool enabled);
    bool isAnimationEnabled() const;

    void setHighlighted(bool highlighted);
    bool isHighlighted() const;

    // Task 87: Enhanced visual features
    void setShowCreaturePositions(bool show);
    bool getShowCreaturePositions() const;

    void setShowSpawnRadius(bool show);
    bool getShowSpawnRadius() const;

    void setShowSpawnInfo(bool show);
    bool getShowSpawnInfo() const;

    void setRadiusColor(const QColor& color);
    QColor getRadiusColor() const;

    void setCreaturePositionColor(const QColor& color);
    QColor getCreaturePositionColor() const;

    void setSelectedColor(const QColor& color);
    QColor getSelectedColor() const;

    // Task 87: Spawn area management
    QList<MapPos> getSpawnAreaPositions() const;
    void updateSpawnAreaVisualization();
    void updateCreaturePositionMarkers();

    // Task 87: Scale and zoom support
    void setTileSize(int tileSize);
    int getTileSize() const;

    void setZoomFactor(double zoomFactor);
    double getZoomFactor() const;

    // Position and geometry
    void updatePosition();
    void updateFromSpawn();
    void updateVisualProperties();

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
    void onSpawnChanged();
    void onAnimationTimer();

signals:
    void spawnClicked(Spawn* spawn);
    void spawnDoubleClicked(Spawn* spawn);
    void spawnMoved(Spawn* spawn, const MapPos& newPosition);
    void spawnContextMenu(Spawn* spawn, const QPoint& globalPos);
    void spawnSelectionChanged(Spawn* spawn, bool selected);

private:
    Spawn* spawn_;
    Map* mapContext_;
    
    // Visual properties
    SpawnStyle style_;
    InteractionMode interactionMode_;
    qreal iconSize_;
    bool showSpawnArea_;
    bool showCreatureCount_;
    bool showSpawnTime_;

    // Task 87: Enhanced visual properties
    bool showCreaturePositions_;
    bool showSpawnRadius_;
    bool showSpawnInfo_;
    QColor radiusColor_;
    QColor creaturePositionColor_;
    QColor selectedColor_;
    int tileSize_;
    double zoomFactor_;
    
    // Appearance
    QPen spawnPen_;
    QBrush spawnBrush_;
    QPen areaPen_;
    QBrush areaBrush_;
    QColor textColor_;
    QFont textFont_;
    
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
    QPointF spawnToScenePos() const;
    MapPos scenePosToSpawn(const QPointF& scenePos) const;
    
    // Drawing methods
    void drawCreatureIcon(QPainter* painter) const;
    void drawSpawnArea(QPainter* painter) const;
    void drawSpawnMarker(QPainter* painter) const;
    void drawDetailedView(QPainter* painter) const;
    void drawCreatureCount(QPainter* painter) const;
    void drawSpawnTime(QPainter* painter) const;
    void drawHighlight(QPainter* painter) const;

    // Task 87: Enhanced drawing methods
    void drawSpawnRadius(QPainter* painter) const;
    void drawCreaturePositions(QPainter* painter) const;
    void drawSpawnInfo(QPainter* painter) const;
    void drawSelectionIndicator(QPainter* painter) const;
    void drawSpawnCenter(QPainter* painter) const;
    
    // Style helpers
    QPen getEffectivePen(const QPen& basePen) const;
    QBrush getEffectiveBrush(const QBrush& baseBrush) const;
    QColor getEffectiveColor(const QColor& baseColor) const;
    
    // Icon helpers
    QPixmap getCreatureIcon() const;
    QPixmap createDefaultSpawnIcon() const;
    
    // Spawn data helpers
    QString getCreatureName() const;
    int getCreatureCount() const;
    int getSpawnTime() const;
    QRectF getSpawnAreaRect() const;
    
    // Interaction helpers
    bool isPositionValid(const MapPos& position) const;
    void startDrag(const QPointF& startPos);
    void updateDrag(const QPointF& currentPos);
    void finishDrag(const QPointF& endPos);
    void cancelDrag();
};

#endif // SPAWNITEM_H
