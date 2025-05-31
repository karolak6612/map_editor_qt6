#include "Brush.h"
#include "MapView.h" // Include MapView if used in method signatures or implementations
// Intentionally not including TableBrush.h, CarpetBrush.h, or OptionalBorderBrush.h for default implementations

// static uint32_t Brush::id_counter = 0; // If using static ID counter

Brush::Brush(QObject *parent) : QObject(parent) /*, id_(++id_counter)*/ {
    // Constructor
}

Brush::~Brush() {
    // Destructor
}

// Default implementations for virtual methods from wxBrush

int Brush::getLookID() const {
    return 0; // Default, subclasses should override
}

bool Brush::needBorders() const {
    return false;
}

bool Brush::canDrag() const {
    return false;
}

bool Brush::canSmear() const {
    // Original wxBrush defaulted to true, but for Qt, let's default to false
    // as smearing is a specific behavior. Subclasses can override.
    return false;
}

bool Brush::oneSizeFitsAll() const {
    return false;
}

int Brush::getMaxVariation() const {
    return 0;
}

// Default implementations for is[Type]() methods
bool Brush::isRaw() const { return false; }
bool Brush::isDoodad() const { return false; }
bool Brush::isTerrain() const { return false; }
bool Brush::isGround() const { return false; }
bool Brush::isWall() const { return false; }
bool Brush::isWallDecoration() const { return false; }
bool Brush::isTable() const { return false; }
bool Brush::isCarpet() const { return false; }
bool Brush::isOptionalBorder() const { return false; } // <-- Add this
// ... (add other is[Type] from wxBrush as needed, defaulting to false) ...
bool Brush::isEraser() const { return false; }


// Default implementations for as[Type]() methods
TableBrush* Brush::asTable() { return nullptr; }
const TableBrush* Brush::asTable() const { return nullptr; }

CarpetBrush* Brush::asCarpet() { return nullptr; }
const CarpetBrush* Brush::asCarpet() const { return nullptr; }

OptionalBorderBrush* Brush::asOptionalBorder() { return nullptr; } // <-- Add this
const OptionalBorderBrush* Brush::asOptionalBorder() const { return nullptr; } // <-- Add this


// Implement other common methods or provide default behaviors here if any

// Note: The pure virtual functions (mouse events, name, type, applyBrush, etc.)
// do not have implementations in the base class cpp file.
