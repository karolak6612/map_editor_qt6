#include "EraseCommand.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "Creature.h"
#include "Spawn.h"
#include <QDebug>

EraseCommand::EraseCommand(Map* map,
                          const QPointF& tilePos,
                          EraseMode mode,
                          QUndoCommand* parent)
    : QUndoCommand(parent)
    , map_(map)
    , tilePos_(tilePos)
    , eraseMode_(mode)
    , tilePreviouslyExisted_(false)
    , tileWasEmpty_(false)
{
    QString modeText;
    switch (mode) {
        case EraseAll:
            modeText = "All Items";
            break;
        case EraseItems:
            modeText = "Items";
            break;
        case EraseCreatures:
            modeText = "Creatures";
            break;
        case EraseSpawns:
            modeText = "Spawns";
            break;
        case EraseDecorations:
            modeText = "Decorations";
            break;
    }
    
    setText(QObject::tr("Erase %1 at (%2,%3)")
                .arg(modeText)
                .arg(tilePos_.x())
                .arg(tilePos_.y()));
}

EraseCommand::~EraseCommand()
{
    // Cleanup is handled by tile ownership
}

void EraseCommand::undo()
{
    if (!map_) {
        qWarning() << "EraseCommand::undo(): Map pointer is null.";
        return;
    }

    // Get or create tile if it was removed
    Tile* tile = map_->getTile(tilePos_);
    if (!tile && tilePreviouslyExisted_) {
        tile = map_->getOrCreateTile(tilePos_);
        if (!tile) {
            qWarning() << "EraseCommand::undo(): Could not recreate tile at" << tilePos_;
            return;
        }
    }

    if (tile) {
        // Restore erased items
        restoreErasedItems(tile);
        qDebug() << "EraseCommand: Undone - Restored" << erasedItems_.size() << "items at" << tilePos_;
    }

    // Ensure map is marked as modified
    map_->setModified(true);
}

void EraseCommand::redo()
{
    if (!map_) {
        qWarning() << "EraseCommand::redo(): Map pointer is null.";
        return;
    }

    Tile* tile = map_->getTile(tilePos_);
    if (!tile) {
        qDebug() << "EraseCommand::redo(): No tile found at" << tilePos_ << "- nothing to erase";
        return;
    }

    tilePreviouslyExisted_ = true;
    tileWasEmpty_ = tile->isEmpty();

    // Store items that will be erased
    storeItemsToErase(tile);

    // Erase items based on mode
    switch (eraseMode_) {
        case EraseAll: {
            // Remove all items except ground
            QVector<Item*> itemsToRemove;
            const QVector<Item*>& items = tile->getItems();
            for (Item* item : items) {
                if (item && !item->isGround()) {
                    itemsToRemove.append(item);
                }
            }
            for (Item* item : itemsToRemove) {
                tile->removeItem(item);
                delete item;
            }
            
            // Remove creatures
            const QList<Creature*>& creatures = tile->getCreatures();
            for (Creature* creature : creatures) {
                tile->removeCreature(creature);
                delete creature;
            }
            
            // Remove spawn
            if (tile->spawn()) {
                Spawn* spawn = tile->spawn();
                tile->setSpawn(nullptr);
                delete spawn;
            }
            break;
        }
        case EraseItems: {
            QVector<Item*> itemsToRemove;
            const QVector<Item*>& items = tile->getItems();
            for (Item* item : items) {
                if (item && item->isMoveable()) {
                    itemsToRemove.append(item);
                }
            }
            for (Item* item : itemsToRemove) {
                tile->removeItem(item);
                delete item;
            }
            break;
        }
        case EraseCreatures: {
            const QList<Creature*>& creatures = tile->getCreatures();
            for (Creature* creature : creatures) {
                tile->removeCreature(creature);
                delete creature;
            }
            break;
        }
        case EraseSpawns: {
            if (tile->spawn()) {
                Spawn* spawn = tile->spawn();
                tile->setSpawn(nullptr);
                delete spawn;
            }
            break;
        }
        case EraseDecorations: {
            QVector<Item*> itemsToRemove;
            const QVector<Item*>& items = tile->getItems();
            for (Item* item : items) {
                if (item && item->isDecoration()) {
                    itemsToRemove.append(item);
                }
            }
            for (Item* item : itemsToRemove) {
                tile->removeItem(item);
                delete item;
            }
            break;
        }
    }

    qDebug() << "EraseCommand: Redone - Erased" << erasedItems_.size() << "items at" << tilePos_;

    // If tile is now empty, we could remove it, but that might be handled elsewhere
    
    // Ensure map is marked as modified
    map_->setModified(true);
}

void EraseCommand::storeItemsToErase(Tile* tile)
{
    if (!tile) return;
    
    erasedItems_.clear();
    
    // Store items that will be erased based on mode
    const QVector<Item*>& items = tile->getItems();
    for (int i = 0; i < items.size(); ++i) {
        Item* item = items[i];
        if (item && shouldEraseItem(item)) {
            ErasedItem erasedItem(item->getID(), i);
            // Could store additional item properties here if needed
            erasedItems_.append(erasedItem);
        }
    }
    
    // Note: This is a simplified implementation. A full implementation would
    // need to store complete item state including all properties, attributes, etc.
}

void EraseCommand::restoreErasedItems(Tile* tile)
{
    if (!tile || erasedItems_.isEmpty()) return;
    
    // This is a simplified restoration - in a full implementation,
    // you would need to recreate items with all their properties
    for (const ErasedItem& erasedItem : erasedItems_) {
        if (erasedItem.itemId != 0) {
            Item* restoredItem = new Item(erasedItem.itemId);
            if (restoredItem) {
                tile->addItem(restoredItem);
            }
        }
    }
    
    qDebug() << "EraseCommand: Restored" << erasedItems_.size() << "items (simplified restoration)";
}

bool EraseCommand::shouldEraseItem(Item* item) const
{
    if (!item) return false;
    
    switch (eraseMode_) {
        case EraseAll:
            return !item->isGround(); // Don't erase ground
        case EraseItems:
            return item->isMoveable();
        case EraseCreatures:
            return false; // Creatures are handled separately
        case EraseSpawns:
            return false; // Spawns are handled separately
        case EraseDecorations:
            return item->isDecoration();
        default:
            return false;
    }
}
