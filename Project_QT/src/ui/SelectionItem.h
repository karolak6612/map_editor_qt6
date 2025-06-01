#ifndef SELECTIONITEM_H
#define SELECTIONITEM_H

#include <QGraphicsObject>
#include <QRectF>
#include <QPen>
#include <QBrush>
#include <QTimer>
#include "../Map.h"

// Forward declarations
class Selection;
class Map;

/**
 * @brief SelectionItem - QGraphicsItem for visualizing map selection
 * 
 * This class provides visual representation of the current map selection,
 * including rubber-band selection rectangles and highlighting of selected tiles.
 * It supports both area selection and individual tile selection highlighting.
 */
class SelectionItem : public QGraphicsObject {
    Q_OBJECT

public:
    enum SelectionStyle {
        RubberBand,      // Rectangular selection area
        TileHighlight,   // Individual tile highlighting
        AreaOutline      // Outline around selected area
    };

    explicit SelectionItem(Selection* selection, Map* mapContext, QGraphicsItem* parent = nullptr);
    ~SelectionItem() override;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    // Selection visualization properties
    void setSelectionStyle(SelectionStyle style);
    SelectionStyle getSelectionStyle() const;

    void setRubberBandRect(const QRectF& rect);
    QRectF getRubberBandRect() const;

    void setVisible(bool visible);
    bool isVisible() const;

    // Visual appearance customization
    void setSelectionPen(const QPen& pen);
    QPen getSelectionPen() const;

    void setSelectionBrush(const QBrush& brush);
    QBrush getSelectionBrush() const;

    void setAnimationEnabled(bool enabled);
    bool isAnimationEnabled() const;

    // Selection area management
    void updateFromSelection();
    void clearSelection();

    // Rubber band selection
    void startRubberBand(const QPointF& startPos);
    void updateRubberBand(const QPointF& currentPos);
    void finishRubberBand();
    void cancelRubberBand();

    bool isRubberBandActive() const;

public slots:
    void onSelectionChanged();
    void onAnimationTimer();

signals:
    void selectionVisualizationChanged();
    void rubberBandSelectionFinished(const QRectF& rect);

private:
    Selection* selection_;
    Map* mapContext_;
    
    // Visual properties
    SelectionStyle style_;
    QPen selectionPen_;
    QBrush selectionBrush_;
    QPen rubberBandPen_;
    QBrush rubberBandBrush_;
    
    // Selection state
    QRectF rubberBandRect_;
    QPointF rubberBandStart_;
    bool rubberBandActive_;
    bool visible_;
    
    // Animation
    bool animationEnabled_;
    QTimer* animationTimer_;
    qreal animationPhase_;
    
    // Cached selection data
    QList<QRectF> selectedTileRects_;
    QRectF selectionBounds_;
    bool selectionCacheValid_;
    
    // Helper methods
    void updateSelectionCache();
    void invalidateSelectionCache();
    QRectF tileToSceneRect(const MapPos& tilePos) const;
    QList<QRectF> getSelectedTileRects() const;
    QRectF calculateSelectionBounds() const;
    
    // Drawing methods
    void drawRubberBand(QPainter* painter) const;
    void drawTileHighlights(QPainter* painter) const;
    void drawAreaOutline(QPainter* painter) const;
    void drawAnimatedSelection(QPainter* painter, qreal phase) const;
    
    // Style helpers
    QPen getAnimatedPen(const QPen& basePen, qreal phase) const;
    QBrush getAnimatedBrush(const QBrush& baseBrush, qreal phase) const;
};

/**
 * @brief SelectionRubberBandItem - Specialized rubber band selection item
 * 
 * A simplified version of SelectionItem specifically for rubber band selection.
 * This can be used when you only need rubber band functionality without
 * the full selection visualization features.
 */
class SelectionRubberBandItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit SelectionRubberBandItem(QGraphicsItem* parent = nullptr);
    ~SelectionRubberBandItem() override;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    // Rubber band control
    void setRect(const QRectF& rect);
    QRectF rect() const;

    void setStartPoint(const QPointF& point);
    void setEndPoint(const QPointF& point);

    void setPen(const QPen& pen);
    QPen pen() const;

    void setBrush(const QBrush& brush);
    QBrush brush() const;

    void setActive(bool active);
    bool isActive() const;

signals:
    void selectionChanged(const QRectF& rect);

private:
    QRectF rect_;
    QPointF startPoint_;
    QPointF endPoint_;
    QPen pen_;
    QBrush brush_;
    bool active_;

    void updateRect();
};

#endif // SELECTIONITEM_H
