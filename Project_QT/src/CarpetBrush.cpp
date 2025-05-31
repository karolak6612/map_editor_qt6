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

void CarpetBrush::draw(Map* map, Tile* tile, void* parameter) {
    if (!map || !tile) return;

    bool layerCarpets = GlobalSettings::getInstance()->getBool("LAYER_CARPETS", false);
    if (!layerCarpets) {
        undraw(map, tile);
    }

    quint16 itemIdToPlace = getRandomCarpetIdByAlignment(CARPET_CENTER_ALIGNMENT_INDEX);

    if (itemIdToPlace != 0) {
        Item* newItem = ItemManager::instance()->createItem(itemIdToPlace);
        if (newItem) {
            tile->addItem(newItem);
            map->markModified();
            doCarpets(map, tile);
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (dx == 0 && dy == 0) continue;
                    Tile* neighbor = map->getTile(tile->x() + dx, tile->y() + dy, tile->z());
                    if (neighbor) {
                        doCarpets(map, neighbor);
                    }
                }
            }
        }
    }
}

void CarpetBrush::undraw(Map* map, Tile* tile) {
    if (!map || !tile) return;
    QMutableVectorIterator<Item*> it(tile->items());
    bool changed = false;
    while (it.hasNext()) {
        Item* item = it.next();
        if (item && item->isCarpet()) {
            Brush* itemBrush = item->getBrush();
            if (itemBrush == this) {
                it.remove();
                delete item;
                changed = true;
            }
        }
    }
    if (changed) {
        map->markModified();
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                Tile* neighbor_or_self = map->getTile(tile->x() + dx, tile->y() + dy, tile->z());
                if (neighbor_or_self) {
                    doCarpets(map, neighbor_or_self);
                }
            }
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
    if (!map || !tile) return;

    bool tileHasAnyCarpetOfThisBrush = false;
    for(Item* itemOnTile : tile->items()){
        if(itemOnTile && itemOnTile->isCarpet() && itemOnTile->getBrush() == this){
            tileHasAnyCarpetOfThisBrush = true;
            break;
        }
    }
     // If the tile doesn't have a carpet flag and no items from this specific brush,
     // we might still need to process it if a neighbor *was* removed, to update this tile to center.
     // However, the main loop is over items of *this* brush.
    if (!tileHasAnyCarpetOfThisBrush && !tile->hasCarpet()) { // Check generic hasCarpet flag too
         // If no carpet of THIS brush, AND no generic carpet flag, then nothing for this brush to do.
         // If there was a carpet from ANOTHER brush, this one shouldn't interfere.
         // If the generic flag was true but no items of THIS brush, it might be a neighbor update case.
         // The logic of calling doCarpets on neighbors from draw/undraw handles this better.
         // For now, if no carpet of this brush type on tile, return.
        bool processAnyway = false; // Check if any neighbor has this carpet brush
        if (!tileHasAnyCarpetOfThisBrush) {
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (dx == 0 && dy == 0) continue;
                    if (hasMatchingCarpetBrushAtTile(map, this, tile->x() + dx, tile->y() + dy, tile->z())) {
                        processAnyway = true; break;
                    }
                }
                if (processAnyway) break;
            }
        }
        if (!tileHasAnyCarpetOfThisBrush && !processAnyway){
            return;
        }
    }


    const QPoint pos = tile->mapPos().toPoint();
    int x = pos.x();
    int y = pos.y();
    int z = tile->z();

    for (Item* item : tile->items()) {
        if (!item || !item->isCarpet()) continue;

        CarpetBrush* itemCarpetBrush = dynamic_cast<CarpetBrush*>(item->getBrush());
        if (!itemCarpetBrush || itemCarpetBrush != this) continue;

        quint8 neighborConfig = 0;
        if (hasMatchingCarpetBrushAtTile(map, this, x - 1, y - 1, z)) neighborConfig |= QT_TILE_NORTHWEST_CB;
        if (hasMatchingCarpetBrushAtTile(map, this, x,     y - 1, z)) neighborConfig |= QT_TILE_NORTH_CB;
        if (hasMatchingCarpetBrushAtTile(map, this, x + 1, y - 1, z)) neighborConfig |= QT_TILE_NORTHEAST_CB;
        if (hasMatchingCarpetBrushAtTile(map, this, x - 1, y,     z)) neighborConfig |= QT_TILE_WEST_CB;
        if (hasMatchingCarpetBrushAtTile(map, this, x + 1, y,     z)) neighborConfig |= QT_TILE_EAST_CB;
        if (hasMatchingCarpetBrushAtTile(map, this, x - 1, y + 1, z)) neighborConfig |= QT_TILE_SOUTHWEST_CB;
        if (hasMatchingCarpetBrushAtTile(map, this, x,     y + 1, z)) neighborConfig |= QT_TILE_SOUTH_CB;
        if (hasMatchingCarpetBrushAtTile(map, this, x + 1, y + 1, z)) neighborConfig |= QT_TILE_SOUTHEAST_CB;

        quint8 targetAlignmentIndex = s_carpet_types_lookup.value(neighborConfig, CARPET_CENTER_ALIGNMENT_INDEX);

        quint16 newItemId = getRandomCarpetIdByAlignment(targetAlignmentIndex);

        if (newItemId == 0 && targetAlignmentIndex != CARPET_CENTER_ALIGNMENT_INDEX) { // If a border piece is missing, try to place a center
             newItemId = getRandomCarpetIdByAlignment(CARPET_CENTER_ALIGNMENT_INDEX);
        }

        if (newItemId == 0) { // No suitable carpet piece found, remove existing
            QMutableVectorIterator<Item*> it_remove(tile->items());
            while(it_remove.hasNext()){
                if(it_remove.next() == item){
                    it_remove.remove();
                    delete item;
                    map->markModified();
                    break;
                }
            }
        } else if (item->getServerId() != newItemId) {
            item->setServerId(newItemId);
            const ItemProperties& newProps = ItemManager::instance()->getItemProperties(newItemId);
            if(newProps.serverId != 0) item->setClientId(newProps.clientId);
            map->markModified();
        }
    }
    // tile->update(); // Tile::update() should correctly set HasCarpet flag.
}
