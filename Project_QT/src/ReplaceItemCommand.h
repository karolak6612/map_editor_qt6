#ifndef REPLACEITEMCOMMAND_H
#define REPLACEITEMCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QPoint>
#include <QVariantMap>
#include "MapSearcher.h"

// Forward declarations
class Map;
class Tile;
class Item;
class MapView;

/**
 * @brief Undo/Redo command for item replacement operations (Task 82)
 * 
 * Complete implementation of replace functionality with full undo support:
 * - Replace items by ID with full undo/redo capability
 * - Replace item properties with state preservation
 * - Delete items with restoration capability
 * - Swap items with bidirectional undo
 * - Batch operations with single undo command
 * - Integration with MapView for UI updates
 */

/**
 * @brief Single item replacement data
 */
struct ItemReplacementData {
    QPoint position;
    int floor;
    
    // Original item data
    quint16 originalItemId;
    QString originalItemName;
    QVariantMap originalProperties;
    QVariantMap originalAttributes;
    
    // New item data
    quint16 newItemId;
    QString newItemName;
    QVariantMap newProperties;
    QVariantMap newAttributes;
    
    // Operation type
    enum OperationType {
        REPLACE_ITEM,
        REPLACE_PROPERTIES,
        DELETE_ITEM,
        SWAP_ITEM
    } operationType;
    
    bool wasSuccessful;
    
    ItemReplacementData() 
        : floor(0), originalItemId(0), newItemId(0), 
          operationType(REPLACE_ITEM), wasSuccessful(false) {}
};

/**
 * @brief Main replace item command
 */
class ReplaceItemCommand : public QUndoCommand
{
public:
    explicit ReplaceItemCommand(Map* map, const QString& description = "Replace Items", QUndoCommand* parent = nullptr);
    ~ReplaceItemCommand() override;

    // Single item operations
    void addItemReplacement(const QPoint& position, int floor, quint16 originalId, quint16 newId);
    void addItemDeletion(const QPoint& position, int floor, quint16 itemId);
    void addItemSwap(const QPoint& position, int floor, quint16 itemId1, quint16 itemId2);
    void addPropertyReplacement(const QPoint& position, int floor, quint16 itemId, const QVariantMap& newProperties);
    
    // Batch operations from search results
    void addReplacementsFromResults(const QList<SearchResult>& results, quint16 newItemId);
    void addDeletionsFromResults(const QList<SearchResult>& results);
    void addPropertyReplacementsFromResults(const QList<SearchResult>& results, const QVariantMap& newProperties);
    
    // Replace operation configuration
    void setUpdateBorders(bool update) { updateBorders_ = update; }
    void setPreserveAttributes(bool preserve) { preserveAttributes_ = preserve; }
    void setCreateBackup(bool backup) { createBackup_ = backup; }
    
    // Integration
    void setMapView(MapView* mapView) { mapView_ = mapView; }
    
    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override { return REPLACE_ITEM_COMMAND_ID; }
    bool mergeWith(const QUndoCommand* other) override;
    
    // Status and statistics
    int getReplacementCount() const { return replacements_.size(); }
    int getSuccessfulReplacements() const;
    QStringList getFailedReplacements() const;
    QString getOperationSummary() const;

private:
    // Core operations
    bool executeReplacement(ItemReplacementData& replacement);
    bool undoReplacement(const ItemReplacementData& replacement);
    
    // Item operations
    bool replaceItemOnTile(Tile* tile, quint16 oldItemId, quint16 newItemId, ItemReplacementData& data);
    bool deleteItemFromTile(Tile* tile, quint16 itemId, ItemReplacementData& data);
    bool restoreItemToTile(Tile* tile, const ItemReplacementData& data);
    bool swapItemsOnTile(Tile* tile, quint16 itemId1, quint16 itemId2, ItemReplacementData& data);
    bool replaceItemProperties(Tile* tile, quint16 itemId, const QVariantMap& newProperties, ItemReplacementData& data);
    
    // Data preservation
    void captureOriginalItemData(Item* item, ItemReplacementData& data);
    void captureNewItemData(Item* item, ItemReplacementData& data);
    Item* createItemFromData(const ItemReplacementData& data, bool useOriginal = true);
    
    // Border and attribute handling
    void updateTileBorders(Tile* tile);
    void preserveItemAttributes(Item* oldItem, Item* newItem);
    QVariantMap extractItemAttributes(Item* item);
    void applyItemAttributes(Item* item, const QVariantMap& attributes);
    
