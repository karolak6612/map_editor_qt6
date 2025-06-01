#ifndef SELECTIONCOMMANDS_H
#define SELECTIONCOMMANDS_H

#include <QUndoCommand>
#include <QSet>
#include <QMap>
#include <QPointF>
#include "Position.h"
#include "ClipboardData.h"

// Forward declarations
class Map;
class Selection;
class Tile;
class Item;
class Creature;
class Spawn;
class ClipboardManager;

/**
 * @brief Enhanced selection commands for Task 79
 * 
 * Complete implementation of all selection tool features including:
 * - Copy/Cut/Paste operations with full undo support
 * - Selection transformation (move, rotate, flip)
 * - Delete selection with undo
 * - Visual updates and signal integration
 */

/**
 * @brief Command for copying selection to clipboard
 */
class CopySelectionCommand : public QUndoCommand
{
public:
    explicit CopySelectionCommand(Map* map, Selection* selection, 
                                 ClipboardManager* clipboardManager,
                                 QUndoCommand* parent = nullptr);
    ~CopySelectionCommand() override;

    void redo() override;
    void undo() override;

    // Information methods
    int getTileCount() const;
    int getItemCount() const;
    bool wasSuccessful() const;

private:
    Map* map_;
    Selection* selection_;
    ClipboardManager* clipboardManager_;
    QSet<MapPos> copiedSelection_;
    bool executed_;
    bool successful_;
    int tileCount_;
    int itemCount_;
};

/**
 * @brief Command for cutting selection to clipboard (copy + delete)
 */
class CutSelectionCommand : public QUndoCommand
{
public:
    explicit CutSelectionCommand(Map* map, Selection* selection, 
                                ClipboardManager* clipboardManager,
                                QUndoCommand* parent = nullptr);
    ~CutSelectionCommand() override;

    void redo() override;
    void undo() override;

    // Information methods
    int getTileCount() const;
    int getItemCount() const;
    bool wasSuccessful() const;

private:
    void storeTileData();
    void restoreTileData();
    void clearSelectedTiles();
    
    Map* map_;
    Selection* selection_;
    ClipboardManager* clipboardManager_;
    QSet<MapPos> cutSelection_;
    QMap<MapPos, Tile*> originalTiles_;
    bool executed_;
    bool successful_;
    int tileCount_;
    int itemCount_;
};

/**
 * @brief Command for pasting clipboard data to map
 */
class PasteSelectionCommand : public QUndoCommand
{
public:
    enum PasteMode {
        REPLACE_MODE,   // Replace existing tiles completely
        MERGE_MODE,     // Merge with existing tiles
        OVERLAY_MODE    // Add on top of existing tiles
    };

    explicit PasteSelectionCommand(Map* map, Selection* selection,
                                  ClipboardManager* clipboardManager,
                                  const MapPos& targetPosition,
                                  PasteMode mode = REPLACE_MODE,
                                  QUndoCommand* parent = nullptr);
    ~PasteSelectionCommand() override;

    void redo() override;
    void undo() override;

    // Information methods
    int getTileCount() const;
    int getItemCount() const;
    bool wasSuccessful() const;
    QSet<MapPos> getAffectedPositions() const;

private:
    void storeTileData();
    void restoreTileData();
    void applyPasteData();
    void updateSelection();
    void triggerAutomagicBordering();
    
    Map* map_;
    Selection* selection_;
    ClipboardManager* clipboardManager_;
    MapPos targetPosition_;
    PasteMode pasteMode_;
    QSet<MapPos> affectedPositions_;
    QMap<MapPos, Tile*> originalTiles_;
    QMap<MapPos, Tile*> pastedTiles_;
    bool executed_;
    bool successful_;
    int tileCount_;
    int itemCount_;
    bool automagicEnabled_;
};

/**
 * @brief Command for deleting selection
 */
class DeleteSelectionCommand : public QUndoCommand
{
public:
    explicit DeleteSelectionCommand(Map* map, Selection* selection,
                                   QUndoCommand* parent = nullptr);
    ~DeleteSelectionCommand() override;

    void redo() override;
    void undo() override;

