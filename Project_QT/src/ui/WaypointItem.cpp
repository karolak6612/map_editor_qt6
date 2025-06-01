#include "WaypointItem.h"
#include "../Waypoint.h"
#include "../Map.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsScene>
#include <QMenu>
#include <QDebug>
#include <QtMath>

WaypointItem::WaypointItem(Waypoint* waypoint, Map* mapContext, QGraphicsItem* parent)
    : QGraphicsObject(parent),
      waypoint_(waypoint),
      mapContext_(mapContext),
      style_(WaypointStyle::LabeledMarker),
      interactionMode_(InteractionMode::Selectable),
      markerSize_(16.0),
      showRadius_(true),
      showLabel_(true),
      highlighted_(false),
      animationEnabled_(true),
      hovering_(false),
      dragging_(false),
      animationPhase_(0.0),
      boundingRectValid_(false) {
    
    // Set up default appearance
    markerPen_ = QPen(Qt::darkBlue, 2);
    markerBrush_ = QBrush(Qt::blue);
    radiusPen_ = QPen(Qt::blue, 1, Qt::DashLine);
    radiusBrush_ = QBrush(QColor(0, 0, 255, 30));
    labelColor_ = Qt::black;
    labelFont_ = QFont("Arial", 10, QFont::Bold);
    
    // Set up animation timer
    animationTimer_ = new QTimer(this);
    animationTimer_->setInterval(50); // 20 FPS
    connect(animationTimer_, &QTimer::timeout, this, &WaypointItem::onAnimationTimer);
    
    // Connect to waypoint changes
    if (waypoint_) {
        connect(waypoint_, &Waypoint::waypointChanged, this, &WaypointItem::onWaypointChanged);
    }
    
    // Set graphics item properties
    setFlag(QGraphicsItem::ItemIsSelectable, interactionMode_ != InteractionMode::ReadOnly);
    setFlag(QGraphicsItem::ItemIsMovable, interactionMode_ == InteractionMode::Draggable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setAcceptHoverEvents(true);
    setZValue(500); // Draw above tiles but below selection
    
    updatePosition();
    updateFromWaypoint();
}

WaypointItem::~WaypointItem() {
    if (animationTimer_) {
        animationTimer_->stop();
    }
}

QRectF WaypointItem::boundingRect() const {
    if (!boundingRectValid_) {
        updateBoundingRect();
    }
    return cachedBoundingRect_;
}

QPainterPath WaypointItem::shape() const {
    QPainterPath path;
    
    if (!waypoint_) {
        return path;
    }
    
    // Create shape based on marker size
    QRectF markerRect(-markerSize_/2, -markerSize_/2, markerSize_, markerSize_);
    path.addEllipse(markerRect);
    
    // Add radius area if shown
    if (showRadius_ && waypoint_->radius() > 0) {
        qreal radiusPixels = waypoint_->radius() * 32.0; // Assuming 32 pixels per tile
        QRectF radiusRect(-radiusPixels, -radiusPixels, radiusPixels * 2, radiusPixels * 2);
        path.addEllipse(radiusRect);
    }
    
    return path;
}

void WaypointItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    if (!painter || !waypoint_) {
        return;
    }
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Draw radius first (background)
    if (showRadius_ && waypoint_->radius() > 0) {
        drawRadius(painter);
    }
    
    // Draw waypoint based on style
    switch (style_) {
        case WaypointStyle::Marker:
            drawMarker(painter);
            break;
        case WaypointStyle::Circle:
            drawCircle(painter);
            break;
        case WaypointStyle::Icon:
            drawIcon(painter);
            break;
        case WaypointStyle::RadiusArea:
            drawRadius(painter);
            break;
        case WaypointStyle::LabeledMarker:
            drawMarker(painter);
            if (showLabel_) {
                drawLabel(painter);
            }
            break;
    }
    
    // Draw highlight if needed
    if (highlighted_ || hovering_ || isSelected()) {
        drawHighlight(painter);
    }
}

Waypoint* WaypointItem::getWaypoint() const {
    return waypoint_;
}

