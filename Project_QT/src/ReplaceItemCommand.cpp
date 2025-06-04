#include "ReplaceItemCommand.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "MapView.h"
#include <QDebug>

// ReplaceItemCommand implementation
ReplaceItemCommand::ReplaceItemCommand(Map* map, const QString& description, QUndoCommand* parent)
    : QUndoCommand(description, parent),
      map_(map),
      mapView_(nullptr),
      updateBorders_(true),
      preserveAttributes_(true),
      createBackup_(false),
      firstExecution_(true) {
}

ReplaceItemCommand::~ReplaceItemCommand() {
    // No cleanup needed
}

void ReplaceItemCommand::addItemReplacement(const QPoint& position, int floor, quint16 originalId, quint16 newId) {
    ItemReplacementData replacement;
    replacement.position = position;
    replacement.floor = floor;
    replacement.originalItemId = originalId;
    replacement.newItemId = newId;
    replacement.operationType = ItemReplacementData::REPLACE_ITEM;
    replacements_.append(replacement);
}

void ReplaceItemCommand::addItemDeletion(const QPoint& position, int floor, quint16 itemId) {
    ItemReplacementData replacement;
    replacement.position = position;
    replacement.floor = floor;
    replacement.originalItemId = itemId;
    replacement.newItemId = 0;
    replacement.operationType = ItemReplacementData::DELETE_ITEM;
    replacements_.append(replacement);
}

void ReplaceItemCommand::addItemSwap(const QPoint& position, int floor, quint16 itemId1, quint16 itemId2) {
    ItemReplacementData replacement;
    replacement.position = position;
    replacement.floor = floor;
    replacement.originalItemId = itemId1;
    replacement.newItemId = itemId2;
    replacement.operationType = ItemReplacementData::SWAP_ITEM;
    replacements_.append(replacement);
}

void ReplaceItemCommand::addPropertyReplacement(const QPoint& position, int floor, quint16 itemId, const QVariantMap& newProperties) {
    ItemReplacementData replacement;
    replacement.position = position;
    replacement.floor = floor;
    replacement.originalItemId = itemId;
    replacement.newItemId = itemId;
    replacement.newProperties = newProperties;
    replacement.operationType = ItemReplacementData::REPLACE_PROPERTIES;
    replacements_.append(replacement);
}

void ReplaceItemCommand::addReplacementsFromResults(const QList<SearchResult>& results, quint16 newItemId) {
    for (const SearchResult& result : results) {
        addItemReplacement(result.position, result.floor, result.itemId, newItemId);
    }
}

void ReplaceItemCommand::addDeletionsFromResults(const QList<SearchResult>& results) {
    for (const SearchResult& result : results) {
        addItemDeletion(result.position, result.floor, result.itemId);
    }
}

void ReplaceItemCommand::addPropertyReplacementsFromResults(const QList<SearchResult>& results, const QVariantMap& newProperties) {
    for (const SearchResult& result : results) {
        addPropertyReplacement(result.position, result.floor, result.itemId, newProperties);
    }
}

void ReplaceItemCommand::undo() {
    if (!map_) return;
    
    // Undo in reverse order
    for (int i = replacements_.size() - 1; i >= 0; --i) {
        const ItemReplacementData& replacement = replacements_[i];
        if (replacement.wasSuccessful) {
            undoReplacement(replacement);
        }
    }
    
    updateMapView();
    qDebug() << "ReplaceItemCommand: Undid" << replacements_.size() << "replacements";
}

void ReplaceItemCommand::redo() {
    if (!map_) return;
    
    clearErrors();
    int successCount = 0;
    
    for (ItemReplacementData& replacement : replacements_) {
        if (executeReplacement(replacement)) {
            successCount++;
        }
    }
    
    updateMapView();
    qDebug() << "ReplaceItemCommand: Executed" << successCount << "of" << replacements_.size() << "replacements";
    firstExecution_ = false;
}

bool ReplaceItemCommand::mergeWith(const QUndoCommand* other) {
    const ReplaceItemCommand* otherCommand = dynamic_cast<const ReplaceItemCommand*>(other);
    if (!otherCommand || otherCommand->map_ != map_) {
        return false;
    }
    
    // Merge replacements
    replacements_.append(otherCommand->replacements_);
    return true;
}

int ReplaceItemCommand::getSuccessfulReplacements() const {
    int count = 0;
    for (const ItemReplacementData& replacement : replacements_) {
        if (replacement.wasSuccessful) {
            count++;
        }
    }
    return count;
}

