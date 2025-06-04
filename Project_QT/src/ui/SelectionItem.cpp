#include "SelectionItem.h"
#include "../Selection.h"
#include "../Map.h"
#include "../Tile.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QDebug>
#include <QtMath>

// SelectionItem implementation
SelectionItem::SelectionItem(Selection* selection, Map* mapContext, QGraphicsItem* parent)
    : QGraphicsObject(parent),
      selection_(selection),
      mapContext_(mapContext),
      style_(SelectionStyle::TileHighlight),
      rubberBandActive_(false),
      visible_(true),
      animationEnabled_(true),
      animationPhase_(0.0),
      selectionCacheValid_(false) {
    
    // Set up visual properties
    selectionPen_ = QPen(Qt::yellow, 2, Qt::DashLine);
    selectionBrush_ = QBrush(QColor(255, 255, 0, 50));
    rubberBandPen_ = QPen(Qt::blue, 1, Qt::DashLine);
    rubberBandBrush_ = QBrush(QColor(0, 0, 255, 30));
    
    // Set up animation timer
    animationTimer_ = new QTimer(this);
    animationTimer_->setInterval(50); // 20 FPS animation
    connect(animationTimer_, &QTimer::timeout, this, &SelectionItem::onAnimationTimer);
    
    // Connect to selection changes
    if (selection_) {
        connect(selection_, &Selection::selectionChanged, this, &SelectionItem::onSelectionChanged);
    }
    
    // Set graphics item properties
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    setZValue(1000); // Draw on top of tiles
    
    updateFromSelection();
}

SelectionItem::~SelectionItem() {
    if (animationTimer_) {
        animationTimer_->stop();
    }
}

QRectF SelectionItem::boundingRect() const {
    if (rubberBandActive_) {
        return rubberBandRect_.normalized().adjusted(-5, -5, 5, 5);
    }
    
    if (!selectionCacheValid_) {
        const_cast<SelectionItem*>(this)->updateSelectionCache();
    }
    
    return selectionBounds_.adjusted(-5, -5, 5, 5);
}

QPainterPath SelectionItem::shape() const {
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void SelectionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    if (!painter || !visible_) {
        return;
    }
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Draw rubber band if active
    if (rubberBandActive_) {
        drawRubberBand(painter);
        return;
    }
    
    // Draw selection based on style
    switch (style_) {
        case SelectionStyle::RubberBand:
            drawRubberBand(painter);
            break;
        case SelectionStyle::TileHighlight:
            drawTileHighlights(painter);
            break;
        case SelectionStyle::AreaOutline:
            drawAreaOutline(painter);
            break;
    }
}

void SelectionItem::setSelectionStyle(SelectionStyle style) {
    if (style_ != style) {
        style_ = style;
        invalidateSelectionCache();
        update();
        emit selectionVisualizationChanged();
    }
}

SelectionItem::SelectionStyle SelectionItem::getSelectionStyle() const {
    return style_;
}

void SelectionItem::setRubberBandRect(const QRectF& rect) {
    if (rubberBandRect_ != rect) {
        prepareGeometryChange();
        rubberBandRect_ = rect;
        update();
    }
}

QRectF SelectionItem::getRubberBandRect() const {
    return rubberBandRect_;
}

void SelectionItem::setVisible(bool visible) {
    if (visible_ != visible) {
        visible_ = visible;
        QGraphicsObject::setVisible(visible);
        
        if (visible && animationEnabled_) {
            animationTimer_->start();
        } else {
            animationTimer_->stop();
        }
    }
}

bool SelectionItem::isVisible() const {
    return visible_;
}

void SelectionItem::setSelectionPen(const QPen& pen) {
    selectionPen_ = pen;
    update();
}

QPen SelectionItem::getSelectionPen() const {
    return selectionPen_;
}

void SelectionItem::setSelectionBrush(const QBrush& brush) {
    selectionBrush_ = brush;
    update();
}

QBrush SelectionItem::getSelectionBrush() const {
    return selectionBrush_;
}

void SelectionItem::setAnimationEnabled(bool enabled) {
    if (animationEnabled_ != enabled) {
        animationEnabled_ = enabled;
        
        if (enabled && visible_) {
            animationTimer_->start();
        } else {
            animationTimer_->stop();
            animationPhase_ = 0.0;
        }
        
        update();
    }
}

bool SelectionItem::isAnimationEnabled() const {
    return animationEnabled_;
}

