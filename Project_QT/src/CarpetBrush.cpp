#include "CarpetBrush.h"
#include "Map.h"
#include "Tile.h"
#include "Item.h"
#include "ItemManager.h" // For ItemTypeData and g_itemManager or equivalent
#include "Randomizer.h"  // For Randomizer::getRandom
// #include "GlobalSettings.h" // For g_settings->getBoolean(Config::LAYER_CARPETS) - Assuming this will be available

#include <QDomElement>
#include <QString>
#include <QDebug>
#include <QMutableVectorIterator> // For undraw method


// Placeholder for GlobalSettings if not available
// This is a common pattern for accessing global configurations.
#ifndef GLOBALSETTINGS_H // Simple guard to allow external definition
class GlobalSettings {
public:
    static GlobalSettings* getInstance() {
        static GlobalSettings instance;
        return &instance;
    }
    bool getBool(const QString& key, bool defaultValue) const {
        // Dummy implementation
        if (key == "LAYER_CARPETS") return true; // Default to true for testing if not set
        return defaultValue;
    }
private:
    GlobalSettings() = default;
};
#endif


// Static member initialization
QHash<quint8, quint8> CarpetBrush::s_carpet_types_lookup;

// Helper for TILE_ defines from wxwidgets, assuming direct bitmasks for Qt
// These should ideally be in a common header if used by multiple brush types.
const quint8 QT_TILE_NORTHWEST_CB = 1 << 0; // 1
const quint8 QT_TILE_NORTH_CB     = 1 << 1; // 2
const quint8 QT_TILE_NORTHEAST_CB = 1 << 2; // 4
const quint8 QT_TILE_WEST_CB      = 1 << 3; // 8
const quint8 QT_TILE_EAST_CB      = 1 << 4; // 16
const quint8 QT_TILE_SOUTHWEST_CB = 1 << 5; // 32
const quint8 QT_TILE_SOUTH_CB     = 1 << 6; // 64
const quint8 QT_TILE_SOUTHEAST_CB = 1 << 7; // 128

// Enum values for BorderType from wxwidgets/brush_enums.h (used as indices for m_carpet_items)
enum QtBorderType {
    BORDER_NONE = 0,
    NORTHWEST_CORNER = 1,
    NORTH_HORIZONTAL = 2,
    NORTHEAST_CORNER = 3,
    WEST_HORIZONTAL = 4,
    EAST_HORIZONTAL = 5,
    SOUTHWEST_CORNER = 6,
    SOUTH_HORIZONTAL = 7,
    SOUTHEAST_CORNER = 8,
    NORTHWEST_DIAGONAL = 9,
    NORTHEAST_DIAGONAL = 10,
    SOUTHWEST_DIAGONAL = 11,
    SOUTHEAST_DIAGONAL = 12,
};


CarpetBrush::CarpetBrush() : m_look_id(0) {
    m_carpet_items.resize(MAX_CARPET_ALIGNMENTS);
    if (s_carpet_types_lookup.isEmpty()) {
        initLookupTable();
    }
}

CarpetBrush::~CarpetBrush() {}

// Type identification overrides
bool CarpetBrush::isCarpet() const { return true; }
CarpetBrush* CarpetBrush::asCarpet() { return this; }
const CarpetBrush* CarpetBrush::asCarpet() const { return this; }