QStringList ReplaceItemCommand::getFailedReplacements() const {
    return failedReplacements_;
}

QString ReplaceItemCommand::getOperationSummary() const {
    return QString("Replaced %1 of %2 items").arg(getSuccessfulReplacements()).arg(replacements_.size());
}

bool ReplaceItemCommand::executeReplacement(ItemReplacementData& replacement) {
    if (!map_) return false;
    
    Tile* tile = map_->getTile(replacement.position.x(), replacement.position.y(), replacement.floor);
    if (!tile) {
        recordError("Tile not found", replacement);
        return false;
    }
    
    bool success = false;
    switch (replacement.operationType) {
        case ItemReplacementData::REPLACE_ITEM:
            success = replaceItemOnTile(tile, replacement.originalItemId, replacement.newItemId, replacement);
            break;
        case ItemReplacementData::DELETE_ITEM:
            success = deleteItemFromTile(tile, replacement.originalItemId, replacement);
            break;
        case ItemReplacementData::SWAP_ITEM:
            success = swapItemsOnTile(tile, replacement.originalItemId, replacement.newItemId, replacement);
            break;
        case ItemReplacementData::REPLACE_PROPERTIES:
            success = replaceItemProperties(tile, replacement.originalItemId, replacement.newProperties, replacement);
            break;
    }
    
    replacement.wasSuccessful = success;
    if (success && updateBorders_) {
        updateTileBorders(tile);
    }
    
    return success;
}

bool ReplaceItemCommand::undoReplacement(const ItemReplacementData& replacement) {
    if (!map_) return false;
    
    Tile* tile = map_->getTile(replacement.position.x(), replacement.position.y(), replacement.floor);
    if (!tile) return false;
    
    return restoreItemToTile(tile, replacement);
}

bool ReplaceItemCommand::replaceItemOnTile(Tile* tile, quint16 oldItemId, quint16 newItemId, ItemReplacementData& data) {
    // TODO: Implement item replacement logic
    qDebug() << "ReplaceItemCommand: Replace item" << oldItemId << "with" << newItemId << "at" << data.position;
    return true; // Placeholder
}

bool ReplaceItemCommand::deleteItemFromTile(Tile* tile, quint16 itemId, ItemReplacementData& data) {
    // TODO: Implement item deletion logic
    qDebug() << "ReplaceItemCommand: Delete item" << itemId << "at" << data.position;
    return true; // Placeholder
}

bool ReplaceItemCommand::restoreItemToTile(Tile* tile, const ItemReplacementData& data) {
    // TODO: Implement item restoration logic
    qDebug() << "ReplaceItemCommand: Restore item at" << data.position;
    return true; // Placeholder
}

bool ReplaceItemCommand::swapItemsOnTile(Tile* tile, quint16 itemId1, quint16 itemId2, ItemReplacementData& data) {
    // TODO: Implement item swapping logic
    qDebug() << "ReplaceItemCommand: Swap items" << itemId1 << "and" << itemId2 << "at" << data.position;
    return true; // Placeholder
}

bool ReplaceItemCommand::replaceItemProperties(Tile* tile, quint16 itemId, const QVariantMap& newProperties, ItemReplacementData& data) {
    // TODO: Implement property replacement logic
    qDebug() << "ReplaceItemCommand: Replace properties for item" << itemId << "at" << data.position;
    return true; // Placeholder
}

void ReplaceItemCommand::captureOriginalItemData(Item* item, ItemReplacementData& data) {
    // TODO: Implement data capture
}

void ReplaceItemCommand::captureNewItemData(Item* item, ItemReplacementData& data) {
    // TODO: Implement data capture
}

Item* ReplaceItemCommand::createItemFromData(const ItemReplacementData& data, bool useOriginal) {
    // TODO: Implement item creation
    return nullptr; // Placeholder
}

void ReplaceItemCommand::updateTileBorders(Tile* tile) {
    // TODO: Implement border updates
}

void ReplaceItemCommand::preserveItemAttributes(Item* oldItem, Item* newItem) {
    // TODO: Implement attribute preservation
}

QVariantMap ReplaceItemCommand::extractItemAttributes(Item* item) {
    // TODO: Implement attribute extraction
    return QVariantMap();
}

void ReplaceItemCommand::applyItemAttributes(Item* item, const QVariantMap& attributes) {
    // TODO: Implement attribute application
}

void ReplaceItemCommand::updateMapView() {
    if (mapView_) {
        mapView_->update();
    }
}

