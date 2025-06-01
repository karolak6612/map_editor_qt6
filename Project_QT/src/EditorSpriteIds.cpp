#include "EditorSpriteIds.h"
#include <QMap>

/**
 * @brief Editor sprite resource mapping
 * 
 * This table maps editor sprite IDs to their resource paths.
 * Paths starting with ":" are Qt resource paths from .qrc files.
 * Other paths are filesystem paths.
 */
const EditorSpriteResource EDITOR_SPRITE_RESOURCES[] = {
    // Selection and drawing tools
    { ":/icons/selection_marker_16.png", ":/icons/selection_marker_32.png", nullptr },  // EDITOR_SPRITE_SELECTION_MARKER
    { ":/icons/gem_edit.png", nullptr, nullptr },                                        // EDITOR_SPRITE_SELECTION_GEM
    { ":/icons/gem_move.png", nullptr, nullptr },                                        // EDITOR_SPRITE_DRAWING_GEM
    
    // Brush cursors - Circular
    { ":/brushes/circular_1_small.png", ":/brushes/circular_1.png", nullptr },          // EDITOR_SPRITE_BRUSH_CD_1x1
    { ":/brushes/circular_2_small.png", ":/brushes/circular_2.png", nullptr },          // EDITOR_SPRITE_BRUSH_CD_3x3
    { ":/brushes/circular_3_small.png", ":/brushes/circular_3.png", nullptr },          // EDITOR_SPRITE_BRUSH_CD_5x5
    { ":/brushes/circular_4_small.png", ":/brushes/circular_4.png", nullptr },          // EDITOR_SPRITE_BRUSH_CD_7x7
    { ":/brushes/circular_5_small.png", ":/brushes/circular_5.png", nullptr },          // EDITOR_SPRITE_BRUSH_CD_9x9
    { ":/brushes/circular_6_small.png", ":/brushes/circular_6.png", nullptr },          // EDITOR_SPRITE_BRUSH_CD_15x15
    { ":/brushes/circular_7_small.png", ":/brushes/circular_7.png", nullptr },          // EDITOR_SPRITE_BRUSH_CD_19x19
    
    // Brush cursors - Square
    { ":/brushes/square_1_small.png", ":/brushes/square_1.png", nullptr },              // EDITOR_SPRITE_BRUSH_SD_1x1
    { ":/brushes/square_2_small.png", ":/brushes/square_2.png", nullptr },              // EDITOR_SPRITE_BRUSH_SD_3x3
    { ":/brushes/square_3_small.png", ":/brushes/square_3.png", nullptr },              // EDITOR_SPRITE_BRUSH_SD_5x5
    { ":/brushes/square_4_small.png", ":/brushes/square_4.png", nullptr },              // EDITOR_SPRITE_BRUSH_SD_7x7
    { ":/brushes/square_5_small.png", ":/brushes/square_5.png", nullptr },              // EDITOR_SPRITE_BRUSH_SD_9x9
    { ":/brushes/square_6_small.png", ":/brushes/square_6.png", nullptr },              // EDITOR_SPRITE_BRUSH_SD_15x15
    { ":/brushes/square_7_small.png", ":/brushes/square_7.png", nullptr },              // EDITOR_SPRITE_BRUSH_SD_19x19
    
    // Tool icons
    { ":/tools/optional_border_small.png", ":/tools/optional_border.png", nullptr },    // EDITOR_SPRITE_OPTIONAL_BORDER_TOOL
    { ":/tools/eraser_small.png", ":/tools/eraser.png", nullptr },                      // EDITOR_SPRITE_ERASER
    { ":/tools/pz_small.png", ":/tools/pz.png", nullptr },                              // EDITOR_SPRITE_PZ_TOOL
    { ":/tools/pvpz_small.png", ":/tools/pvpz.png", nullptr },                          // EDITOR_SPRITE_PVPZ_TOOL
    { ":/tools/no_logout_small.png", ":/tools/no_logout.png", nullptr },               // EDITOR_SPRITE_NOLOG_TOOL
    { ":/tools/zone_brush_small.png", ":/tools/zone_brush_zone.png", nullptr },        // EDITOR_SPRITE_ZONE_TOOL
    { ":/tools/no_pvp_small.png", ":/tools/no_pvp.png", nullptr },                     // EDITOR_SPRITE_NOPVP_TOOL
    
    // Door types
    { ":/doors/door_normal_small.png", ":/doors/door_normal.png", nullptr },            // EDITOR_SPRITE_DOOR_NORMAL
    { ":/doors/door_locked_small.png", ":/doors/door_locked.png", nullptr },           // EDITOR_SPRITE_DOOR_LOCKED
    { ":/doors/door_magic_small.png", ":/doors/door_magic.png", nullptr },             // EDITOR_SPRITE_DOOR_MAGIC
    { ":/doors/door_quest_small.png", ":/doors/door_quest.png", nullptr },             // EDITOR_SPRITE_DOOR_QUEST
    { ":/doors/door_normal_alt_small.png", ":/doors/door_normal_alt.png", nullptr },   // EDITOR_SPRITE_DOOR_NORMAL_ALT
    { ":/doors/door_archway_small.png", ":/doors/door_archway.png", nullptr },         // EDITOR_SPRITE_DOOR_ARCHWAY
    
    // Window types
    { ":/windows/window_normal_small.png", ":/windows/window_normal.png", nullptr },    // EDITOR_SPRITE_WINDOW_NORMAL
    { ":/windows/window_hatch_small.png", ":/windows/window_hatch.png", nullptr },     // EDITOR_SPRITE_WINDOW_HATCH
};

// Static map for fast lookup
static QMap<EditorSpriteId, const EditorSpriteResource*> s_resourceMap;
static bool s_mapInitialized = false;

static void initializeResourceMap() {
    if (s_mapInitialized) return;
    
    // Calculate the number of entries
    const int numEntries = sizeof(EDITOR_SPRITE_RESOURCES) / sizeof(EditorSpriteResource);
    
    // Map each sprite ID to its resource
    EditorSpriteId currentId = EDITOR_SPRITE_SELECTION_MARKER;
    for (int i = 0; i < numEntries && currentId < EDITOR_SPRITE_LAST; ++i) {
        s_resourceMap[currentId] = &EDITOR_SPRITE_RESOURCES[i];
        currentId = static_cast<EditorSpriteId>(static_cast<int>(currentId) + 1);
    }
    
    s_mapInitialized = true;
}

const EditorSpriteResource* getEditorSpriteResource(EditorSpriteId spriteId) {
    if (!s_mapInitialized) {
        initializeResourceMap();
    }
    
    return s_resourceMap.value(spriteId, nullptr);
}
