#include "OptionalBorderBrush.h"
#include "Tile.h"
#include "Map.h"
#include "Item.h" // Required for Tile::getGround() returning Item*
#include "GroundBrush.h" // For dynamic_cast and GroundBrush::hasOptionalBorder()
// #include "SetTileStateCommand.h" // Assuming a generic command for tile state flags
#include "QUndoCommand.h" // For QUndoCommand return type, and SetTileStateCommand might inherit it
// #include "EditorSprites.h"   // Assuming EDITOR_SPRITE_OPTIONAL_BORDER_TOOL is here
#include <QMouseEvent>
#include <QDebug>
#include <QObject> // For tr() if used, though m_name is hardcoded here

// Placeholder for EditorSprites if not available
#ifndef EDITORSPRITES_H // Simple guard
class EditorSprites {
public:
    static bool getSpriteId(const QString& name, int& id) {
        Q_UNUSED(name);
        id = 0; // Default
        return false; // Simulate not found
    }
    // Example constant if used directly:
    // static const int EDITOR_SPRITE_OPTIONAL_BORDER_TOOL = 123;
};
#endif

// Placeholder for SetTileStateCommand if not available
#ifndef SETTILESTATECOMMAND_H // Simple guard
class SetTileStateCommand : public QUndoCommand {
public:
    SetTileStateCommand(Tile* tile, Tile::TileStateFlag flag, bool value,
                        const QString& text = "Set Tile State", QUndoCommand* parent = nullptr)
        : QUndoCommand(text, parent), m_tile(tile), m_flag(flag), m_newValue(value), m_oldValue(false) {
        if (m_tile) {
            m_oldValue = m_tile->hasStateFlag(m_flag);
        }
    }
    void undo() override {
        if (m_tile) {
            m_tile->setStateFlag(m_flag, m_oldValue);
            // m_tile->update(); // Or rely on signals from setStateFlag
        }
    }
    void redo() override {
        if (m_tile) {
            m_tile->setStateFlag(m_flag, m_newValue);
            // m_tile->update(); // Or rely on signals from setStateFlag
        }
    }
private:
    Tile* m_tile;
    Tile::TileStateFlag m_flag;
    bool m_newValue;
    bool m_oldValue;
};
#endif


OptionalBorderBrush::OptionalBorderBrush(QObject *parent)
    : Brush(parent) {
    m_name = QObject::tr("Optional Border Tool"); // Use QObject::tr for potential localization
    if (!EditorSprites::getSpriteId("optional_border_tool", m_look_id)) {
        m_look_id = 0;
        // qWarning() << "OptionalBorderBrush: Look ID for 'optional_border_tool' not found in EditorSprites. Using default 0.";
    }
    // Or: m_look_id = EditorSprites::EDITOR_SPRITE_OPTIONAL_BORDER_TOOL;
}

OptionalBorderBrush::~OptionalBorderBrush() {}

Brush::Type OptionalBorderBrush::type() const {
    return Brush::Type::OptionalBorder;
}

QString OptionalBorderBrush::name() const {
    return m_name;
}

int OptionalBorderBrush::getLookID() const {
    return m_look_id;
}

bool OptionalBorderBrush::canDrag() const {
    return true;
}

