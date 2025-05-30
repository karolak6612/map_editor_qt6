#include "MapTileItem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <QVariant>
#include <QPainterPath>

// Assuming Tile.h provides full definition for Tile methods like isBlocking, getPosition, draw
#include "../Tile.h" 
// Assuming Map.h provides full definition for Map methods like getSelection
#include "../Map.h"   
// Assuming Selection.h provides full definition for Selection methods like isSelected
#include "../Selection.h" 

MapTileItem::MapTileItem(Tile* tile, Map* mapContext, QGraphicsItem* parent)
    : QGraphicsObject(parent), tile_(tile), mapContext_(mapContext) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    // Consider ItemSendsGeometryChanges if its position can change and you need to react to it.
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
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

    // Start with the item's own drawing options, then potentially override with view-specific ones.
    // For this task, the passed 'option' (QStyleOptionGraphicsItem) is key for selection state.
    DrawingOptions currentDrawingOptions = drawingOptions_; // Copy member options

    // Update selection highlight based on QGraphicsItem's selection state
    if (option->state & QStyle::State_Selected) {
        // The QGraphicsScene handles drawing the default selection rectangle if ItemIsSelectable is true.
        // So, we might not need to do this manually unless we want a custom highlight
        // that Tile::draw itself provides when options.highlightSelectedTile is true.
        // Let's assume Tile::draw handles its own selection highlight if currentDrawingOptions.highlightSelectedTile is true.
        // The 'isSelected()' on the Tile itself is now the primary source for Tile::draw.
        // Here, we just ensure the option is passed correctly.
        // This logic might be redundant if Tile::isSelected() is the sole source of truth for selection.
        // However, QGraphicsItem selection is a separate state.
        // For now, we let Tile::draw handle its own selection based on its internal Tile::isSelected()
        // and the options.highlightSelectedTile flag.
        // If we want QGraphicsItem selection to drive Tile's selection appearance:
        if (tile_) { // Ensure tile exists
             currentDrawingOptions.highlightSelectedTile = true; // Tell Tile::draw to highlight
        }
    } else {
         if (tile_) {
             currentDrawingOptions.highlightSelectedTile = false; // Tell Tile::draw NOT to highlight based on this QGraphicsItem state
         }
    }
    // However, the tile_ itself might be marked as selected via its own setSelected method.
    // The Tile::draw method should primarily use tile_->isSelected() and options.highlightSelectedTile.
    // So, we just pass the currentDrawingOptions which might have highlightSelectedTile true by default.

    if (tile_) {
        // If the tile_ itself knows it's selected (e.g. via tile_->isSelected()),
        // and currentDrawingOptions.highlightSelectedTile is true (which it is by default),
        // then Tile::draw should render the highlight.
        // If QGraphicsItem selection state should override tile's own selection state for drawing,
        // then we would do: tile_->setSelected(option->state & QStyle::State_Selected); before drawing.
        // For now, let's assume Tile::draw uses a combination of options.highlightSelectedTile AND tile_->isSelected().
        // The currentDrawingOptions passed to tile_->draw already has its highlightSelectedTile field.
        // We need to ensure that if the QGraphicsItem is selected, the tile knows to draw highlighted.
        // This means the DrawingOptions passed to tile_->draw must reflect this.
        
        // Simpler: Let Tile::draw handle its own selection state.
        // QStyleOptionGraphicsItem's selection state is mainly for the view/scene to draw its own indicators.
        // We can use it to *inform* our custom drawing if needed.
        // If Tile::draw's selection highlight is driven by `options.highlightSelectedTile && tile->isSelected()`,
        // then we need to ensure `tile->isSelected()` is accurate or `options.highlightSelectedTile` from `currentDrawingOptions` is set.
        // The most straightforward is to let Tile::draw handle it based on its own state and the general option.
        // If QStyle::State_Selected is set, we can force the highlight for Tile::draw if desired.
        DrawingOptions finalOptions = drawingOptions_; // Start with the item's base options
        if (option->state & QStyle::State_Selected) {
            finalOptions.highlightSelectedTile = true; // Force highlight if QGraphicsItem is selected
        } else {
            // If not selected by QGraphicsItem, respect the original setting in drawingOptions_
            // (which might be true if the tile is part of a persistent selection independent of view focus)
            finalOptions.highlightSelectedTile = drawingOptions_.highlightSelectedTile && (tile_ ? tile_->isSelected() : false);
        }


        tile_->draw(painter, boundingRect(), finalOptions);
    } else {
        // Draw a placeholder for a null tile
        painter->fillRect(boundingRect(), Qt::magenta);
        painter->setPen(Qt::black);
        painter->drawText(boundingRect(), Qt::AlignCenter, "No Tile Data");
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
        update(); // Repaint with new options
    }
}
