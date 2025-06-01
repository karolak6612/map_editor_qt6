#ifndef DRAWINGOPTIONS_H
#define DRAWINGOPTIONS_H

// No external includes needed for these simple types,
// unless a very specific type from another header was used (not the case here).

struct DrawingOptions {
    bool showGround = true;
    bool showItems = true;
    bool showCreatures = true;
    bool showSpawns = true; // For spawn indicators, drawing might be separate from Tile::draw
    bool showEffects = true; // For particle effects, etc. (placeholder for now)
    bool showInvisibleItems = false; // E.g., for GM mode
    bool showTileFlags = false; // E.g., show PZ/NoPVP text, zone info

    // Task 54: Sprite rendering control
    bool useSprites = true; // Enable full sprite rendering instead of placeholders

    int currentFloor = 7; // The Z-level currently being viewed

    bool showHigherFloorsTransparent = true; // For seeing floors above currentFloor
    bool showLowerFloorsTransparent = false; // For seeing floors below currentFloor

    float itemOpacity = 1.0f;
    float creatureOpacity = 1.0f;

    bool highlightSelectedTile = true; // If the tile itself should indicate selection
    bool drawDebugInfo = false;      // For drawing bounding boxes, IDs, etc.

    // Enhanced options from wxWidgets
    float zoom = 1.0f; // Current zoom level
    bool hideItemsWhenZoomed = true; // Hide items at high zoom levels
    bool showPreview = true; // Enable animations and previews
    bool transparentItems = false; // Make items semi-transparent
    bool highlightLockedDoors = false; // Highlight locked doors differently
    bool showTooltips = false; // Enable tooltip generation
    bool showOnlyModified = false; // Show only modified tiles
    bool showAsMinimapColors = false; // Render as minimap colors
    bool showOnlyColors = false; // Show only zone colors
    bool alwaysShowZones = false; // Always show zone indicators
    bool ingame = false; // Ingame rendering mode

    // Task 75: Complete view settings from wxwidgets
    bool ghostItems = false; // Show items on other floors transparently (GHOST_ITEMS)
    bool ghostHigherFloors = false; // Show higher floors transparently (GHOST_HIGHER_FLOORS)
    bool transparentFloors = false; // Make floors transparent (TRANSPARENT_FLOORS)
    bool showGrid = false; // Show grid overlay (SHOW_GRID)
    bool showZones = true; // Show zone areas (SHOW_ZONES)
    bool showLights = false; // Show light sources (SHOW_LIGHTS)
    bool showLightStrength = true; // Show light strength values (SHOW_LIGHT_STR)
    bool showWaypoints = true; // Show waypoints (SHOW_WAYPOINTS)
    bool showHouses = true; // Show house areas (SHOW_HOUSES)
    bool showShade = true; // Show tile shading (SHOW_SHADE)
    bool showSpecialTiles = true; // Show special tiles (SHOW_SPECIAL_TILES)
    bool showTechnicalItems = true; // Show technical items (SHOW_TECHNICAL_ITEMS)
    bool showIngameBox = false; // Show ingame view box (SHOW_INGAME_BOX)
    bool showWallHooks = false; // Show wall hooks (SHOW_WALL_HOOKS)
    bool showTowns = true; // Show town areas (SHOW_TOWNS)
    bool showBlocking = false; // Show blocking/pathing info (SHOW_BLOCKING)
    bool highlightItems = false; // Highlight items (HIGHLIGHT_ITEMS)
    bool showExtra = true; // Show extra items (SHOW_EXTRA)
    bool showAllFloors = true; // Show all floors (SHOW_ALL_FLOORS)
    bool experimentalFog = false; // Experimental fog effect (EXPERIMENTAL_FOG)
    bool extHouseShader = false; // Extended house shader (EXT_HOUSE_SHADER)

    // Zoom thresholds
    float itemDisplayZoomThreshold = 10.0f; // Hide items above this zoom
    float groundOnlyZoomThreshold = 20.0f; // Show only ground above this zoom
    float animationZoomThreshold = 5.0f; // Disable animations above this zoom
    float tooltipMaxZoom = 3.0f; // Maximum zoom for tooltips

    // Color modulation for special effects
    int redModulation = 255;
    int greenModulation = 255;
    int blueModulation = 255;
    int alphaModulation = 255;

    // Default constructor
    DrawingOptions() {
        // Initialize to default sensible values
        showGround = true;
        showItems = true;
        showCreatures = true;
        showSpawns = true;
        showEffects = true;
        showInvisibleItems = false;
        showTileFlags = false;
        useSprites = true; // Task 54: Enable sprite rendering by default
        currentFloor = 7; // Typical ground floor in Tibia
        showHigherFloorsTransparent = true;
        showLowerFloorsTransparent = false;
        itemOpacity = 1.0f;
        creatureOpacity = 1.0f;
        highlightSelectedTile = true;
        drawDebugInfo = false;

        // Enhanced options
        zoom = 1.0f;
        hideItemsWhenZoomed = true;
        showPreview = true;
        transparentItems = false;
        highlightLockedDoors = false;
        showTooltips = false;
        showOnlyModified = false;
        showAsMinimapColors = false;
        showOnlyColors = false;
        alwaysShowZones = false;
        ingame = false;

        // Zoom thresholds
        itemDisplayZoomThreshold = 10.0f;
        groundOnlyZoomThreshold = 20.0f;
        animationZoomThreshold = 5.0f;
        tooltipMaxZoom = 3.0f;

        // Color modulation
        redModulation = 255;
        greenModulation = 255;
        blueModulation = 255;
        alphaModulation = 255;
    }

    // Task 75: Methods for setting view modes
    void setDefaultMode();
    void setIngameMode();
    bool isDrawLight() const;

    // Task 75: Comparison operator for change detection
    bool operator==(const DrawingOptions& other) const;
    bool operator!=(const DrawingOptions& other) const { return !(*this == other); }
};

#endif // DRAWINGOPTIONS_H
