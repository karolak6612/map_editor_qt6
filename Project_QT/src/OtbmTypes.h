#ifndef OTBMTYPES_H
#define OTBMTYPES_H

#include <QtGlobal> // For quint8, etc.

// OTBM Node Types (based on wxwidgets/iomap_otbm.h)
enum OTBM_NodeTypes_t : quint8 {
    OTBM_ROOTV1          = 1,
    OTBM_MAP_DATA        = 2,
    OTBM_ITEM_DEF        = 3,
    OTBM_TILE_AREA       = 4,
    OTBM_TILE            = 5,
    OTBM_ITEM            = 6,
    OTBM_TILE_SQUARE     = 7,
    OTBM_TILE_REF        = 8,
    OTBM_SPAWNS          = 9,
    OTBM_SPAWN_AREA      = 10,
    OTBM_MONSTER         = 11,
    OTBM_TOWNS           = 12,
    OTBM_TOWN            = 13,
    OTBM_HOUSETILE       = 14,
    OTBM_WAYPOINTS       = 15,
    OTBM_WAYPOINT        = 16
};

// OTBM Root Node Attributes (often for versioning)
enum OTBM_RootAttribute : quint8 {
    OTBM_ROOT_ATTR_VERSION_MAJOR    = 160,
    OTBM_ROOT_ATTR_VERSION_MINOR    = 161,
    OTBM_ROOT_ATTR_VERSION_BUILD    = 162,
    OTBM_ROOT_ATTR_VERSION_DESC_STRING = 163 // Client/File format version description
};

// OTBM Item Attributes (based on wxwidgets/iomap_otbm.h)
enum OTBM_ItemAttribute : quint8 {
    // Using names exactly as in wxwidgets/iomap_otbm.h for direct mapping
    OTBM_ATTR_DESCRIPTION      = 1,
    OTBM_ATTR_EXT_FILE         = 2,
    OTBM_ATTR_TILE_FLAGS       = 3,
    OTBM_ATTR_ACTION_ID        = 4,
    OTBM_ATTR_UNIQUE_ID        = 5,
    OTBM_ATTR_TEXT             = 6,
    OTBM_ATTR_DESC             = 7, // Often used for detailed descriptions
    OTBM_ATTR_TELE_DEST        = 8,
    OTBM_ATTR_ITEM             = 9, // For items within container attributes (older formats)
    OTBM_ATTR_DEPOT_ID         = 10,
    OTBM_ATTR_EXT_SPAWN_FILE   = 11,
    OTBM_ATTR_RUNE_CHARGES     = 12,
    OTBM_ATTR_EXT_HOUSE_FILE   = 13,
    OTBM_ATTR_HOUSEDOORID      = 14,
    OTBM_ATTR_COUNT            = 15,
    OTBM_ATTR_DURATION         = 16,
    OTBM_ATTR_DECAYING_STATE   = 17,
    OTBM_ATTR_WRITTENDATE      = 18,
    OTBM_ATTR_WRITTENBY        = 19,
    OTBM_ATTR_SLEEPERGUID      = 20,
    OTBM_ATTR_SLEEPSTART       = 21,
    OTBM_ATTR_CHARGES          = 22,
    // OTBM_ATTR_EXT_SPAWN_NPC_FILE = 23, // Canary RME specific, might not be needed generally
    OTBM_ATTR_PODIUMOUTFIT     = 40, // From wxwidgets list
    OTBM_ATTR_TIER             = 41, // From wxwidgets list
    OTBM_ATTR_ATTRIBUTE_MAP    = 128 // From wxwidgets list (for TFS 1.x style extended attributes)
};

#endif // OTBMTYPES_H
