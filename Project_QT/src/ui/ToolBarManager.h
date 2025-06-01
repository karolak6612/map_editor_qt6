#ifndef TOOLBARMANAGER_H
#define TOOLBARMANAGER_H

#include <QObject>
#include <QToolBar>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QActionGroup>
#include "MenuBar.h"

// Forward declarations
class MainWindow;

/**
 * @brief Manages all toolbars for MainWindow
 * 
 * This class extracts toolbar creation and management from MainWindow
 * to improve maintainability and comply with M6 file size management.
 */
class ToolBarManager : public QObject
{
    Q_OBJECT

public:
    explicit ToolBarManager(MainWindow* mainWindow, QObject* parent = nullptr);
    ~ToolBarManager() = default;

    // Toolbar creation methods
    void setupToolBars();
    QToolBar* createStandardToolBar();
    QToolBar* createBrushesToolBar();
    QToolBar* createPositionToolBar();
    QToolBar* createSizesToolBar();

    // Toolbar state management
    void updateToolbarStates();
    void updateStandardToolbarStates();
    void updateBrushToolbarStates();

    // Task 46: Enhanced state management and interactions
    void saveToolBarState();
    void restoreToolBarState();
    void connectToolBarSignals();
    void setupHotkeys();
    void updateMapContext(const QString& mapName, int currentLayer, int zoomLevel);
    void updateBrushContext(const QString& brushName, int brushSize, bool isCircular);

    // Toolbar access methods
    QToolBar* getStandardToolBar() const { return standardToolBar_; }
    QToolBar* getBrushesToolBar() const { return brushesToolBar_; }
    QToolBar* getPositionToolBar() const { return positionToolBar_; }
    QToolBar* getSizesToolBar() const { return sizesToolBar_; }

    // Control access methods
    QSpinBox* getZoomSpinBox() const { return zoomSpinBox_; }
    QComboBox* getLayerComboBox() const { return layerComboBox_; }
    QSpinBox* getXCoordSpinBox() const { return xCoordSpinBox_; }
    QSpinBox* getYCoordSpinBox() const { return yCoordSpinBox_; }
    QSpinBox* getZCoordSpinBox() const { return zCoordSpinBox_; }

signals:
    // Signals for toolbar control changes
    void zoomControlChanged(int value);
    void layerControlChanged(int index);
    void positionControlChanged();
    void brushShapeActionTriggered();
    void brushSizeActionTriggered();
    void brushActionTriggered();

    // Task 77: Enhanced toolbar action signals
    void toolbarActionTriggered(const QString& actionName, bool active);

private:
    // Helper methods
    QAction* createActionWithId(MenuBar::ActionID actionId, const QString& text, 
                               const QIcon& icon = QIcon(), const QString& shortcut = "", 
                               const QString& statusTip = "", bool checkable = false, 
                               bool checked = false);

    // MainWindow reference
    MainWindow* mainWindow_;

    // Toolbars
    QToolBar* standardToolBar_;
    QToolBar* brushesToolBar_;
    QToolBar* positionToolBar_;
    QToolBar* sizesToolBar_;

    // Toolbar controls
    QSpinBox* zoomSpinBox_;
    QComboBox* layerComboBox_;
    QSpinBox* xCoordSpinBox_;
    QSpinBox* yCoordSpinBox_;
    QSpinBox* zCoordSpinBox_;

    // Action groups
    QActionGroup* brushGroup_;
    QActionGroup* sizeGroup_;
    QActionGroup* shapeGroup_;
};

#endif // TOOLBARMANAGER_H
