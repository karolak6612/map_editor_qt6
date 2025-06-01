#ifndef PASTECOMMAND_H
#define PASTECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include "Map.h"
#include "ClipboardData.h"

// Forward declarations
class Tile;

/**
 * Command for pasting clipboard data to a map
 */
class PasteCommand : public QUndoCommand {
public:
    enum PasteMode {
        REPLACE_MODE,  // Replace existing tiles completely
        MERGE_MODE     // Merge with existing tiles
    };

    PasteCommand(Map* map, const ClipboardData& clipboardData, 
                 const MapPos& targetPosition, PasteMode mode = REPLACE_MODE,
                 QUndoCommand* parent = nullptr);
    ~PasteCommand();

    void undo() override;
    void redo() override;

    // Get the positions that were modified by this paste operation
    QList<MapPos> getModifiedPositions() const;

private:
    Map* map_;
    ClipboardData clipboardData_;
    MapPos targetPosition_;
    PasteMode pasteMode_;
    
    // Store original tiles for undo
    struct OriginalTileData {
        MapPos position;
        Tile* originalTile; // nullptr if tile didn't exist
        bool tileExisted;
    };
    
    QList<OriginalTileData> originalTiles_;
    QList<MapPos> modifiedPositions_;
    bool executed_;
    
    // Helper methods
    void storeOriginalTiles();
    void restoreOriginalTiles();
    void applyPaste();
    Tile* createTileFromClipboardData(const ClipboardTileData& clipboardTile, const MapPos& targetPos);
    void mergeTileWithClipboardData(Tile* existingTile, const ClipboardTileData& clipboardTile);
};

/**
 * Command for cutting tiles (copy + delete)
 */
class CutCommand : public QUndoCommand {
public:
    CutCommand(Map* map, const QSet<MapPos>& selection, ClipboardData& clipboardData,
               QUndoCommand* parent = nullptr);
    ~CutCommand();

    void undo() override;
    void redo() override;

private:
    Map* map_;
    QSet<MapPos> selection_;
    ClipboardData* clipboardData_;
    
    // Store original tiles for undo
    QList<Tile*> originalTiles_;
    QList<MapPos> originalPositions_;
    bool executed_;
    
    // Helper methods
    void storeOriginalTiles();
    void restoreOriginalTiles();
    void applyCut();
};

#endif // PASTECOMMAND_H
