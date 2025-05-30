#ifndef QT_SPRITELIGHT_H
#define QT_SPRITELIGHT_H

#include <QtGlobal> // For quint8

struct SpriteLight {
    quint8 intensity = 0;
    quint8 color = 0;

    // Default constructor is fine
    // Optionally, add a constructor for convenience:
    // SpriteLight(quint8 i = 0, quint8 c = 0) : intensity(i), color(c) {}
};

#endif // QT_SPRITELIGHT_H
