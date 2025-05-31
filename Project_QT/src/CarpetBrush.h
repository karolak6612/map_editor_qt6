#ifndef QT_CARPETBRUSH_H
#define QT_CARPETBRUSH_H

#include "Brush.h"
#include <QString>
#include <QList>
#include <QVector>
#include <QHash>
#include <QDomElement>
#include <QPoint> // For canDraw's QPoint parameter

// Forward declarations
class Map;
class Tile;
class Item;

// Define an enum for carpet alignment indices, if helpful, though direct quint8 might be used.
// Based on wxwidgets, there are 13 border types (0-12) and a general "center" (13).
const quint8 MAX_CARPET_ALIGNMENTS = 14; // 0-12 for borders, 13 for center.
const quint8 CARPET_CENTER_ALIGNMENT_INDEX = 13;


struct QtCarpetVariation {
    int chance;
    quint16 item_id;
};

struct QtCarpetNode {
    int total_chance;
    QList<QtCarpetVariation> items;

    QtCarpetNode() : total_chance(0) {}
};

class CarpetBrush : public Brush {
public:
    CarpetBrush();
    virtual ~CarpetBrush();

    // Virtual methods from Brush base class
    bool load(const QDomElement& element, QString& warnings) override;
    void draw(Map* map, Tile* tile, void* parameter = nullptr) override;
    void undraw(Map* map, Tile* tile) override;
    bool canDraw(Map* map, const QPoint& position) const override;

    QString getName() const override;
    void setName(const QString& newName) override;

    int getLookID() const override;

    bool needBorders() const override;
    bool canDrag() const override;

    // Type identification
    // For isCarpet, it should be:
    // bool isCarpet() const override { return true; }
    // CarpetBrush* asCarpet() override { return this; }
    // const CarpetBrush* asCarpet() const override { return this; } // Added const version
    // The base class Brush needs to declare these as virtual in the first place.
    // Assuming Brush.h will be updated to include:
    // virtual bool isCarpet() const;
    // virtual CarpetBrush* asCarpet();
    // virtual const CarpetBrush* asCarpet() const;
    // For this file, we implement the overrides.
    bool isCarpet() const override;
    CarpetBrush* asCarpet() override;
    const CarpetBrush* asCarpet() const override;


    // Static methods
    static void initLookupTable();
    static void doCarpets(Map* map, Tile* tile);

protected:
    // Helper to get an item ID for a given alignment, considering chances and fallbacks
    quint16 getRandomCarpetIdByAlignment(quint8 alignment_idx) const;

private:
    QString m_name;
    quint16 m_look_id;

    // Stores variations for each alignment type.
    // Indices 0-12 for BorderType, index 13 for CARPET_CENTER.
    QVector<QtCarpetNode> m_carpet_items;

    // Lookup table: maps an 8-bit neighbor configuration to a BorderType enum value (0-12).
    // These BorderType values are then used as indices for m_carpet_items.
    static QHash<quint8, quint8> s_carpet_types_lookup;
};

#endif // QT_CARPETBRUSH_H
