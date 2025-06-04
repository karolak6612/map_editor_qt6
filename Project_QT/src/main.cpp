#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QMessageBox>

#include "ui/MainWindow.h"
#include "SettingsManager.h"
#include "ResourceManager.h"

/**
 * @brief Main entry point for the Qt6 Map Editor application
 * 
 * This is the main function that initializes the Qt application,
 * sets up the application environment, and starts the main window.
 * 
 * Key initialization steps:
 * 1. Create QApplication instance
 * 2. Set application metadata
 * 3. Initialize core managers (Settings, Resources)
 * 4. Create and show main window
 * 5. Start event loop
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    app.setApplicationName("Map Editor Qt6");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Map Editor Team");
    app.setOrganizationDomain("mapeditor.org");
    
    // Set application icon if available
    QIcon appIcon(":/images/icon.png");
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }
    
    // Initialize application directories
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    
    QDir().mkpath(configDir);
    QDir().mkpath(dataDir);
    
    qDebug() << "Application starting...";
    qDebug() << "Config directory:" << configDir;
    qDebug() << "Data directory:" << dataDir;
    
    try {
        // Initialize core managers
        SettingsManager::getInstance().initialize();
        ResourceManager::getInstance().initialize();
        
        // Create and show main window
        MainWindow window;
        window.show();
        
        qDebug() << "Main window created and shown";
        
        // Start the event loop
        int result = app.exec();
        
        qDebug() << "Application exiting with code:" << result;
        return result;
        
    } catch (const std::exception& e) {
        QString errorMsg = QString("Failed to start application: %1").arg(e.what());
        qCritical() << errorMsg;
        
        QMessageBox::critical(nullptr, "Application Error", errorMsg);
        return -1;
        
    } catch (...) {
        QString errorMsg = "Unknown error occurred during application startup";
        qCritical() << errorMsg;
        
        QMessageBox::critical(nullptr, "Application Error", errorMsg);
        return -1;
    }
}