void ReplaceItemCommand::refreshTileDisplay(const QPoint& position, int floor) {
    // TODO: Implement tile refresh
}

bool ReplaceItemCommand::validateReplacement(const ItemReplacementData& replacement) {
    // TODO: Implement validation
    return true;
}

bool ReplaceItemCommand::canReplaceItem(Tile* tile, quint16 oldItemId, quint16 newItemId) {
    // TODO: Implement replacement check
    return true;
}

void ReplaceItemCommand::recordError(const QString& error, const ItemReplacementData& replacement) {
    QString errorMsg = QString("%1 at (%2,%3,%4)").arg(error).arg(replacement.position.x()).arg(replacement.position.y()).arg(replacement.floor);
    errors_.append(errorMsg);
    failedReplacements_.append(errorMsg);
}

void ReplaceItemCommand::clearErrors() {
    errors_.clear();
    failedReplacements_.clear();
}

// BatchReplaceCommand implementation
BatchReplaceCommand::BatchReplaceCommand(Map* map, const ReplaceOperation& operation, const QString& description, QUndoCommand* parent)
    : QUndoCommand(description, parent),
      map_(map),
      operation_(operation),
      processedCount_(0),
      successfulCount_(0),
      firstExecution_(true) {
}

BatchReplaceCommand::~BatchReplaceCommand() {
    qDeleteAll(subCommands_);
}

void BatchReplaceCommand::undo() {
    undoBatchOperation();
}

void BatchReplaceCommand::redo() {
    executeBatchOperation();
}

QString BatchReplaceCommand::getOperationSummary() const {
    return QString("Batch operation: %1 of %2 processed successfully").arg(successfulCount_).arg(processedCount_);
}

void BatchReplaceCommand::executeBatchOperation() {
    // TODO: Implement batch operation
    qDebug() << "BatchReplaceCommand: Execute batch operation";
}

void BatchReplaceCommand::undoBatchOperation() {
    // TODO: Implement batch undo
    qDebug() << "BatchReplaceCommand: Undo batch operation";
}

// SwapItemsCommand implementation
SwapItemsCommand::SwapItemsCommand(Map* map, quint16 itemId1, quint16 itemId2, const QString& description, QUndoCommand* parent)
    : QUndoCommand(description, parent),
      map_(map),
      itemId1_(itemId1),
      itemId2_(itemId2),
      updateBorders_(true),
      firstExecution_(true) {
}

SwapItemsCommand::~SwapItemsCommand() {
    // No cleanup needed
}

void SwapItemsCommand::undo() {
    undoSwap();
}

void SwapItemsCommand::redo() {
    executeSwap();
}

QString SwapItemsCommand::getOperationSummary() const {
    return QString("Swapped %1 items").arg(swappedPositions_.size());
}

void SwapItemsCommand::executeSwap() {
    // TODO: Implement swap execution
    qDebug() << "SwapItemsCommand: Execute swap of items" << itemId1_ << "and" << itemId2_;
}

void SwapItemsCommand::undoSwap() {
    // TODO: Implement swap undo
    qDebug() << "SwapItemsCommand: Undo swap";
}

// DeleteItemsCommand implementation
DeleteItemsCommand::DeleteItemsCommand(Map* map, const SearchCriteria& criteria, const QString& description, QUndoCommand* parent)
    : QUndoCommand(description, parent),
      map_(map),
      searchCriteria_(criteria),
      useSearchCriteria_(true),
      firstExecution_(true) {
}

DeleteItemsCommand::DeleteItemsCommand(Map* map, const QList<SearchResult>& results, const QString& description, QUndoCommand* parent)
    : QUndoCommand(description, parent),
      map_(map),
      targetResults_(results),
      useSearchCriteria_(false),
      firstExecution_(true) {
}

DeleteItemsCommand::~DeleteItemsCommand() {
    // No cleanup needed
}

void DeleteItemsCommand::undo() {
    undoDelete();
}

void DeleteItemsCommand::redo() {
    executeDelete();
}

QString DeleteItemsCommand::getOperationSummary() const {
    return QString("Deleted %1 items").arg(deletedItems_.size());
}

void DeleteItemsCommand::executeDelete() {
    // TODO: Implement delete execution
    qDebug() << "DeleteItemsCommand: Execute delete";
}

void DeleteItemsCommand::undoDelete() {
    // TODO: Implement delete undo
    qDebug() << "DeleteItemsCommand: Undo delete";
}
