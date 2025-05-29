#ifndef GAMESPRITE_H
#define GAMESPRITE_H

#include "Sprite.h"
#include <QVector>
#include <QMap> // For potential caching or complex structures
#include <QColor>
#include <QPoint> // For QPoint

// Forward declarations if needed (e.g. for Outfit class if it's complex)
// For now, assume Outfit might be a simpler struct or basic class
struct Outfit {
    int lookType = 0;
    int lookHead = 0;
    int lookBody = 0;
    int lookLegs = 0;
    int lookFeet = 0;
    // Add other relevant members from wxwidgets version
    uint32_t getColorHash() const { // Example hash function
        return (lookHead << 24) | (lookBody << 16) | (lookLegs << 8) | lookFeet;
    }
};


class GameSprite : public Sprite {
public:
    GameSprite();
    ~GameSprite() override;

    void draw(QPainter* painter, int x, int y, int width = -1, int height = -1) override;
    virtual void drawOutfit(QPainter* painter, int x, int y, const Outfit& outfit, int width = -1, int height = -1);

    int getDrawHeight() const;
    QPoint getDrawOffset() const; 
    QRgb getMiniMapColor() const; 

    bool hasLight() const;
    // SpriteLight getLight() const; // Define SpriteLight struct if needed

    // GameSprite specific properties from wxwidgets version
    uint8_t height = 0; 
    uint8_t width = 0;  
    uint8_t layers = 0; 
    uint8_t pattern_x = 0; 
    uint8_t pattern_y = 0; 
    uint8_t pattern_z = 0; 
    uint8_t frames = 1; // Default to 1 frame to avoid division by zero
    uint32_t numsprites = 0;

    uint16_t draw_height_offset = 0; 
    QPoint draw_offset; 

    QRgb minimap_color_val = qRgb(0,0,0); 

    bool m_has_light = false;
    // SpriteLight light_properties; 

    QVector<QImage> sprite_parts; 

    QImage getSpritePart(int x_offset, int y_offset, int layer_offset, int pattern_x_offset, int pattern_y_offset, int pattern_z_offset, int frame_num) const;
    int getIndex(int w, int h, int l, int px, int py, int pz, int fr) const;

    bool loadFromSpriteSheet(const QImage& spriteSheet, int sheet_total_width, int sheet_total_height, int part_width, int part_height);
    bool loadIndividualSpriteParts(const QStringList& filePaths); 

    QImage colorizeSpritePart(const QImage& part, const Outfit& outfit) const;

public: 
    void colorizePixel(const QColor& outfit_color_base, QColor& target_pixel_color) const; // Removed const Outfit& outfit from here, it's not used
    static const QVector<QRgb> TemplateOutfitLookupTable; 
private: 
    static QVector<QRgb> initializeLookupTable();
};

#endif // GAMESPRITE_H
