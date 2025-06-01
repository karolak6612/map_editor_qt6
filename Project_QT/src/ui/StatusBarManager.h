#ifndef STATUSBARMANAGER_H
#define STATUSBARMANAGER_H

#include <QObject>
#include <QLabel>
#include <QFrame>
#include <QStatusBar>
#include <QPointF>
#include <QVariantMap>

// Forward declarations
class Item;

/**
 * @brief Manages the status bar for MainWindow
 * 
 * This class extracts status bar management from MainWindow
 * to improve maintainability and comply with M6 file size management.
 */
class StatusBarManager : public QObject
{
    Q_OBJECT

public:
    explicit StatusBarManager(QStatusBar* statusBar, QObject* parent = nullptr);
    ~StatusBarManager() = default;

    // Setup methods
    void setupStatusBar();
    void initializeValues();

    // Status update methods
    void updateMouseMapCoordinates(const QPointF& mapPos, int floor);
    void updateZoomLevel(double zoom);
    void updateCurrentLayer(int layer);
    void updateCurrentBrush(const QString& brushName);
    void updateSelectedItemInfo(const QString& itemInfo);
    void updateSelectedItemInfo(const Item* item);
    void updateProgress(const QString& operation, int progress);
    void showTemporaryMessage(const QString& message, int timeout = 0);

    // Task 77: Enhanced status update methods
    void updateActionId(quint16 actionId, bool enabled);
    void updateDrawingMode(const QString& modeName, const QString& description);
    void updateToolbarState(const QString& toolName, bool active);

    // Status update handler for signals
    void handleStatusUpdateRequest(const QString& type, const QVariantMap& data);

private:
    // Setup helper methods
    void setupStatusBarFields();
    void setupStatusBarStyling();
    QFrame* createStatusBarSeparator();
    QString getLayerName(int layer) const;

    // Status bar and labels
    QStatusBar* statusBar_;
    QLabel* mouseCoordsLabel_;
    QLabel* itemInfoLabel_;
    QLabel* zoomLevelLabel_;
    QLabel* currentLayerLabel_;
    QLabel* brushInfoLabel_;

    // Task 77: Enhanced status bar labels
    QLabel* actionIdLabel_;
    QLabel* drawingModeLabel_;
    QLabel* toolbarStateLabel_;
};

#endif // STATUSBARMANAGER_H
