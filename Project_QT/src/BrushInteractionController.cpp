// BrushInteractionController.cpp - Implementation of comprehensive brush interaction system

#include "BrushInteractionController.h"
#include "Brush.h"
#include "Map.h"
#include "MapView.h"
#include "BrushManager.h"
#include <QMouseEvent>
#include <QDebug>

// BrushDrawCommand implementation
BrushDrawCommand::BrushDrawCommand(Brush* brush, Map* map, const QList<QPointF>& tiles, 
                                 const QString& description, QUndoCommand* parent)
    : QUndoCommand(description, parent), brush_(brush), map_(map), affectedTiles_(tiles) {
    
    // Capture original tile data for undo
    for (const QPointF& tile : tiles) {
        // This would capture the actual tile data - placeholder implementation
        originalTileData_.append(QVariant(QString("original_data_%1_%2").arg(tile.x()).arg(tile.y())));
        newTileData_.append(QVariant(QString("new_data_%1_%2").arg(tile.x()).arg(tile.y())));
    }
}

void BrushDrawCommand::undo() {
    if (!brush_ || !map_) return;
    
    // Restore original tile data
    for (int i = 0; i < affectedTiles_.size() && i < originalTileData_.size(); ++i) {
        const QPointF& tile = affectedTiles_[i];
        const QVariant& originalData = originalTileData_[i];
        
        // Restore tile to original state
        // map_->setTileData(tile, originalData);
        qDebug() << "BrushDrawCommand::undo: Restoring tile" << tile << "to" << originalData;
    }
}

void BrushDrawCommand::redo() {
    if (!brush_ || !map_) return;
    
    // Apply brush to tiles
    for (int i = 0; i < affectedTiles_.size() && i < newTileData_.size(); ++i) {
        const QPointF& tile = affectedTiles_[i];
        
        // Apply brush effect to tile
        brush_->applyBrush(map_, tile, nullptr);
        qDebug() << "BrushDrawCommand::redo: Drawing on tile" << tile;
    }
}

// BrushEraseCommand implementation
BrushEraseCommand::BrushEraseCommand(Brush* brush, Map* map, const QList<QPointF>& tiles,
                                   const QString& description, QUndoCommand* parent)
    : QUndoCommand(description, parent), brush_(brush), map_(map), affectedTiles_(tiles) {
    
    // Capture original tile data for undo
    for (const QPointF& tile : tiles) {
        originalTileData_.append(QVariant(QString("original_data_%1_%2").arg(tile.x()).arg(tile.y())));
    }
}

void BrushEraseCommand::undo() {
    if (!brush_ || !map_) return;
    
    // Restore original tile data
    for (int i = 0; i < affectedTiles_.size() && i < originalTileData_.size(); ++i) {
        const QPointF& tile = affectedTiles_[i];
        const QVariant& originalData = originalTileData_[i];
        
        // Restore tile to original state
        qDebug() << "BrushEraseCommand::undo: Restoring tile" << tile << "to" << originalData;
    }
}

void BrushEraseCommand::redo() {
    if (!brush_ || !map_) return;
    
    // Erase from tiles
    for (const QPointF& tile : affectedTiles_) {
        brush_->removeBrush(map_, tile, nullptr);
        qDebug() << "BrushEraseCommand::redo: Erasing from tile" << tile;
    }
}

// BrushBatchCommand implementation
BrushBatchCommand::BrushBatchCommand(const QString& description, QUndoCommand* parent)
    : QUndoCommand(description, parent) {
}

void BrushBatchCommand::addDrawCommand(const QList<QPointF>& tiles, Brush* brush, Map* map) {
    if (tiles.isEmpty() || !brush || !map) return;
    
    BrushDrawCommand* drawCmd = new BrushDrawCommand(brush, map, tiles, "Draw", this);
    subCommands_.append(drawCmd);
}

void BrushBatchCommand::addEraseCommand(const QList<QPointF>& tiles, Brush* brush, Map* map) {
    if (tiles.isEmpty() || !brush || !map) return;
    
    BrushEraseCommand* eraseCmd = new BrushEraseCommand(brush, map, tiles, "Erase", this);
    subCommands_.append(eraseCmd);
}