    // UI updates
    void updateMapView();
    void refreshTileDisplay(const QPoint& position, int floor);
    
    // Validation
    bool validateReplacement(const ItemReplacementData& replacement);
    bool canReplaceItem(Tile* tile, quint16 oldItemId, quint16 newItemId);
    
    // Error handling
    void recordError(const QString& error, const ItemReplacementData& replacement);
    void clearErrors();

private:
    Map* map_;
    MapView* mapView_;
    
    QList<ItemReplacementData> replacements_;
    
    // Operation settings
    bool updateBorders_;
    bool preserveAttributes_;
    bool createBackup_;
    bool firstExecution_;
    
    // Error tracking
    QStringList errors_;
    QStringList failedReplacements_;
    
    // Constants
    static const int REPLACE_ITEM_COMMAND_ID = 1001;
};

/**
 * @brief Specialized command for batch replace operations
 */
class BatchReplaceCommand : public QUndoCommand
{
public:
    explicit BatchReplaceCommand(Map* map, const ReplaceOperation& operation, const QString& description = "Batch Replace", QUndoCommand* parent = nullptr);
    ~BatchReplaceCommand() override;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override { return BATCH_REPLACE_COMMAND_ID; }
    
    // Status
    int getProcessedCount() const { return processedCount_; }
    int getSuccessfulCount() const { return successfulCount_; }
    QString getOperationSummary() const;

private:
    void executeBatchOperation();
    void undoBatchOperation();
    
    Map* map_;
    ReplaceOperation operation_;
    QList<ReplaceItemCommand*> subCommands_;
    
    int processedCount_;
    int successfulCount_;
    bool firstExecution_;
    
    static const int BATCH_REPLACE_COMMAND_ID = 1002;
};

/**
 * @brief Command for swapping two item types across the entire map
 */
class SwapItemsCommand : public QUndoCommand
{
public:
    explicit SwapItemsCommand(Map* map, quint16 itemId1, quint16 itemId2, const QString& description = "Swap Items", QUndoCommand* parent = nullptr);
    ~SwapItemsCommand() override;

    // Configuration
    void setSearchArea(const QRect& area) { searchArea_ = area; }
    void setLayerFilter(const QStringList& layers) { layerFilter_ = layers; }
    void setUpdateBorders(bool update) { updateBorders_ = update; }
    
    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override { return SWAP_ITEMS_COMMAND_ID; }
    
    // Status
    int getSwappedCount() const { return swappedPositions_.size(); }
    QString getOperationSummary() const;

private:
    void executeSwap();
    void undoSwap();
    
    Map* map_;
    quint16 itemId1_;
    quint16 itemId2_;
    
    QRect searchArea_;
    QStringList layerFilter_;
    bool updateBorders_;
    bool firstExecution_;
    
    // Track swapped positions for undo
    QList<QPoint> swappedPositions_;
    QList<int> swappedFloors_;
    
    static const int SWAP_ITEMS_COMMAND_ID = 1003;
};

/**
 * @brief Command for deleting items based on search criteria
 */
class DeleteItemsCommand : public QUndoCommand
{
public:
    explicit DeleteItemsCommand(Map* map, const SearchCriteria& criteria, const QString& description = "Delete Items", QUndoCommand* parent = nullptr);
    explicit DeleteItemsCommand(Map* map, const QList<SearchResult>& results, const QString& description = "Delete Selected Items", QUndoCommand* parent = nullptr);
    ~DeleteItemsCommand() override;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override { return DELETE_ITEMS_COMMAND_ID; }
    
    // Status
    int getDeletedCount() const { return deletedItems_.size(); }
    QString getOperationSummary() const;

private:
    void executeDelete();
    void undoDelete();
    
    Map* map_;
    SearchCriteria searchCriteria_;
    QList<SearchResult> targetResults_;
    bool useSearchCriteria_;
    bool firstExecution_;
    
    // Store deleted items for restoration
    struct DeletedItemData {
        QPoint position;
        int floor;
        quint16 itemId;
        QVariantMap properties;
        QVariantMap attributes;
        int stackPosition; // Position in tile's item stack
    };
    
    QList<DeletedItemData> deletedItems_;
    
    static const int DELETE_ITEMS_COMMAND_ID = 1004;
};

#endif // REPLACEITEMCOMMAND_H
