#include "GameSprite.h"
#include <QPainter>
#include <QBitmap> // For mask operations if needed

// Initialize static member
const QVector<QRgb> GameSprite::TemplateOutfitLookupTable = GameSprite::initializeLookupTable();

QVector<QRgb> GameSprite::initializeLookupTable() {
    // Copied from wxwidgets/graphics.cpp TemplateOutfitLookupTable
    // Qt uses ARGB for QRgb, so 0xFF prefix for opaque alpha might be needed if directly used as QRgb.
    // However, this table seems to store RGB values, which QColor can handle.
    // For direct QRgb, ensure format matches or convert.
    // Using QColor to parse hex and then to QRgb ensures correctness.
    QVector<QRgb> table;
    const uint32_t rawTable[] = {
        0xFFFFFF, 0xFFD4BF, 0xFFE9BF, 0xFFFFBF, 0xE9FFBF, 0xD4FFBF, 0xBFFFBF, 0xBFFFD4,
        0xBFFFE9, 0xBFFFFF, 0xBFE9FF, 0xBFD4FF, 0xBFBFFF, 0xD4BFFF, 0xE9BFFF, 0xFFBFFF,
        0xFFBFE9, 0xFFBFD4, 0xFFBFBF, 0xDADADA, 0xBF9F8F, 0xBFAF8F, 0xBFBF8F, 0xAFBF8F,
        0x9FBF8F, 0x8FBF8F, 0x8FBF9F, 0x8FBFAF, 0x8FBFBF, 0x8FAFBF, 0x8F9FBF, 0x8F8FBF,
        0x9F8FBF, 0xAF8FBF, 0xBF8FBF, 0xBF8FAF, 0xBF8F9F, 0xBF8F8F, 0xB6B6B6, 0xBF7F5F,
        0xBFAF8F, 0xBFBF5F, 0x9FBF5F, 0x7FBF5F, 0x5FBF5F, 0x5FBF7F, 0x5FBF9F, 0x5FBFBF,
        0x5F9FBF, 0x5F7FBF, 0x5F5FBF, 0x7F5FBF, 0x9F5FBF, 0xBF5FBF, 0xBF5F9F, 0xBF5F7F,
        0xBF5F5F, 0x919191, 0xBF6A3F, 0xBF943F, 0xBFBF3F, 0x94BF3F, 0x6ABF3F, 0x3FBF3F,
        0x3FBF6A, 0x3FBF94, 0x3FBFBF, 0x3F94BF, 0x3F6ABF, 0x3F3FBF, 0x6A3FBF, 0x943FBF,
        0xBF3FBF, 0xBF3F94, 0xBF3F6A, 0xBF3F3F, 0x6D6D6D, 0xFF5500, 0xFFAA00, 0xFFFF00,
        0xAAFF00, 0x54FF00, 0x00FF00, 0x00FF54, 0x00FFAA, 0x00FFFF, 0x00A9FF, 0x0055FF,
        0x0000FF, 0x5500FF, 0xA900FF, 0xFE00FF, 0xFF00AA, 0xFF0055, 0xFF0000, 0x484848,
        0xBF3F00, 0xBF7F00, 0xBFBF00, 0x7FBF00, 0x3FBF00, 0x00BF00, 0x00BF3F, 0x00BF7F,
        0x00BFBF, 0x007FBF, 0x003FBF, 0x0000BF, 0x3F00BF, 0x7F00BF, 0xBF00BF, 0xBF007F,
        0xBF003F, 0xBF0000, 0x242424, 0x7F2A00, 0x7F5500, 0x7F7F00, 0x557F00, 0x2A7F00,
        0x007F00, 0x007F2A, 0x007F55, 0x007F7F, 0x00547F, 0x002A7F, 0x00007F, 0x2A007F,
        0x54007F, 0x7F007F, 0x7F0055, 0x7F002A, 0x7F0000,
    };
    for (uint32_t val : rawTable) {
        table.append(qRgb( (val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF ));
    }
    return table;
}


GameSprite::GameSprite() : Sprite() {
    // Initialize members
    // animator = nullptr; // Or initialize with a default animator if applicable
}

GameSprite::~GameSprite() {
    // delete animator;
    // sprite_parts will be cleared automatically
}

void GameSprite::draw(QPainter* painter, int x, int y, int width_override, int height_override) {
    if (!painter ) { 
        if (m_image.isNull()) return; 
        // Call base class draw method if this one cannot proceed (e.g. no sprite_parts)
        // This assumes Sprite::draw is not pure virtual or GameSprite always has a drawable state.
        // Since Sprite::draw is pure virtual, this path should ideally not be taken
        // or GameSprite ensures it always has something to draw if it doesn't override fully.
        // For now, if sprite_parts is empty, it relies on m_image from Sprite.
        // If m_image is also just the default 1x1, it will draw that.
        painter->drawImage(QRect(x,y, width_override == -1 ? m_image.width() : width_override, height_override == -1 ? m_image.height() : height_override), m_image);
        return;
    }
    if (sprite_parts.isEmpty()){ 
         if (m_image.isNull()) return; 
        painter->drawImage(QRect(x,y, width_override == -1 ? m_image.width() : width_override, height_override == -1 ? m_image.height() : height_override), m_image);
        return;
    }

    const QImage& current_part = sprite_parts.at(0); // Simplification!
    if (current_part.isNull()) return;

    int w = (width_override == -1) ? current_part.width() : width_override;
    int h = (height_override == -1) ? current_part.height() : height_override;
    
    painter->drawImage(QRect(x, y, w, h), current_part);
}

void GameSprite::drawOutfit(QPainter* painter, int x, int y, const Outfit& outfit, int width_override, int height_override) {
     if (!painter) return; 

    QImage base_image_to_use;

    if (!sprite_parts.isEmpty()) {
        base_image_to_use = getSpritePart(0,0,0,0,0,0,0); 
        if(base_image_to_use.isNull()){ 
            base_image_to_use = sprite_parts.at(0);
        }
    } else if (!m_image.isNull()) { 
        base_image_to_use = m_image;
    } else {
        return; 
    }
    
    if (base_image_to_use.isNull()) return;


    QImage colorized_part = colorizeSpritePart(base_image_to_use, outfit);
    if (colorized_part.isNull()) return;

    int w = (width_override == -1) ? colorized_part.width() : width_override;
    int h = (height_override == -1) ? colorized_part.height() : height_override;

    painter->drawImage(QRect(x, y, w, h), colorized_part);
}


int GameSprite::getDrawHeight() const {
    return draw_height_offset;
}

QPoint GameSprite::getDrawOffset() const {
    return draw_offset;
}

QRgb GameSprite::getMiniMapColor() const {
    return minimap_color_val;
}

bool GameSprite::hasLight() const {
    return m_has_light;
}

int GameSprite::getIndex(int w, int h, int l, int px, int py, int pz, int fr) const {
    if (this->frames == 0) return 0; 
    return ((((((fr % this->frames) * this->pattern_z + pz) * this->pattern_y + py) * this->pattern_x + px) * this->layers + l) * this->height + h) * this->width + w;
}


QImage GameSprite::getSpritePart(int x_offset, int y_offset, int layer_offset, int pattern_x_offset, int pattern_y_offset, int pattern_z_offset, int frame_num) const {
    if (sprite_parts.isEmpty()) {
        return m_image; 
    }
    int calculated_index = getIndex(x_offset, y_offset, layer_offset, pattern_x_offset, pattern_y_offset, pattern_z_offset, frame_num);

    if (calculated_index >= 0 && calculated_index < sprite_parts.size()) {
        return sprite_parts.at(calculated_index);
    }
    return QImage(); 
}


bool GameSprite::loadFromSpriteSheet(const QImage& spriteSheet, int sheet_total_width, int sheet_total_height, int part_width, int part_height) {
    if (spriteSheet.isNull() || part_width <= 0 || part_height <= 0) {
        return false;
    }
    sprite_parts.clear();
    this->width = sheet_total_width / part_width; 
    this->height = sheet_total_height / part_height; 

    for (int y_coord = 0; y_coord < sheet_total_height; y_coord += part_height) {
        for (int x_coord = 0; x_coord < sheet_total_width; x_coord += part_width) {
            if (x_coord + part_width <= sheet_total_width && y_coord + part_height <= sheet_total_height) {
                sprite_parts.append(spriteSheet.copy(x_coord, y_coord, part_width, part_height));
            }
        }
    }
    numsprites = sprite_parts.size();
    if (!sprite_parts.isEmpty()) {
         Sprite::setImage(sprite_parts.at(0)); 
    } else {
        Sprite::setImage(QImage(1,1,QImage::Format_ARGB32_Premultiplied)); // Ensure base image is not null
    }
    return !sprite_parts.isEmpty();
}

bool GameSprite::loadIndividualSpriteParts(const QStringList& filePaths) {
    sprite_parts.clear();
    for (const QString& path : filePaths) {
        QImage part;
        if (part.load(path)) {
            sprite_parts.append(part);
        } else {
            sprite_parts.clear();
            return false;
        }
    }
    numsprites = sprite_parts.size();
    if (!sprite_parts.isEmpty()) {
         Sprite::setImage(sprite_parts.at(0)); 
    } else {
        Sprite::setImage(QImage(1,1,QImage::Format_ARGB32_Premultiplied)); 
    }
    return !sprite_parts.isEmpty();
}


QImage GameSprite::colorizeSpritePart(const QImage& part, const Outfit& outfit) const {
    if (part.isNull()) return part;
    // No colorization if no specific outfit colors are set
    if (outfit.lookHead == 0 && outfit.lookBody == 0 && outfit.lookLegs == 0 && outfit.lookFeet == 0) {
        return part; 
    }

    QImage colorizedImage = part.convertToFormat(QImage::Format_ARGB32_Premultiplied); 

    for (int y = 0; y < colorizedImage.height(); ++y) {
        for (int x = 0; x < colorizedImage.width(); ++x) {
            QColor originalPixelColor = colorizedImage.pixelColor(x, y);
            QColor templateColor = originalPixelColor; 
            QColor finalPixelColor = originalPixelColor; 
            
            if (templateColor.red() == 255 && templateColor.green() == 255 && templateColor.blue() == 0 && outfit.lookHead > 0 && outfit.lookHead < TemplateOutfitLookupTable.size()) {
                colorizePixel(QColor(TemplateOutfitLookupTable[outfit.lookHead]), finalPixelColor);
            }
            else if (templateColor.red() == 255 && templateColor.green() == 0 && templateColor.blue() == 0 && outfit.lookBody > 0 && outfit.lookBody < TemplateOutfitLookupTable.size()) {
                 colorizePixel(QColor(TemplateOutfitLookupTable[outfit.lookBody]), finalPixelColor);
            }
            else if (templateColor.red() == 0 && templateColor.green() == 255 && templateColor.blue() == 0 && outfit.lookLegs > 0 && outfit.lookLegs < TemplateOutfitLookupTable.size()) {
                colorizePixel(QColor(TemplateOutfitLookupTable[outfit.lookLegs]), finalPixelColor);
            }
            else if (templateColor.red() == 0 && templateColor.green() == 0 && templateColor.blue() == 255 && outfit.lookFeet > 0 && outfit.lookFeet < TemplateOutfitLookupTable.size()) {
                colorizePixel(QColor(TemplateOutfitLookupTable[outfit.lookFeet]), finalPixelColor);
            }

            colorizedImage.setPixelColor(x, y, finalPixelColor);
        }
    }
    return colorizedImage;
}

void GameSprite::colorizePixel(const QColor& outfit_color_base, QColor& target_pixel_color) const {
    float ro = outfit_color_base.redF();
    float go = outfit_color_base.greenF();
    float bo = outfit_color_base.blueF();

    float current_r = target_pixel_color.redF();
    float current_g = target_pixel_color.greenF();
    float current_b = target_pixel_color.blueF();

    target_pixel_color.setRedF(current_r * ro);
    target_pixel_color.setGreenF(current_g * go);
    target_pixel_color.setBlueF(current_b * bo);
}
