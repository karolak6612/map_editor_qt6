// DrawingOptions.cpp - Implementation of drawing options methods

#include "DrawingOptions.h"

// Task 75: Set default view mode (equivalent to wxwidgets SetDefault)
void DrawingOptions::setDefaultMode() {
    // Basic display options
    showGround = true;
    showItems = true;
    showCreatures = true;
    showSpawns = true;
    showEffects = true;
    showInvisibleItems = false;
    showTileFlags = false;
    
    // Floor display
    showAllFloors = true;
    showHigherFloorsTransparent = true;
    showLowerFloorsTransparent = false;
    
    // Transparency settings
    transparentFloors = false;
    transparentItems = false;
    ghostItems = false;
    ghostHigherFloors = false;
    
    // Visual elements
    showGrid = false;
    showZones = true;
    showLights = false;
    showLightStrength = true;
    showWaypoints = true;
    showHouses = true;
    showShade = true;
    showSpecialTiles = true;
    showTechnicalItems = true;
    showIngameBox = false;
    showWallHooks = false;
    showTowns = true;
    showBlocking = false;
    
    // Highlighting
    highlightItems = false;
    highlightLockedDoors = true;
    highlightSelectedTile = true;
    
    // Special modes
    showExtra = true;
    showPreview = false;
    showTooltips = false;
    showAsMinimapColors = false;
    showOnlyColors = false;
    showOnlyModified = false;
    alwaysShowZones = false;
    
    // Technical settings
    ingame = false;
    hideItemsWhenZoomed = true;
    useSprites = true;
    
    // Effects
    experimentalFog = false;
    extHouseShader = false;
    
    // Opacity settings
    itemOpacity = 1.0f;
    creatureOpacity = 1.0f;
    
    // Debug
    drawDebugInfo = false;
}

// Task 75: Set ingame view mode (equivalent to wxwidgets SetIngame)
void DrawingOptions::setIngameMode() {
    // Basic display options
    showGround = true;
    showItems = true;
    showCreatures = true;
    showSpawns = false; // Hide spawns in ingame mode
    showEffects = true;
    showInvisibleItems = false;
    showTileFlags = false;
    
    // Floor display
    showAllFloors = true;
    showHigherFloorsTransparent = true;
    showLowerFloorsTransparent = false;
    
    // Transparency settings
    transparentFloors = false;
    transparentItems = false;
    ghostItems = false;
    ghostHigherFloors = false;
    
    // Visual elements (most disabled for ingame view)
    showGrid = false;
    showZones = false; // Hide zones in ingame mode
    showLights = false;
    showLightStrength = false;
    showWaypoints = false; // Hide waypoints in ingame mode
    showHouses = false; // Hide houses in ingame mode
    showShade = false; // Disable shading in ingame mode
    showSpecialTiles = false; // Hide special tiles in ingame mode
    showTechnicalItems = false; // Hide technical items in ingame mode
    showIngameBox = false;
    showWallHooks = false;
    showTowns = false;
    showBlocking = false;
    
    // Highlighting (disabled for ingame)
    highlightItems = false;
    highlightLockedDoors = false;
    highlightSelectedTile = false; // No selection in ingame mode
    
    // Special modes
    showExtra = true;
    showPreview = false;
    showTooltips = false;
    showAsMinimapColors = false;
    showOnlyColors = false;
    showOnlyModified = false;
    alwaysShowZones = false;
    
    // Technical settings
    ingame = true; // Mark as ingame mode
    hideItemsWhenZoomed = false; // Don't hide items when zoomed in ingame
    useSprites = true;
    
    // Effects
    experimentalFog = false;
    extHouseShader = false;
    
    // Opacity settings
    itemOpacity = 1.0f;
    creatureOpacity = 1.0f;
    
    // Debug
    drawDebugInfo = false;
}

// Task 75: Check if light drawing is enabled
bool DrawingOptions::isDrawLight() const {
    return showLights && !ingame;
}

// Task 75: Comparison operator for change detection
bool DrawingOptions::operator==(const DrawingOptions& other) const {
    return showGround == other.showGround &&
           showItems == other.showItems &&
           showCreatures == other.showCreatures &&
           showSpawns == other.showSpawns &&
           showEffects == other.showEffects &&
           showInvisibleItems == other.showInvisibleItems &&
           showTileFlags == other.showTileFlags &&
           useSprites == other.useSprites &&
           currentFloor == other.currentFloor &&
           showHigherFloorsTransparent == other.showHigherFloorsTransparent &&
           showLowerFloorsTransparent == other.showLowerFloorsTransparent &&
           itemOpacity == other.itemOpacity &&
           creatureOpacity == other.creatureOpacity &&
           highlightSelectedTile == other.highlightSelectedTile &&
           drawDebugInfo == other.drawDebugInfo &&
           zoom == other.zoom &&
           hideItemsWhenZoomed == other.hideItemsWhenZoomed &&
           showPreview == other.showPreview &&
           transparentItems == other.transparentItems &&
           highlightLockedDoors == other.highlightLockedDoors &&
           showTooltips == other.showTooltips &&
           showOnlyModified == other.showOnlyModified &&
           showAsMinimapColors == other.showAsMinimapColors &&
           showOnlyColors == other.showOnlyColors &&
           alwaysShowZones == other.alwaysShowZones &&
           ingame == other.ingame &&
           ghostItems == other.ghostItems &&
           ghostHigherFloors == other.ghostHigherFloors &&
           transparentFloors == other.transparentFloors &&
           showGrid == other.showGrid &&
           showZones == other.showZones &&
           showLights == other.showLights &&
           showLightStrength == other.showLightStrength &&
           showWaypoints == other.showWaypoints &&
           showHouses == other.showHouses &&
           showShade == other.showShade &&
           showSpecialTiles == other.showSpecialTiles &&
           showTechnicalItems == other.showTechnicalItems &&
           showIngameBox == other.showIngameBox &&
           showWallHooks == other.showWallHooks &&
           showTowns == other.showTowns &&
           showBlocking == other.showBlocking &&
           highlightItems == other.highlightItems &&
           showExtra == other.showExtra &&
           showAllFloors == other.showAllFloors &&
           experimentalFog == other.experimentalFog &&
           extHouseShader == other.extHouseShader &&
           itemDisplayZoomThreshold == other.itemDisplayZoomThreshold &&
           groundOnlyZoomThreshold == other.groundOnlyZoomThreshold &&
           animationZoomThreshold == other.animationZoomThreshold &&
           tooltipMaxZoom == other.tooltipMaxZoom &&
           redModulation == other.redModulation &&
           greenModulation == other.greenModulation &&
           blueModulation == other.blueModulation &&
           alphaModulation == other.alphaModulation;
}
