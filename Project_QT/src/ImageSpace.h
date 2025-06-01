// ImageSpace.h - ImageSpace concept implementation for Task 35
// This file implements the ImageSpace concept from wxwidgets for intermediate sprite storage

#ifndef IMAGESPACE_H
#define IMAGESPACE_H

#include <QObject>
#include <QImage>
#include <QByteArray>
#include <QMap>
#include <QSharedPointer>
#include <QMutex>

// Forward declarations
class GameSprite;

// ImageSpace concept from wxwidgets (Task 35 requirement)
// This manages raw sprite pixel data before conversion to displayable formats
class ImageSpace : public QObject {
    Q_OBJECT
    
public:
    // Image types (based on wxwidgets implementation)
    enum class ImageType {
        Normal,      // Regular sprite image
        Template,    // Template image for outfit coloring
        Editor       // Editor-specific images
    };
    
    // Base Image class (equivalent to wxwidgets GameSprite::Image)
    class Image {
    public:
        Image(quint32 id = 0, ImageType type = ImageType::Normal);
        virtual ~Image();
        
        // Core properties
        quint32 getId() const { return id_; }
        ImageType getType() const { return type_; }
        bool isLoaded() const { return loaded_; }
        qint64 getLastAccess() const { return lastAccess_; }
        
        // Data access
        virtual QByteArray getRawData() const = 0;
        virtual QImage getQImage() const = 0;
        virtual bool loadFromData(const QByteArray& data) = 0;
        
        // Memory management
        virtual void unload();
        virtual void clean(qint64 currentTime, qint64 maxAge = 5000);
        void visit(); // Update last access time
        
        // Size information
        virtual int getWidth() const { return 32; }
        virtual int getHeight() const { return 32; }
        virtual int getDataSize() const = 0;
        
    protected:
        quint32 id_;
        ImageType type_;
        bool loaded_;
        qint64 lastAccess_;
        mutable QMutex mutex_;
    };
    
    // Normal Image class (equivalent to wxwidgets GameSprite::NormalImage)
    class NormalImage : public Image {
    public:
        NormalImage(quint32 id = 0);
        ~NormalImage() override;
        
        // Data management
        QByteArray getRawData() const override;
        QImage getQImage() const override;
        bool loadFromData(const QByteArray& data) override;
        
        // RLE data handling
        bool loadFromRleData(const QByteArray& rleData, bool hasAlpha = true);
        QByteArray getRleData() const { return rleData_; }
        
        // Size information
        int getDataSize() const override { return rawData_.size(); }
        
        // Memory management
        void unload() override;
        void clean(qint64 currentTime, qint64 maxAge = 5000) override;
        
    private:
        QByteArray rawData_;     // Decoded pixel data (RGBA)
        QByteArray rleData_;     // Original RLE compressed data
        mutable QImage cachedImage_;
        mutable bool imageCacheValid_;
        
        QImage decodeRleData(const QByteArray& rleData, bool hasAlpha) const;
    };
    
    // Template Image class (equivalent to wxwidgets GameSprite::TemplateImage)
    class TemplateImage : public Image {
    public:
        TemplateImage(quint32 id, const QSharedPointer<NormalImage>& baseImage);
        ~TemplateImage() override;
        
        // Data management
        QByteArray getRawData() const override;
        QImage getQImage() const override;
        bool loadFromData(const QByteArray& data) override;
        
        // Template-specific methods
        void setOutfitColors(quint8 head, quint8 body, quint8 legs, quint8 feet);
        QImage getColorizedImage(quint8 head, quint8 body, quint8 legs, quint8 feet) const;
        
        // Size information
        int getDataSize() const override;
        
        // Memory management
        void unload() override;
        
    private:
        QSharedPointer<NormalImage> baseImage_;
        quint8 outfitHead_;
        quint8 outfitBody_;
        quint8 outfitLegs_;
        quint8 outfitFeet_;
        mutable QImage cachedColorizedImage_;
        mutable bool colorizedCacheValid_;
        
        void colorizePixel(quint8 color, quint8& r, quint8& g, quint8& b) const;
    };
    
public:
    explicit ImageSpace(QObject* parent = nullptr);
    ~ImageSpace();
    
    // Image management (Task 35 requirement)
    QSharedPointer<Image> getImage(quint32 id) const;
    QSharedPointer<NormalImage> getNormalImage(quint32 id) const;
    QSharedPointer<TemplateImage> getTemplateImage(quint32 id) const;
    
    // Image creation and storage
    QSharedPointer<NormalImage> createNormalImage(quint32 id);
    QSharedPointer<TemplateImage> createTemplateImage(quint32 id, const QSharedPointer<NormalImage>& baseImage);
    
    // Image loading from data
    bool loadImageFromRleData(quint32 id, const QByteArray& rleData, bool hasAlpha = true);
    bool loadImageFromRawData(quint32 id, const QByteArray& rawData);
    
    // Memory management
    void cleanupImages(qint64 maxAge = 5000);
    void unloadImage(quint32 id);
    void unloadAllImages();
    void clearCache();
    
    // Statistics and information
    int getImageCount() const;
    int getLoadedImageCount() const;
    qint64 getTotalMemoryUsage() const;
    QList<quint32> getLoadedImageIds() const;
    
    // Configuration
    void setMaxCacheSize(qint64 maxSize) { maxCacheSize_ = maxSize; }
    qint64 getMaxCacheSize() const { return maxCacheSize_; }
    
signals:
    // Image management signals
    void imageLoaded(quint32 id);
    void imageUnloaded(quint32 id);
    void cacheCleared();
    void memoryLimitReached(qint64 currentUsage, qint64 maxSize);
    
public slots:
    // Automatic cleanup
    void performCleanup();
    
private slots:
    void onCleanupTimer();
    
private:
    // Helper methods
    void enforceMemoryLimit();
    void removeOldestImages(int count);
    qint64 calculateMemoryUsage() const;
    
    // Image storage
    QMap<quint32, QSharedPointer<Image>> images_;
    mutable QMutex imagesMutex_;
    
    // Memory management
    qint64 maxCacheSize_;
    QTimer* cleanupTimer_;
    
    // Statistics
    mutable qint64 cachedMemoryUsage_;
    mutable bool memoryUsageDirty_;
};

#endif // IMAGESPACE_H
