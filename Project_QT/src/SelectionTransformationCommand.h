#ifndef SELECTIONTRANSFORMATIONCOMMAND_H
#define SELECTIONTRANSFORMATIONCOMMAND_H

#include <QUndoCommand>
#include <QPoint>
#include <QList>
#include <QSet>
#include <QMap>
#include "Map.h"
#include "Selection.h"
#include "Tile.h"
#include "Item.h"

// Forward declarations
class Map;
class Selection;
class Tile;
class Item;

// Task 69: Base class for selection transformation commands
class SelectionTransformationCommand : public QUndoCommand {
public:
    explicit SelectionTransformationCommand(Map* map, Selection* selection, 
                                           const QString& description = "Transform Selection",
                                           QUndoCommand* parent = nullptr);
    virtual ~SelectionTransformationCommand();

    void undo() override;
    void redo() override;

protected:
    // Pure virtual methods for specific transformations
    virtual void executeTransformation() = 0;
    virtual void reverseTransformation() = 0;
    
    // Helper methods for tile management
    void storeTileData();
    void restoreTileData();
    void applyTileChanges();
    
    // Helper methods for selection management
    void updateSelectionAfterTransformation();
    void clearOldSelection();
    
    // Helper methods for automagic integration
    void triggerAutomagicBordering();
    QList<MapPos> getAffectedPositions() const;
    
    // Data members
    Map* map_;
    Selection* selection_;
    QSet<MapPos> originalSelection_;
    QSet<MapPos> transformedSelection_;
    
    // Tile storage for undo/redo
    QMap<MapPos, Tile*> originalTiles_;
    QMap<MapPos, Tile*> transformedTiles_;
    
    bool executed_;
    bool automagicEnabled_;
};

// Task 69: Move selection command
class MoveSelectionCommand : public SelectionTransformationCommand {
public:
    MoveSelectionCommand(Map* map, Selection* selection, const QPoint& offset,
                        QUndoCommand* parent = nullptr);
    ~MoveSelectionCommand() override;

protected:
    void executeTransformation() override;
    void reverseTransformation() override;

private:
    QPoint moveOffset_;
    QPoint reverseOffset_;
    
    // Helper methods for move operation
    void performMove(const QPoint& offset);
    void validateMoveDestination();
    bool isValidMoveDestination(const MapPos& pos) const;
};

// Task 69: Rotate selection command
class RotateSelectionCommand : public SelectionTransformationCommand {
public:
    enum RotationDirection {
        Clockwise90,
        CounterClockwise90,
        Rotate180
    };
    
    RotateSelectionCommand(Map* map, Selection* selection, RotationDirection direction,
                          QUndoCommand* parent = nullptr);
    ~RotateSelectionCommand() override;

protected:
    void executeTransformation() override;
    void reverseTransformation() override;

private:
    RotationDirection rotationDirection_;
    RotationDirection reverseDirection_;
    
    // Helper methods for rotation
    void performRotation(RotationDirection direction);
    MapPos rotatePosition(const MapPos& pos, RotationDirection direction, const MapPos& center) const;
    void rotateItems(Tile* tile, RotationDirection direction);
    void rotateItem(Item* item, RotationDirection direction);
    MapPos getSelectionCenter() const;
    
    // Item rotation helpers
    quint16 getRotatedItemId(quint16 itemId, RotationDirection direction) const;
    RotationDirection getOppositeDirection(RotationDirection direction) const;
};

// Task 69: Flip selection command
class FlipSelectionCommand : public SelectionTransformationCommand {
public:
    enum FlipDirection {
        Horizontal,
        Vertical
    };
    
    FlipSelectionCommand(Map* map, Selection* selection, FlipDirection direction,
                        QUndoCommand* parent = nullptr);
    ~FlipSelectionCommand() override;

protected:
    void executeTransformation() override;
    void reverseTransformation() override;

private:
    FlipDirection flipDirection_;
    
    // Helper methods for flipping
    void performFlip(FlipDirection direction);
    MapPos flipPosition(const MapPos& pos, FlipDirection direction, const MapPos& center) const;
    void flipItems(Tile* tile, FlipDirection direction);
    void flipItem(Item* item, FlipDirection direction);
    MapPos getSelectionCenter() const;
    
    // Item flipping helpers
    quint16 getFlippedItemId(quint16 itemId, FlipDirection direction) const;
};

// Task 69: Composite transformation command for complex operations
class CompositeTransformationCommand : public QUndoCommand {
public:
    CompositeTransformationCommand(const QString& description = "Multiple Transformations",
                                  QUndoCommand* parent = nullptr);
    ~CompositeTransformationCommand() override;
    
    // Add transformation commands
    void addMoveCommand(Map* map, Selection* selection, const QPoint& offset);
    void addRotateCommand(Map* map, Selection* selection, RotateSelectionCommand::RotationDirection direction);
    void addFlipCommand(Map* map, Selection* selection, FlipSelectionCommand::FlipDirection direction);
    
    // Execute all transformations as a single undoable operation
    void executeTransformations();

private:
    QList<SelectionTransformationCommand*> transformationCommands_;
};

// Task 69: Helper class for item transformation logic
class ItemTransformationHelper {
public:
    // Static methods for item transformation
    static bool canRotateItem(quint16 itemId);
    static bool canFlipItem(quint16 itemId);
    static quint16 rotateItemClockwise(quint16 itemId);
    static quint16 rotateItemCounterClockwise(quint16 itemId);
    static quint16 rotateItem180(quint16 itemId);
    static quint16 flipItemHorizontal(quint16 itemId);
    static quint16 flipItemVertical(quint16 itemId);
    
    // Item property queries
    static bool isDirectionalItem(quint16 itemId);
    static bool isWallItem(quint16 itemId);
    static bool isDoorItem(quint16 itemId);
    static bool isTableItem(quint16 itemId);
    static bool isCarpetItem(quint16 itemId);
    
    // Transformation validation
    static bool isTransformationValid(quint16 fromId, quint16 toId);
    static QList<quint16> getTransformationChain(quint16 itemId);
    
private:
    // Internal transformation tables (would be populated from ItemManager)
    static QMap<quint16, quint16> clockwiseRotationMap_;
    static QMap<quint16, quint16> counterClockwiseRotationMap_;
    static QMap<quint16, quint16> horizontalFlipMap_;
    static QMap<quint16, quint16> verticalFlipMap_;
    
    // Initialize transformation maps
    static void initializeTransformationMaps();
    static void loadTransformationsFromItemManager(ItemManager* itemManager);
    static void addCommonItemTransformations();
    static void buildReverseMappings();

    // Helper methods for specific item types
    static void addDirectionalItemTransformations(quint16 itemId, const ItemProperties& props);
    static void addFurnitureTransformations(quint16 itemId, const ItemProperties& props);
    static quint16 findDirectionalVariant(quint16 itemId, const ItemProperties& props);
    static void addDoorRotationChain(quint16 itemId, const ItemProperties& props);
    static void addTableRotations(quint16 itemId, const ItemProperties& props);
    static void addCarpetTransformations(quint16 itemId, const ItemProperties& props);

    static bool mapsInitialized_;
};

#endif // SELECTIONTRANSFORMATIONCOMMAND_H
