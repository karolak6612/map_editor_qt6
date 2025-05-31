#ifndef QT_TABLEBRUSH_H
#define QT_TABLEBRUSH_H

#include "Brush.h" // Assuming Brush.h is in the same directory
#include <QString>
#include <QList>
#include <QVector>
#include <QHash>
#include <QDomElement> // For QDomElement in load()
// QPoint might be included via other headers like QMouseEvent, QPainter, QWidget, etc.
// If not, and it's used directly in a way that requires its definition,
// #include <QPoint> would be necessary. For now, assume it's covered.

// Forward declarations
class Map;
class Tile;
class Item;

// Enum for table alignment, mirroring wxwidgets' TableAlignment
// These values are typically used as indices for table_items array.
enum QtTableAlignment {
    TABLE_ALONE = 0,
    TABLE_VERTICAL = 1,
    TABLE_HORIZONTAL = 2,
    TABLE_SOUTH_END = 3,
    TABLE_EAST_END = 4,
    TABLE_NORTH_END = 5,
    TABLE_WEST_END = 6,
    // MAX_TABLE_ALIGNMENTS = 7 // Implicit size for arrays
};

struct QtTableVariation {
    int chance;
    quint16 item_id; // Using quint16 for item ID as in wxwidgets
};

struct QtTableNode {
    int total_chance;
    QList<QtTableVariation> items;

    QtTableNode() : total_chance(0) {}
};

class TableBrush : public Brush {
public:
    TableBrush();
    virtual ~TableBrush();

    // Virtual methods from Brush base class
    bool load(const QDomElement& element, QString& warnings) override;
    void draw(Map* map, Tile* tile, void* parameter = nullptr) override;
    void undraw(Map* map, Tile* tile) override;
    bool canDraw(Map* map, const QPoint& position) const override; // Assuming QPoint for Qt position

    QString getName() const override;
    void setName(const QString& newName) override;

    // For preview in UI, might be an ID or direct pixmap
    int getLookID() const override; // Returning an ID for now
    // QPixmap getLookPixmap() const; // Alternative for direct sprite preview

    bool needBorders() const override; // Typically true for brushes affecting neighbors

    // Type identification
    bool isTable() const override { return true; }
    TableBrush* asTable() override { return this; }

    // Static methods
    static void initLookupTable(); // To populate table_types_lookup
    static void doTables(Map* map, Tile* tile); // Main logic for connections

private:
    QString m_name;
    quint16 m_look_id; // Or QPixmap m_look_sprite_preview;

    // Stores variations for each alignment type (e.g., TABLE_ALONE, TABLE_VERTICAL, etc.)
    // Indexed by QtTableAlignment enum values. Should be size 7.
    QVector<QtTableNode> m_table_items;

    // Lookup table: maps an 8-bit neighbor configuration to a QtTableAlignment value.
    // Key: quint8 (neighbor mask), Value: quint8 (QtTableAlignment)
    static QHash<quint8, quint8> s_table_types_lookup;

    // Helper to get an item ID for a given alignment, considering chances
    quint16 getRandomItemIdForAlignment(QtTableAlignment alignment) const;
};

#endif // QT_TABLEBRUSH_H
