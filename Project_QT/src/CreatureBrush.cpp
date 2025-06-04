#include "CreatureBrush.h"
#include "Map.h"
#include "Tile.h"
#include "Creature.h"
#include "PlaceCreatureCommand.h"
#include "RemoveCreatureCommand.h"
#include <QDebug>
#include <QMouseEvent>
#include <QUndoCommand>

CreatureBrush::CreatureBrush(QObject *parent) : Brush(parent) {
}

// Brush type identification
Brush::Type CreatureBrush::type() const {
    return Type::Creature;
}

QString CreatureBrush::name() const {
    return QStringLiteral("Creature Brush");
}

bool CreatureBrush::isCreature() const {
    return true;
}

CreatureBrush* CreatureBrush::asCreature() {
    return this;
}

const CreatureBrush* CreatureBrush::asCreature() const {
    return this;
}

// Brush properties
int CreatureBrush::getBrushSize() const {
    return brushSize_;
}

Brush::BrushShape CreatureBrush::getBrushShape() const {
    return brushShape_;
}

bool CreatureBrush::canDrag() const {
    return true; // Creature brushes can be dragged for multiple placements
}

bool CreatureBrush::canSmear() const {
    return false; // Creatures typically don't smear (one per tile)
}

bool CreatureBrush::oneSizeFitsAll() const {
    return true; // Creatures typically ignore brush size
}

int CreatureBrush::getLookID() const {
    return creatureId_; // Use the creature ID as the look ID
}

// Core brush action interface (stubs for now)
bool CreatureBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(map)
    Q_UNUSED(tilePos)
    Q_UNUSED(drawingContext)
    return creatureId_ != 0; // Can only draw if we have a valid creature ID
}

QUndoCommand* CreatureBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);

    if (!map || !canDraw(map, tilePos, drawingContext)) {
        return nullptr;
    }

    // Create and return a proper undo command for creature placement
    PlaceCreatureCommand* command = new PlaceCreatureCommand(
        map,
        tilePos,
        creatureId_,
        QString("Creature %1").arg(creatureId_),
        parentCommand
    );

    qDebug() << "CreatureBrush::applyBrush creating PlaceCreatureCommand for creature" << creatureId_ << "at" << tilePos;
    return command;
}

QUndoCommand* CreatureBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);

    if (!map) {
        return nullptr;
    }

    // Check if there's a tile at this position
    int x = static_cast<int>(tilePos.x());
    int y = static_cast<int>(tilePos.y());
    int z = map->getCurrentFloor();
    Tile* tile = map->getTile(x, y, z);
    if (!tile) {
        qDebug() << "CreatureBrush::removeBrush - No tile found at" << tilePos;
        return nullptr;
    }

    // Check if there are any creatures to remove
    const QList<Creature*>& creatures = tile->getCreatures();
    if (creatures.isEmpty()) {
        qDebug() << "CreatureBrush::removeBrush - No creatures found at" << tilePos;
        return nullptr;
    }

    // Create and return a proper undo command for creature removal
    RemoveCreatureCommand* command = new RemoveCreatureCommand(
        map,
        tilePos,
        nullptr, // Will find first creature automatically
        parentCommand
    );

    qDebug() << "CreatureBrush::removeBrush creating RemoveCreatureCommand at" << tilePos;
    return command;
}

// Mouse event handlers with proper signatures
QUndoCommand* CreatureBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                            Map* map, QUndoStack* undoStack,
                                            bool shiftPressed, bool ctrlPressed, bool altPressed,
                                            QUndoCommand* parentCommand) {
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    qDebug() << "CreatureBrush::mousePressEvent at" << mapPos 
             << "button:" << event->button()
             << "creatureId:" << creatureId_
             << "shift:" << shiftPressed 
             << "ctrl:" << ctrlPressed 
             << "alt:" << altPressed;
    
    // Placeholder: Identify target tile(s) based on brush size and shape
    qDebug() << "Target tile(s): [" << static_cast<int>(mapPos.x()) << "," << static_cast<int>(mapPos.y()) << "]";
    qDebug() << "Brush size:" << getBrushSize() << "shape:" << static_cast<int>(getBrushShape());
    
    if (ctrlPressed) {
        // Ctrl+click typically removes creatures
        return removeBrush(map, mapPos, nullptr, parentCommand);
    } else {
        // Normal click places creatures
        return applyBrush(map, mapPos, nullptr, parentCommand);
    }
}

