// ScreenshotManager.h - Qt-based screenshot system (Task 76: OpenGL replacement)

#ifndef SCREENSHOTMANAGER_H
#define SCREENSHOTMANAGER_H

#include <QObject>
#include <QString>
#include <QPixmap>
#include <QWidget>
#include <QScreen>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

// Forward declarations
class MapView;

// Task 76: Qt-based screenshot system to replace OpenGL TakeScreenshot
class ScreenshotManager : public QObject {
    Q_OBJECT

public:
    enum class Format {
        PNG,
        JPEG,
        BMP,
        TIFF
    };

    enum class Quality {
        Low = 25,
        Medium = 50,
        High = 75,
        Maximum = 100
    };

    explicit ScreenshotManager(QObject* parent = nullptr);
    virtual ~ScreenshotManager();

    // Task 76: Main screenshot methods (replaces OpenGL TakeScreenshot)
    bool takeScreenshot(QWidget* widget, const QString& filePath, 
                       Format format = Format::PNG, Quality quality = Quality::High);
    bool takeMapViewScreenshot(MapView* mapView, const QString& filePath,
                              Format format = Format::PNG, Quality quality = Quality::High);
    bool takeFullScreenScreenshot(const QString& filePath,
                                 Format format = Format::PNG, Quality quality = Quality::High);

    // Task 76: Screenshot with automatic naming
    QString takeScreenshotAuto(QWidget* widget, const QString& directory = QString(),
                              Format format = Format::PNG, Quality quality = Quality::High);
    QString takeMapViewScreenshotAuto(MapView* mapView, const QString& directory = QString(),
                                     Format format = Format::PNG, Quality quality = Quality::High);

    // Task 76: Configuration
    void setDefaultDirectory(const QString& directory) { defaultDirectory_ = directory; }
    QString getDefaultDirectory() const { return defaultDirectory_; }
    
    void setDefaultFormat(Format format) { defaultFormat_ = format; }
    Format getDefaultFormat() const { return defaultFormat_; }
    
    void setDefaultQuality(Quality quality) { defaultQuality_ = quality; }
    Quality getDefaultQuality() const { return defaultQuality_; }

    // Task 76: Utility methods
    static QString formatToString(Format format);
    static QString formatToExtension(Format format);
    static Format stringToFormat(const QString& formatStr);
    
    static QString generateTimestampFilename(const QString& prefix = "screenshot",
                                           Format format = Format::PNG);

signals:
    // Task 76: Screenshot events
    void screenshotTaken(const QString& filePath, bool success);
    void screenshotFailed(const QString& error);

private:
    // Task 76: Internal screenshot methods
    QPixmap captureWidget(QWidget* widget);
    QPixmap captureMapView(MapView* mapView);
    QPixmap captureScreen();
    
    bool savePixmap(const QPixmap& pixmap, const QString& filePath, 
                   Format format, Quality quality);
    
    QString ensureDirectory(const QString& directory);
    QString getUniqueFilePath(const QString& basePath);

    // Task 76: Member variables
    QString defaultDirectory_;
    Format defaultFormat_;
    Quality defaultQuality_;
    
    // Task 76: Statistics
    int screenshotCount_;
    QDateTime lastScreenshotTime_;
};

#endif // SCREENSHOTMANAGER_H
