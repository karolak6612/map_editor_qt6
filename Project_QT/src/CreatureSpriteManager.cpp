#include "CreatureSpriteManager.h"
// GameSprite.h is already included in CreatureSpriteManager.h for Outfit
// #include "GraphicManager.h" // Dependency to be managed if actual GameSprites are fetched
#include <QImage>
#include <QPixmap> // Added for QPixmap
#include <QPainter> 
#include <QDebug>

CreatureSpriteManager::CreatureSpriteManager() {
}

CreatureSpriteManager::~CreatureSpriteManager() {
    clear();
}

void CreatureSpriteManager::clear() {
    m_sprite_pixmap_cache.clear();
}

QString CreatureSpriteManager::generateCacheKey(int looktype, const Outfit* outfit_ptr, int width, int height) const {
    QString key = QString("lt:%1_w:%2_h:%3")
                      .arg(looktype)
                      .arg(width)
                      .arg(height);
    if (outfit_ptr) { 
        key += QString("_head:%1_body:%2_legs:%3_feet:%4")
                   .arg(static_cast<int>(outfit_ptr->lookHead))
                   .arg(static_cast<int>(outfit_ptr->lookBody))
                   .arg(static_cast<int>(outfit_ptr->lookLegs))
                   .arg(static_cast<int>(outfit_ptr->lookFeet));
    }
    return key;
}

QPixmap CreatureSpriteManager::getSpritePixmap(int looktype, int width, int height) {
    Outfit default_outfit; 
    default_outfit.lookType = looktype;
    return getSpritePixmap(looktype, default_outfit, width, height);
}

QPixmap CreatureSpriteManager::getSpritePixmap(int looktype, const Outfit& outfit, int width, int height) {
    QString key = generateCacheKey(looktype, &outfit, width, height);
    
    if (m_sprite_pixmap_cache.contains(key)) {
        return m_sprite_pixmap_cache.value(key);
    }
    
    QPixmap pixmap = createSpritePixmap(looktype, outfit, width, height);
    if (!pixmap.isNull()) {
        m_sprite_pixmap_cache.insert(key, pixmap);
    } else {
        qWarning() << "CreatureSpriteManager: createSpritePixmap returned null for key" << key;
        return QPixmap(width, height); 
    }
    return pixmap;
}

void CreatureSpriteManager::generateCreatureSpritePixmaps(const BrushVector& creatures, int width, int height) {
    for (CreatureBrush* cb : creatures) { 
        if (cb) { 
            const Outfit& creature_outfit = cb->getOutfit();
            int lt = creature_outfit.lookType; 
            if (lt > 0) {
                if (creature_outfit.lookHead || creature_outfit.lookBody || creature_outfit.lookLegs || creature_outfit.lookFeet) {
                    getSpritePixmap(lt, creature_outfit, width, height);
                } else {
                    getSpritePixmap(lt, width, height); 
                }
            }
        }
    }
}

QPixmap CreatureSpriteManager::createSpritePixmap(int looktype, int width, int height) {
    Outfit default_outfit;
    default_outfit.lookType = looktype;
    return createSpritePixmap(looktype, default_outfit, width, height);
}

QPixmap CreatureSpriteManager::createSpritePixmap(int looktype, const Outfit& outfit, int target_width, int target_height) {
    QSharedPointer<GameSprite> base_game_sprite = QSharedPointer<GameSprite>(new GameSprite());
    if (base_game_sprite->getImage().isNull() || base_game_sprite->getImage().size().isEmpty()) {
        QImage dummy_base(32,32, QImage::Format_ARGB32_Premultiplied);
        dummy_base.fill(Qt::cyan); 
        for(int y=0; y < 8; ++y) for(int x=0; x < 8; ++x) dummy_base.setPixelColor(x,y, QColor(255,255,0));
        for(int y=0; y < 8; ++y) for(int x=8; x < 16; ++x) dummy_base.setPixelColor(x,y, QColor(255,0,0));
        for(int y=8; y < 16; ++y) for(int x=0; x < 8; ++x) dummy_base.setPixelColor(x,y, QColor(0,255,0));
        for(int y=8; y < 16; ++y) for(int x=8; x < 16; ++x) dummy_base.setPixelColor(x,y, QColor(0,0,255));
        base_game_sprite->setImage(dummy_base);
        base_game_sprite->sprite_parts.append(dummy_base); 
        base_game_sprite->width = 1; base_game_sprite->height = 1; 
        base_game_sprite->layers = 1; base_game_sprite->frames = 1;
        base_game_sprite->pattern_x =1; base_game_sprite->pattern_y =1; base_game_sprite->pattern_z = 1;
    }

    if (!base_game_sprite || base_game_sprite->getImage().isNull()) { 
        qWarning() << "CreatureSpriteManager: Could not get/create base GameSprite for looktype" << looktype;
        return QPixmap(); 
    }

    QImage source_image_part = base_game_sprite->getSpritePart(0,0,0,0,0,0,0); 
    if(source_image_part.isNull()) source_image_part = base_game_sprite->getImage(); 

    QImage colorized_image = base_game_sprite->colorizeSpritePart(source_image_part, outfit);

    if (colorized_image.isNull()) {
        qWarning() << "CreatureSpriteManager: Colorization resulted in null image for looktype" << looktype;
        return QPixmap();
    }
    
    QImage final_image = colorized_image.scaled(target_width, target_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    if (final_image.width() != target_width || final_image.height() != target_height) {
        QImage exact_size_image(target_width, target_height, QImage::Format_ARGB32_Premultiplied);
        exact_size_image.fill(Qt::transparent);
        QPainter painter(&exact_size_image);
        int draw_x = (target_width - final_image.width()) / 2;
        int draw_y = (target_height - final_image.height()) / 2;
        painter.drawImage(draw_x, draw_y, final_image);
        painter.end();
        final_image = exact_size_image;
    }

    return QPixmap::fromImage(final_image);
}
