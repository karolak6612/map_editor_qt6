#include "TableBrush.h"
#include "Map.h"        // For Map, Tile
#include "Tile.h"       // For Tile
#include "Item.h"       // For Item
#include "ItemManager.h" // For g_itemManager or equivalent
#include "Randomizer.h" // For a random number generator utility

#include <QDomElement>
#include <QString>
#include <QDebug> // For warnings or debug output
#include <QMutableListIterator> // Required for undraw

// Initialize static member
QHash<quint8, quint8> TableBrush::s_table_types_lookup;

// Helper for TILE_ defines from wxwidgets, assuming direct bitmasks for Qt
// These might need to be defined in a common header if used by other brushes too.
const quint8 QT_TILE_NORTHWEST = 1 << 0; // 1
const quint8 QT_TILE_NORTH = 1 << 1;     // 2
const quint8 QT_TILE_NORTHEAST = 1 << 2; // 4
const quint8 QT_TILE_WEST = 1 << 3;      // 8
const quint8 QT_TILE_EAST = 1 << 4;      // 16
const quint8 QT_TILE_SOUTHWEST = 1 << 5; // 32
const quint8 QT_TILE_SOUTH = 1 << 6;     // 64
const quint8 QT_TILE_SOUTHEAST = 1 << 7; // 128


TableBrush::TableBrush() : m_look_id(0) {
    // Initialize m_table_items to have 7 default-constructed QtTableNode elements
    m_table_items.resize(7);
    // It's good practice to ensure the lookup table is initialized.
    if (s_table_types_lookup.isEmpty()) {
        initLookupTable();
    }
}

TableBrush::~TableBrush() {
    // Destructor
}

// Static method to initialize the lookup table
void TableBrush::initLookupTable() {
    // This is a direct port of the table_types array from wxwidgets/brush_tables.cpp
    // Each entry maps a neighborhood configuration (key) to a QtTableAlignment value.
    s_table_types_lookup[0] = TABLE_ALONE;
    s_table_types_lookup[QT_TILE_NORTHWEST] = TABLE_ALONE;
    s_table_types_lookup[QT_TILE_NORTH] = TABLE_SOUTH_END;
    s_table_types_lookup[QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_NORTHEAST] = TABLE_ALONE;
    s_table_types_lookup[QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_ALONE;
    s_table_types_lookup[QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_WEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_WEST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_EAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_NORTHEAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_WEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHWEST] = TABLE_ALONE;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_NORTHWEST] = TABLE_ALONE;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_NORTH] = TABLE_SOUTH_END; // wx: SOUTH_END
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST] = TABLE_ALONE;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_ALONE;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_WEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH] = TABLE_NORTH_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_NORTHWEST] = TABLE_NORTH_END; // wx: NORTH_END
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_NORTH] = TABLE_VERTICAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_NORTH_END; // wx: VERTICAL, but tablebrush.cpp specific value is NORTH_END
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_NORTHEAST] = TABLE_NORTH_END; // wx: NORTH_END
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_NORTH_END; // wx: NORTH_END
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_NORTH_END; // wx: VERTICAL, but tablebrush.cpp specific value is NORTH_END
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_NORTH_END; // wx: VERTICAL, but tablebrush.cpp specific value is NORTH_END
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_WEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_NORTHEAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_WEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST] = TABLE_ALONE; // wx: NORTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: NORTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_NORTH] = TABLE_SOUTH_END; // wx: VERTICAL, but tablebrush.cpp specific value is SOUTH_END
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: VERTICAL, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST] = TABLE_ALONE; // wx: NORTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: NORTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_ALONE; // wx: VERTICAL, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: VERTICAL, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_WEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTH | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST] = TABLE_ALONE;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_NORTHWEST] = TABLE_ALONE;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_NORTH] = TABLE_SOUTH_END; // wx: SOUTH_END
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_NORTHEAST] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_WEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_NORTHEAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_WEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST] = TABLE_ALONE; // wx: NORTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: NORTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_NORTH] = TABLE_SOUTH_END; // wx: VERTICAL, but tablebrush.cpp specific value is SOUTH_END
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: VERTICAL, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: SOUTH_END, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_ALONE; // wx: VERTICAL, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_ALONE; // wx: VERTICAL, but tablebrush.cpp specific value is ALONE
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_WEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_EAST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_WEST_END;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH] = TABLE_HORIZONTAL;
    s_table_types_lookup[QT_TILE_SOUTHEAST | QT_TILE_SOUTHWEST | QT_TILE_EAST | QT_TILE_WEST | QT_TILE_NORTHEAST | QT_TILE_NORTH | QT_TILE_NORTHWEST] = TABLE_HORIZONTAL;

    // Ensure all 256 entries are covered, default to TABLE_ALONE if an entry was missed.
    // (This loop is more of a safeguard, the explicit assignments should cover all)
    for (int i = 0; i < 256; ++i) {
        if (!s_table_types_lookup.contains(static_cast<quint8>(i))) {
            // This warning helps identify if any specific combinations were missed in the direct port.
            // qDebug() << "Warning: TableBrush::initLookupTable missing configuration for " << i;
            s_table_types_lookup[static_cast<quint8>(i)] = TABLE_ALONE;
        }
    }
}