void SelectionItem::updateFromSelection() {
    if (!selection_) {
        clearSelection();
        return;
    }
    
    invalidateSelectionCache();
    prepareGeometryChange();
    update();
    
    // Start animation if we have a selection and animation is enabled
    bool hasSelection = !selection_->isEmpty();
    if (hasSelection && animationEnabled_ && visible_) {
        animationTimer_->start();
    } else if (!hasSelection) {
        animationTimer_->stop();
        animationPhase_ = 0.0;
    }
}

void SelectionItem::clearSelection() {
    prepareGeometryChange();
    selectedTileRects_.clear();
    selectionBounds_ = QRectF();
    selectionCacheValid_ = true;
    animationTimer_->stop();
    animationPhase_ = 0.0;
    update();
}

void SelectionItem::startRubberBand(const QPointF& startPos) {
    rubberBandStart_ = startPos;
    rubberBandRect_ = QRectF(startPos, startPos);
    rubberBandActive_ = true;
    prepareGeometryChange();
    update();
}

void SelectionItem::updateRubberBand(const QPointF& currentPos) {
    if (!rubberBandActive_) {
        return;
    }
    
    QRectF newRect = QRectF(rubberBandStart_, currentPos).normalized();
    setRubberBandRect(newRect);
}

void SelectionItem::finishRubberBand() {
    if (rubberBandActive_) {
        QRectF finalRect = rubberBandRect_;
        rubberBandActive_ = false;
        prepareGeometryChange();
        update();
        
        emit rubberBandSelectionFinished(finalRect);
    }
}

void SelectionItem::cancelRubberBand() {
    if (rubberBandActive_) {
        rubberBandActive_ = false;
        rubberBandRect_ = QRectF();
        prepareGeometryChange();
        update();
    }
}

bool SelectionItem::isRubberBandActive() const {
    return rubberBandActive_;
}

void SelectionItem::onSelectionChanged() {
    updateFromSelection();
}

void SelectionItem::onAnimationTimer() {
    animationPhase_ += 0.1;
    if (animationPhase_ >= 2.0 * M_PI) {
        animationPhase_ = 0.0;
    }
    update();
}

void SelectionItem::updateSelectionCache() {
    selectedTileRects_.clear();
    selectionBounds_ = QRectF();
    
    if (!selection_ || selection_->isEmpty()) {
        selectionCacheValid_ = true;
        return;
    }
    
    selectedTileRects_ = getSelectedTileRects();
    selectionBounds_ = calculateSelectionBounds();
    selectionCacheValid_ = true;
}

void SelectionItem::invalidateSelectionCache() {
    selectionCacheValid_ = false;
}

QRectF SelectionItem::tileToSceneRect(const MapPos& tilePos) const {
    // Convert tile position to scene coordinates
    // Assuming each tile is 32x32 pixels
    const qreal tileSize = 32.0;
    return QRectF(tilePos.x * tileSize, tilePos.y * tileSize, tileSize, tileSize);
}

QList<QRectF> SelectionItem::getSelectedTileRects() const {
    QList<QRectF> rects;
    
    if (!selection_) {
        return rects;
    }
    
    const auto& selectedTiles = selection_->getSelectedTiles();
    for (const MapPos& tilePos : selectedTiles) {
        rects.append(tileToSceneRect(tilePos));
    }
    
    return rects;
}

QRectF SelectionItem::calculateSelectionBounds() const {
    if (selectedTileRects_.isEmpty()) {
        return QRectF();
    }
    
    QRectF bounds = selectedTileRects_.first();
    for (const QRectF& rect : selectedTileRects_) {
        bounds = bounds.united(rect);
    }
    
    return bounds;
}

void SelectionItem::drawRubberBand(QPainter* painter) const {
    if (rubberBandRect_.isEmpty()) {
        return;
    }
    
    painter->save();
    
    QPen pen = rubberBandPen_;
    QBrush brush = rubberBandBrush_;
    
    if (animationEnabled_) {
        pen = getAnimatedPen(pen, animationPhase_);
        brush = getAnimatedBrush(brush, animationPhase_);
    }
    
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawRect(rubberBandRect_);
    
    painter->restore();
}

