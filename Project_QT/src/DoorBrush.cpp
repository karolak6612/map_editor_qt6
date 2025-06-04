#include "DoorBrush.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "PlaceDoorCommand.h"  // This includes both PlaceDoorCommand and RemoveDoorCommand
#include "MapView.h"           // For MapView* parameter in mouse events
#include <QDebug>
#include <QMouseEvent>
#include <QUndoCommand>
#include <QUndoStack>

DoorBrush::DoorBrush(DoorType doorType, QObject *parent) : Brush(parent), doorType_(doorType) {
}

// Brush type identification
Brush::Type DoorBrush::type() const {
    return Type::Door;
}

QString DoorBrush::name() const {
    // Return appropriate name based on door type
    switch (doorType_) {
        case DoorType::Normal: return QStringLiteral("Normal Door");
        case DoorType::Locked: return QStringLiteral("Locked Door");
        case DoorType::Magic: return QStringLiteral("Magic Door");
        case DoorType::Quest: return QStringLiteral("Quest Door");
        case DoorType::Hatch: return QStringLiteral("Hatch");
        case DoorType::Archway: return QStringLiteral("Archway");
        case DoorType::NormalAlt: return QStringLiteral("Normal Door (Alt)");
        case DoorType::Window: return QStringLiteral("Window");
        default: return QStringLiteral("Door Brush");
    }
}

bool DoorBrush::isDoor() const {
    return true;
}

DoorBrush* DoorBrush::asDoor() {
    return this;
}

const DoorBrush* DoorBrush::asDoor() const {
    return this;
}

// Brush properties
int DoorBrush::getBrushSize() const {
    return brushSize_;
}

Brush::BrushShape DoorBrush::getBrushShape() const {
    return brushShape_;
}

bool DoorBrush::canDrag() const {
    return false; // Door brushes typically don't drag in wxWidgets
}

bool DoorBrush::canSmear() const {
    return false; // Door brushes typically don't smear
}

bool DoorBrush::oneSizeFitsAll() const {
    return true; // Matching wxWidgets DoorBrush behavior
}

int DoorBrush::getLookID() const {
    // Return appropriate sprite ID based on door type
    switch (doorType_) {
        case DoorType::Normal: return 1; // TODO: Define proper sprite constants
        case DoorType::Locked: return 2;
        case DoorType::Magic: return 3;
        case DoorType::Quest: return 4;
        case DoorType::Hatch: return 5;
        case DoorType::Archway: return 6;
        case DoorType::NormalAlt: return 7;
        case DoorType::Window: return 8;
        default: return 0;
    }
}

// Core brush action interface (stubs for now)
bool DoorBrush::canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext) const {
    Q_UNUSED(map)
    Q_UNUSED(tilePos)
    Q_UNUSED(drawingContext)
    // TODO: Check if tile has walls (doors can only be placed on walls)
    return true; // Placeholder
}

QUndoCommand* DoorBrush::applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);

    if (!map || !canDraw(map, tilePos, drawingContext)) {
        return nullptr;
    }

    // Get door item ID based on door type
    quint16 doorItemId = getDoorItemId();
    if (doorItemId == 0) {
        qWarning() << "DoorBrush::applyBrush: No door item ID configured for door type" << static_cast<int>(doorType_);
        return nullptr;
    }

    // Convert DoorBrush::DoorType to PlaceDoorCommand::DoorType
    PlaceDoorCommand::DoorType commandDoorType = static_cast<PlaceDoorCommand::DoorType>(doorType_);

    // Create and return door placement command
    PlaceDoorCommand* command = new PlaceDoorCommand(
        map,
        tilePos,
        doorItemId,
        commandDoorType,
        0, // doorId - could be configured later
        parentCommand
    );

    qDebug() << "DoorBrush::applyBrush creating PlaceDoorCommand for door type" << static_cast<int>(doorType_) << "at" << tilePos;
    return command;
}

QUndoCommand* DoorBrush::removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext, QUndoCommand* parentCommand) {
    Q_UNUSED(drawingContext);

    if (!map) {
        return nullptr;
    }

    // Check if there's a tile at this position
    Tile* tile = map->getTile(tilePos);
    if (!tile) {
        qDebug() << "DoorBrush::removeBrush - No tile found at" << tilePos;
        return nullptr;
    }

    // Check if there are any doors to remove
    const QVector<Item*>& items = tile->getItems();
    Item* doorToRemove = nullptr;
    for (Item* item : items) {
        if (item && item->isDoor()) {
            doorToRemove = item;
            break;
        }
    }

    if (!doorToRemove) {
        qDebug() << "DoorBrush::removeBrush - No doors found at" << tilePos;
        return nullptr;
    }

    // Create and return door removal command
    RemoveDoorCommand* command = new RemoveDoorCommand(
        map,
        tilePos,
        doorToRemove,
        parentCommand
    );

    qDebug() << "DoorBrush::removeBrush creating RemoveDoorCommand at" << tilePos;
    return command;
}