void WaypointItem::setWaypoint(Waypoint* waypoint) {
    if (waypoint_ != waypoint) {
        // Disconnect from old waypoint
        if (waypoint_) {
            disconnect(waypoint_, &Waypoint::waypointChanged, this, &WaypointItem::onWaypointChanged);
        }
        
        waypoint_ = waypoint;
        
        // Connect to new waypoint
        if (waypoint_) {
            connect(waypoint_, &Waypoint::waypointChanged, this, &WaypointItem::onWaypointChanged);
        }
        
        updateFromWaypoint();
    }
}

void WaypointItem::setWaypointStyle(WaypointStyle style) {
    if (style_ != style) {
        style_ = style;
        invalidateBoundingRect();
        update();
    }
}

WaypointItem::WaypointStyle WaypointItem::getWaypointStyle() const {
    return style_;
}

void WaypointItem::setInteractionMode(InteractionMode mode) {
    if (interactionMode_ != mode) {
        interactionMode_ = mode;
        
        setFlag(QGraphicsItem::ItemIsSelectable, mode != InteractionMode::ReadOnly);
        setFlag(QGraphicsItem::ItemIsMovable, mode == InteractionMode::Draggable);
        
        update();
    }
}

WaypointItem::InteractionMode WaypointItem::getInteractionMode() const {
    return interactionMode_;
}

void WaypointItem::setMarkerSize(qreal size) {
    if (!qFuzzyCompare(markerSize_, size)) {
        markerSize_ = qMax(4.0, size);
        invalidateBoundingRect();
        update();
    }
}

qreal WaypointItem::getMarkerSize() const {
    return markerSize_;
}

void WaypointItem::setShowRadius(bool show) {
    if (showRadius_ != show) {
        showRadius_ = show;
        invalidateBoundingRect();
        update();
    }
}

bool WaypointItem::getShowRadius() const {
    return showRadius_;
}

void WaypointItem::setShowLabel(bool show) {
    if (showLabel_ != show) {
        showLabel_ = show;
        invalidateBoundingRect();
        update();
    }
}

bool WaypointItem::getShowLabel() const {
    return showLabel_;
}

void WaypointItem::setLabelFont(const QFont& font) {
    if (labelFont_ != font) {
        labelFont_ = font;
        invalidateBoundingRect();
        update();
    }
}

QFont WaypointItem::getLabelFont() const {
    return labelFont_;
}

void WaypointItem::setMarkerPen(const QPen& pen) {
    markerPen_ = pen;
    update();
}

QPen WaypointItem::getMarkerPen() const {
    return markerPen_;
}

void WaypointItem::setMarkerBrush(const QBrush& brush) {
    markerBrush_ = brush;
    update();
}

QBrush WaypointItem::getMarkerBrush() const {
    return markerBrush_;
}

void WaypointItem::setRadiusPen(const QPen& pen) {
    radiusPen_ = pen;
    update();
}

QPen WaypointItem::getRadiusPen() const {
    return radiusPen_;
}

void WaypointItem::setRadiusBrush(const QBrush& brush) {
    radiusBrush_ = brush;
    update();
}

QBrush WaypointItem::getRadiusBrush() const {
    return radiusBrush_;
}

void WaypointItem::setLabelColor(const QColor& color) {
    labelColor_ = color;
    update();
}

QColor WaypointItem::getLabelColor() const {
    return labelColor_;
}

void WaypointItem::setAnimationEnabled(bool enabled) {
    if (animationEnabled_ != enabled) {
        animationEnabled_ = enabled;
        
        if (enabled && (highlighted_ || hovering_)) {
            animationTimer_->start();
        } else {
            animationTimer_->stop();
            animationPhase_ = 0.0;
        }
        
        update();
    }
}

bool WaypointItem::isAnimationEnabled() const {
    return animationEnabled_;
}

void WaypointItem::setHighlighted(bool highlighted) {
    if (highlighted_ != highlighted) {
        highlighted_ = highlighted;
        
        if (highlighted && animationEnabled_) {
            animationTimer_->start();
        } else if (!hovering_) {
            animationTimer_->stop();
            animationPhase_ = 0.0;
        }
        
        update();
    }
}

