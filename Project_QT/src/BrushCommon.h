#ifndef BRUSHCOMMON_H
#define BRUSHCOMMON_H

// QObject include might be needed if Q_ENUMS are used directly in this file with a Q_NAMESPACE
// For now, this file only contains C++ enum definitions.
// If these enums need to be available to Qt's meta-object system directly from this file
// without being nested in a Q_OBJECT class, a Q_NAMESPACE would be needed.
// Alternatively, Q_ENUM can be used in classes that use these enums as properties.

enum class WallAlignment {
    Undefined = 0,      // Default or no specific alignment
    Vertical,           // |
    Horizontal,         // -

    // Corners (L-shape, opening indicates direction)
    NorthWest_Corner,   // L shape, open to SE:  _|
    NorthEast_Corner,   // Backwards L, open to SW: |_
    SouthWest_Corner,   // Mirrored L, open to NE:  ‾|
    SouthEast_Corner,   // Mirrored Backwards L, open to NW: |‾

    // T-junctions (stem points in direction)
    North_T,            // T shape, stem points North: T
    South_T,            // Upside down T, stem points South: ⊥
    East_T,             // T rotated right, stem points East: vdash (or similar representation)
    West_T,             // T rotated left, stem points West: dashv (or similar representation)

    Intersection,       // + Cross-shape

    Pole,               // Single standing segment, like a pillar or fence post

    // Diagonal types - these often represent specific items rather than standard wall pieces
    // Their auto-bordering behavior can be complex.
    // Names indicate the slope direction.
    NorthWest_Diagonal, // \ (visual slope from top-left down to bottom-right)
    NorthEast_Diagonal, // / (visual slope from top-right down to bottom-left)
    // SouthWest_Diagonal and SouthEast_Diagonal are visually identical to NorthEast and NorthWest respectively
    // if they represent the same kind of diagonal but placed differently.
    // Often, just two diagonal types are needed, representing the two main slash directions.

    // Ends - representing the termination of a wall segment
    North_End,          // Wall segment ending, open to the North
    South_End,          // Wall segment ending, open to the South
    East_End,           // Wall segment ending, open to the East
    West_End,           // Wall segment ending, open to the West

    Untouchable         // A wall piece that auto-bordering algorithms should not change
};

enum class DoorTypeQt {
    Undefined = 0,      // Corresponds to wx WALL_UNDEFINED or no door/window
    Normal,             // WALL_DOOR_NORMAL
    Locked,             // WALL_DOOR_LOCKED
    Magic,              // WALL_DOOR_MAGIC
    Quest,              // WALL_DOOR_QUEST
    NormalAlt,          // WALL_DOOR_NORMAL_ALT (alternative appearance)
    Archway,            // WALL_ARCHWAY
    Window,             // WALL_WINDOW
    HatchWindow         // WALL_HATCH_WINDOW
    // Add any other specific door/window types if identified from wx sources
};

// Enum for specific border piece alignments, corresponding to indices 0-12
// that wxAutoBorder::tiles array likely used.
// Based on wxwidgets/brush_enums.h BorderType values and AutoBorder::edgeNameToID
enum class BorderEdgeType {
    InvalidOrNone = 0, // Corresponds to BORDER_NONE or an unused slot

    NorthHorizontal = 1,    // wx BORDER_TYPE NORTH_HORIZONTAL
    EastHorizontal = 2,     // wx BORDER_TYPE EAST_HORIZONTAL (usually means a vertical segment on the East side)
    SouthHorizontal = 3,    // wx BORDER_TYPE SOUTH_HORIZONTAL
    WestHorizontal = 4,     // wx BORDER_TYPE WEST_HORIZONTAL (usually means a vertical segment on the West side)
    NorthWestCorner = 5,    // wx BORDER_TYPE NORTHWEST_CORNER
    NorthEastCorner = 6,    // wx BORDER_TYPE NORTHEAST_CORNER
    SouthWestCorner = 7,    // wx BORDER_TYPE SOUTHWEST_CORNER
    SouthEastCorner = 8,    // wx BORDER_TYPE SOUTHEAST_CORNER
    NorthWestDiagonal = 9,  // wx BORDER_TYPE NORTHWEST_DIAGONAL ( \ )
    NorthEastDiagonal = 10, // wx BORDER_TYPE NORTHEAST_DIAGONAL ( / )
    SouthEastDiagonal = 11, // wx BORDER_TYPE SOUTHEAST_DIAGONAL ( \ )
    SouthWestDiagonal = 12, // wx BORDER_TYPE SOUTHWEST_DIAGONAL ( / )

    MaxEdgeTypes // Helper for array sizing or iteration. Value will be 13.
};


// Other common brush-related enums or structs could go here in the future.

#endif // BRUSHCOMMON_H
