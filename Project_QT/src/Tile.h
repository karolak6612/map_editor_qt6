#ifndef TILE_H
#define TILE_H

#include <QObject> // Or QPoint if position is stored directly
#include <QVector> // For getItems() return type example

// Minimal Tile class declaration for Map.cpp to compile
// Full implementation will be in Task 12.
class Item; // Forward declare
class Creature; // Forward declare

class Tile : public QObject { // Inherit QObject if it might have signals/slots later
    Q_OBJECT
public:
    // Store coordinates for debugging or if Tile needs to know its own position
    int x, y, z; 
    explicit Tile(int x_pos, int y_pos, int z_pos, QObject *parent = nullptr) : QObject(parent), x(x_pos), y(y_pos), z(z_pos) {}
    ~Tile() override {} // Ensure virtual destructor if inheriting QObject and being deleted via base pointer

    // Minimal methods that might be called by Map or for basic compilation (placeholders)
    // void addItem(Item* item) {}
    // void removeItem(Item* item) {}
    // Item* getGround() const { return nullptr; }
    // const QVector<Item*>& getItems() const { static QVector<Item*> emptyItems; return emptyItems; }
    // Creature* getCreature() const { return nullptr; }
};

#endif // TILE_H