void BrushBatchCommand::undo() {
    // Undo in reverse order
    for (int i = subCommands_.size() - 1; i >= 0; --i) {
        subCommands_[i]->undo();
    }
}

void BrushBatchCommand::redo() {
    // Redo in forward order
    for (QUndoCommand* cmd : subCommands_) {
        cmd->redo();
    }
}

// BrushInteractionController implementation
BrushInteractionController::BrushInteractionController(MapView* mapView, BrushManager* brushManager, 
                                                     Map* map, QUndoStack* undoStack, QObject* parent)
    : QObject(parent), mapView_(mapView), brushManager_(brushManager), map_(map), undoStack_(undoStack),
      currentBatchCommand_(nullptr), visualFeedbackActive_(false), updateAreaDirty_(false) {
    
    // Connect to brush manager signals
    if (brushManager_) {
        connect(brushManager_, &BrushManager::currentBrushChanged, 
                this, &BrushInteractionController::onCurrentBrushChanged);
        connect(brushManager_, &BrushManager::brushPropertiesChanged,
                this, &BrushInteractionController::onBrushPropertiesChanged);
    }
    
    // Connect to undo stack signals
    if (undoStack_) {
        connect(undoStack_, &QUndoStack::indexChanged,
                this, &BrushInteractionController::onUndoStackIndexChanged);
    }
    
    qDebug() << "BrushInteractionController: Initialized";
}

BrushInteractionController::~BrushInteractionController() {
    cancelBatchOperation();
    clearVisualFeedback();
}

QUndoCommand* BrushInteractionController::executeBrushAction(Brush* brush, const QPointF& mapPos, 
                                                           QMouseEvent* event, bool isErase) {
    if (!brush || !map_) return nullptr;
    
    // Validate tile position
    if (!canApplyBrushToTile(brush, mapPos)) {
        qWarning() << "BrushInteractionController::executeBrushAction: Cannot apply brush to tile" << mapPos;
        return nullptr;
    }
    
    // Get affected tiles
    QList<QPointF> tiles;
    tiles.append(mapPos);
    
    // Create and execute command
    QUndoCommand* command = nullptr;
    if (isErase) {
        command = createEraseCommand(brush, tiles);
    } else {
        command = createDrawCommand(brush, tiles);
    }
    
    if (command) {
        if (currentBatchCommand_) {
            // Command is already parented to currentBatchCommand_ in createDrawCommand/createEraseCommand
            // No need to set parent here as it's set in constructor
        } else {
            // Execute immediately
            if (undoStack_) {
                undoStack_->push(command);
            } else {
                command->redo();
                delete command;
                command = nullptr;
            }
        }
        
        // Signal map update
        signalTileUpdate(mapPos);
    }
    
    return command;
}

QUndoCommand* BrushInteractionController::executeBrushArea(Brush* brush, const QList<QPointF>& tiles, 
                                                         QMouseEvent* event, bool isErase) {
    if (!brush || !map_ || tiles.isEmpty()) return nullptr;
    
    // Validate tile positions
    QList<QPointF> validTiles = validateTilePositions(tiles);
    if (validTiles.isEmpty()) {
        qWarning() << "BrushInteractionController::executeBrushArea: No valid tiles";
        return nullptr;
    }
    
    // Create and execute command
    QUndoCommand* command = nullptr;
    if (isErase) {
        command = createEraseCommand(brush, validTiles);
    } else {
        command = createDrawCommand(brush, validTiles);
    }
    
    if (command) {
        if (currentBatchCommand_) {
            // Command is already parented to currentBatchCommand_ in createDrawCommand/createEraseCommand
            // No need to set parent here as it's set in constructor
        } else {
            // Execute immediately
            if (undoStack_) {
                undoStack_->push(command);
            } else {
                command->redo();
                delete command;
                command = nullptr;
            }
        }
        
        // Signal map update for affected area
        QRectF updateArea;
        for (const QPointF& tile : validTiles) {
            if (updateArea.isEmpty()) {
                updateArea = QRectF(tile, QSizeF(1, 1));
            } else {
                updateArea = updateArea.united(QRectF(tile, QSizeF(1, 1)));
            }
        }
        signalMapUpdate(updateArea);
    }
    
    return command;
}