void CarpetBrush::initLookupTable() {
    s_carpet_types_lookup.clear(); // Start fresh

    // Port all 256 entries from wxwidgets/brush_tables.cpp CarpetBrush::init()
    s_carpet_types_lookup[0] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_NORTH_CB] = CARPET_CENTER_ALIGNMENT_INDEX;     // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_NORTHEAST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_WEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_EAST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_WEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_NORTH_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER; // wx typo: NORTHEAST_CORNER, should be WEST_HORIZONTAL
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB] = NORTHEAST_CORNER; // wx typo: NORTHEAST_CORNER, should be something else
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER; // wx typo: NORTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB] = SOUTH_HORIZONTAL; // Corrected assumption from previous review
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER; // wx: NORTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_NORTH_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_NORTHEAST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_WEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = WEST_HORIZONTAL; // wx: WEST_HORIZONTAL
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = SOUTHWEST_CORNER; // wx: SOUTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER; // wx: NORTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHWEST_CORNER; // wx: NORTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHEAST_CORNER; // wx: SOUTHEAST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHEAST_CORNER; // wx: SOUTHEAST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_DIAGONAL; // wx: NORTHEAST_DIAGONAL
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHWEST_CB] = SOUTHWEST_CORNER; // wx: SOUTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTH_CB] = SOUTHWEST_CORNER; // wx: SOUTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB] = SOUTHWEST_CORNER; // wx: SOUTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB] = SOUTHEAST_CORNER; // wx: SOUTHEAST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHEAST_CORNER; // wx: SOUTHEAST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = SOUTHWEST_DIAGONAL; // wx: SOUTHWEST_DIAGONAL
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = SOUTHEAST_DIAGONAL; // wx: SOUTHEAST_DIAGONAL
    s_carpet_types_lookup[QT_TILE_SOUTH_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_NORTH_CB] = SOUTHEAST_CORNER; // wx: SOUTHEAST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_NORTHEAST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_WEST_CB] = SOUTH_HORIZONTAL; // wx: SOUTH_HORIZONTAL
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER; // wx: NORTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = NORTHWEST_CORNER; // wx: NORTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = EAST_HORIZONTAL; // wx: EAST_HORIZONTAL
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;

    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB] = EAST_HORIZONTAL;

    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTH_CB] = SOUTH_HORIZONTAL; // wx: SOUTH_HORIZONTAL
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = NORTH_HORIZONTAL; // wx: NORTH_HORIZONTAL
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL; // wx: NORTH_HORIZONTAL
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTHWEST_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = NORTH_HORIZONTAL; // wx: NORTH_HORIZONTAL
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB] = SOUTHEAST_CORNER; // wx: SOUTHEAST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_NORTHWEST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_NORTH_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_NORTHEAST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_WEST_CB] = SOUTHWEST_CORNER; // wx: SOUTHWEST_CORNER
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHWEST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = WEST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = SOUTHEAST_CORNER;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = EAST_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHWEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB] = CARPET_CENTER_ALIGNMENT_INDEX;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = SOUTHWEST_DIAGONAL; // wx: SOUTHWEST_DIAGONAL
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTHWEST_CB] = SOUTH_HORIZONTAL;
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB] = SOUTHEAST_DIAGONAL; // wx: SOUTHEAST_DIAGONAL
    s_carpet_types_lookup[QT_TILE_SOUTHEAST_CB | QT_TILE_SOUTH_CB | QT_TILE_EAST_CB | QT_TILE_WEST_CB | QT_TILE_NORTHEAST_CB | QT_TILE_NORTH_CB | QT_TILE_NORTHWEST_CB] = CARPET_CENTER_ALIGNMENT_INDEX; // wx: CARPET_CENTER

    // The loop below is a safeguard but ideally all 256 should be explicitly defined from wxwidgets source
    for (int i = 0; i < 256; ++i) {
        if (!s_carpet_types_lookup.contains(static_cast<quint8>(i))) {
            // This line indicates a combination was missed in the explicit porting.
            // qDebug() << "Warning: CarpetBrush::initLookupTable missing configuration for " << i;
            s_carpet_types_lookup[static_cast<quint8>(i)] = CARPET_CENTER_ALIGNMENT_INDEX;
        }
    }
}