    // Information methods
    int getTileCount() const;
    int getItemCount() const;
    bool wasSuccessful() const;

private:
    void storeTileData();
    void restoreTileData();
    void deleteSelectedItems();
    void triggerAutomagicBordering();
    
    Map* map_;
    Selection* selection_;
    QSet<MapPos> deletedSelection_;
    QMap<MapPos, Tile*> originalTiles_;
    bool executed_;
    bool successful_;
    int tileCount_;
    int itemCount_;
    bool automagicEnabled_;
};

/**
 * @brief Enhanced transformation command for selections
 */
class SelectionTransformCommand : public QUndoCommand
{
public:
    enum TransformationType {
        MOVE_TRANSFORM,
        ROTATE_TRANSFORM,
        FLIP_HORIZONTAL_TRANSFORM,
        FLIP_VERTICAL_TRANSFORM
    };

    explicit SelectionTransformCommand(Map* map, Selection* selection,
                                      TransformationType type,
                                      const QVariant& parameters,
                                      QUndoCommand* parent = nullptr);
    ~SelectionTransformCommand() override;

    void redo() override;
    void undo() override;

    // Information methods
    int getTileCount() const;
    int getItemCount() const;
    bool wasSuccessful() const;
    QSet<MapPos> getAffectedPositions() const;

private:
    void storeTileData();
    void restoreTileData();
    void executeTransformation();
    void reverseTransformation();
    void updateSelection();
    void triggerAutomagicBordering();
    
    // Transformation methods
    void executeMoveTransform(const QPointF& offset);
    void executeRotateTransform(int degrees);
    void executeFlipHorizontalTransform();
    void executeFlipVerticalTransform();
    
    // Helper methods
    MapPos transformPosition(const MapPos& pos) const;
    void transformItem(Item* item) const;
    void transformCreature(Creature* creature) const;
    void transformSpawn(Spawn* spawn) const;
    
    Map* map_;
    Selection* selection_;
    TransformationType transformationType_;
    QVariant transformParameters_;
    QSet<MapPos> originalSelection_;
    QSet<MapPos> transformedSelection_;
    QMap<MapPos, Tile*> originalTiles_;
    QMap<MapPos, Tile*> transformedTiles_;
    bool executed_;
    bool successful_;
    int tileCount_;
    int itemCount_;
    bool automagicEnabled_;
};

/**
 * @brief Command for selecting rectangular area
 */
class SelectRectangleCommand : public QUndoCommand
{
public:
    explicit SelectRectangleCommand(Map* map, Selection* selection,
                                   const MapPos& startPos, const MapPos& endPos,
                                   bool addToSelection = false,
                                   QUndoCommand* parent = nullptr);
    ~SelectRectangleCommand() override;

    void redo() override;
    void undo() override;

    // Information methods
    int getTileCount() const;
    bool wasSuccessful() const;

private:
    void calculateSelectionArea();
    void storeOriginalSelection();
    void restoreOriginalSelection();
    void applyRectangleSelection();
    
    Map* map_;
    Selection* selection_;
    MapPos startPosition_;
    MapPos endPosition_;
    bool addToSelection_;
    QSet<MapPos> originalSelection_;
    QSet<MapPos> rectangleSelection_;
    bool executed_;
    bool successful_;
    int tileCount_;
};

/**
 * @brief Command for selecting single tile/item
 */
class SelectSingleCommand : public QUndoCommand
{
public:
    explicit SelectSingleCommand(Map* map, Selection* selection,
                                const MapPos& position,
                                Item* specificItem = nullptr,
                                bool addToSelection = false,
                                QUndoCommand* parent = nullptr);
    ~SelectSingleCommand() override;

    void redo() override;
    void undo() override;

    // Information methods
    bool wasSuccessful() const;

private:
    void storeOriginalSelection();
    void restoreOriginalSelection();
    void applySingleSelection();
    
    Map* map_;
    Selection* selection_;
    MapPos position_;
    Item* specificItem_;
    bool addToSelection_;
    QSet<MapPos> originalSelection_;
    bool executed_;
    bool successful_;
};

#endif // SELECTIONCOMMANDS_H
