#ifndef SELECTIONCOMMANDS_H
#define SELECTIONCOMMANDS_H

#include <QUndoCommand>
#include <QVariant>
#include <QString>
#include <QMap>
#include <QSet>
#include <QList>
#include <QPointF>

#include "MapPos.h"
#include "selection/SelectionTransformationEngine.h"

// Forward declarations
class Map;
class Selection;
class Tile;
class Item;

/**
 * @brief Task 95: Selection Commands for undo/redo functionality
 * 
 * Complete undo/redo system for selection operations:
 * - Selection change commands (add, remove, replace)
 * - Transformation commands (move, rotate, flip, scale)
 * - Cut/copy/paste commands with proper data handling
 * - Batch operation commands for performance
 * - Map integration for automatic updates
 * - Efficient memory usage and command merging
 */

/**
 * @brief Change Selection Command
 */
class ChangeSelectionCommand : public QUndoCommand
{
public:
    explicit ChangeSelectionCommand(Selection* selection, const QSet<MapPos>& newSelection,
                                   const QSet<MapPos>& oldSelection, QUndoCommand* parent = nullptr);
    ~ChangeSelectionCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Selection* getSelection() const { return selection_; }
    QSet<MapPos> getOldSelection() const { return oldSelection_; }
    QSet<MapPos> getNewSelection() const { return newSelection_; }

private:
    Selection* selection_;
    QSet<MapPos> oldSelection_;
    QSet<MapPos> newSelection_;
    
    static const int COMMAND_ID = 3001;
};

/**
 * @brief Transform Selection Command
 */
class TransformSelectionCommand : public QUndoCommand
{
public:
    explicit TransformSelectionCommand(Map* map, Selection* selection,
                                      const TransformationParameters& params,
                                      QUndoCommand* parent = nullptr);
    ~TransformSelectionCommand() override;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    Map* getMap() const { return map_; }
    Selection* getSelection() const { return selection_; }
    TransformationParameters getParameters() const { return parameters_; }
    TransformationResult getResult() const { return result_; }

private:
    void executeTransformation(bool forward);
    void restoreOriginalState();
    void applyTransformationResult();

private:
    Map* map_;
    Selection* selection_;
    TransformationParameters parameters_;
    TransformationResult result_;
    QMap<MapPos, QVariantMap> originalTileStates_;
    QSet<MapPos> originalSelection_;
    QSet<MapPos> transformedSelection_;
    bool transformationExecuted_;
    
    static const int COMMAND_ID = 3002;
};

/**
 * @brief Move Selection Command
 */
class MoveSelectionCommand : public QUndoCommand
{
public:
    explicit MoveSelectionCommand(Map* map, Selection* selection,
                                 const QPointF& offset, bool cutAndPaste = true,
                                 QUndoCommand* parent = nullptr);
    ~MoveSelectionCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    QPointF getOffset() const { return offset_; }
    bool isCutAndPaste() const { return cutAndPaste_; }

private:
    void executeMoveOperation(bool forward);
    void cutSelection();
    void pasteSelection();
    void moveInPlace();

private:
    Map* map_;
    Selection* selection_;
    QPointF offset_;
    bool cutAndPaste_;
    QMap<MapPos, QVariantMap> cutTileData_;
    QSet<MapPos> originalPositions_;
    QSet<MapPos> newPositions_;
    bool moveExecuted_;
    
    static const int COMMAND_ID = 3003;
};

/**
 * @brief Rotate Selection Command
 */
class RotateSelectionCommand : public QUndoCommand
{
public:
    explicit RotateSelectionCommand(Map* map, Selection* selection,
                                   double degrees, const MapPos& pivot,
                                   QUndoCommand* parent = nullptr);
    ~RotateSelectionCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    double getDegrees() const { return degrees_; }
    MapPos getPivot() const { return pivot_; }

private:
    void executeRotation(bool forward);
    void rotateItems(bool forward);
    void updateItemOrientations(bool forward);

private:
    Map* map_;
    Selection* selection_;
    double degrees_;
    MapPos pivot_;
    QMap<MapPos, QVariantMap> originalTileStates_;
    QMap<Item*, int> originalItemOrientations_;
    QMap<Item*, quint16> originalItemTypes_;
    bool rotationExecuted_;
    
    static const int COMMAND_ID = 3004;
};

/**
 * @brief Flip Selection Command
 */
class FlipSelectionCommand : public QUndoCommand
{
public:
    explicit FlipSelectionCommand(Map* map, Selection* selection,
                                 bool horizontal, bool vertical, const MapPos& pivot,
                                 QUndoCommand* parent = nullptr);
    ~FlipSelectionCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    bool isHorizontal() const { return horizontal_; }
    bool isVertical() const { return vertical_; }
    MapPos getPivot() const { return pivot_; }

private:
    void executeFlip(bool forward);
    void flipItems(bool forward);
    void updateItemTypes(bool forward);

private:
    Map* map_;
    Selection* selection_;
    bool horizontal_;
    bool vertical_;
    MapPos pivot_;
    QMap<MapPos, QVariantMap> originalTileStates_;
    QMap<Item*, quint16> originalItemTypes_;
    bool flipExecuted_;
    
    static const int COMMAND_ID = 3005;
};

/**
 * @brief Cut Selection Command
 */
