#ifndef SPRITE_H
#define SPRITE_H

#include <QImage>
#include <QPixmap> // Added for QPixmap
#include <QPainter> 
#include <QSize>
#include <QColor> 

class QPaintDevice;

class Sprite {
public:
    Sprite();
    virtual ~Sprite();

    virtual void draw(QPainter* painter, int x, int y, int width = -1, int height = -1) = 0;
    virtual bool load(const QString& path); 
    virtual bool loadFromData(const QByteArray& data, const char* format = nullptr);

    virtual QImage getImage() const;
    virtual void setImage(const QImage& image);

    virtual QPixmap getPixmap() const; 

    virtual QImage scale(const QSize& size, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio, Qt::TransformationMode transformMode = Qt::SmoothTransformation);
    virtual void setTransparency(int alpha); 
    virtual void setMaskColor(const QColor& color, bool enable = true);

protected:
    QImage m_image;
};

#endif // SPRITE_H
