#ifndef REPLACEOPERATIONMANAGER_H
#define REPLACEOPERATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QList>
#include <QPoint>
#include <QRect>
#include <QUndoCommand>
#include <QUndoStack>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <functional>

#include "MapSearcher.h"
#include "ui/ProgressDialog.h"

/**
 * @brief Task 89: Replace Operation Manager for complex item replacement operations
 * 
 * Complete replace operation system:
 * - Handles all types of replace operations (ID, properties, delete, swap)
 * - Undo/redo support with QUndoCommand integration
 * - Progress tracking and cancellation support
 * - Batch operations with optimization
 * - Validation and error handling
 * - Integration with MapSearcher and Map systems
 * - 1:1 compatibility with wxwidgets replace functionality
 */

/**
 * @brief Enhanced Replace Operation structure
 */
struct EnhancedReplaceOperation {
    enum OperationType {
        REPLACE_ITEM_ID,        // Replace with different item ID
        REPLACE_PROPERTIES,     // Replace specific properties
        DELETE_ITEMS,           // Delete matching items
        SWAP_ITEMS,            // Swap two item types
        TRANSFORM_ITEMS,       // Apply custom transformation
        BATCH_OPERATIONS       // Multiple operations in sequence
    };
    
    OperationType type = REPLACE_ITEM_ID;
    
    // Source criteria (what to replace)
    SearchCriteria sourceCriteria;
    QList<SearchResult> sourceResults;
    
    // Target parameters (what to replace with)
    quint16 targetItemId = 0;
    QVariantMap targetProperties;
    std::function<void(class Item*, class Tile*)> transformFunction;
    
    // Batch operations
    QList<EnhancedReplaceOperation> batchOperations;
    
    // Replace scope and options
    enum ReplaceScope {
        REPLACE_ALL_MATCHING,     // Replace all items matching criteria
        REPLACE_SELECTED_RESULTS, // Replace only selected search results
        REPLACE_IN_SELECTION,     // Replace only in current map selection
        REPLACE_IN_AREA          // Replace only in specified area
    };
    
    ReplaceScope scope = REPLACE_ALL_MATCHING;
    QRect replaceArea;
    QList<QPoint> specificPositions;
    
    // Operation options
    bool createBackup = true;
    bool updateBorders = true;
    bool preserveAttributes = false;
    bool validateBeforeReplace = true;
    bool skipInvalidItems = true;
    
    // Progress and cancellation
    bool allowCancellation = true;
    int maxOperationsPerUpdate = 100;
    
    EnhancedReplaceOperation() = default;
};

/**
 * @brief Replace Operation Result
 */
struct ReplaceOperationResult {
    bool success = false;
    int totalItemsProcessed = 0;
    int itemsReplaced = 0;
    int itemsSkipped = 0;
    int itemsDeleted = 0;
    QStringList errors;
    QStringList warnings;
    QString summary;
    qint64 operationTime = 0;
    
    ReplaceOperationResult() = default;
    
    QString getDetailedSummary() const;
    bool hasErrors() const { return !errors.isEmpty(); }
    bool hasWarnings() const { return !warnings.isEmpty(); }
};

/**
 * @brief Undo Command for Replace Operations
 */
class ReplaceItemsCommand : public QUndoCommand
{
public:
    explicit ReplaceItemsCommand(const EnhancedReplaceOperation& operation, 
                                class Map* map, QUndoCommand* parent = nullptr);
    ~ReplaceItemsCommand() override = default;

    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private:
    struct TileChange {
        QPoint position;
        int floor;
        class Tile* oldTile;
        class Tile* newTile;
    };
    
    EnhancedReplaceOperation operation_;
    class Map* map_;
    QList<TileChange> tileChanges_;
    bool firstRedo_;
    
    void executeOperation();
    void storeOriginalTiles();
    void applyChanges();
    void revertChanges();
};

/**
 * @brief Replace Operation Manager
 */
class ReplaceOperationManager : public QObject
{
    Q_OBJECT

public:
    explicit ReplaceOperationManager(QObject* parent = nullptr);
    ~ReplaceOperationManager() override;

    // Configuration
    void setMap(class Map* map);
    void setMapSearcher(class MapSearcher* mapSearcher);
    void setUndoStack(QUndoStack* undoStack);

    // Main replace operations
    ReplaceOperationResult replaceItems(const EnhancedReplaceOperation& operation);
    ReplaceOperationResult replaceItemsAsync(const EnhancedReplaceOperation& operation);
    
    // Specialized replace methods
    ReplaceOperationResult replaceItemId(const QList<SearchResult>& results, quint16 newItemId);
    ReplaceOperationResult replaceProperties(const QList<SearchResult>& results, const QVariantMap& properties);
    ReplaceOperationResult deleteItems(const QList<SearchResult>& results);
    ReplaceOperationResult swapItems(quint16 itemId1, quint16 itemId2, const QRect& area = QRect());
    ReplaceOperationResult transformItems(const QList<SearchResult>& results, 
                                         std::function<void(class Item*, class Tile*)> transformation);
    
    // Batch operations
    ReplaceOperationResult executeBatchOperations(const QList<EnhancedReplaceOperation>& operations);
    
    // Validation and preview
    bool validateOperation(const EnhancedReplaceOperation& operation, QStringList& errors) const;
    QList<SearchResult> previewOperation(const EnhancedReplaceOperation& operation) const;
    int estimateOperationTime(const EnhancedReplaceOperation& operation) const;
    