QUndoCommand* CreatureBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                           Map* map, QUndoStack* undoStack,
                                           bool shiftPressed, bool ctrlPressed, bool altPressed,
                                           QUndoCommand* parentCommand) {
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    if (event->buttons() != Qt::NoButton && canDrag()) {
        qDebug() << "CreatureBrush::mouseMoveEvent at" << mapPos 
                 << "creatureId:" << creatureId_
                 << "shift:" << shiftPressed 
                 << "ctrl:" << ctrlPressed 
                 << "alt:" << altPressed;
        
        // Placeholder: Dragging creature placement
        qDebug() << "Dragging creature placement at tile: [" << static_cast<int>(mapPos.x()) << "," << static_cast<int>(mapPos.y()) << "]";
        
        if (ctrlPressed) {
            return removeBrush(map, mapPos, nullptr, parentCommand);
        } else {
            return applyBrush(map, mapPos, nullptr, parentCommand);
        }
    }
    
    return nullptr;
}

QUndoCommand* CreatureBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                              Map* map, QUndoStack* undoStack,
                                              bool shiftPressed, bool ctrlPressed, bool altPressed,
                                              QUndoCommand* parentCommand) {
    Q_UNUSED(map)
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    qDebug() << "CreatureBrush::mouseReleaseEvent at" << mapPos 
             << "button:" << event->button()
             << "creatureId:" << creatureId_
             << "shift:" << shiftPressed 
             << "ctrl:" << ctrlPressed 
             << "alt:" << altPressed;
    
    // Placeholder: Finalize creature operation
    qDebug() << "Finalizing creature operation";
    
    return nullptr; // No additional command needed on release for creature brush
}

void CreatureBrush::cancel() {
    qDebug() << "CreatureBrush::cancel - Canceling ongoing creature operation";
    // TODO: Implement cancellation logic when needed
}

// CreatureBrush specific methods
void CreatureBrush::setCreatureId(quint16 creatureId) {
    creatureId_ = creatureId;
    qDebug() << "CreatureBrush::setCreatureId to" << creatureId;
}

quint16 CreatureBrush::getCreatureId() const {
    return creatureId_;
}

// Task 53: Core creature placement functionality
void CreatureBrush::draw(Map* map, Tile* tile, void* parameter) {
    Q_UNUSED(parameter);

    if (!map || !tile || creatureId_ == 0) {
        return;
    }

    // Remove existing creature first
    undraw(map, tile);

    // Create new creature
    Creature* creature = new Creature();
    creature->setLookType(creatureId_);  // Use lookType instead of setId
    creature->setName(QString("Creature %1").arg(creatureId_));

    // Add creature to tile
    tile->addCreature(creature);

    // Mark map as modified
    map->setModified(true);

    qDebug() << "CreatureBrush::draw placed creature" << creatureId_ << "at"
             << tile->x() << "," << tile->y() << "," << tile->z();
}

void CreatureBrush::undraw(Map* map, Tile* tile) {
    if (!map || !tile) {
        return;
    }

    // Remove all creatures from tile
    const QList<Creature*>& creatures = tile->getCreatures();
    for (Creature* creature : creatures) {
        tile->removeCreature(creature);
        delete creature;
    }

    // Mark map as modified
    map->setModified(true);

    qDebug() << "CreatureBrush::undraw removed creatures from"
             << tile->x() << "," << tile->y() << "," << tile->z();
}
