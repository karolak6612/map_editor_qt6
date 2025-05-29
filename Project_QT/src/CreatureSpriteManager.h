#ifndef CREATURESPRITEMANAGER_H
#define CREATURESPRITEMANAGER_H

#include <QMap>
#include <QString>
#include <QSharedPointer> 
#include <QVector> 
#include <QImage>  
#include <QPixmap> 

#include "GameSprite.h" 

class Brush {}; 
class CreatureBrush : public Brush { 
public:
    int lookType = 0; 
    Outfit m_outfit; 
    
    CreatureBrush(int type, const Outfit& outfit_details) : lookType(type), m_outfit(outfit_details) {}

    bool isCreature() const { return true; } 
    const Outfit& getOutfit() const { return m_outfit; } 
    int getLookType() const { return m_outfit.lookType; }
};
using BrushVector = QVector<CreatureBrush*>; 

class CreatureSpriteManager {
public:
    CreatureSpriteManager();
    ~CreatureSpriteManager();

    QPixmap getSpritePixmap(int looktype, int width = 32, int height = 32);
    QPixmap getSpritePixmap(int looktype, const Outfit& outfit, int width = 32, int height = 32);
    
    void generateCreatureSpritePixmaps(const BrushVector& creatures, int width = 32, int height = 32); 
    void clear();

private:
    QMap<QString, QPixmap> m_sprite_pixmap_cache; 
    
    QPixmap createSpritePixmap(int looktype, int width, int height); 
    QPixmap createSpritePixmap(int looktype, const Outfit& outfit, int width, int height); 

    QString generateCacheKey(int looktype, const Outfit* outfit_ptr, int width, int height) const;
};

#endif // CREATURESPRITEMANAGER_H