QUndoCommand* BrushInteractionController::executeBrushDrag(Brush* brush, const QPointF& startPos, 
                                                         const QPointF& endPos, QMouseEvent* event) {
    if (!brush || !map_) return nullptr;
    
    // Calculate tiles in drag area based on brush shape
    QList<QPointF> tiles;
    
    if (brush->getBrushShape() == Brush::BrushShape::Circle) {
        // Calculate circle from start to end
        QPointF center = (startPos + endPos) / 2.0;
        qreal radius = QLineF(startPos, endPos).length() / 2.0;
        // tiles = getTilesInCircle(center, qRound(radius)); // Would need access to MapViewInputHandler
    } else {
        // Rectangle from start to end
        // tiles = getTilesInRectangle(startPos, endPos); // Would need access to MapViewInputHandler
    }
    
    // For now, create a simple line of tiles
    tiles.append(startPos);
    tiles.append(endPos);
    
    return executeBrushArea(brush, tiles, event, false);
}

void BrushInteractionController::startBatchOperation(const QString& description) {
    if (currentBatchCommand_) {
        qWarning() << "BrushInteractionController::startBatchOperation: Batch already active";
        return;
    }
    
    currentBatchCommand_ = new BrushBatchCommand(description);
    currentBatchDescription_ = description;
    
    emit batchOperationStarted(description);
    qDebug() << "BrushInteractionController: Started batch operation:" << description;
}

void BrushInteractionController::addToBatch(const QList<QPointF>& tiles, Brush* brush, bool isErase) {
    if (!currentBatchCommand_ || !brush || tiles.isEmpty()) return;
    
    QList<QPointF> validTiles = validateTilePositions(tiles);
    if (validTiles.isEmpty()) return;
    
    if (isErase) {
        currentBatchCommand_->addEraseCommand(validTiles, brush, map_);
    } else {
        currentBatchCommand_->addDrawCommand(validTiles, brush, map_);
    }
}

QUndoCommand* BrushInteractionController::finishBatchOperation() {
    if (!currentBatchCommand_) {
        qWarning() << "BrushInteractionController::finishBatchOperation: No batch active";
        return nullptr;
    }
    
    BrushBatchCommand* batchCommand = currentBatchCommand_;
    currentBatchCommand_ = nullptr;
    
    // Execute the batch command
    if (undoStack_) {
        undoStack_->push(batchCommand);
    } else {
        batchCommand->redo();
        delete batchCommand;
        batchCommand = nullptr;
    }
    
    emit batchOperationFinished();
    qDebug() << "BrushInteractionController: Finished batch operation:" << currentBatchDescription_;
    currentBatchDescription_.clear();
    
    return batchCommand;
}

void BrushInteractionController::cancelBatchOperation() {
    if (!currentBatchCommand_) return;

    delete currentBatchCommand_;
    currentBatchCommand_ = nullptr;

    emit batchOperationCancelled();
    qDebug() << "BrushInteractionController: Cancelled batch operation:" << currentBatchDescription_;
    currentBatchDescription_.clear();
}

// Map and selection update signaling (Task 34 requirement)
void BrushInteractionController::signalMapUpdate(const QRectF& area) {
    if (area.isEmpty()) return;

    // Accumulate update areas for performance
    if (accumulatedUpdateArea_.isEmpty()) {
        accumulatedUpdateArea_ = area;
    } else {
        accumulatedUpdateArea_ = accumulatedUpdateArea_.united(area);
    }
    updateAreaDirty_ = true;

    emit mapAreaChanged(area);

    // Update map display
    updateMapDisplay(area);
}

void BrushInteractionController::signalTileUpdate(const QPointF& tilePos) {
    emit tileChanged(tilePos);
    updateTileDisplay(tilePos);
}

void BrushInteractionController::signalSelectionUpdate() {
    emit selectionChanged();
}

// Visual feedback management
void BrushInteractionController::updateVisualFeedback(const QList<QPointF>& previewTiles) {
    currentPreviewTiles_ = previewTiles;
    visualFeedbackActive_ = !previewTiles.isEmpty();

    emit brushPreviewChanged(previewTiles);
}

