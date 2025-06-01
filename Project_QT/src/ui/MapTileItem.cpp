#include "MapTileItem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QVariant>
#include <QPainterPath>
#include <QDateTime>

// Assuming Tile.h provides full definition for Tile methods like isBlocking, getPosition, draw
#include "../Tile.h"
// Assuming Map.h provides full definition for Map methods like getSelection
#include "../Map.h"
// Assuming Selection.h provides full definition for Selection methods like isSelected
#include "../Selection.h"

MapTileItem::MapTileItem(Tile* tile, Map* mapContext, QGraphicsItem* parent)
    : QGraphicsObject(parent), tile_(tile), mapContext_(mapContext) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // Connect to tile changes for cache invalidation
    if (tile_) {
        connect(tile_, &Tile::tileChanged, this, &MapTileItem::invalidateCache);
        connect(tile_, &Tile::visualChanged, this, &MapTileItem::invalidateCache);
        lastTileModification_ = QDateTime::currentMSecsSinceEpoch();
    }

    // drawingOptions_ is default-initialized by its own constructor
}

MapTileItem::~MapTileItem() {
    // tile_ and mapContext_ are not owned, so no deletion here.
}

QRectF MapTileItem::boundingRect() const {
    return QRectF(0, 0, TILE_PIXEL_SIZE, TILE_PIXEL_SIZE);
}

QPainterPath MapTileItem::shape() const {
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void MapTileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(widget);

    if (!painter) {
        return;
    }

    // Sync selection state between QGraphicsItem and Tile
    syncSelectionWithTile();

    if (!tile_) {
        // Enhanced placeholder for a null tile
        drawNullTilePlaceholder(painter, boundingRect());
        return;
    }

    // Prepare drawing options based on current state
    DrawingOptions finalOptions = drawingOptions_;

    // Update selection highlighting
    bool isItemSelected = (option->state & QStyle::State_Selected) || tile_->isSelected();
    finalOptions.highlightSelectedTile = isItemSelected;

    // Check if we can use cached rendering
    updateCacheIfNeeded();

    if (isCacheValid() && cachedOptions_.highlightSelectedTile == finalOptions.highlightSelectedTile) {
        // Use cached pixmap for performance
        painter->drawPixmap(boundingRect().toRect(), cachedPixmap_);

        // Draw selection highlight on top if needed
        if (isItemSelected && finalOptions.highlightSelectedTile) {
            painter->save();
            QPen pen(Qt::yellow, 2);
            pen.setStyle(Qt::DotLine);
            painter->setPen(pen);
            painter->setBrush(QColor(255, 255, 0, 50));
            painter->drawRect(boundingRect());
            painter->restore();
        }
    } else {
        // Direct rendering - cache will be updated by updateCacheIfNeeded
        tile_->draw(painter, boundingRect(), finalOptions);
    }
}

QVariant MapTileItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == QGraphicsItem::ItemSelectedHasChanged) {
        // This is triggered when this QGraphicsItem's selection state changes.
        // We might want to update the underlying Tile's selection state here too,
        // or just rely on the paint method using option->state.
        if (tile_) {
            // tile_->setSelected(value.toBool()); // Option 1: Sync Tile's own selected state
            qDebug() << "MapTileItem for Tile at"
                     << tile_->mapPos().x << "," << tile_->mapPos().y << "," << tile_->mapPos().z
                     << "QGraphicsItem selection changed to" << value.toBool();
        } else {
            qDebug() << "MapTileItem (null tile) selection changed to" << value.toBool();
        }
        update(); // Trigger a repaint to reflect selection change if Tile::draw depends on it
    }
    return QGraphicsObject::itemChange(change, value);
}

bool MapTileItem::isSolid() const {
    if (tile_) {
        return tile_->isBlocking(); // Assuming Tile::isBlocking() gives the effective blocking state
    }
    return true; // Default to solid (non-walkable) if no tile data, safer default
}

bool MapTileItem::isWalkable() const {
    if (tile_) {
        return !tile_->isBlocking(); // Simple inverse, actual walkability can be more complex
    }
    return false; // Default to not walkable if no tile data
}

void MapTileItem::setDrawingOptions(const DrawingOptions& options) {
    bool needsUpdate = false;
    // Compare individual members if DrawingOptions doesn't have operator==
    // For simplicity, assume any call to setDrawingOptions might change something.
    if (drawingOptions_.showGround != options.showGround ||
        drawingOptions_.showItems != options.showItems ||
        drawingOptions_.showCreatures != options.showCreatures ||
        drawingOptions_.showSpawns != options.showSpawns ||
        drawingOptions_.showTileFlags != options.showTileFlags ||
        drawingOptions_.highlightSelectedTile != options.highlightSelectedTile /* etc. */) {
        needsUpdate = true;
    }
    drawingOptions_ = options;
    if (needsUpdate) {
        invalidateCache();
        update(); // Repaint with new options
    }
}

