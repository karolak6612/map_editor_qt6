#include "CreatureSpriteManager.h"
#include "GameSprite.h" // For Outfit and potentially GameSprite fetching
// #include "GraphicManager.h" // If needed to fetch base sprites (dependency to be managed)
#include <QImage>
#include <QPainter> // For scaling and manipulation if not directly on QImage
#include <QDebug>

// Global instance, similar to g_creature_sprites in wxwidgets
// Consider if a global instance is the best approach in Qt (singleton or dependency injection might be alternatives)
// For a direct port, a global static instance can be declared.
// CreatureSpriteManager g_qt_creature_sprites; // Definition would go here if it's global static.


CreatureSpriteManager::CreatureSpriteManager() {
    // Constructor
    // If depending on GraphicManager, it might be passed here or set via a method.
    // m_graphic_manager = nullptr; // Initialize if it's a member
}

CreatureSpriteManager::~CreatureSpriteManager() {
    clear();
}

void CreatureSpriteManager::clear() {
    m_sprite_image_cache.clear();
}

QString CreatureSpriteManager::generateCacheKey(int looktype, const Outfit* outfit_ptr, int width, int height) const {
    QString key = QString("lt:%1_w:%2_h:%3")
                      .arg(looktype)
                      .arg(width)
                      .arg(height);
    if (outfit_ptr) { // Check if outfit_ptr is not null
        key += QString("_head:%1_body:%2_legs:%3_feet:%4")
                   .arg(outfit_ptr->lookHead)
                   .arg(outfit_ptr->lookBody)
                   .arg(outfit_ptr->lookLegs)
                   .arg(outfit_ptr->lookFeet);
    }
    return key;
}


QImage CreatureSpriteManager::getSpriteImage(int looktype, int width, int height) {
    // For the base version without specific outfit colors, we can pass a default Outfit struct
    // or handle the null outfit_ptr case in generateCacheKey and createSpriteImage.
    Outfit default_outfit; // An outfit with all colors as 0, or just lookType set
    default_outfit.lookType = looktype;
    return getSpriteImage(looktype, default_outfit, width, height);
}

QImage CreatureSpriteManager::getSpriteImage(int looktype, const Outfit& outfit, int width, int height) {
    QString key = generateCacheKey(looktype, &outfit, width, height);
    
    if (m_sprite_image_cache.contains(key)) {
        return m_sprite_image_cache.value(key);
    }
    
    QImage image = createSpriteImage(looktype, outfit, width, height);
    if (!image.isNull()) {
        m_sprite_image_cache.insert(key, image);
    } else {
        qWarning() << "CreatureSpriteManager: createSpriteImage returned null for key" << key;
        // Return a default/placeholder image or an empty QImage
        return QImage(width, height, QImage::Format_ARGB32_Premultiplied); // Example: transparent placeholder
    }
    return image;
}

void CreatureSpriteManager::generateCreatureSpriteImages(const BrushVector& creatures, int width, int height) {
    for (CreatureBrush* cb : creatures) { // Assuming BrushVector is now QVector<CreatureBrush*>
        if (cb) { 
            const Outfit& creature_outfit = cb->getOutfit();
            int lt = creature_outfit.lookType; 
            if (lt > 0) {
                // If outfit has specific colors (non-zero head, body, etc.), generate with them
                if (creature_outfit.lookHead || creature_outfit.lookBody || creature_outfit.lookLegs || creature_outfit.lookFeet) {
                    getSpriteImage(lt, creature_outfit, width, height);
                } else {
                    // Otherwise, generate the base looktype image
                    getSpriteImage(lt, width, height); // This will use the default outfit
                }
            }
        }
    }
}

QImage CreatureSpriteManager::createSpriteImage(int looktype, int width, int height) {
    Outfit default_outfit;
    default_outfit.lookType = looktype;
    // Default colors (0) will mean base sprite in colorizeSpritePart
    return createSpriteImage(looktype, default_outfit, width, height);
}