quint8 carpetAlignStringToIdx(const QString& alignString, QString& warnings) {
    QString lowerAlign = alignString.toLower();
    if (lowerAlign == "center") return CARPET_CENTER_ALIGNMENT_INDEX;
    if (lowerAlign == "northwest_corner" || lowerAlign == "nw_corner") return NORTHWEST_CORNER;
    if (lowerAlign == "north_horizontal" || lowerAlign == "n_edge") return NORTH_HORIZONTAL;
    if (lowerAlign == "northeast_corner" || lowerAlign == "ne_corner") return NORTHEAST_CORNER;
    if (lowerAlign == "west_horizontal" || lowerAlign == "w_edge") return WEST_HORIZONTAL;
    if (lowerAlign == "east_horizontal" || lowerAlign == "e_edge") return EAST_HORIZONTAL;
    if (lowerAlign == "southwest_corner" || lowerAlign == "sw_corner") return SOUTHWEST_CORNER;
    if (lowerAlign == "south_horizontal" || lowerAlign == "s_edge") return SOUTH_HORIZONTAL;
    if (lowerAlign == "southeast_corner" || lowerAlign == "se_corner") return SOUTHEAST_CORNER;
    if (lowerAlign == "northwest_diagonal" || lowerAlign == "nw_diag") return NORTHWEST_DIAGONAL;
    if (lowerAlign == "northeast_diagonal" || lowerAlign == "ne_diag") return NORTHEAST_DIAGONAL;
    if (lowerAlign == "southwest_diagonal" || lowerAlign == "sw_diag") return SOUTHWEST_DIAGONAL;
    if (lowerAlign == "southeast_diagonal" || lowerAlign == "se_diag") return SOUTHEAST_DIAGONAL;

    warnings += "CarpetBrush: Unknown alignment string: " + alignString + ". Defaulting to Center.\n";
    return CARPET_CENTER_ALIGNMENT_INDEX;
}

bool CarpetBrush::load(const QDomElement& element, QString& warnings) {
    m_name = element.attribute("name");

    QString serverLookIdStr = element.attribute("server_lookid");
    if (!serverLookIdStr.isEmpty()) {
        const ItemProperties& itemProps = ItemManager::instance()->getItemProperties(serverLookIdStr.toUShort());
        if (itemProps.serverId != 0) m_look_id = itemProps.clientId; // Use clientId from loaded properties
        else m_look_id = serverLookIdStr.toUShort(); // Fallback or if item not in ItemManager
    } else {
        m_look_id = element.attribute("lookid").toUShort();
    }

    for (QDomElement carpetNodeEl = element.firstChildElement("carpet");
         !carpetNodeEl.isNull();
         carpetNodeEl = carpetNodeEl.nextSiblingElement("carpet")) {

        QString alignStr = carpetNodeEl.attribute("align");
        if (alignStr.isEmpty()) {
            warnings += "CarpetBrush: Carpet node missing 'align' attribute for brush " + m_name + "\n";
            continue;
        }
        quint8 alignment_idx = carpetAlignStringToIdx(alignStr, warnings);

        QtCarpetNode& currentCarpetNode = m_carpet_items[alignment_idx];
        currentCarpetNode.items.clear();
        currentCarpetNode.total_chance = 0;

        bool hasChildItemNodes = carpetNodeEl.firstChildElement("item").isElement();

        if (hasChildItemNodes) {
            for (QDomElement itemNodeEl = carpetNodeEl.firstChildElement("item");
                 !itemNodeEl.isNull();
                 itemNodeEl = itemNodeEl.nextSiblingElement("item")) {
                 bool ok_id, ok_chance;
                quint16 id = itemNodeEl.attribute("id").toUShort(&ok_id);
                int chance = itemNodeEl.attribute("chance").toInt(&ok_chance);

                if (!ok_id || id == 0) { warnings += "CarpetBrush: Invalid item ID for brush " + m_name + "\n"; continue; }
                if (!ok_chance || chance <= 0) chance = 1;

                const ItemProperties& itProps = ItemManager::instance()->getItemProperties(id);
                if (itProps.serverId == 0) { warnings += "CarpetBrush: Item ID " + QString::number(id) + " not found in ItemManager for brush " + m_name + "\n"; continue; }
                // Conceptual: itProps.isCarpet = true; itProps.brush = this;
                // This requires ItemManager to allow modification or a registration step.

                QtCarpetVariation var;
                var.item_id = id;
                var.chance = chance;
                currentCarpetNode.items.append(var);
                currentCarpetNode.total_chance += chance;
            }
        } else {
            bool ok_id;
            quint16 id = carpetNodeEl.attribute("id").toUShort(&ok_id);
            if (!ok_id || id == 0) {
                warnings += "CarpetBrush: Carpet node for align '" + alignStr + "' missing child <item> nodes and valid 'id' attribute for brush " + m_name + "\n";
                continue;
            }
            const ItemProperties& itProps = ItemManager::instance()->getItemProperties(id);
            if (itProps.serverId == 0) { warnings += "CarpetBrush: Item ID " + QString::number(id) + " not found for brush " + m_name + "\n"; continue; }
            // Conceptual: itProps.isCarpet = true; itProps.brush = this;

            QtCarpetVariation var;
            var.item_id = id;
            var.chance = 1;
            currentCarpetNode.items.append(var);
            currentCarpetNode.total_chance = 1;
        }
    }
    return true;
}