bool TableBrush::load(const QDomElement& element, QString& warnings) {
    m_name = element.attribute("name");

    QString serverLookIdStr = element.attribute("server_lookid");
    if (!serverLookIdStr.isEmpty()) {
        // Placeholder for ItemManager interaction
        // ItemType& it = ItemManager::getInstance().getItemType(serverLookIdStr.toUShort());
        // if (it.isValid()) m_look_id = it.clientId;
        // else m_look_id = serverLookIdStr.toUShort();
        m_look_id = serverLookIdStr.toUShort(); // Simplified
    } else {
        m_look_id = element.attribute("lookid").toUShort();
    }

    for (QDomElement tableNodeEl = element.firstChildElement("table");
         !tableNodeEl.isNull();
         tableNodeEl = tableNodeEl.nextSiblingElement("table")) {

        QString alignString = tableNodeEl.attribute("align").toLower();
        QtTableAlignment alignment;

        if (alignString == "vertical") alignment = TABLE_VERTICAL;
        else if (alignString == "horizontal") alignment = TABLE_HORIZONTAL;
        else if (alignString == "south") alignment = TABLE_SOUTH_END;
        else if (alignString == "east") alignment = TABLE_EAST_END;
        else if (alignString == "north") alignment = TABLE_NORTH_END;
        else if (alignString == "west") alignment = TABLE_WEST_END;
        else if (alignString == "alone") alignment = TABLE_ALONE;
        else {
            warnings += QString("Unknown table alignment '%1' for brush %2\n").arg(alignString, m_name);
            continue;
        }

        if (alignment < 0 || alignment >= m_table_items.size()) {
             warnings += QString("Alignment index out of bounds for brush %1\n").arg(m_name);
             continue;
        }

        QtTableNode& currentTableNode = m_table_items[alignment];
        currentTableNode.items.clear(); // Clear previous items if reloading
        currentTableNode.total_chance = 0;

        for (QDomElement itemNodeEl = tableNodeEl.firstChildElement("item");
             !itemNodeEl.isNull();
             itemNodeEl = itemNodeEl.nextSiblingElement("item")) {

            bool ok;
            quint16 id = itemNodeEl.attribute("id").toUShort(&ok);
            if (!ok || id == 0) {
                warnings += QString("Could not read valid item ID for brush %1, table align %2\n").arg(m_name, alignString);
                continue;
            }

            ItemType* it = ItemManager::getInstance().getItemType(id);
            if (!it) {
                warnings += QString("ItemType with ID %1 not found for brush %2\n").arg(id).arg(m_name);
                continue;
            }
            it->isTable = true;
            it->brush = this;

            QtTableVariation variation;
            variation.item_id = id;
            variation.chance = itemNodeEl.attribute("chance").toInt(&ok);
            if (!ok || variation.chance <= 0) {
                variation.chance = 1;
            }

            currentTableNode.items.append(variation);
            currentTableNode.total_chance += variation.chance;
        }
    }
    return true;
}

