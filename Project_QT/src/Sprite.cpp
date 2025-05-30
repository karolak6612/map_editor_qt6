#include "Sprite.h"

// Sprite.cpp for the abstract Sprite interface.

// Since Sprite is an abstract class where:
// - Constructor is defaulted: Sprite() = default; (in header)
// - Destructor is virtual and defaulted: virtual ~Sprite() = default; (in header)
// - All other core methods are pure virtual.
// - Copy/move operations are deleted.
//
// This .cpp file is primarily to ensure the class's compilation unit exists
// and to provide a location for potential out-of-line definitions of
// static members or non-pure virtual methods if they were added to Sprite in the future.
//
// For a defaulted virtual destructor in C++, if it's not defined inline in the header
// (e.g. `virtual ~Sprite() = default;` inside the class body), it requires an out-of-line
// definition, even if it's just `= default;`. Some compilers/linkers might require this
// for the vtable. Given `= default;` is in the header, this is often treated as inline.
//
// If linking issues were to arise related to ~Sprite, an out-of-line defaulted destructor would be:
// Sprite::~Sprite() = default;
//
// For now, this minimal content is sufficient.
