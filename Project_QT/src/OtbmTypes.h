#ifndef OTBMTYPES_H
#define OTBMTYPES_H

#include <QtGlobal> // For quint8, etc.

// OTBM Version Constants (Task 60 requirement)
enum OTBM_Version : quint32 {
    MAP_OTBM_1 = 0,  // OTBM Version 1 (legacy)
    MAP_OTBM_2 = 1,  // OTBM Version 2 (improved item handling)
    MAP_OTBM_3 = 2,  // OTBM Version 3 (waypoints support)
    MAP_OTBM_4 = 3   // OTBM Version 4 (attribute map support)
};

// Client Version Constants for compatibility
enum ClientVersion : quint32 {
    CLIENT_VERSION_750 = 750,
    CLIENT_VERSION_760 = 760,
    CLIENT_VERSION_770 = 770,
    CLIENT_VERSION_780 = 780,
    CLIENT_VERSION_790 = 790,
    CLIENT_VERSION_792 = 792,
    CLIENT_VERSION_800 = 800,
    CLIENT_VERSION_810 = 810,
    CLIENT_VERSION_811 = 811,
    CLIENT_VERSION_820 = 820,
    CLIENT_VERSION_830 = 830,
    CLIENT_VERSION_840 = 840,
    CLIENT_VERSION_850 = 850,
    CLIENT_VERSION_854 = 854,
    CLIENT_VERSION_860 = 860,
    CLIENT_VERSION_870 = 870,
    CLIENT_VERSION_910 = 910,
    CLIENT_VERSION_940 = 940,
    CLIENT_VERSION_944 = 944,
    CLIENT_VERSION_953 = 953,
    CLIENT_VERSION_960 = 960,
    CLIENT_VERSION_961 = 961,
    CLIENT_VERSION_963 = 963,
    CLIENT_VERSION_970 = 970,
    CLIENT_VERSION_980 = 980,
    CLIENT_VERSION_986 = 986,
    CLIENT_VERSION_1010 = 1010,
    CLIENT_VERSION_1020 = 1020,
    CLIENT_VERSION_1021 = 1021,
    CLIENT_VERSION_1030 = 1030,
    CLIENT_VERSION_1031 = 1031,
    CLIENT_VERSION_1035 = 1035,
    CLIENT_VERSION_1036 = 1036,
    CLIENT_VERSION_1038 = 1038,
    CLIENT_VERSION_1057 = 1057,
    CLIENT_VERSION_1058 = 1058,
    CLIENT_VERSION_1059 = 1059,
    CLIENT_VERSION_1060 = 1060,
    CLIENT_VERSION_1061 = 1061,
    CLIENT_VERSION_1062 = 1062,
    CLIENT_VERSION_1063 = 1063,
    CLIENT_VERSION_1064 = 1064,
    CLIENT_VERSION_1092 = 1092,
    CLIENT_VERSION_1093 = 1093,
    CLIENT_VERSION_1094 = 1094,
    CLIENT_VERSION_1095 = 1095,
    CLIENT_VERSION_1096 = 1096,
    CLIENT_VERSION_1097 = 1097,
    CLIENT_VERSION_1098 = 1098,
    CLIENT_VERSION_1099 = 1099,
    CLIENT_VERSION_1100 = 1100,
    CLIENT_VERSION_1110 = 1110,
    CLIENT_VERSION_1132 = 1132,
    CLIENT_VERSION_1140 = 1140,
    CLIENT_VERSION_1150 = 1150,
    CLIENT_VERSION_1171 = 1171,
    CLIENT_VERSION_1180 = 1180,
    CLIENT_VERSION_1185 = 1185,
    CLIENT_VERSION_1188 = 1188,
    CLIENT_VERSION_1200 = 1200,
    CLIENT_VERSION_1210 = 1210,
    CLIENT_VERSION_1215 = 1215,
    CLIENT_VERSION_1220 = 1220,
    CLIENT_VERSION_1240 = 1240,
    CLIENT_VERSION_1250 = 1250,
    CLIENT_VERSION_1260 = 1260,
    CLIENT_VERSION_1270 = 1270,
    CLIENT_VERSION_1280 = 1280,
    CLIENT_VERSION_1281 = 1281,
    CLIENT_VERSION_1300 = 1300
};

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