quint16 TableBrush::getRandomItemIdForAlignment(QtTableAlignment alignment) const {
    if (alignment < 0 || alignment >= m_table_items.size()) {
        return 0;
    }

    const QtTableNode& node = m_table_items[alignment];
    if (node.total_chance <= 0 || node.items.isEmpty()) {
        return 0;
    }

    int randomRoll = Randomizer::getRandom(1, node.total_chance);
    for (const QtTableVariation& variation : node.items) {
        if (randomRoll <= variation.chance) {
            return variation.item_id;
        }
        randomRoll -= variation.chance;
    }
    return node.items.isEmpty() ? 0 : node.items.first().item_id; // Fallback, should ideally not be reached if logic is correct
}

void TableBrush::draw(Map* map, Tile* tile, void* parameter) {
    if (!map || !tile) return;

    undraw(map, tile);

    quint16 itemIdToPlace = getRandomItemIdForAlignment(TABLE_ALONE);

    if (itemIdToPlace != 0) {
        Item* newItem = ItemManager::getInstance().createItem(itemIdToPlace);
        if (newItem) {
            // TODO: Handle action ID
            // if (g_gui.IsCurrentActionIDEnabled()) {
            //     newItem->setActionID(g_gui.GetCurrentActionID());
            // }
            tile->addItem(newItem);
            map->markModified();
        }
    }
}

void TableBrush::undraw(Map* map, Tile* tile) {
    if (!map || !tile) return;

    QMutableListIterator<Item*> it(tile->getItemsForWrite()); // Assuming Tile provides this for modification
    bool changed = false;
    while (it.hasNext()) {
        Item* item = it.next();
        if (item && item->isTable()) {
            Brush* itemBrush = item->getBrush();
            if (itemBrush == this) { // Check if it's the same brush instance
                it.remove();
                delete item;
                changed = true;
            }
        }
    }
    if (changed) {
        map->markModified();
        // tile->update(); // Assuming update might be needed, e.g. to clear TILESTATE_HAS_TABLE
                        // This depends on how Tile::hasTable() is determined.
                        // If hasTable() checks items, removing items is enough.
                        // If it's a flag, it needs to be cleared if no tables remain.
    }
}

bool TableBrush::canDraw(Map* map, const QPoint& position) const {
    if (!map) return false;
    // Assuming getCurrentFloor() is part of Map or accessible globally for the current view context
    // For now, let's assume map->getTile can handle a 'current floor' concept internally or via a parameter.
    // If Map needs current floor explicitly:
    // int currentFloor = map->getCurrentFloor(); // Or however it's obtained
    // return map->getTile(position.x(), position.y(), currentFloor) != nullptr;
    return map->getTile(position.x(), position.y(), map->getCurrentFloor()) != nullptr;
}

QString TableBrush::getName() const {
    return m_name;
}

void TableBrush::setName(const QString& newName) {
    m_name = newName;
}

int TableBrush::getLookID() const {
    return m_look_id;
}

bool TableBrush::needBorders() const {
    return true;
}

static bool hasMatchingTableBrushAtTile(Map* map, TableBrush* table_brush, int x, int y, int z) {
    Tile* t = map->getTile(x, y, z);
    if (!t) {
        return false;
    }

    for (Item* item : t->getItems()) {
        if (item && item->isTable()) {
            Brush* b = item->getBrush();
            // Check if the brush is a TableBrush and if it's the *same instance* of TableBrush
            // This ensures that different table types don't connect.
            if (b && b->asTable() == table_brush) {
                return true;
            }
        }
    }
    return false;
}

