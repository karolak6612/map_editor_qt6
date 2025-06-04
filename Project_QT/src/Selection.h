#ifndef SELECTION_H
#define SELECTION_H

#include <QObject>
#include <QSet>
#include <QList>
#include "MapPos.h" // Direct include for MapPos typedef

// Forward declarations
class Map;
class Tile;
class Item;
class Creature;
class Spawn;

enum class SelectionMode {
    Tiles,
    Items,
    Areas
};

enum SelectionType {
    SELECT_CURRENT_FLOOR,
    SELECT_ALL_FLOORS,
    SELECT_VISIBLE_FLOORS
};

enum SessionFlags {
    NONE = 0,
    INTERNAL = 1,
    SUBTHREAD = 2
};

class Selection : public QObject {
    Q_OBJECT

public:
    explicit Selection(Map* mapParent = nullptr);
    ~Selection();

    // Basic tile selection
    void addTile(const MapPos& tilePos);
    void removeTile(const MapPos& tilePos);
    void clear();
    bool isSelected(const MapPos& tilePos) const;
    bool isEmpty() const;
    QSet<MapPos> getSelectedTiles() const;

    // Enhanced selection methods (wxWidgets compatibility)
    void add(Tile* tile);
    void add(Tile* tile, Item* item);
    void add(Tile* tile, Creature* creature);
    void add(Tile* tile, Spawn* spawn);
    void remove(Tile* tile);
    void remove(Tile* tile, Item* item);
    void remove(Tile* tile, Creature* creature);
    void remove(Tile* tile, Spawn* spawn);

    // Internal selection (no undo/action tracking)
    void addInternal(Tile* tile);
    void removeInternal(Tile* tile);

    // Mode and type management
    void setMode(SelectionMode mode);
    SelectionMode getMode() const;
    void setSelectionType(SelectionType type);
    SelectionType getSelectionType() const;

    // Collection methods
    int count() const;
    size_t size() const;
    QList<Tile*> getTiles() const;
    Tile* getSelectedTile() const; // Returns single tile (asserts size == 1)

    // Bounding box methods
    MapPos minPosition() const;
    MapPos maxPosition() const;

    // Session management (wxWidgets compatibility)
    void start(SessionFlags flags = NONE);
    void commit();
    void finish(SessionFlags flags = NONE);
    bool isBusy() const;

    // Iterator support
    QSet<MapPos>::iterator begin();
    QSet<MapPos>::iterator end();
    QSet<MapPos>::const_iterator begin() const;
    QSet<MapPos>::const_iterator end() const;

    // Selection validation
    void updateSelectionCount();
    bool isCompensatedSelect() const;
    void setCompensatedSelect(bool enabled);

signals:
    void selectionChanged();
    void selectionCleared();
    void selectionStarted();
    void selectionFinished();

private:
    Map* mapParent_ = nullptr;
    QSet<MapPos> selectedTiles_;
    QSet<Tile*> selectedTileObjects_; // For direct tile object tracking
    SelectionMode currentMode_ = SelectionMode::Tiles;
    SelectionType selectionType_ = SELECT_CURRENT_FLOOR;
    bool busy_ = false;
    bool compensatedSelect_ = true;

    // Session management
    SessionFlags currentSessionFlags_ = NONE;
};

#endif // SELECTION_H
