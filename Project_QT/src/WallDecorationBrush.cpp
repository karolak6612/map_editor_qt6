#include "WallDecorationBrush.h"
#include "PlaceDecorationCommand.h" // For creating decoration placement commands
#include "Map.h"
#include "Tile.h"
#include "Item.h"                 // For checking item->isWall()
#include "QUndoCommand.h"
#include <QMouseEvent>
#include <QDebug>
#include <QObject>                // For tr()

WallDecorationBrush::WallDecorationBrush(QObject* parent)
    : WallBrush(parent),
      currentDecorationItemId_(0) {
    setSpecificName(tr("Wall Decoration Brush"));
    // Optionally set a default lookId if wall decorations have a generic icon
    // setLookID(SOME_DEFAULT_DECORATION_ICON_ID);
}

WallDecorationBrush::~WallDecorationBrush() {
}

// --- Overrides from Brush Interface ---
Brush::Type WallDecorationBrush::type() const {
    return Brush::Type::WallDecoration;
}

bool WallDecorationBrush::isWallDecoration() const {
    return true;
}

// Core action methods
bool WallDecorationBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(drawingContext);
    if (!map) {
        qWarning() << "WallDecorationBrush::canDraw: Map pointer is null.";
        return false;
    }
    if (currentDecorationItemId_ == 0) {
        qWarning() << "WallDecorationBrush::canDraw: No currentDecorationItemId_ set.";
        return false; // Cannot place if no item is selected for the brush
    }

    Tile* tile = map->getTile(qFloor(tilePos.x()), qFloor(tilePos.y()), qFloor(tilePos.z()));
    if (!tile) {
        // Depending on game logic, decorations might only be placeable on existing tiles.
        // Or, applyBrush could create the tile. For now, assume tile must exist.
        // However, PlaceDecorationCommand uses getOrCreateTile, so this check might be relaxed if applyBrush creates tiles.
        // Let's assume for canDraw, the tile should ideally exist.
        qWarning() << "WallDecorationBrush::canDraw: Tile does not exist at" << tilePos;
        return false;
    }

    // Check if there is a wall item on the tile to attach the decoration to.
    bool wallFound = false;
    for (Item* item : tile->items()) { // Check non-ground items
        if (item && item->isWall()) { // Assumes Item::isWall()
            wallFound = true;
            break;
        }
    }
    // Also check ground item if ground can be a wall structure (e.g. cave walls)
    if (!wallFound && tile->getGround() && tile->getGround()->isWall()) {
        wallFound = true;
    }

    if (!wallFound) {
        qDebug() << "WallDecorationBrush::canDraw: No wall found at" << tilePos << "to place decoration.";
    }
    return wallFound;
}

QUndoCommand* WallDecorationBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    // canDraw checks map and currentDecorationItemId_ indirectly.
    // Re-check currentDecorationItemId_ explicitly before creating command.
    if (!map || currentDecorationItemId_ == 0) {
         qWarning() << "WallDecorationBrush::applyBrush: Map null or no currentDecorationItemId_ set.";
         return nullptr;
    }
    // The canDraw method should ideally be called by the input handler *before* calling applyBrush.
    // If canDraw is false, applyBrush shouldn't be called.
    // However, having a check here too is safer.
    if (!this->canDraw(map, tilePos, drawingContext)) { // Use this-> to call own canDraw
         qWarning() << "WallDecorationBrush::applyBrush: canDraw returned false for" << tilePos;
        return nullptr;
    }

    qDebug() << "WallDecorationBrush: Applying decoration ID" << currentDecorationItemId_ << "at" << tilePos;
    return new PlaceDecorationCommand(map, tilePos, currentDecorationItemId_, parentCommand);
}