quint16 CarpetBrush::getRandomCarpetIdByAlignment(quint8 alignment_idx) const {
    if (alignment_idx >= MAX_CARPET_ALIGNMENTS) return 0;

    const QtCarpetNode& node = m_carpet_items[alignment_idx];
    if (node.total_chance > 0 && !node.items.isEmpty()) {
        int randomRoll = Randomizer::getRandom(1, node.total_chance);
        for (const QtCarpetVariation& var : node.items) {
            if (randomRoll <= var.chance) return var.item_id;
            randomRoll -= var.chance;
        }
        return node.items.first().item_id;
    }

    if (alignment_idx != CARPET_CENTER_ALIGNMENT_INDEX) {
        const QtCarpetNode& centerNode = m_carpet_items[CARPET_CENTER_ALIGNMENT_INDEX];
        if (centerNode.total_chance > 0 && !centerNode.items.isEmpty()) {
            int randomRoll = Randomizer::getRandom(1, centerNode.total_chance);
            for (const QtCarpetVariation& var : centerNode.items) {
                if (randomRoll <= var.chance) return var.item_id;
                randomRoll -= var.chance;
            }
            return centerNode.items.first().item_id;
        }
    }

    for(quint8 i = 0; i < MAX_CARPET_ALIGNMENTS; ++i) {
        const QtCarpetNode& anyNode = m_carpet_items[i];
        if (anyNode.total_chance > 0 && !anyNode.items.isEmpty()) {
             int randomRoll = Randomizer::getRandom(1, anyNode.total_chance);
            for (const QtCarpetVariation& var : anyNode.items) {
                if (randomRoll <= var.chance) return var.item_id;
                randomRoll -= var.chance;
            }
            return anyNode.items.first().item_id;
        }
    }
    return 0;
}

// Direct migration from wxwidgets CarpetBrush::getRandomCarpet
quint16 CarpetBrush::getRandomCarpet(BorderType alignment) {
    auto findRandomCarpet = [](const QtCarpetNode& node) -> quint16 {
        int chance = Randomizer::getRandom(1, node.total_chance);
        for (const QtCarpetVariation& carpetType : node.items) {
            if (chance <= carpetType.chance) {
                return carpetType.item_id;
            }
            chance -= carpetType.chance;
        }
        return 0;
    };

    QtCarpetNode node = m_carpet_items[alignment];
    if (node.total_chance > 0) {
        return findRandomCarpet(node);
    }

    node = m_carpet_items[CARPET_CENTER_ALIGNMENT_INDEX];
    if (alignment != CARPET_CENTER_ALIGNMENT_INDEX && node.total_chance > 0) {
        quint16 id = findRandomCarpet(node);
        if (id != 0) {
            return id;
        }
    }

    // Find an item to place on the tile, first center, then the rest.
    for (int i = 0; i < 12; ++i) {
        node = m_carpet_items[i];
        if (node.total_chance > 0) {
            quint16 id = findRandomCarpet(node);
            if (id != 0) {
                return id;
            }
        }
    }
    return 0;
}

