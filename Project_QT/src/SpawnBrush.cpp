#include "SpawnBrush.h"
#include "SpawnCommand.h"
#include "Spawn.h"
#include "Map.h"
#include "Tile.h"
#include <QDebug>
#include <QMouseEvent>
#include <QUndoCommand>

SpawnBrush::SpawnBrush(QObject *parent) : Brush(parent) {
}

// Brush type identification
Brush::Type SpawnBrush::type() const {
    return Type::Spawn;
}

QString SpawnBrush::name() const {
    return QStringLiteral("Spawn Brush");
}

bool SpawnBrush::isSpawn() const {
    return true;
}

SpawnBrush* SpawnBrush::asSpawn() {
    return this;
}

const SpawnBrush* SpawnBrush::asSpawn() const {
    return this;
}

// Brush properties
int SpawnBrush::getBrushSize() const {
    return brushSize_;
}

Brush::BrushShape SpawnBrush::getBrushShape() const {
    return brushShape_;
}

bool SpawnBrush::canDrag() const {
    return true; // Spawn brushes can be dragged for multiple placements
}

bool SpawnBrush::canSmear() const {
    return false; // Spawns typically don't smear (one per tile)
}

bool SpawnBrush::oneSizeFitsAll() const {
    return true; // Spawns typically ignore brush size
}

int SpawnBrush::getLookID() const {
    return 0; // TODO: Define EDITOR_SPRITE_SPAWN constant
}

// Core brush action interface
bool SpawnBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(drawingContext)

    if (!map) {
        return false;
    }

    // Check if position is within map bounds
    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());
    int z = 0; // Assuming ground floor for now

    return map->isValidPosition(x, y, z);
}

QUndoCommand* SpawnBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext)

    if (!canDraw(map, tilePos, drawingContext)) {
        return nullptr;
    }

    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());
    int z = 0; // Assuming ground floor for now
    MapPos position(x, y, z);

    // Check if there's already a spawn at this position
    Tile* tile = map->getTile(x, y, z);
    if (tile && tile->spawn()) {
        qDebug() << "Spawn already exists at" << x << y << z;
        return nullptr; // Don't place spawn if one already exists
    }

    // Create a new spawn with default values
    Spawn* spawn = new Spawn();
    spawn->setPosition(position);
    spawn->setRadius(3);
    spawn->setInterval(10000); // 10 seconds
    spawn->setMaxCreatures(3);
    spawn->setCreatureNames(QStringList() << "rat"); // Default creature

    qDebug() << "SpawnBrush::applyBrush placing spawn at" << x << y << z;
    return new PlaceSpawnCommand(map, position, spawn, parentCommand);
}

QUndoCommand* SpawnBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext)

    if (!map) {
        return nullptr;
    }

    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());
    int z = 0; // Assuming ground floor for now
    MapPos position(x, y, z);

    // Check if there's a spawn to remove
    Tile* tile = map->getTile(x, y, z);
    if (!tile || !tile->spawn()) {
        qDebug() << "No spawn to remove at" << x << y << z;
        return nullptr;
    }

    qDebug() << "SpawnBrush::removeBrush removing spawn at" << x << y << z;
    return new RemoveSpawnCommand(map, position, parentCommand);
}

// Mouse event handlers with proper signatures
QUndoCommand* SpawnBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                         Map* map, QUndoStack* undoStack,
                                         bool shiftPressed, bool ctrlPressed, bool altPressed,
                                         QUndoCommand* parentCommand) {
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    qDebug() << "SpawnBrush::mousePressEvent at" << mapPos 
             << "button:" << event->button()
             << "shift:" << shiftPressed 
             << "ctrl:" << ctrlPressed 
             << "alt:" << altPressed;
    
    // Placeholder: Identify target tile(s) based on brush size and shape
    qDebug() << "Target tile(s): [" << static_cast<int>(mapPos.x()) << "," << static_cast<int>(mapPos.y()) << "]";
    qDebug() << "Brush size:" << getBrushSize() << "shape:" << static_cast<int>(getBrushShape());
    
    if (ctrlPressed) {
        // Ctrl+click typically removes spawns
        return removeBrush(map, mapPos, nullptr, parentCommand);
    } else {
        // Normal click places spawns
        return applyBrush(map, mapPos, nullptr, parentCommand);
    }
}

QUndoCommand* SpawnBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                        Map* map, QUndoStack* undoStack,
                                        bool shiftPressed, bool ctrlPressed, bool altPressed,
                                        QUndoCommand* parentCommand) {
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    if (event->buttons() != Qt::NoButton && canDrag()) {
        qDebug() << "SpawnBrush::mouseMoveEvent at" << mapPos 
                 << "shift:" << shiftPressed 
                 << "ctrl:" << ctrlPressed 
                 << "alt:" << altPressed;
        
        // Placeholder: Dragging spawn placement
        qDebug() << "Dragging spawn placement at tile: [" << static_cast<int>(mapPos.x()) << "," << static_cast<int>(mapPos.y()) << "]";
        
        if (ctrlPressed) {
            return removeBrush(map, mapPos, nullptr, parentCommand);
        } else {
            return applyBrush(map, mapPos, nullptr, parentCommand);
        }
    }
    
    return nullptr;
}

QUndoCommand* SpawnBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                           Map* map, QUndoStack* undoStack,
                                           bool shiftPressed, bool ctrlPressed, bool altPressed,
                                           QUndoCommand* parentCommand) {
    Q_UNUSED(map)
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    qDebug() << "SpawnBrush::mouseReleaseEvent at" << mapPos 
             << "button:" << event->button()
             << "shift:" << shiftPressed 
             << "ctrl:" << ctrlPressed 
             << "alt:" << altPressed;
    
    // Placeholder: Finalize spawn operation
    qDebug() << "Finalizing spawn operation";
    
    return nullptr; // No additional command needed on release for spawn brush
}

void SpawnBrush::cancel() {
    qDebug() << "SpawnBrush::cancel - Canceling ongoing spawn operation";
    // TODO: Implement cancellation logic when needed
}

// Task 53: Core spawn placement functionality
void SpawnBrush::draw(Map* map, Tile* tile, void* parameter) {
    Q_UNUSED(parameter);

    if (!map || !tile) {
        return;
    }

    // Remove existing spawn first
    undraw(map, tile);

    // Create new spawn
    Spawn* spawn = new Spawn();
    spawn->setPosition(MapPos(tile->x(), tile->y(), tile->z()));
    spawn->setRadius(3);
    spawn->setInterval(10000); // 10 seconds
    spawn->setMaxCreatures(3);
    spawn->setCreatureNames(QStringList() << "rat"); // Default creature

    // Set spawn on tile
    tile->setSpawn(spawn);

    // Mark map as modified
    map->setModified(true);

    qDebug() << "SpawnBrush::draw placed spawn at"
             << tile->x() << "," << tile->y() << "," << tile->z();
}

void SpawnBrush::undraw(Map* map, Tile* tile) {
    if (!map || !tile) {
        return;
    }

    // Remove spawn from tile
    Spawn* spawn = tile->spawn();
    if (spawn) {
        tile->setSpawn(nullptr);
        delete spawn;

        // Mark map as modified
        map->setModified(true);

        qDebug() << "SpawnBrush::undraw removed spawn from"
                 << tile->x() << "," << tile->y() << "," << tile->z();
    }
}
