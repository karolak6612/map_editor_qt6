#ifndef CREATUREPROPERTIES_H
#define CREATUREPROPERTIES_H

#include "Outfit.h"
#include <QString>

struct CreatureProperties {
    int id = 0;
    QString name;
    bool isNpc = false;
    Outfit outfit;
    bool missing = false;
    bool standard = true;
    bool inOtherTileset = false;

    // Default constructor
    CreatureProperties() = default;

    // Copy constructor
    CreatureProperties(const CreatureProperties& other) = default;

    // Assignment operator
    CreatureProperties& operator=(const CreatureProperties& other) = default;

    // Equality operator
    bool operator==(const CreatureProperties& other) const {
        return id == other.id &&
               name == other.name &&
               isNpc == other.isNpc &&
               missing == other.missing &&
               standard == other.standard &&
               inOtherTileset == other.inOtherTileset;
    }
};

#endif // CREATUREPROPERTIES_H
