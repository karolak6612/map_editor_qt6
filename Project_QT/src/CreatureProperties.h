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
};

#endif // CREATUREPROPERTIES_H