QUndoCommand* WallDecorationBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext); Q_UNUSED(parentCommand); // parentCommand might be used with a future RemoveDecorationCommand
    if (!map) {
        qWarning() << "WallDecorationBrush::removeBrush: Map pointer is null.";
        return nullptr;
    }

    if (currentDecorationItemId_ == 0) {
        qWarning() << "WallDecorationBrush::removeBrush: No specific decoration item ID set to target for removal. Implement generic decoration removal or configure brush.";
        // Placeholder: Could try to find *any* decoration item on the tile if currentDecorationItemId_ is 0.
        // This would require Tile to have a way to identify decoration items.
        // For now, if no specific ID is set, do nothing.
        return nullptr;
    }

    qDebug() << "WallDecorationBrush: Attempting to remove decoration ID" << currentDecorationItemId_ << "at" << tilePos;
    // This requires a command that can remove a *specific item instance* or *an item of a specific ID*.
    // PlaceDecorationCommand is for ADDING. Its undo() removes the item IT added.
    // To remove an existing item, we need a different command or an enhanced PlaceDecorationCommand.
    // For example, a conceptual RemoveItemCommand(map, tile, item_id_to_remove, parent).
    // For now, returning nullptr as a proper remove command for decorations isn't defined yet based on current item ID.
    // A simple PlaceDecorationCommand(map, tilePos, 0, parent) would clear ALL decorations if modified to do so,
    // or do nothing if it only places. This is not what we want for targeted removal.
    qWarning() << "WallDecorationBrush::removeBrush: Specific decoration removal command not yet implemented. Returning nullptr.";
    return nullptr;
}

// Inherit getBrushSize and getBrushShape from WallBrush (typically 0 and Square)
// Override if decorations have different default size/shape needs.
// For instance, getBrushSize might always be 0.
int WallDecorationBrush::getBrushSize() const {
    return 0; // Decorations are placed one at a time
}

// Mouse event methods
QUndoCommand* WallDecorationBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                                 Map* map, QUndoStack* undoStack,
                                                 bool shiftPressed, bool ctrlPressed, bool altPressed,
                                                 QUndoCommand* parentCommand) {
    Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    if (ctrlPressed) {
        return removeBrush(map, mapPos, nullptr, parentCommand);
    } else {
        // canDraw checks if currentDecorationItemId_ is set and if a wall exists
        if (!canDraw(map, mapPos, nullptr)) {
            return nullptr;
        }
        return applyBrush(map, mapPos, nullptr, parentCommand);
    }
}

QUndoCommand* WallDecorationBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                             Map* map, QUndoStack* undoStack,
                                             bool shiftPressed, bool ctrlPressed, bool altPressed,
                                             QUndoCommand* parentCommand) {
    Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(altPressed);

    // Wall decorations are typically not "smeared". Action on click usually.
    // If smearing is desired, this logic would be similar to mousePressEvent.
    // Default canSmear() is inherited from WallBrush (false).
    if (canSmear() && (event->buttons() & Qt::LeftButton)) {
        if (ctrlPressed) {
            return removeBrush(map, mapPos, nullptr, parentCommand);
        } else {
            if (!canDraw(map, mapPos, nullptr)) {
                return nullptr;
            }
            return applyBrush(map, mapPos, nullptr, parentCommand);
        }
    }
    return nullptr;
}

QUndoCommand* WallDecorationBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                                Map* map, QUndoStack* undoStack,
                                                bool shiftPressed, bool ctrlPressed, bool altPressed,
                                                QUndoCommand* parentCommand) {
    Q_UNUSED(mapPos); Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(map); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(ctrlPressed); Q_UNUSED(altPressed); Q_UNUSED(parentCommand);
    return nullptr;
}

void WallDecorationBrush::cancel() {
    // Call base class cancel if it has any logic.
    WallBrush::cancel();
    qDebug() << "WallDecorationBrush::cancel called";
    // Reset any internal state specific to WallDecorationBrush if it had a multi-step operation.
}

// --- WallDecorationBrush specific methods ---
void WallDecorationBrush::setCurrentDecorationItemId(quint16 itemId) {
    currentDecorationItemId_ = itemId;
    // Optionally, update lookId_ from TerrainBrush if the brush icon should change:
    // if (itemId != 0) setLookID(itemId);
    // else setLookID(DEFAULT_DECORATION_BRUSH_ICON_ID_OR_SIMILAR);
}

quint16 WallDecorationBrush::getCurrentDecorationItemId() const {
    return currentDecorationItemId_;
}