QImage CreatureSpriteManager::createSpriteImage(int looktype, const Outfit& outfit, int target_width, int target_height) {
    // 1. Get the base GameSprite for the looktype.
    //    This is a critical dependency. For now, we'll simulate getting a GameSprite.
    //    In a real scenario, this would come from GraphicManager or a similar source.
    
    // --- Placeholder for GameSprite acquisition ---
    // This part needs to be replaced with actual logic to get a GameSprite,
    // possibly involving GraphicManager. For now, create a dummy one.
    QSharedPointer<GameSprite> base_game_sprite = QSharedPointer<GameSprite>(new GameSprite());
    // Setup a default image for the dummy GameSprite if it's not already initialized
    if (base_game_sprite->getImage().isNull() || base_game_sprite->getImage().size().isEmpty()) {
        QImage dummy_base(32,32, QImage::Format_ARGB32_Premultiplied);
        dummy_base.fill(Qt::cyan); // Fill with a distinct color for easy identification if it's used
        // Add some template marker colors for colorizeSpritePart to work on
        // Yellow for head (top-left)
        for(int y=0; y < 8; ++y) for(int x=0; x < 8; ++x) dummy_base.setPixelColor(x,y, QColor(255,255,0));
        // Red for body (top-right)
        for(int y=0; y < 8; ++y) for(int x=8; x < 16; ++x) dummy_base.setPixelColor(x,y, QColor(255,0,0));
        // Green for legs (bottom-left)
        for(int y=8; y < 16; ++y) for(int x=0; x < 8; ++x) dummy_base.setPixelColor(x,y, QColor(0,255,0));
        // Blue for feet (bottom-right)
        for(int y=8; y < 16; ++y) for(int x=8; x < 16; ++x) dummy_base.setPixelColor(x,y, QColor(0,0,255));
        base_game_sprite->setImage(dummy_base);
        base_game_sprite->sprite_parts.append(dummy_base); // Ensure it has at least one part
        base_game_sprite->width = 1; base_game_sprite->height = 1; // Assuming 1x1 pattern for base part
        base_game_sprite->layers = 1; base_game_sprite->frames = 1;
        base_game_sprite->pattern_x =1; base_game_sprite->pattern_y =1; base_game_sprite->pattern_z = 1;
    }
    // --- End Placeholder ---


    if (!base_game_sprite || base_game_sprite->getImage().isNull()) { 
        qWarning() << "CreatureSpriteManager: Could not get/create base GameSprite for looktype" << looktype;
        return QImage(); 
    }

    QImage source_image_part = base_game_sprite->getSpritePart(0,0,0,0,0,0,0); 
    if(source_image_part.isNull()) source_image_part = base_game_sprite->getImage(); 


    QImage colorized_image = base_game_sprite->colorizeSpritePart(source_image_part, outfit);

    if (colorized_image.isNull()) {
        qWarning() << "CreatureSpriteManager: Colorization resulted in null image for looktype" << looktype;
        return QImage();
    }
    
    QImage final_image = colorized_image.scaled(target_width, target_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // Ensure the image has the exact target dimensions, padding with transparency if needed (like original)
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

    // Original wxImage version set a mask color (e.g., magenta) for transparency.
    // QImage handles transparency with its alpha channel. If the source images and
    // colorization process correctly maintain alpha, this should be fine.
    // If a specific color needs to become transparent (like magenta in wxwidgets):
    // final_image.convertTo(QImage::Format_ARGB32); // Ensure alpha channel
    // for (int y = 0; y < final_image.height(); ++y) {
    //     for (int x = 0; x < final_image.width(); ++x) {
    //         if (final_image.pixel(x, y) == qRgb(255, 0, 255)) { // Magenta
    //             final_image.setPixel(x, y, Qt::transparent);
    //         }
    //     }
    // }
    return final_image;
}
