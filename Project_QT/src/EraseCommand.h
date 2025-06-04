#ifndef ERASECOMMAND_H
#define ERASECOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QString>
#include <QVector>

// Forward declarations
class Map;
class Item;
class Tile;

/**
 * @brief Command for erasing items from tiles
 * 
 * This command handles removing all or specific types of items from tiles
 * in an undoable manner. It stores the removed items to allow restoration.
 */
class EraseCommand : public QUndoCommand
{
public:
    enum EraseMode {
        EraseAll,           // Erase all items except ground
        EraseItems,         // Erase only moveable items
        EraseCreatures,     // Erase only creatures
        EraseSpawns,        // Erase only spawns
        EraseDecorations    // Erase only decorations
    };

    EraseCommand(Map* map,
                const QPointF& tilePos,
                EraseMode mode = EraseAll,
                QUndoCommand* parent = nullptr);
    ~EraseCommand() override;

    void undo() override;
    void redo() override;

private:
    struct ErasedItem {
        quint16 itemId;
        QVariant itemData; // Additional item properties if needed
        int stackPosition; // Position in tile's item stack
        
        ErasedItem(quint16 id = 0, int pos = 0) : itemId(id), stackPosition(pos) {}
    };

    Map* map_;
    QPointF tilePos_;
    EraseMode eraseMode_;
    QVector<ErasedItem> erasedItems_;
    bool tilePreviouslyExisted_;
    bool tileWasEmpty_;
    
    void storeItemsToErase(Tile* tile);
    void restoreErasedItems(Tile* tile);
    bool shouldEraseItem(Item* item) const;
};

#endif // ERASECOMMAND_H