void BrushInteractionController::clearVisualFeedback() {
    if (!visualFeedbackActive_) return;

    currentPreviewTiles_.clear();
    visualFeedbackActive_ = false;

    emit brushPreviewCleared();
}

// Brush context management
void BrushInteractionController::setBrushContext(const QString& key, const QVariant& value) {
    brushContext_[key] = value;
}

QVariant BrushInteractionController::getBrushContext(const QString& key) const {
    return brushContext_.value(key);
}

void BrushInteractionController::clearBrushContext() {
    brushContext_.clear();
}

// Slots
void BrushInteractionController::onCurrentBrushChanged(Brush* newBrush, Brush* previousBrush) {
    Q_UNUSED(previousBrush);

    // Clear any active visual feedback when brush changes
    clearVisualFeedback();

    // Cancel any active batch operation
    if (currentBatchCommand_) {
        cancelBatchOperation();
    }

    qDebug() << "BrushInteractionController: Current brush changed to"
             << (newBrush ? newBrush->name() : "None");
}

void BrushInteractionController::onBrushPropertiesChanged(Brush* brush) {
    if (!brush) return;

    qDebug() << "BrushInteractionController: Brush properties changed for" << brush->name();

    // Update visual feedback if active
    if (visualFeedbackActive_ && !currentPreviewTiles_.isEmpty()) {
        // Recalculate preview tiles based on new brush properties
        // This would require access to the current mouse position
        updateVisualFeedback(currentPreviewTiles_);
    }
}

void BrushInteractionController::onMapChanged() {
    // Clear any cached data when map changes
    clearVisualFeedback();
    clearBrushContext();

    qDebug() << "BrushInteractionController: Map changed";
}

void BrushInteractionController::onMapTileChanged(const QPointF& tilePos) {
    signalTileUpdate(tilePos);
}

void BrushInteractionController::onUndoStackIndexChanged(int index) {
    Q_UNUSED(index);

    // Update map display when undo/redo occurs
    if (mapView_) {
        mapView_->update();
    }
}

// Helper methods
QList<QPointF> BrushInteractionController::validateTilePositions(const QList<QPointF>& tiles) const {
    QList<QPointF> validTiles;

    for (const QPointF& tile : tiles) {
        if (tile.x() >= 0 && tile.y() >= 0) { // Basic validation
            validTiles.append(tile);
        }
    }

    return validTiles;
}

bool BrushInteractionController::canApplyBrushToTile(Brush* brush, const QPointF& tilePos) const {
    if (!brush || !map_) return false;

    // Basic validation - would be expanded based on actual map and brush requirements
    return (tilePos.x() >= 0 && tilePos.y() >= 0);
}

QVariant BrushInteractionController::captureTileData(const QPointF& tilePos) const {
    if (!map_) return QVariant();

    // Capture current tile data for undo - placeholder implementation
    return QVariant(QString("tile_data_%1_%2").arg(tilePos.x()).arg(tilePos.y()));
}

void BrushInteractionController::restoreTileData(const QPointF& tilePos, const QVariant& data) {
    if (!map_) return;

    // Restore tile data - placeholder implementation
    qDebug() << "BrushInteractionController: Restoring tile" << tilePos << "to" << data;
}

QUndoCommand* BrushInteractionController::createDrawCommand(Brush* brush, const QList<QPointF>& tiles) {
    if (!brush || tiles.isEmpty()) return nullptr;

    return new BrushDrawCommand(brush, map_, tiles,
                               QString("Draw with %1").arg(brush->name()),
                               currentBatchCommand_);
}

QUndoCommand* BrushInteractionController::createEraseCommand(Brush* brush, const QList<QPointF>& tiles) {
    if (!brush || tiles.isEmpty()) return nullptr;

    return new BrushEraseCommand(brush, map_, tiles,
                                QString("Erase with %1").arg(brush->name()),
                                currentBatchCommand_);
}

void BrushInteractionController::updateMapDisplay(const QRectF& area) {
    if (!mapView_) return;

    // Update the map view for the specified area
    mapView_->update();
}

void BrushInteractionController::updateTileDisplay(const QPointF& tilePos) {
    if (!mapView_) return;

    // Update the map view for the specific tile
    mapView_->update();
}