void SelectionItem::drawTileHighlights(QPainter* painter) const {
    if (!selectionCacheValid_) {
        const_cast<SelectionItem*>(this)->updateSelectionCache();
    }
    
    if (selectedTileRects_.isEmpty()) {
        return;
    }
    
    painter->save();
    
    QPen pen = selectionPen_;
    QBrush brush = selectionBrush_;
    
    if (animationEnabled_) {
        pen = getAnimatedPen(pen, animationPhase_);
        brush = getAnimatedBrush(brush, animationPhase_);
    }
    
    painter->setPen(pen);
    painter->setBrush(brush);
    
    for (const QRectF& rect : selectedTileRects_) {
        painter->drawRect(rect);
    }
    
    painter->restore();
}

void SelectionItem::drawAreaOutline(QPainter* painter) const {
    if (!selectionCacheValid_) {
        const_cast<SelectionItem*>(this)->updateSelectionCache();
    }
    
    if (selectionBounds_.isEmpty()) {
        return;
    }
    
    painter->save();
    
    QPen pen = selectionPen_;
    pen.setWidth(pen.width() + 1); // Thicker for area outline
    
    if (animationEnabled_) {
        pen = getAnimatedPen(pen, animationPhase_);
    }
    
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(selectionBounds_);
    
    painter->restore();
}

QPen SelectionItem::getAnimatedPen(const QPen& basePen, qreal phase) const {
    QPen animatedPen = basePen;
    
    // Animate opacity
    QColor color = basePen.color();
    qreal alpha = 0.5 + 0.5 * qSin(phase);
    color.setAlphaF(color.alphaF() * alpha);
    animatedPen.setColor(color);
    
    return animatedPen;
}

QBrush SelectionItem::getAnimatedBrush(const QBrush& baseBrush, qreal phase) const {
    QBrush animatedBrush = baseBrush;

    // Animate opacity
    QColor color = baseBrush.color();
    qreal alpha = 0.3 + 0.3 * qSin(phase);
    color.setAlphaF(color.alphaF() * alpha);
    animatedBrush.setColor(color);

    return animatedBrush;
}

// SelectionRubberBandItem implementation
SelectionRubberBandItem::SelectionRubberBandItem(QGraphicsItem* parent)
    : QGraphicsObject(parent),
      active_(false) {

    // Set up default appearance
    pen_ = QPen(Qt::blue, 1, Qt::DashLine);
    brush_ = QBrush(QColor(0, 0, 255, 30));

    // Set graphics item properties
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    setZValue(1001); // Draw on top of selection items
    setVisible(false);
}

SelectionRubberBandItem::~SelectionRubberBandItem() {
    // No cleanup needed
}

QRectF SelectionRubberBandItem::boundingRect() const {
    if (!active_ || rect_.isEmpty()) {
        return QRectF();
    }

    qreal penWidth = pen_.widthF();
    return rect_.normalized().adjusted(-penWidth, -penWidth, penWidth, penWidth);
}

void SelectionRubberBandItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!painter || !active_ || rect_.isEmpty()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(pen_);
    painter->setBrush(brush_);
    painter->drawRect(rect_);

    painter->restore();
}

void SelectionRubberBandItem::setRect(const QRectF& rect) {
    if (rect_ != rect) {
        prepareGeometryChange();
        rect_ = rect;
        update();
        emit selectionChanged(rect_);
    }
}

QRectF SelectionRubberBandItem::rect() const {
    return rect_;
}

void SelectionRubberBandItem::setStartPoint(const QPointF& point) {
    startPoint_ = point;
    updateRect();
}

void SelectionRubberBandItem::setEndPoint(const QPointF& point) {
    endPoint_ = point;
    updateRect();
}

void SelectionRubberBandItem::setPen(const QPen& pen) {
    if (pen_ != pen) {
        pen_ = pen;
        update();
    }
}

QPen SelectionRubberBandItem::pen() const {
    return pen_;
}

void SelectionRubberBandItem::setBrush(const QBrush& brush) {
    if (brush_ != brush) {
        brush_ = brush;
        update();
    }
}

QBrush SelectionRubberBandItem::brush() const {
    return brush_;
}

void SelectionRubberBandItem::setActive(bool active) {
    if (active_ != active) {
        active_ = active;
        setVisible(active);

        if (!active) {
            rect_ = QRectF();
            prepareGeometryChange();
        }

        update();
    }
}

bool SelectionRubberBandItem::isActive() const {
    return active_;
}

void SelectionRubberBandItem::updateRect() {
    if (!startPoint_.isNull() && !endPoint_.isNull()) {
        setRect(QRectF(startPoint_, endPoint_).normalized());
    }
}


