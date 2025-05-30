#ifndef SELECTION_H
#define SELECTION_H

#include "Map.h" // Assuming MapPos is defined here or accessible through it
#include <QSet>

// Forward declare Map if Selection needs to reference it (e.g., as a parent or context)
class Map;

enum class SelectionMode {
    Tiles,
    Items,
    Areas
};

class Selection {
public:
    explicit Selection(Map* mapParent = nullptr);
    ~Selection();

    void addTile(const MapPos& tilePos);
    void removeTile(const MapPos& tilePos);
    void clear();
    bool isSelected(const MapPos& tilePos) const;
    bool isEmpty() const;
    QSet<MapPos> getSelectedTiles() const;

    void setMode(SelectionMode mode);
    SelectionMode getMode() const;

    int count() const; // Returns the number of selected tiles

private:
    Map* mapParent_ = nullptr; // Optional: if Selection needs to interact with or reference its map context
    QSet<MapPos> selectedTiles_;
    SelectionMode currentMode_ = SelectionMode::Tiles;
};

#endif // SELECTION_H