class CutSelectionCommand : public QUndoCommand
{
public:
    explicit CutSelectionCommand(Map* map, Selection* selection,
                                QUndoCommand* parent = nullptr);
    ~CutSelectionCommand() override;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    QMap<MapPos, QVariantMap> getCutData() const { return cutData_; }

private:
    void executeCut(bool forward);
    void storeTileData();
    void restoreTileData();
    void clearTiles();

private:
    Map* map_;
    Selection* selection_;
    QMap<MapPos, QVariantMap> cutData_;
    QSet<MapPos> cutPositions_;
    bool cutExecuted_;
    
    static const int COMMAND_ID = 3006;
};

/**
 * @brief Paste Selection Command
 */
class PasteSelectionCommand : public QUndoCommand
{
public:
    explicit PasteSelectionCommand(Map* map, const QMap<MapPos, QVariantMap>& pasteData,
                                  const MapPos& targetPosition, bool merge = false,
                                  QUndoCommand* parent = nullptr);
    ~PasteSelectionCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    QMap<MapPos, QVariantMap> getPasteData() const { return pasteData_; }
    MapPos getTargetPosition() const { return targetPosition_; }
    bool isMerge() const { return merge_; }

private:
    void executePaste(bool forward);
    void storePreviousData();
    void restorePreviousData();
    void applyPasteData();

private:
    Map* map_;
    QMap<MapPos, QVariantMap> pasteData_;
    QMap<MapPos, QVariantMap> previousData_;
    MapPos targetPosition_;
    QSet<MapPos> affectedPositions_;
    bool merge_;
    bool pasteExecuted_;
    
    static const int COMMAND_ID = 3007;
};

/**
 * @brief Duplicate Selection Command
 */
class DuplicateSelectionCommand : public QUndoCommand
{
public:
    explicit DuplicateSelectionCommand(Map* map, Selection* selection,
                                      const QPointF& offset,
                                      QUndoCommand* parent = nullptr);
    ~DuplicateSelectionCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    QPointF getOffset() const { return offset_; }

private:
    void executeDuplicate(bool forward);
    void copySelectionData();
    void pasteAtOffset();
    void removeDuplicates();

private:
    Map* map_;
    Selection* selection_;
    QPointF offset_;
    QMap<MapPos, QVariantMap> duplicateData_;
    QSet<MapPos> originalPositions_;
    QSet<MapPos> duplicatePositions_;
    bool duplicateExecuted_;
    
    static const int COMMAND_ID = 3008;
};

/**
 * @brief Batch Selection Operations Command
 */
class BatchSelectionOperationsCommand : public QUndoCommand
{
public:
    struct SelectionOperation {
        enum Type {
            CHANGE_SELECTION,
            TRANSFORM_SELECTION,
            MOVE_SELECTION,
            ROTATE_SELECTION,
            FLIP_SELECTION,
            CUT_SELECTION,
            PASTE_SELECTION,
            DUPLICATE_SELECTION
        };
        
        Type type;
        QVariantMap parameters;
        
        SelectionOperation(Type t = CHANGE_SELECTION) : type(t) {}
    };

    explicit BatchSelectionOperationsCommand(Map* map, Selection* selection,
                                            const QList<SelectionOperation>& operations,
                                            QUndoCommand* parent = nullptr);
    ~BatchSelectionOperationsCommand() override = default;

    // QUndoCommand interface
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

    // Property access
    QList<SelectionOperation> getOperations() const { return operations_; }
    int getOperationCount() const { return operations_.size(); }

private:
    void executeOperations(bool forward);
    void executeOperation(const SelectionOperation& operation, bool forward);
    void storeOperationState();
    void restoreOperationState();

private:
    Map* map_;
    Selection* selection_;
    QList<SelectionOperation> operations_;
    QList<QVariantMap> operationStates_;
    bool operationsExecuted_;
    
    static const int COMMAND_ID = 3009;
};

/**
 * @brief Selection Command Factory
 */
class SelectionCommandFactory
{
public:
    // Selection commands
    static ChangeSelectionCommand* createChangeSelectionCommand(Selection* selection,
                                                               const QSet<MapPos>& newSelection,
                                                               const QSet<MapPos>& oldSelection);
    
    // Transformation commands
    static TransformSelectionCommand* createTransformSelectionCommand(Map* map, Selection* selection,
                                                                     const TransformationParameters& params);
    static MoveSelectionCommand* createMoveSelectionCommand(Map* map, Selection* selection,
                                                           const QPointF& offset, bool cutAndPaste = true);
    static RotateSelectionCommand* createRotateSelectionCommand(Map* map, Selection* selection,
                                                               double degrees, const MapPos& pivot);
    static FlipSelectionCommand* createFlipSelectionCommand(Map* map, Selection* selection,
                                                           bool horizontal, bool vertical, const MapPos& pivot);
    
    // Cut/copy/paste commands
    static CutSelectionCommand* createCutSelectionCommand(Map* map, Selection* selection);
    static PasteSelectionCommand* createPasteSelectionCommand(Map* map, const QMap<MapPos, QVariantMap>& pasteData,
                                                             const MapPos& targetPosition, bool merge = false);
    static DuplicateSelectionCommand* createDuplicateSelectionCommand(Map* map, Selection* selection,
                                                                     const QPointF& offset);
    
    // Batch commands
    static BatchSelectionOperationsCommand* createBatchSelectionCommand(Map* map, Selection* selection,
                                                                       const QList<BatchSelectionOperationsCommand::SelectionOperation>& operations);

private:
    SelectionCommandFactory() = default;
};

#endif // SELECTIONCOMMANDS_H
