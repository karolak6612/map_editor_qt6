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
    
    int currentFloor = 7; // The Z-level currently being viewed
    
    bool showHigherFloorsTransparent = true; // For seeing floors above currentFloor
    bool showLowerFloorsTransparent = false; // For seeing floors below currentFloor
    
    float itemOpacity = 1.0f;
    float creatureOpacity = 1.0f;
    
    bool highlightSelectedTile = true; // If the tile itself should indicate selection

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
        currentFloor = 7; // Typical ground floor in Tibia
        showHigherFloorsTransparent = true;
        showLowerFloorsTransparent = false;
        itemOpacity = 1.0f;
        creatureOpacity = 1.0f;
        highlightSelectedTile = true;
    }
};

#endif // DRAWINGOPTIONS_H