// --- Cache Management ---

void MapTileItem::invalidateCache() {
    cacheValid_ = false;
    update(); // Trigger repaint
}

void MapTileItem::updateCache() {
    if (!tile_) return;

    cachedPixmap_ = QPixmap(TILE_PIXEL_SIZE, TILE_PIXEL_SIZE);
    cachedPixmap_.fill(Qt::transparent);

    QPainter cachePainter(&cachedPixmap_);
    cachePainter.setRenderHint(QPainter::Antialiasing);

    // Draw tile content to cache (without selection highlight)
    DrawingOptions cacheOptions = drawingOptions_;
    cacheOptions.highlightSelectedTile = false; // Selection drawn separately

    tile_->draw(&cachePainter, QRectF(0, 0, TILE_PIXEL_SIZE, TILE_PIXEL_SIZE), cacheOptions);

    cachePainter.end();

    cachedOptions_ = cacheOptions;
    cacheValid_ = true;
    lastTileModification_ = QDateTime::currentMSecsSinceEpoch();
}

bool MapTileItem::isCacheValid() const {
    return cacheValid_ && !cachedPixmap_.isNull();
}

void MapTileItem::drawNullTilePlaceholder(QPainter* painter, const QRectF& rect) const {
    if (!painter) return;

    painter->save();

    // Draw a distinctive pattern for null tiles
    QColor backgroundColor = Qt::magenta;
    backgroundColor.setAlpha(150);
    painter->fillRect(rect, backgroundColor);

    // Draw diagonal stripes to indicate missing data
    QPen stripePen(Qt::black, 1);
    painter->setPen(stripePen);

    const qreal stripeSpacing = 8.0;
    for (qreal x = rect.left(); x < rect.right() + rect.height(); x += stripeSpacing) {
        painter->drawLine(QPointF(x, rect.top()),
                         QPointF(x - rect.height(), rect.bottom()));
    }

    // Draw border
    QPen borderPen(Qt::black, 2);
    painter->setPen(borderPen);
    painter->drawRect(rect);

    // Draw text if there's enough space
    if (rect.width() > 40 && rect.height() > 20) {
        QFont font = painter->font();
        font.setPointSize(qMax(6, font.pointSize() - 2));
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(Qt::white);

        QRectF textRect = rect.adjusted(2, 2, -2, -2);
        painter->drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, "NULL\nTILE");
    }

    painter->restore();
}

// --- Enhanced Collision Interface ---

bool MapTileItem::hasBlockingItems() const {
    if (!tile_) return false;

    // Check if any items on the tile are blocking
    for (Item* item : tile_->items()) {
        if (item && item->isBlocking()) {
            return true;
        }
    }

    // Check ground
    Item* ground = tile_->getGround();
    return ground && ground->isBlocking();
}

bool MapTileItem::hasWalkableGround() const {
    if (!tile_) return false;

    Item* ground = tile_->getGround();
    return ground && !ground->isBlocking();
}

int MapTileItem::getMovementCost() const {
    if (!tile_) return 100; // High cost for invalid tiles

    if (tile_->isBlocking()) return 100; // Blocked tiles have high cost

    // Base movement cost
    int cost = 10;

    // Modify based on ground type
    Item* ground = tile_->getGround();
    if (ground) {
        // Different ground types could have different movement costs
        // This would need to be implemented based on item properties
        cost = 10; // Default for now
    }

    return cost;
}

// --- Mouse Event Handling ---

void MapTileItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Handle tile selection
        setSelected(!isSelected());

        if (tile_) {
            qDebug() << "MapTileItem clicked at" << tile_->mapPos().x << "," << tile_->mapPos().y << "," << tile_->mapPos().z;
        }
    }

    QGraphicsObject::mousePressEvent(event);
}

void MapTileItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsObject::mouseReleaseEvent(event);
}

// --- Helper Methods ---

void MapTileItem::syncSelectionWithTile() {
    if (!tile_) return;

    bool graphicsItemSelected = isSelected();
    bool tileSelected = tile_->isSelected();

    // Sync selection states if they differ
    if (graphicsItemSelected != tileSelected) {
        if (graphicsItemSelected) {
            tile_->setSelected(true);
        } else {
            // Only deselect tile if graphics item is not selected
            // This allows for external tile selection to persist
        }
    }
}

void MapTileItem::updateCacheIfNeeded() const {
    if (!isCacheValid()) {
        const_cast<MapTileItem*>(this)->updateCache();
    }
}
