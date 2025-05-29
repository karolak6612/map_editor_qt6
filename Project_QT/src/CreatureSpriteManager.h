#ifndef CREATURESPRITEMANAGER_H
#define CREATURESPRITEMANAGER_H

#include <QMap>
#include <QString>
#include <QSharedPointer> // For managing GameSprite objects
#include <QVector> // For BrushVector
#include <QImage>  // For QImage

// Forward declarations
class GameSprite;
// struct Outfit; // Already in GameSprite.h, assuming it's included where needed.
#include "GameSprite.h" // Include for Outfit struct, if not forward declared and used by value/ref

// If BrushVector and CreatureBrush are complex, they need their own headers.
// For now, providing simple placeholders.
class Brush {}; // Base Brush placeholder
class CreatureBrush : public Brush { // CreatureBrush placeholder
public:
    // Example members based on wxwidgets/creature_brush.h
    int lookType = 0; // Should be part of Outfit in a cleaner design
    Outfit m_outfit; // Store the actual outfit object
    
    CreatureBrush(int type, const Outfit& outfit_details) : lookType(type), m_outfit(outfit_details) {}

    bool isCreature() const { return true; } // Example method
    const Outfit& getOutfit() const { return m_outfit; } 
    int getLookType() const { return m_outfit.lookType; }
};
using BrushVector = QVector<CreatureBrush*>; // Using QVector of CreatureBrush pointers


class CreatureSpriteManager {
public:
    CreatureSpriteManager();
    ~CreatureSpriteManager();

    QImage getSpriteImage(int looktype, int width = 32, int height = 32);
    QImage getSpriteImage(int looktype, const Outfit& outfit, int width = 32, int height = 32);
    
    void generateCreatureSpriteImages(const BrushVector& creatures, int width = 32, int height = 32);
    void clear();

private:
    QMap<QString, QImage> m_sprite_image_cache; 
    
    QImage createSpriteImage(int looktype, int width, int height); // Base without specific outfit
    QImage createSpriteImage(int looktype, const Outfit& outfit, int width, int height); // With outfit

    // Dependency: Needs access to GameSprite definitions and potentially GraphicManager
    // to get base GameSprite objects. 
    // For now, assume it can create/get them or it's passed in.
    // GraphicManager* m_graphic_manager; // Example: if it needs to fetch base sprites

    QString generateCacheKey(int looktype, const Outfit* outfit_ptr, int width, int height) const;
};


#endif // CREATURESPRITEMANAGER_H
