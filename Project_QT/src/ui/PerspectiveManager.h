#ifndef PERSPECTIVEMANAGER_H
#define PERSPECTIVEMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QString>

// Forward declarations
class MainWindow;

/**
 * @brief Manages window perspectives and layout for MainWindow
 * 
 * This class extracts perspective management from MainWindow
 * to improve maintainability and comply with M6 file size management.
 */
class PerspectiveManager : public QObject
{
    Q_OBJECT

public:
    explicit PerspectiveManager(MainWindow* mainWindow, QObject* parent = nullptr);
    ~PerspectiveManager() = default;

    // Perspective management
    void savePerspective();
    void loadPerspective();
    void resetPerspective();
    
    // Dock and palette management
    void createNewPalette();
    void destroyCurrentPalette();
    void createDockableMapView();
    void closeDockableViews();
    
    // Settings persistence
    void saveWindowState();
    void restoreWindowState();

private:
    // Helper methods
    QString getPerspectiveFilePath() const;
    QByteArray getDefaultPerspective() const;
    void applyPerspective(const QByteArray& perspectiveData);
    
    MainWindow* mainWindow_;
    QByteArray currentPerspective_;
};

#endif // PERSPECTIVEMANAGER_H
