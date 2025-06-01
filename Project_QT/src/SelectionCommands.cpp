#include "SelectionCommands.h"
#include "Map.h"
#include "Selection.h"
#include "Tile.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
#include "ClipboardManager.h"
#include "ClipboardData.h"
#include "SettingsManager.h"
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <algorithm>

// --- CopySelectionCommand Implementation ---

CopySelectionCommand::CopySelectionCommand(Map* map, Selection* selection, 
                                          ClipboardManager* clipboardManager,
                                          QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , selection_(selection)
    , clipboardManager_(clipboardManager)
    , executed_(false)
    , successful_(false)
    , tileCount_(0)
    , itemCount_(0)
{
    if (selection_) {
        copiedSelection_ = selection_->getSelectedTiles();
        tileCount_ = copiedSelection_.size();
    }
    
    setText(QString("Copy %1 tiles").arg(tileCount_));
}

CopySelectionCommand::~CopySelectionCommand() = default;

void CopySelectionCommand::redo() {
    if (executed_ || !map_ || !selection_ || !clipboardManager_) {
        return;
    }
    
    try {
        // Copy selection to clipboard
        successful_ = clipboardManager_->copySelection(copiedSelection_, *map_);
        
        if (successful_) {
            // Count items for status reporting
            itemCount_ = 0;
            for (const MapPos& pos : copiedSelection_) {
                Tile* tile = map_->getTile(pos.x, pos.y, pos.z);
                if (tile) {
                    itemCount_ += tile->getItemCount();
                    if (tile->getCreature()) itemCount_++;
                    if (tile->getSpawn()) itemCount_++;
                }
            }
            
            qDebug() << "CopySelectionCommand: Copied" << tileCount_ << "tiles with" << itemCount_ << "items";
        }
        
        executed_ = true;
    } catch (const std::exception& e) {
        qWarning() << "CopySelectionCommand failed:" << e.what();
        successful_ = false;
        executed_ = true;
    }
}

void CopySelectionCommand::undo() {
    // Copy operations don't need undo - they don't modify the map
    // The clipboard state is managed by ClipboardManager
}

int CopySelectionCommand::getTileCount() const {
    return tileCount_;
}

int CopySelectionCommand::getItemCount() const {
    return itemCount_;
}

bool CopySelectionCommand::wasSuccessful() const {
    return successful_;
}

// --- CutSelectionCommand Implementation ---

CutSelectionCommand::CutSelectionCommand(Map* map, Selection* selection, 
                                        ClipboardManager* clipboardManager,
                                        QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , selection_(selection)
    , clipboardManager_(clipboardManager)
    , executed_(false)
    , successful_(false)
    , tileCount_(0)
    , itemCount_(0)
{
    if (selection_) {
        cutSelection_ = selection_->getSelectedTiles();
        tileCount_ = cutSelection_.size();
    }
    
    setText(QString("Cut %1 tiles").arg(tileCount_));
}

CutSelectionCommand::~CutSelectionCommand() {
    // Clean up stored tiles
    for (auto it = originalTiles_.begin(); it != originalTiles_.end(); ++it) {
        delete it.value();
    }
    originalTiles_.clear();
}

void CutSelectionCommand::redo() {
    if (executed_ || !map_ || !selection_ || !clipboardManager_) {
        return;
    }
    
    try {
        // Store original tile data for undo
        storeTileData();
        
        // Copy selection to clipboard first
        successful_ = clipboardManager_->copySelection(cutSelection_, *map_);
        
        if (successful_) {
            // Mark clipboard data as cut operation
            ClipboardData& clipboardData = clipboardManager_->getClipboardData();
            clipboardData.setCutOperation(true);
            
            // Clear selected items from map
            clearSelectedTiles();
            
            // Clear the selection
            if (selection_) {
                selection_->clear();
            }
            
            qDebug() << "CutSelectionCommand: Cut" << tileCount_ << "tiles with" << itemCount_ << "items";
        }
        
        executed_ = true;
    } catch (const std::exception& e) {
        qWarning() << "CutSelectionCommand failed:" << e.what();
        successful_ = false;
        executed_ = true;
    }
}

void CutSelectionCommand::undo() {
    if (!executed_ || !successful_) {
        return;
    }
    
    try {
        // Restore original tile data
        restoreTileData();
        
        // Restore selection
        if (selection_) {
            selection_->clear();
            for (const MapPos& pos : cutSelection_) {
                selection_->addTile(pos);
            }
        }
        
        qDebug() << "CutSelectionCommand: Undone cut operation";
    } catch (const std::exception& e) {
        qWarning() << "CutSelectionCommand undo failed:" << e.what();
    }
}

void CutSelectionCommand::storeTileData() {
    // Clear existing data
    for (auto it = originalTiles_.begin(); it != originalTiles_.end(); ++it) {
        delete it.value();
    }
    originalTiles_.clear();
    
    itemCount_ = 0;
    
    // Store original tiles
    for (const MapPos& pos : cutSelection_) {
        Tile* tile = map_->getTile(pos.x, pos.y, pos.z);
        if (tile) {
            originalTiles_[pos] = tile->deepCopy();
            itemCount_ += tile->getItemCount();
            if (tile->getCreature()) itemCount_++;
            if (tile->getSpawn()) itemCount_++;
        }
    }
}

void CutSelectionCommand::restoreTileData() {
    for (auto it = originalTiles_.begin(); it != originalTiles_.end(); ++it) {
        const MapPos& pos = it.key();
        Tile* originalTile = it.value();
        
        if (originalTile) {
            // Create a copy of the original tile and place it back
            Tile* restoredTile = originalTile->deepCopy();
            map_->setTile(pos.x, pos.y, pos.z, restoredTile);
        }
    }
}

void CutSelectionCommand::clearSelectedTiles() {
    for (const MapPos& pos : cutSelection_) {
        Tile* tile = map_->getTile(pos.x, pos.y, pos.z);
        if (tile) {
            // Clear all selected items from the tile
            tile->clearSelectedItems();
            
            // If tile is empty after clearing, remove it completely
            if (tile->isEmpty()) {
                map_->removeTile(pos.x, pos.y, pos.z);
            }
        }
    }
}

int CutSelectionCommand::getTileCount() const {
    return tileCount_;
}

int CutSelectionCommand::getItemCount() const {
    return itemCount_;
}

bool CutSelectionCommand::wasSuccessful() const {
    return successful_;
}

// --- PasteSelectionCommand Implementation ---

PasteSelectionCommand::PasteSelectionCommand(Map* map, Selection* selection,
                                            ClipboardManager* clipboardManager,
                                            const MapPos& targetPosition,
                                            PasteMode mode,
                                            QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , selection_(selection)
    , clipboardManager_(clipboardManager)
    , targetPosition_(targetPosition)
    , pasteMode_(mode)
    , executed_(false)
    , successful_(false)
    , tileCount_(0)
    , itemCount_(0)
    , automagicEnabled_(false)
{
    if (clipboardManager_) {
        const ClipboardData& clipboardData = clipboardManager_->getClipboardData();
        tileCount_ = clipboardData.getTileCount();
    }
    
    // Check if automagic is enabled
    SettingsManager* settings = SettingsManager::getInstance();
    if (settings) {
        automagicEnabled_ = settings->getBool("automagic/enabled", true) &&
                           settings->getBool("automagic/borderize_paste", true);
    }
    
    setText(QString("Paste %1 tiles").arg(tileCount_));
}

PasteSelectionCommand::~PasteSelectionCommand() {
    // Clean up stored tiles
    for (auto it = originalTiles_.begin(); it != originalTiles_.end(); ++it) {
        delete it.value();
    }
    originalTiles_.clear();
    
    for (auto it = pastedTiles_.begin(); it != pastedTiles_.end(); ++it) {
        delete it.value();
    }
    pastedTiles_.clear();
}

void PasteSelectionCommand::redo() {
    if (executed_ || !map_ || !clipboardManager_) {
        return;
    }
    
    try {
        // Store original tile data for undo
        storeTileData();
        
        // Apply paste data
        applyPasteData();
        
        // Update selection to show pasted area
        updateSelection();
        
        // Apply automagic bordering if enabled
        if (automagicEnabled_) {
            triggerAutomagicBordering();
        }
        
        successful_ = true;
        executed_ = true;
        
        qDebug() << "PasteSelectionCommand: Pasted" << tileCount_ << "tiles with" << itemCount_ << "items";
    } catch (const std::exception& e) {
        qWarning() << "PasteSelectionCommand failed:" << e.what();
        successful_ = false;
        executed_ = true;
    }
}

void PasteSelectionCommand::undo() {
    if (!executed_ || !successful_) {
        return;
    }
    
    try {
        // Restore original tile data
        restoreTileData();
        
        // Clear selection
        if (selection_) {
            selection_->clear();
        }
        
        qDebug() << "PasteSelectionCommand: Undone paste operation";
    } catch (const std::exception& e) {
        qWarning() << "PasteSelectionCommand undo failed:" << e.what();
    }
}

void PasteSelectionCommand::storeTileData() {
    // This will be implemented to store affected tiles before pasting
    // For now, we'll implement a basic version
    qDebug() << "PasteSelectionCommand::storeTileData - storing affected tiles";
}

void PasteSelectionCommand::restoreTileData() {
    // This will restore the original tiles
    qDebug() << "PasteSelectionCommand::restoreTileData - restoring original tiles";
}

void PasteSelectionCommand::applyPasteData() {
    // This will apply the clipboard data to the map
    qDebug() << "PasteSelectionCommand::applyPasteData - applying paste data";
}

void PasteSelectionCommand::updateSelection() {
    // Update selection to show pasted area
    if (selection_) {
        selection_->clear();
        for (const MapPos& pos : affectedPositions_) {
            selection_->addTile(pos);
        }
    }
}

void PasteSelectionCommand::triggerAutomagicBordering() {
    // Apply automagic bordering to affected tiles
    qDebug() << "PasteSelectionCommand::triggerAutomagicBordering - applying borders";
}

int PasteSelectionCommand::getTileCount() const {
    return tileCount_;
}

int PasteSelectionCommand::getItemCount() const {
    return itemCount_;
}

bool PasteSelectionCommand::wasSuccessful() const {
    return successful_;
}

QSet<MapPos> PasteSelectionCommand::getAffectedPositions() const {
    return affectedPositions_;
}

// --- DeleteSelectionCommand Implementation ---

DeleteSelectionCommand::DeleteSelectionCommand(Map* map, Selection* selection,
                                               QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , selection_(selection)
    , executed_(false)
    , successful_(false)
    , tileCount_(0)
    , itemCount_(0)
    , automagicEnabled_(false)
{
    if (selection_) {
        deletedSelection_ = selection_->getSelectedTiles();
        tileCount_ = deletedSelection_.size();
    }

    // Check if automagic is enabled
    SettingsManager* settings = SettingsManager::getInstance();
    if (settings) {
        automagicEnabled_ = settings->getBool("automagic/enabled", true);
    }

    setText(QString("Delete %1 tiles").arg(tileCount_));
}

DeleteSelectionCommand::~DeleteSelectionCommand() {
    // Clean up stored tiles
    for (auto it = originalTiles_.begin(); it != originalTiles_.end(); ++it) {
        delete it.value();
    }
    originalTiles_.clear();
}

void DeleteSelectionCommand::redo() {
    if (executed_ || !map_ || !selection_) {
        return;
    }

    try {
        // Store original tile data for undo
        storeTileData();

        // Delete selected items
        deleteSelectedItems();

        // Clear the selection
        selection_->clear();

        // Apply automagic bordering if enabled
        if (automagicEnabled_) {
            triggerAutomagicBordering();
        }

        successful_ = true;
        executed_ = true;

        qDebug() << "DeleteSelectionCommand: Deleted" << tileCount_ << "tiles with" << itemCount_ << "items";
    } catch (const std::exception& e) {
        qWarning() << "DeleteSelectionCommand failed:" << e.what();
        successful_ = false;
        executed_ = true;
    }
}

void DeleteSelectionCommand::undo() {
    if (!executed_ || !successful_) {
        return;
    }

    try {
        // Restore original tile data
        restoreTileData();

        // Restore selection
        if (selection_) {
            selection_->clear();
            for (const MapPos& pos : deletedSelection_) {
                selection_->addTile(pos);
            }
        }

        qDebug() << "DeleteSelectionCommand: Undone delete operation";
    } catch (const std::exception& e) {
        qWarning() << "DeleteSelectionCommand undo failed:" << e.what();
    }
}

void DeleteSelectionCommand::storeTileData() {
    // Clear existing data
    for (auto it = originalTiles_.begin(); it != originalTiles_.end(); ++it) {
        delete it.value();
    }
    originalTiles_.clear();

    itemCount_ = 0;

    // Store original tiles
    for (const MapPos& pos : deletedSelection_) {
        Tile* tile = map_->getTile(pos.x, pos.y, pos.z);
        if (tile) {
            originalTiles_[pos] = tile->deepCopy();
            itemCount_ += tile->getItemCount();
            if (tile->getCreature()) itemCount_++;
            if (tile->getSpawn()) itemCount_++;
        }
    }
}

void DeleteSelectionCommand::restoreTileData() {
    for (auto it = originalTiles_.begin(); it != originalTiles_.end(); ++it) {
        const MapPos& pos = it.key();
        Tile* originalTile = it.value();

        if (originalTile) {
            // Create a copy of the original tile and place it back
            Tile* restoredTile = originalTile->deepCopy();
            map_->setTile(pos.x, pos.y, pos.z, restoredTile);
        }
    }
}

void DeleteSelectionCommand::deleteSelectedItems() {
    for (const MapPos& pos : deletedSelection_) {
        Tile* tile = map_->getTile(pos.x, pos.y, pos.z);
        if (tile) {
            // Clear all selected items from the tile
            tile->clearSelectedItems();

            // If tile is empty after clearing, remove it completely
            if (tile->isEmpty()) {
                map_->removeTile(pos.x, pos.y, pos.z);
            }
        }
    }
}

void DeleteSelectionCommand::triggerAutomagicBordering() {
    // Apply automagic bordering to surrounding tiles
    qDebug() << "DeleteSelectionCommand::triggerAutomagicBordering - applying borders";
}

int DeleteSelectionCommand::getTileCount() const {
    return tileCount_;
}

int DeleteSelectionCommand::getItemCount() const {
    return itemCount_;
}

bool DeleteSelectionCommand::wasSuccessful() const {
    return successful_;
}

// --- SelectRectangleCommand Implementation ---

SelectRectangleCommand::SelectRectangleCommand(Map* map, Selection* selection,
                                              const MapPos& startPos, const MapPos& endPos,
                                              bool addToSelection,
                                              QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , selection_(selection)
    , startPosition_(startPos)
    , endPosition_(endPos)
    , addToSelection_(addToSelection)
    , executed_(false)
    , successful_(false)
    , tileCount_(0)
{
    calculateSelectionArea();
    setText(QString("Select %1 tiles").arg(tileCount_));
}

SelectRectangleCommand::~SelectRectangleCommand() = default;

void SelectRectangleCommand::redo() {
    if (executed_ || !map_ || !selection_) {
        return;
    }

    try {
        // Store original selection for undo
        storeOriginalSelection();

        // Apply rectangle selection
        applyRectangleSelection();

        successful_ = true;
        executed_ = true;

        qDebug() << "SelectRectangleCommand: Selected" << tileCount_ << "tiles in rectangle";
    } catch (const std::exception& e) {
        qWarning() << "SelectRectangleCommand failed:" << e.what();
        successful_ = false;
        executed_ = true;
    }
}

void SelectRectangleCommand::undo() {
    if (!executed_ || !successful_) {
        return;
    }

    try {
        // Restore original selection
        restoreOriginalSelection();

        qDebug() << "SelectRectangleCommand: Undone rectangle selection";
    } catch (const std::exception& e) {
        qWarning() << "SelectRectangleCommand undo failed:" << e.what();
    }
}

void SelectRectangleCommand::calculateSelectionArea() {
    rectangleSelection_.clear();
    tileCount_ = 0;

    int minX = qMin(startPosition_.x, endPosition_.x);
    int maxX = qMax(startPosition_.x, endPosition_.x);
    int minY = qMin(startPosition_.y, endPosition_.y);
    int maxY = qMax(startPosition_.y, endPosition_.y);
    int minZ = qMin(startPosition_.z, endPosition_.z);
    int maxZ = qMax(startPosition_.z, endPosition_.z);

    for (int z = minZ; z <= maxZ; ++z) {
        for (int x = minX; x <= maxX; ++x) {
            for (int y = minY; y <= maxY; ++y) {
                MapPos pos(x, y, z);
                rectangleSelection_.insert(pos);
                tileCount_++;
            }
        }
    }
}

void SelectRectangleCommand::storeOriginalSelection() {
    if (selection_) {
        originalSelection_ = selection_->getSelectedTiles();
    }
}

void SelectRectangleCommand::restoreOriginalSelection() {
    if (selection_) {
        selection_->clear();
        for (const MapPos& pos : originalSelection_) {
            selection_->addTile(pos);
        }
    }
}

void SelectRectangleCommand::applyRectangleSelection() {
    if (!selection_) {
        return;
    }

    if (!addToSelection_) {
        selection_->clear();
    }

    for (const MapPos& pos : rectangleSelection_) {
        selection_->addTile(pos);
    }
}

int SelectRectangleCommand::getTileCount() const {
    return tileCount_;
}

bool SelectRectangleCommand::wasSuccessful() const {
    return successful_;
}