// Mouse event handlers with proper signatures
QUndoCommand* DoorBrush::mousePressEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                        Map* map, QUndoStack* undoStack,
                                        bool shiftPressed, bool ctrlPressed, bool altPressed,
                                        QUndoCommand* parentCommand) {
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    qDebug() << "DoorBrush::mousePressEvent at" << mapPos 
             << "button:" << event->button()
             << "doorType:" << static_cast<int>(doorType_)
             << "shift:" << shiftPressed 
             << "ctrl:" << ctrlPressed 
             << "alt:" << altPressed;
    
    // Placeholder: Identify target tile(s) based on brush size and shape
    qDebug() << "Target tile(s): [" << static_cast<int>(mapPos.x()) << "," << static_cast<int>(mapPos.y()) << "]";
    qDebug() << "Brush size:" << getBrushSize() << "shape:" << static_cast<int>(getBrushShape());
    
    if (ctrlPressed) {
        // Ctrl+click typically removes doors
        return removeBrush(map, mapPos, nullptr, parentCommand);
    } else {
        // Normal click places doors
        // altPressed parameter could be used to control door open/closed state
        return applyBrush(map, mapPos, nullptr, parentCommand);
    }
}

QUndoCommand* DoorBrush::mouseMoveEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                       Map* map, QUndoStack* undoStack,
                                       bool shiftPressed, bool ctrlPressed, bool altPressed,
                                       QUndoCommand* parentCommand) {
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    Q_UNUSED(event)
    Q_UNUSED(mapPos)
    Q_UNUSED(map)
    Q_UNUSED(shiftPressed)
    Q_UNUSED(ctrlPressed)
    Q_UNUSED(altPressed)
    
    // Door brushes typically don't respond to mouse move events
    return nullptr;
}

QUndoCommand* DoorBrush::mouseReleaseEvent(const QPointF& mapPos, QMouseEvent* event, MapView* mapView,
                                          Map* map, QUndoStack* undoStack,
                                          bool shiftPressed, bool ctrlPressed, bool altPressed,
                                          QUndoCommand* parentCommand) {
    Q_UNUSED(map)
    Q_UNUSED(mapView)
    Q_UNUSED(undoStack)
    Q_UNUSED(parentCommand)
    
    qDebug() << "DoorBrush::mouseReleaseEvent at" << mapPos 
             << "button:" << event->button()
             << "doorType:" << static_cast<int>(doorType_)
             << "shift:" << shiftPressed 
             << "ctrl:" << ctrlPressed 
             << "alt:" << altPressed;
    
    // Placeholder: Finalize door operation
    qDebug() << "Finalizing door operation";
    
    return nullptr; // No additional command needed on release for door brush
}

void DoorBrush::cancel() {
    qDebug() << "DoorBrush::cancel - Canceling ongoing door operation";
    // TODO: Implement cancellation logic when needed
}

// DoorBrush specific methods
DoorType DoorBrush::getDoorType() const {
    return doorType_;
}

void DoorBrush::setDoorType(DoorType doorType) {
    doorType_ = doorType;
    qDebug() << "DoorBrush::setDoorType to" << static_cast<int>(doorType);
}

quint16 DoorBrush::getDoorItemId() const {
    // Map door types to item IDs (these would typically come from item database)
    // These are placeholder values - in a real implementation, these would be loaded from data files
    switch (doorType_) {
        case DoorType::Normal:
            return 1209; // Normal door item ID
        case DoorType::Locked:
            return 1210; // Locked door item ID
        case DoorType::Magic:
            return 1211; // Magic door item ID
        case DoorType::Quest:
            return 1212; // Quest door item ID
        case DoorType::Hatch:
            return 1213; // Hatch item ID
        case DoorType::Archway:
            return 1214; // Archway item ID
        case DoorType::NormalAlt:
            return 1215; // Alternative normal door item ID
        case DoorType::Window:
            return 1216; // Window item ID
        default:
            return 1209; // Default to normal door
    }
}

// Static utility method (matching wxWidgets)
void DoorBrush::switchDoor(void* door) {
    Q_UNUSED(door)
    qDebug() << "DoorBrush::switchDoor (stub implementation)";
    // TODO: Implement door switching logic when Item class is available
}