bool WaypointItem::isHighlighted() const {
    return highlighted_;
}

void WaypointItem::updatePosition() {
    if (!waypoint_) {
        return;
    }
    
    QPointF scenePos = waypointToScenePos();
    setPos(scenePos);
}

void WaypointItem::updateFromWaypoint() {
    if (!waypoint_) {
        return;
    }
    
    // Update visual properties from waypoint
    QColor waypointColor = waypoint_->color();
    if (waypointColor.isValid()) {
        markerBrush_.setColor(waypointColor);
        radiusBrush_.setColor(QColor(waypointColor.red(), waypointColor.green(), waypointColor.blue(), 30));
    }
    
    updatePosition();
    invalidateBoundingRect();
    update();
}

void WaypointItem::onWaypointChanged() {
    updateFromWaypoint();
}

void WaypointItem::onAnimationTimer() {
    animationPhase_ += 0.15;
    if (animationPhase_ >= 2.0 * M_PI) {
        animationPhase_ = 0.0;
    }
    update();
}

void WaypointItem::updateBoundingRect() const {
    cachedBoundingRect_ = calculateBoundingRect();
    boundingRectValid_ = true;
}

void WaypointItem::invalidateBoundingRect() {
    boundingRectValid_ = false;
    prepareGeometryChange();
}

QRectF WaypointItem::calculateBoundingRect() const {
    if (!waypoint_) {
        return QRectF();
    }
    
    QRectF rect;
    
    // Start with marker bounds
    qreal halfSize = markerSize_ / 2.0;
    rect = QRectF(-halfSize, -halfSize, markerSize_, markerSize_);
    
    // Expand for radius if shown
    if (showRadius_ && waypoint_->radius() > 0) {
        qreal radiusPixels = waypoint_->radius() * 32.0; // 32 pixels per tile
        QRectF radiusRect(-radiusPixels, -radiusPixels, radiusPixels * 2, radiusPixels * 2);
        rect = rect.united(radiusRect);
    }
    
    // Expand for label if shown
    if (showLabel_ && !waypoint_->name().isEmpty()) {
        QFontMetrics fm(labelFont_);
        QRectF textRect = fm.boundingRect(waypoint_->name());
        textRect.moveCenter(QPointF(0, markerSize_ + textRect.height()));
        rect = rect.united(textRect);
    }
    
    // Add some padding
    return rect.adjusted(-2, -2, 2, 2);
}

QPointF WaypointItem::waypointToScenePos() const {
    if (!waypoint_) {
        return QPointF();
    }
    
    MapPos pos = waypoint_->position();
    // Convert tile coordinates to scene coordinates (32 pixels per tile)
    return QPointF(pos.x * 32.0, pos.y * 32.0);
}

MapPos WaypointItem::scenePosToWaypoint(const QPointF& scenePos) const {
    // Convert scene coordinates to tile coordinates
    int x = qRound(scenePos.x() / 32.0);
    int y = qRound(scenePos.y() / 32.0);
    int z = waypoint_ ? waypoint_->position().z : 0;
    return MapPos(x, y, z);
}

// Mouse interaction
void WaypointItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (interactionMode_ == InteractionMode::ReadOnly) {
        event->ignore();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (interactionMode_ == InteractionMode::Draggable || interactionMode_ == InteractionMode::Editable) {
            startDrag(event->pos());
        }

        emit waypointClicked(waypoint_);
        event->accept();
    } else if (event->button() == Qt::RightButton) {
        // Task 74: Right-click for context menu
        emit waypointRightClicked(waypoint_, event->screenPos());
        event->accept();
    } else {
        QGraphicsObject::mousePressEvent(event);
    }
}

void WaypointItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (dragging_) {
        updateDrag(event->scenePos());
        event->accept();
    } else {
        QGraphicsObject::mouseMoveEvent(event);
    }
}

void WaypointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && dragging_) {
        finishDrag(event->scenePos());
        event->accept();
    } else {
        QGraphicsObject::mouseReleaseEvent(event);
    }
}

void WaypointItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    if (interactionMode_ != InteractionMode::ReadOnly) {
        emit waypointDoubleClicked(waypoint_);
        event->accept();
    } else {
        QGraphicsObject::mouseDoubleClickEvent(event);
    }
}

// Hover effects
void WaypointItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    hovering_ = true;

    if (animationEnabled_) {
        animationTimer_->start();
    }

    update();
}

void WaypointItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    hovering_ = false;

    if (!highlighted_) {
        animationTimer_->stop();
        animationPhase_ = 0.0;
    }

    update();
}

// Context menu
void WaypointItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    if (interactionMode_ != InteractionMode::ReadOnly) {
        emit waypointContextMenu(waypoint_, event->screenPos());
        event->accept();
    } else {
        QGraphicsObject::contextMenuEvent(event);
    }
}

// Drawing methods
void WaypointItem::drawMarker(QPainter* painter) const {
    painter->save();

    QPen pen = getEffectivePen(markerPen_);
    QBrush brush = getEffectiveBrush(markerBrush_);

    painter->setPen(pen);
    painter->setBrush(brush);

    qreal size = markerSize_;
    if (animationEnabled_ && (highlighted_ || hovering_)) {
        size += 2.0 * qSin(animationPhase_);
    }

    QRectF markerRect(-size/2, -size/2, size, size);
    painter->drawEllipse(markerRect);

    painter->restore();
}

void WaypointItem::drawCircle(QPainter* painter) const {
    painter->save();

    QPen pen = getEffectivePen(markerPen_);
    QBrush brush = getEffectiveBrush(markerBrush_);

    painter->setPen(pen);
    painter->setBrush(brush);

    qreal radius = markerSize_ / 2.0;
    if (animationEnabled_ && (highlighted_ || hovering_)) {
        radius += qSin(animationPhase_);
    }

    painter->drawEllipse(QPointF(0, 0), radius, radius);

    painter->restore();
}

void WaypointItem::drawIcon(QPainter* painter) const {
    QPixmap icon = getWaypointIcon();
    if (icon.isNull()) {
        drawMarker(painter); // Fallback to marker
        return;
    }

    painter->save();

    qreal scale = 1.0;
    if (animationEnabled_ && (highlighted_ || hovering_)) {
        scale += 0.1 * qSin(animationPhase_);
    }

    if (!qFuzzyCompare(scale, 1.0)) {
        painter->scale(scale, scale);
    }

    QRectF iconRect(-markerSize_/2, -markerSize_/2, markerSize_, markerSize_);
    painter->drawPixmap(iconRect, icon, icon.rect());

    painter->restore();
}

void WaypointItem::drawRadius(QPainter* painter) const {
    if (!waypoint_ || waypoint_->radius() <= 0) {
        return;
    }

    painter->save();

    QPen pen = getEffectivePen(radiusPen_);
    QBrush brush = getEffectiveBrush(radiusBrush_);

    painter->setPen(pen);
    painter->setBrush(brush);

    qreal radiusPixels = waypoint_->radius() * 32.0; // 32 pixels per tile
    if (animationEnabled_ && (highlighted_ || hovering_)) {
        radiusPixels += 2.0 * qSin(animationPhase_);
    }

    painter->drawEllipse(QPointF(0, 0), radiusPixels, radiusPixels);

    painter->restore();
}

void WaypointItem::drawLabel(QPainter* painter) const {
    if (!waypoint_ || waypoint_->name().isEmpty()) {
        return;
    }

    painter->save();

    painter->setFont(labelFont_);
    painter->setPen(getEffectiveColor(labelColor_));

    QFontMetrics fm(labelFont_);
    QRectF textRect = fm.boundingRect(waypoint_->name());
    textRect.moveCenter(QPointF(0, markerSize_ + textRect.height()));

    // Draw text background
    painter->fillRect(textRect.adjusted(-2, -1, 2, 1), QColor(255, 255, 255, 200));

    painter->drawText(textRect, Qt::AlignCenter, waypoint_->name());

    painter->restore();
}

