#ifndef TERRAINBRUSH_H
#define TERRAINBRUSH_H

#include "Brush.h" // Base class
#include <QString>
#include <QVector>

// Forward declarations (if needed, e.g. Map, QUndoCommand are already in Brush.h's scope for virtual methods)

class TerrainBrush : public Brush {
    Q_OBJECT
public:
    explicit TerrainBrush(QObject* parent = nullptr);
    ~TerrainBrush() override;

    // --- Overrides from Brush ---
    // Name will be generic like "Terrain Brush", specificName() gives "Grass", "Dirt" etc.
    QString name() const override;
    Type type() const override;    // Returns Brush::Type::Terrain

    // Convenience method, already in Brush but good to show intent for this class type
    bool isTerrain() const override;

    // Core action methods (will be pure virtual or have basic stubs in .cpp,
    // forcing concrete terrain brushes to implement actual logic)
    bool canDraw(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr) const override = 0;
    QUndoCommand* applyBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override = 0;
    QUndoCommand* removeBrush(Map* map, const QPointF& tilePos, QObject* drawingContext = nullptr, QUndoCommand* parentCommand = nullptr) override = 0;

    // Common properties for terrain brushes - provide default implementations
    bool needBorders() const override;
    bool canDrag() const override; // Terrain brushes are usually draggable
    int getLookID() const override;

    // getBrushSize(), getBrushShape() remain pure virtual from Brush, to be implemented by concrete terrain brushes.
    // mousePressEvent, etc. are inherited. Concrete terrain brushes will implement them using applyBrush/removeBrush.
    // cancel() is inherited. Concrete terrain brushes can override if they have specific cancel state.

    // --- TerrainBrush specific methods ---
    QString specificName() const;
    void setSpecificName(const QString& name);

    void setLookID(quint16 id); // For the item ID this terrain brush might represent or place

    void addFriendBrushType(Brush::Type friendType);
    void removeFriendBrushType(Brush::Type friendType); // Might not be needed if list is fixed at creation
    void setHatesFriends(bool hatesFriends);
    bool hatesFriends() const; // Getter for hatesFriends_
    bool isFriendWith(Brush::Type otherType) const; // Checks against other Brush::Type

protected:
    QString specificBrushName_ = "Unnamed Terrain"; // Concrete name like "Grass", set by subclass constructor
    quint16 lookId_ = 0;        // Item ID this terrain brush might represent or place
    QVector<Brush::Type> friendBrushTypes_; // Stores Brush::Type enum values
    bool hatesFriends_ = false; // If true, friend list becomes an enemy list (for auto-bordering)
};

#endif // TERRAINBRUSH_H
