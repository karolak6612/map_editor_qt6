#ifndef EDITORSPRITEIDS_H
#define EDITORSPRITEIDS_H

/**
 * @brief Editor sprite ID enumeration
 * 
 * These IDs correspond to the original wxWidgets editor sprite system
 * and are used to identify specific editor UI graphics.
 */
enum EditorSpriteId {
    // Selection and drawing tools
    EDITOR_SPRITE_SELECTION_MARKER = -1000,
    EDITOR_SPRITE_SELECTION_GEM,
    EDITOR_SPRITE_DRAWING_GEM,
    
    // Brush cursors - Circular
    EDITOR_SPRITE_BRUSH_CD_1x1,
    EDITOR_SPRITE_BRUSH_CD_3x3,
    EDITOR_SPRITE_BRUSH_CD_5x5,
    EDITOR_SPRITE_BRUSH_CD_7x7,
    EDITOR_SPRITE_BRUSH_CD_9x9,
    EDITOR_SPRITE_BRUSH_CD_15x15,
    EDITOR_SPRITE_BRUSH_CD_19x19,
    
    // Brush cursors - Square
    EDITOR_SPRITE_BRUSH_SD_1x1,
    EDITOR_SPRITE_BRUSH_SD_3x3,
    EDITOR_SPRITE_BRUSH_SD_5x5,
    EDITOR_SPRITE_BRUSH_SD_7x7,
    EDITOR_SPRITE_BRUSH_SD_9x9,
    EDITOR_SPRITE_BRUSH_SD_15x15,
    EDITOR_SPRITE_BRUSH_SD_19x19,
    
    // Tool icons
    EDITOR_SPRITE_OPTIONAL_BORDER_TOOL,
    EDITOR_SPRITE_ERASER,
    EDITOR_SPRITE_PZ_TOOL,
    EDITOR_SPRITE_PVPZ_TOOL,
    EDITOR_SPRITE_NOLOG_TOOL,
    EDITOR_SPRITE_ZONE_TOOL,
    EDITOR_SPRITE_NOPVP_TOOL,
    
    // Door types
    EDITOR_SPRITE_DOOR_NORMAL,
    EDITOR_SPRITE_DOOR_LOCKED,
    EDITOR_SPRITE_DOOR_MAGIC,
    EDITOR_SPRITE_DOOR_QUEST,
    EDITOR_SPRITE_DOOR_NORMAL_ALT,
    EDITOR_SPRITE_DOOR_ARCHWAY,
    
    // Window types
    EDITOR_SPRITE_WINDOW_NORMAL,
    EDITOR_SPRITE_WINDOW_HATCH,
    
    // Sentinel value
    EDITOR_SPRITE_LAST
};

/**
 * @brief Resource paths for editor sprites
 * 
 * These paths correspond to Qt resource files (.qrc) or filesystem paths
 * for editor graphics.
 */
struct EditorSpriteResource {
    const char* path16x16;
    const char* path32x32;
    const char* path64x64;
};

/**
 * @brief Editor sprite resource mapping
 * 
 * Maps editor sprite IDs to their resource paths.
 * Paths starting with ":" are Qt resource paths.
 */
extern const EditorSpriteResource EDITOR_SPRITE_RESOURCES[];

/**
 * @brief Get resource paths for an editor sprite ID
 * @param spriteId The editor sprite ID
 * @return Resource paths structure, or nullptr if not found
 */
const EditorSpriteResource* getEditorSpriteResource(EditorSpriteId spriteId);

/**
 * @brief Check if a path is a Qt resource path
 * @param path The path to check
 * @return True if the path starts with ":"
 */
inline bool isQtResourcePath(const QString& path) {
    return path.startsWith(":");
}

#endif // EDITORSPRITEIDS_H