void WaypointItem::drawHighlight(QPainter* painter) const {
    painter->save();

    QPen highlightPen(Qt::yellow, 3);
    if (animationEnabled_) {
        qreal alpha = 0.5 + 0.5 * qSin(animationPhase_);
        QColor color = highlightPen.color();
        color.setAlphaF(alpha);
        highlightPen.setColor(color);
    }

    painter->setPen(highlightPen);
    painter->setBrush(Qt::NoBrush);

    qreal size = markerSize_ + 4;
    QRectF highlightRect(-size/2, -size/2, size, size);
    painter->drawEllipse(highlightRect);

    painter->restore();
}

// Style helpers
QPen WaypointItem::getEffectivePen(const QPen& basePen) const {
    QPen pen = basePen;

    if (isSelected()) {
        pen.setWidth(pen.width() + 1);
        pen.setColor(Qt::yellow);
    }

    return pen;
}

QBrush WaypointItem::getEffectiveBrush(const QBrush& baseBrush) const {
    QBrush brush = baseBrush;

    if (hovering_ && !isSelected()) {
        QColor color = brush.color();
        color = color.lighter(120);
        brush.setColor(color);
    }

    return brush;
}

QColor WaypointItem::getEffectiveColor(const QColor& baseColor) const {
    QColor color = baseColor;

    if (isSelected()) {
        color = Qt::yellow;
    } else if (hovering_) {
        color = color.darker(120);
    }

    return color;
}

// Icon helpers
QPixmap WaypointItem::getWaypointIcon() const {
    if (!waypoint_) {
        return QPixmap();
    }

    QString iconName = waypoint_->icon();
    if (iconName.isEmpty()) {
        return createDefaultIcon();
    }

    // Try to load icon from resources or file system
    QPixmap icon(QString(":/icons/waypoints/%1.png").arg(iconName));
    if (icon.isNull()) {
        icon = QPixmap(QString("icons/waypoints/%1.png").arg(iconName));
    }

    if (icon.isNull()) {
        return createDefaultIcon();
    }

    return icon.scaled(markerSize_, markerSize_, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QPixmap WaypointItem::createDefaultIcon() const {
    QPixmap icon(16, 16);
    icon.fill(Qt::transparent);

    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::darkBlue, 2));
    painter.setBrush(QBrush(Qt::blue));
    painter.drawEllipse(2, 2, 12, 12);

    return icon;
}

// Interaction helpers
bool WaypointItem::isPositionValid(const MapPos& position) const {
    if (!mapContext_) {
        return true; // Can't validate without map context
    }

    return mapContext_->isValidCoordinate(position.x, position.y, position.z);
}

void WaypointItem::startDrag(const QPointF& startPos) {
    if (!waypoint_) {
        return;
    }

    dragging_ = true;
    dragStartPos_ = startPos;
    originalPosition_ = waypoint_->position();
    setCursor(Qt::ClosedHandCursor);
}

void WaypointItem::updateDrag(const QPointF& currentPos) {
    if (!dragging_ || !waypoint_) {
        return;
    }

    MapPos newPosition = scenePosToWaypoint(currentPos);
    if (isPositionValid(newPosition)) {
        waypoint_->setPosition(newPosition);
        updatePosition();
    }
}

void WaypointItem::finishDrag(const QPointF& endPos) {
    if (!dragging_ || !waypoint_) {
        return;
    }

    MapPos newPosition = scenePosToWaypoint(endPos);
    if (isPositionValid(newPosition)) {
        waypoint_->setPosition(newPosition);
        emit waypointMoved(waypoint_, newPosition);
    } else {
        // Revert to original position
        waypoint_->setPosition(originalPosition_);
    }

    dragging_ = false;
    unsetCursor();
    updatePosition();
}

void WaypointItem::cancelDrag() {
    if (!dragging_ || !waypoint_) {
        return;
    }

    waypoint_->setPosition(originalPosition_);
    dragging_ = false;
    unsetCursor();
    updatePosition();
}

#include "WaypointItem.moc"