void CarpetBrush::draw(Map* map, Tile* tile, void* parameter) {
    // Direct migration from wxwidgets CarpetBrush::draw
    Q_UNUSED(parameter);

    if (!map || !tile) return;

    // Only remove old carpets if layering is disabled (migrated from wxwidgets)
    bool layerCarpets = GlobalSettings::getInstance()->getBool("LAYER_CARPETS", false);
    if (!layerCarpets) {
        undraw(map, tile);
    }

    // Place carpet item using CARPET_CENTER (migrated from wxwidgets)
    quint16 itemIdToPlace = getRandomCarpetIdByAlignment(CARPET_CENTER_ALIGNMENT_INDEX);

    if (itemIdToPlace != 0) {
        Item* newItem = ItemManager::instance()->createItem(itemIdToPlace);
        if (newItem) {
            tile->addItem(newItem);
            map->markModified();
        }
    }
}

void CarpetBrush::undraw(Map* map, Tile* tile) {
    // Direct migration from wxwidgets CarpetBrush::undraw
    if (!map || !tile) return;

    // Migrate wxwidgets iterator logic
    auto items = tile->getItems(); // Get copy for safe iteration
    for (auto it = items.begin(); it != items.end(); ) {
        Item* item = *it;
        if (item && item->isCarpet()) {
            CarpetBrush* carpetBrush = item->getCarpetBrush();
            if (carpetBrush == this) {
                tile->removeItem(item);
                delete item;
                it = items.erase(it);
                map->markModified();
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}

bool CarpetBrush::canDraw(Map* map, const QPoint& position) const {
    return map && map->getTile(position.x(), position.y(), map->getCurrentFloor()) != nullptr;
}

QString CarpetBrush::getName() const { return m_name; }
void CarpetBrush::setName(const QString& newName) { m_name = newName; }
int CarpetBrush::getLookID() const { return m_look_id; }
bool CarpetBrush::needBorders() const { return true; }
bool CarpetBrush::canDrag() const { return true; }

static bool hasMatchingCarpetBrushAtTile(Map* map, CarpetBrush* carpet_brush, int x, int y, int z) {
    Tile* t = map->getTile(x, y, z);
    if (!t) return false;
    for (Item* item : t->items()) {
        if (item && item->isCarpet()) {
            Brush* b = item->getBrush();
            if (b && b->asCarpet() == carpet_brush) return true;
        }
    }
    return false;
}

void CarpetBrush::doCarpets(Map* map, Tile* tile) {
    // Direct migration from wxwidgets CarpetBrush::doCarpets
    if (!map || !tile) return;

    if (!tile->hasCarpet()) {
        return;
    }


    const Position& position = tile->getPosition();
    quint32 x = position.x;
    quint32 y = position.y;
    quint32 z = position.z;

    for (Item* item : tile->getItems()) {
        if (!item || !item->isCarpet()) continue;

        CarpetBrush* carpetBrush = item->getCarpetBrush();
        if (!carpetBrush) continue;

        // Check 8 neighbors for matching carpet brush (migrated from wxwidgets)
        bool neighbours[8];
        neighbours[0] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y - 1, z);
        neighbours[1] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x, y - 1, z);
        neighbours[2] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y - 1, z);
        neighbours[3] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y, z);
        neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y, z);
        neighbours[5] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y + 1, z);
        neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x, y + 1, z);
        neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y + 1, z);

        quint32 tileData = 0;
        for (quint32 i = 0; i < 8; ++i) {
            if (neighbours[i]) {
                // Same carpet as this one, calculate what border
                tileData |= static_cast<quint32>(1) << i;
            }
        }

        // Get border type from lookup table (migrated from wxwidgets)
        BorderType bt = static_cast<BorderType>(carpet_types[tileData]);
        quint16 id = carpetBrush->getRandomCarpet(bt);
        if (id != 0) {
            item->setID(id);
        }
    }
}
