#include "PerspectiveManager.h"
#include "MainWindow.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QDockWidget>
#include <QMessageBox>

PerspectiveManager::PerspectiveManager(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , mainWindow_(mainWindow)
{
    Q_ASSERT(mainWindow_);
}

void PerspectiveManager::savePerspective()
{
    if (!mainWindow_) {
        qWarning("PerspectiveManager::savePerspective: MainWindow is null!");
        return;
    }

    qDebug() << "PerspectiveManager: Saving current perspective...";
    
    // Save current window state
    currentPerspective_ = mainWindow_->saveState();
    
    // Save to settings
    QSettings settings;
    settings.setValue("perspective/current", currentPerspective_);
    settings.setValue("perspective/geometry", mainWindow_->saveGeometry());
    
    qDebug() << "PerspectiveManager: Perspective saved successfully";
    mainWindow_->showTemporaryStatusMessage(tr("Perspective saved"), 2000);
}

void PerspectiveManager::loadPerspective()
{
    if (!mainWindow_) {
        qWarning("PerspectiveManager::loadPerspective: MainWindow is null!");
        return;
    }

    qDebug() << "PerspectiveManager: Loading saved perspective...";
    
    QSettings settings;
    QByteArray perspectiveData = settings.value("perspective/current").toByteArray();
    QByteArray geometryData = settings.value("perspective/geometry").toByteArray();
    
    if (perspectiveData.isEmpty()) {
        qDebug() << "PerspectiveManager: No saved perspective found, using default";
        perspectiveData = getDefaultPerspective();
    }
    
    // Restore geometry first
    if (!geometryData.isEmpty()) {
        mainWindow_->restoreGeometry(geometryData);
    }
    
    // Then restore state
    applyPerspective(perspectiveData);
    
    qDebug() << "PerspectiveManager: Perspective loaded successfully";
    mainWindow_->showTemporaryStatusMessage(tr("Perspective loaded"), 2000);
}

void PerspectiveManager::resetPerspective()
{
    if (!mainWindow_) {
        qWarning("PerspectiveManager::resetPerspective: MainWindow is null!");
        return;
    }

    qDebug() << "PerspectiveManager: Resetting to default perspective...";
    
    // Apply default perspective
    QByteArray defaultPerspective = getDefaultPerspective();
    applyPerspective(defaultPerspective);
    
    // Reset window geometry to default
    mainWindow_->resize(1200, 800);
    mainWindow_->move(100, 100);
    
    qDebug() << "PerspectiveManager: Perspective reset to default";
    mainWindow_->showTemporaryStatusMessage(tr("Perspective reset to default"), 2000);
}

void PerspectiveManager::createNewPalette()
{
    qDebug() << "PerspectiveManager: Creating new palette...";
    
    // TODO: Implement palette creation logic
    // This would involve creating a new palette dock widget
    
    mainWindow_->showTemporaryStatusMessage(tr("New palette created (stub implementation)"), 2000);
}

void PerspectiveManager::destroyCurrentPalette()
{
    qDebug() << "PerspectiveManager: Destroying current palette...";
    
    // TODO: Implement palette destruction logic
    // This would involve removing the current palette dock widget
    
    mainWindow_->showTemporaryStatusMessage(tr("Current palette destroyed (stub implementation)"), 2000);
}

void PerspectiveManager::createDockableMapView()
{
    qDebug() << "PerspectiveManager: Creating dockable map view...";
    
    // TODO: Implement dockable map view creation
    // This would involve creating a new map view dock widget
    
    mainWindow_->showTemporaryStatusMessage(tr("Dockable map view created (stub implementation)"), 2000);
}

void PerspectiveManager::closeDockableViews()
{
    qDebug() << "PerspectiveManager: Closing dockable views...";
    
    // TODO: Implement closing of dockable views
    // This would involve closing all non-essential dock widgets
    
    mainWindow_->showTemporaryStatusMessage(tr("Dockable views closed (stub implementation)"), 2000);
}

void PerspectiveManager::saveWindowState()
{
    if (!mainWindow_) return;
    
    QSettings settings;
    settings.setValue("mainwindow/geometry", mainWindow_->saveGeometry());
    settings.setValue("mainwindow/state", mainWindow_->saveState());
    
    qDebug() << "PerspectiveManager: Window state saved to settings";
}

void PerspectiveManager::restoreWindowState()
{
    if (!mainWindow_) return;
    
    QSettings settings;
    QByteArray geometry = settings.value("mainwindow/geometry").toByteArray();
    QByteArray state = settings.value("mainwindow/state").toByteArray();
    
    if (!geometry.isEmpty()) {
        mainWindow_->restoreGeometry(geometry);
    }
    
    if (!state.isEmpty()) {
        mainWindow_->restoreState(state);
    }
    
    qDebug() << "PerspectiveManager: Window state restored from settings";
}

QString PerspectiveManager::getPerspectiveFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(configDir);
    }
    return dir.filePath("perspective.dat");
}

QByteArray PerspectiveManager::getDefaultPerspective() const
{
    // Return empty array - Qt will use default layout
    // In a real implementation, this could contain a predefined layout
    return QByteArray();
}

void PerspectiveManager::applyPerspective(const QByteArray& perspectiveData)
{
    if (!mainWindow_) return;
    
    if (perspectiveData.isEmpty()) {
        qDebug() << "PerspectiveManager: Empty perspective data, using Qt defaults";
        return;
    }
    
    bool success = mainWindow_->restoreState(perspectiveData);
    if (!success) {
        qWarning() << "PerspectiveManager: Failed to restore perspective, using defaults";
    }
}