void TableBrush::doTables(Map* map, Tile* tile) {
    if (!map || !tile ) return; // Removed || !tile->hasTable() to allow fixing items if flag is wrong

    const QPoint pos = tile->getPosition();
    int x = pos.x();
    int y = pos.y();
    int z = tile->getZ();

    // Create a copy of the item list for safe iteration if items might be replaced/re-added
    // QList<Item*> itemsOnTile = tile->getItems(); // Assuming getItems returns a copy or const&

    for (Item* item : tile->getItems()) { // Iterate over original or copy
        if (!item || !item->isTable()) continue;

        TableBrush* itemTableBrush = dynamic_cast<TableBrush*>(item->getBrush());
        if (!itemTableBrush) continue; // Not a table or not managed by a TableBrush

        quint8 neighborConfig = 0;
        if (hasMatchingTableBrushAtTile(map, itemTableBrush, x - 1, y - 1, z)) neighborConfig |= QT_TILE_NORTHWEST;
        if (hasMatchingTableBrushAtTile(map, itemTableBrush, x,     y - 1, z)) neighborConfig |= QT_TILE_NORTH;
        if (hasMatchingTableBrushAtTile(map, itemTableBrush, x + 1, y - 1, z)) neighborConfig |= QT_TILE_NORTHEAST;
        if (hasMatchingTableBrushAtTile(map, itemTableBrush, x - 1, y,     z)) neighborConfig |= QT_TILE_WEST;
        if (hasMatchingTableBrushAtTile(map, itemTableBrush, x + 1, y,     z)) neighborConfig |= QT_TILE_EAST;
        if (hasMatchingTableBrushAtTile(map, itemTableBrush, x - 1, y + 1, z)) neighborConfig |= QT_TILE_SOUTHWEST;
        if (hasMatchingTableBrushAtTile(map, itemTableBrush, x,     y + 1, z)) neighborConfig |= QT_TILE_SOUTH;
        if (hasMatchingTableBrushAtTile(map, itemTableBrush, x + 1, y + 1, z)) neighborConfig |= QT_TILE_SOUTHEAST;

        QtTableAlignment targetAlignment = static_cast<QtTableAlignment>(s_table_types_lookup.value(neighborConfig, TABLE_ALONE));

        quint16 newItemId = itemTableBrush->getRandomItemIdForAlignment(targetAlignment);

        if (newItemId != 0 && item->getID() != newItemId) {
            // The original wxWidgets code modified the item's ID directly.
            // This implies that Item::setID() would change its appearance/type.
            item->setID(newItemId);
            // If setID also implies visual/property changes that need broader updates:
            // item->transform(newItemId); // or similar method if simple setID is not enough
            map->markModified();
            // tile->update(); // If the tile needs explicit update after item change
        }
    }
    // After processing all items, update the tile's general "hasTable" state if necessary
    // tile->updateHasTableFlag(); // Example method
}

// Placeholder for ItemManager::getInstance() if not defined elsewhere
// This is highly dependent on your project structure.
#if 0 // Define if you don't have a global ItemManager accessor
class ItemManager { // Dummy
public:
    static ItemManager& getInstance() {
        static ItemManager instance;
        return instance;
    }
    ItemType* getItemType(quint16 id) { return nullptr; /* dummy */ }
    Item* createItem(quint16 id) { return nullptr; /* dummy */ }
};
#endif

// Placeholder for Randomizer::getRandom() if not defined elsewhere
#if 0 // Define if you don't have a global Randomizer
#include <random>
class Randomizer { // Dummy
public:
    static int getRandom(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(min, max);
        return distrib(gen);
    }
};
#endif

// Placeholders for missing methods often found in Tile or Map
// (These are illustrative and depend on your actual class definitions)
/*
QList<Item*>& Tile::getItemsForWrite() { return items_list_member; } // Example
QPoint Tile::getPosition() const { return QPoint(m_x, m_y); } // Example
int Tile::getZ() const { return m_z; } // Example
bool Tile::hasTable() const { // Example logic
    for(Item* item : items_list_member) {
        if(item && item->isTable()) return true;
    }
    return false;
}
void Tile::addItem(Item* item) { items_list_member.append(item); } // Example
Item* Map::getTile(int x, int y, int z) { return nullptr; } // Example
void Map::markModified() {} // Example
int Map::getCurrentFloor() const { return 0; } // Example

bool Item::isTable() const { return false; } // Example
Brush* Item::getBrush() const { return nullptr; } // Example
quint16 Item::getID() const { return 0; } // Example
void Item::setID(quint16 id) {} // Example
*/