    // Operation control
    void cancelCurrentOperation();
    bool isOperationInProgress() const;
    int getCurrentProgress() const;
    QString getCurrentOperationStatus() const;
    
    // Statistics and history
    QList<ReplaceOperationResult> getOperationHistory() const;
    QVariantMap getOperationStatistics() const;
    void clearOperationHistory();

signals:
    void operationStarted(const EnhancedReplaceOperation& operation);
    void operationProgress(int current, int total, const QString& currentItem);
    void operationCompleted(const ReplaceOperationResult& result);
    void operationCancelled();
    void operationError(const QString& error, const QString& details);
    
    void itemReplaced(const SearchResult& oldResult, const SearchResult& newResult);
    void itemDeleted(const SearchResult& result);
    void batchOperationProgress(int currentOperation, int totalOperations);

private slots:
    void onProgressUpdate();

private:
    // Core operation implementation
    ReplaceOperationResult executeReplaceOperation(const EnhancedReplaceOperation& operation);
    bool replaceItemOnTile(class Tile* tile, class Item* oldItem, quint16 newItemId);
    bool replaceItemProperties(class Item* item, const QVariantMap& properties);
    bool deleteItemFromTile(class Tile* tile, class Item* item);
    bool transformItemOnTile(class Tile* tile, class Item* item, 
                           std::function<void(class Item*, class Tile*)> transformation);
    
    // Validation helpers
    bool isValidItemId(quint16 itemId) const;
    bool isValidProperty(const QString& propertyName, const QVariant& value) const;
    bool canReplaceItem(class Item* item, quint16 newItemId) const;
    bool canDeleteItem(class Item* item) const;
    
    // Progress and cancellation
    void updateProgress(int current, int total, const QString& currentItem);
    void checkCancellation();
    bool shouldCancel() const;
    
    // Optimization
    void optimizeOperation(EnhancedReplaceOperation& operation) const;
    QList<SearchResult> filterValidResults(const QList<SearchResult>& results) const;
    void groupResultsByTile(const QList<SearchResult>& results, 
                           QMap<class Tile*, QList<class Item*>>& groupedResults) const;
    
    // Error handling
    void addError(const QString& error);
    void addWarning(const QString& warning);
    void clearErrors();

private:
    // Core components
    class Map* map_;
    class MapSearcher* mapSearcher_;
    QUndoStack* undoStack_;
    
    // Operation state
    bool operationInProgress_;
    bool operationCancelled_;
    int currentProgress_;
    int maxProgress_;
    QString currentOperationStatus_;
    
    // Current operation
    EnhancedReplaceOperation currentOperation_;
    ReplaceOperationResult currentResult_;
    QStringList currentErrors_;
    QStringList currentWarnings_;
    
    // Progress tracking
    QTimer* progressTimer_;
    ProgressDialog* progressDialog_;
    
    // History and statistics
    QList<ReplaceOperationResult> operationHistory_;
    QVariantMap operationStatistics_;
    
    // Thread safety
    mutable QMutex operationMutex_;
    
    // Constants
    static const int MAX_HISTORY_SIZE = 100;
    static const int PROGRESS_UPDATE_INTERVAL = 100; // milliseconds
};

/**
 * @brief Replace Operation Factory for creating common operations
 */
class ReplaceOperationFactory
{
public:
    // Factory methods for common operations
    static EnhancedReplaceOperation createItemIdReplacement(const QList<SearchResult>& results, quint16 newItemId);
    static EnhancedReplaceOperation createPropertyReplacement(const QList<SearchResult>& results, const QVariantMap& properties);
    static EnhancedReplaceOperation createItemDeletion(const QList<SearchResult>& results);
    static EnhancedReplaceOperation createItemSwap(quint16 itemId1, quint16 itemId2, const QRect& area = QRect());
    static EnhancedReplaceOperation createBatchOperation(const QList<EnhancedReplaceOperation>& operations);
    
    // Specialized operations
    static EnhancedReplaceOperation createDoorReplacement(const QList<SearchResult>& results);
    static EnhancedReplaceOperation createContainerReplacement(const QList<SearchResult>& results, quint16 newContainerId);
    static EnhancedReplaceOperation createGroundReplacement(const QList<SearchResult>& results, quint16 newGroundId);
    
    // Configuration helpers
    static void configureForUndo(EnhancedReplaceOperation& operation);
    static void configureForBatch(EnhancedReplaceOperation& operation);
    static void configureForPreview(EnhancedReplaceOperation& operation);

private:
    ReplaceOperationFactory() = default; // Static factory class
};

/**
 * @brief Replace Operation Validator
 */
class ReplaceOperationValidator
{
public:
    // Validation methods
    static bool validateOperation(const EnhancedReplaceOperation& operation, QStringList& errors);
    static bool validateItemReplacement(quint16 oldItemId, quint16 newItemId, QStringList& errors);
    static bool validatePropertyReplacement(const QVariantMap& properties, QStringList& errors);
    static bool validateScope(const EnhancedReplaceOperation& operation, QStringList& errors);
    
    // Compatibility checks
    static bool areItemsCompatible(quint16 itemId1, quint16 itemId2);
    static bool canReplaceInContext(class Item* item, class Tile* tile, quint16 newItemId);
    static bool willCauseBorderIssues(class Item* item, class Tile* tile, quint16 newItemId);

private:
    ReplaceOperationValidator() = default; // Static utility class
};

#endif // REPLACEOPERATIONMANAGER_H