bool OptionalBorderBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(drawingContext);
    if (!map) return false;

    Tile* tile = map->getTile(tilePos.toPoint());
    if (!tile) return false;

    Item* groundItem = tile->getGround();
    if (groundItem) {
        // Attempt to get the GroundBrush associated with this ground item.
        // This assumes that Item::getBrush() returns the brush that placed/manages it,
        // or that ItemTypeData for this ground item has a brush pointer.
        Brush* brush = groundItem->getBrush();
        GroundBrush* gb = dynamic_cast<GroundBrush*>(brush);
        if (gb && gb->hasOptionalBorder()) {
            return false; // Cannot draw if tile's own ground defines optional border
        }
    }

    // Can draw if any of its 8 neighbors has a ground brush that hasOptionalBorder().
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;

            Tile* neighborTile = map->getTile(tilePos.toPoint() + QPoint(dx, dy)); // Use QPoint for offset
            if (neighborTile) {
                Item* neighborGroundItem = neighborTile->getGround();
                if (neighborGroundItem) {
                    Brush* neighborBrush = neighborGroundItem->getBrush();
                    GroundBrush* neighbor_gb = dynamic_cast<GroundBrush*>(neighborBrush);
                    if (neighbor_gb && neighbor_gb->hasOptionalBorder()) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

QUndoCommand* OptionalBorderBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    if (!map) return nullptr;
    Tile* tile = map->getTile(tilePos.toPoint());
    if (!tile) return nullptr;

    if (tile->hasSetOptionalBorder()) {
        return nullptr;
    }
    return new SetTileStateCommand(tile, Tile::TileStateFlag::OptionalBorder, true, QObject::tr("Apply Optional Border"), parentCommand);
}

QUndoCommand* OptionalBorderBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);
    if (!map) return nullptr;
    Tile* tile = map->getTile(tilePos.toPoint());
    if (!tile) return nullptr;

    if (!tile->hasSetOptionalBorder()) {
        return nullptr;
    }
    return new SetTileStateCommand(tile, Tile::TileStateFlag::OptionalBorder, false, QObject::tr("Remove Optional Border"), parentCommand);
}

QUndoCommand* OptionalBorderBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                                 Map* map, QUndoStack* undoStack,
                                                 bool shiftPressed, bool ctrlPressed, bool altPressed,
                                                 QUndoCommand* parentCommand) {
    Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(altPressed);

    if (!map) return nullptr;
    Tile* tile = map->getTile(mapPos.toPoint());
    if (!tile) return nullptr;

    if (event->button() == Qt::LeftButton) {
        if (shiftPressed || ctrlPressed) {
            if (tile->hasSetOptionalBorder()) {
                 return removeBrush(map, mapPos, nullptr, parentCommand);
            }
        } else {
            if (canDraw(map, mapPos, nullptr)) {
                 if (!tile->hasSetOptionalBorder()){
                    return applyBrush(map, mapPos, nullptr, parentCommand);
                 }
            }
        }
    }
    return nullptr;
}

QUndoCommand* OptionalBorderBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                                Map* map, QUndoStack* undoStack,
                                                bool shiftPressed, bool ctrlPressed, bool altPressed,
                                                QUndoCommand* parentCommand) {
    Q_UNUSED(mapView); Q_UNUSED(undoStack);
    Q_UNUSED(altPressed);

    if (!map) return nullptr;
    Tile* tile = map->getTile(mapPos.toPoint());
    if (!tile) return nullptr;

    if (event->buttons() & Qt::LeftButton) {
        if (canDrag()) {
            if (shiftPressed || ctrlPressed) {
                if (tile->hasSetOptionalBorder()) {
                    return removeBrush(map, mapPos, nullptr, parentCommand);
                }
            } else {
                 if (canDraw(map, mapPos, nullptr)) {
                    if (!tile->hasSetOptionalBorder()){
                        return applyBrush(map, mapPos, nullptr, parentCommand);
                    }
                 }
            }
        }
    }
    return nullptr;
}

QUndoCommand* OptionalBorderBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                                   Map* map, QUndoStack* undoStack,
                                                   bool shiftPressed, bool ctrlPressed, bool altPressed,
                                                   QUndoCommand* parentCommand) {
    Q_UNUSED(mapPos); Q_UNUSED(event); Q_UNUSED(mapView); Q_UNUSED(map); Q_UNUSED(undoStack);
    Q_UNUSED(shiftPressed); Q_UNUSED(ctrlPressed); Q_UNUSED(altPressed); Q_UNUSED(parentCommand);
    return nullptr;
}

void OptionalBorderBrush::cancel() {
    qDebug() << "OptionalBorderBrush::cancel called";
}

int OptionalBorderBrush::getBrushSize() const {
    return 0;
}

Brush::BrushShape OptionalBorderBrush::getBrushShape() const {
    return Brush::BrushShape::Square;
}

// Type identification (already in .h, but good to ensure consistency)
bool OptionalBorderBrush::isOptionalBorder() const { return true; }
OptionalBorderBrush* OptionalBorderBrush::asOptionalBorder() { return this; }
const OptionalBorderBrush* OptionalBorderBrush::asOptionalBorder() const { return this; }

```
