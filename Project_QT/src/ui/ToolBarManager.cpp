#include "ToolBarManager.h"
#include "MainWindow.h"
#include <QDebug>
#include <QIcon>
#include <QSettings>
#include <QShortcut>
#include <QKeySequence>

ToolBarManager::ToolBarManager(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , mainWindow_(mainWindow)
    , standardToolBar_(nullptr)
    , brushesToolBar_(nullptr)
    , positionToolBar_(nullptr)
    , sizesToolBar_(nullptr)
    , zoomSpinBox_(nullptr)
    , layerComboBox_(nullptr)
    , xCoordSpinBox_(nullptr)
    , yCoordSpinBox_(nullptr)
    , zCoordSpinBox_(nullptr)
    , brushGroup_(nullptr)
    , sizeGroup_(nullptr)
    , shapeGroup_(nullptr)
{
    Q_ASSERT(mainWindow_);
}

void ToolBarManager::setupToolBars()
{
    if (!mainWindow_) {
        qWarning("ToolBarManager::setupToolBars: MainWindow is null!");
        return;
    }

    // Create all toolbars
    standardToolBar_ = createStandardToolBar();
    brushesToolBar_ = createBrushesToolBar();
    positionToolBar_ = createPositionToolBar();
    sizesToolBar_ = createSizesToolBar();

    // Add toolbars to main window
    if (standardToolBar_) {
        mainWindow_->addToolBar(Qt::TopToolBarArea, standardToolBar_);
    }
    if (brushesToolBar_) {
        mainWindow_->addToolBar(Qt::TopToolBarArea, brushesToolBar_);
    }
    if (positionToolBar_) {
        mainWindow_->addToolBar(Qt::TopToolBarArea, positionToolBar_);
    }
    if (sizesToolBar_) {
        mainWindow_->addToolBar(Qt::TopToolBarArea, sizesToolBar_);
    }

    // Initialize toolbar states
    updateToolbarStates();

    // Task 46: Connect signals and setup hotkeys
    connectToolBarSignals();
    setupHotkeys();

    // Restore toolbar state from settings
    restoreToolBarState();

    qDebug() << "ToolBarManager: All toolbars created and configured";
}

QToolBar* ToolBarManager::createStandardToolBar()
{
    QToolBar* tb = new QToolBar(tr("Standard"), mainWindow_);
    tb->setObjectName(QStringLiteral("StandardToolBar"));

    // File actions (using ActionID system)
    if (auto action = mainWindow_->getAction(MenuBar::NEW)) tb->addAction(action);
    if (auto action = mainWindow_->getAction(MenuBar::OPEN)) tb->addAction(action);
    if (auto action = mainWindow_->getAction(MenuBar::SAVE)) tb->addAction(action);
    if (auto action = mainWindow_->getAction(MenuBar::SAVE_AS)) tb->addAction(action);
    tb->addSeparator();

    // Edit actions
    if (auto action = mainWindow_->getAction(MenuBar::UNDO)) tb->addAction(action);
    if (auto action = mainWindow_->getAction(MenuBar::REDO)) tb->addAction(action);
    tb->addSeparator();
    if (auto action = mainWindow_->getAction(MenuBar::CUT)) tb->addAction(action);
    if (auto action = mainWindow_->getAction(MenuBar::COPY)) tb->addAction(action);
    if (auto action = mainWindow_->getAction(MenuBar::PASTE)) tb->addAction(action);
    tb->addSeparator();

    // Zoom control
    tb->addWidget(new QLabel(tr("Zoom:"), mainWindow_));
    zoomSpinBox_ = new QSpinBox(mainWindow_);
    zoomSpinBox_->setRange(10, 400);
    zoomSpinBox_->setValue(100);
    zoomSpinBox_->setSuffix(tr("%"));
    zoomSpinBox_->setToolTip(tr("Set map zoom level"));
    connect(zoomSpinBox_, qOverload<int>(&QSpinBox::valueChanged), 
            this, &ToolBarManager::zoomControlChanged);
    tb->addWidget(zoomSpinBox_);

    // Layer control
    tb->addWidget(new QLabel(tr("Layer:"), mainWindow_));
    layerComboBox_ = new QComboBox(mainWindow_);
    for (int i = 0; i <= 15; ++i) {
        layerComboBox_->addItem(QString(tr("Floor %1")).arg(i), i);
    }
    layerComboBox_->setCurrentIndex(7);
    layerComboBox_->setToolTip(tr("Select current map layer/floor"));
    connect(layerComboBox_, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ToolBarManager::layerControlChanged);
    tb->addWidget(layerComboBox_);

    return tb;
}

QToolBar* ToolBarManager::createBrushesToolBar()
{
    QToolBar* tb = new QToolBar(tr("Brushes"), mainWindow_);
    tb->setObjectName(QStringLiteral("BrushesToolBar"));
    
    brushGroup_ = new QActionGroup(this);
    brushGroup_->setExclusive(true);

    // Helper lambda to create brush actions
    auto addBrushActionWithId = [&](MenuBar::ActionID actionId, const QString& text, 
                                   const QString& iconName, const QString& themeIconName, 
                                   const QString& toolTip) {
        QIcon icon(QStringLiteral(":/icons/%1.png").arg(iconName));
        if (icon.isNull()) {
            icon = QIcon::fromTheme(themeIconName);
        }
        if (icon.isNull()) {
            qDebug() << "Icon not found for ActionID" << static_cast<int>(actionId);
            icon = QIcon(); // Empty icon
        }
        QAction* action = createActionWithId(actionId, text, icon, "", toolTip, true);
        tb->addAction(action);
        brushGroup_->addAction(action);
        return action;
    };

    // Create brush actions
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, "Optional Border", 
                        "optional_border_small", "draw-border", "Toggle optional border brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_ERASER, "Eraser", 
                        "eraser_small", "edit-clear", "Toggle eraser brush");
    tb->addSeparator();
    
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_PZ_TOOL, "Protected Zone", 
                        "pz_zone", "security-high", "Toggle Protected Zone brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_NOPVP_TOOL, "No PvP Zone", 
                        "nopvp_zone", "user-block", "Toggle No PvP Zone brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_NOLOGOUT_TOOL, "No Logout Zone", 
                        "nologout_zone", "system-log-out", "Toggle No Logout Zone brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_PVPZONE_TOOL, "PvP Zone", 
                        "pvp_zone", "security-medium", "Toggle PvP Zone brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_ZONE_BRUSH, "Zone Brush", 
                        "zone_brush", "draw-polygon", "Toggle generic zone brush");
    tb->addSeparator();
    
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_NORMAL_DOOR, "Normal Door", 
                        "door_normal_small", "object-UNSET", "Toggle normal door brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_LOCKED_DOOR, "Locked Door", 
                        "door_locked_small", "object-UNSET", "Toggle locked door brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_MAGIC_DOOR, "Magic Door", 
                        "door_magic_small", "object-UNSET", "Toggle magic door brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_QUEST_DOOR, "Quest Door", 
                        "door_quest_small", "object-UNSET", "Toggle quest door brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_NORMAL_ALT_DOOR, "Normal Door (alt)", 
                        "door_normal_alt_small", "object-UNSET", "Toggle alternative normal door brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_ARCHWAY_DOOR, "Archway", 
                        "door_archway_small", "object-UNSET", "Toggle archway brush");
    tb->addSeparator();
    
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_HATCH_DOOR, "Hatch Window", 
                        "window_hatch_small", "object-UNSET", "Toggle hatch window brush");
    addBrushActionWithId(MenuBar::PALETTE_TERRAIN_WINDOW_DOOR, "Window", 
                        "window_normal_small", "object-UNSET", "Toggle normal window brush");

    // Set first brush as active by default
    if (!tb->actions().isEmpty() && tb->actions().first()->isCheckable()) {
        tb->actions().first()->setChecked(true);
    }

    return tb;
}

QToolBar* ToolBarManager::createPositionToolBar()
{
    QToolBar* tb = new QToolBar(tr("Position"), mainWindow_);
    tb->setObjectName(QStringLiteral("PositionToolBar"));

    // Position controls
    tb->addWidget(new QLabel(tr("X:"), mainWindow_));
    xCoordSpinBox_ = new QSpinBox(mainWindow_);
    xCoordSpinBox_->setRange(0, 65535);
    xCoordSpinBox_->setValue(1000);
    xCoordSpinBox_->setToolTip(tr("X coordinate"));
    connect(xCoordSpinBox_, qOverload<int>(&QSpinBox::valueChanged),
            this, &ToolBarManager::positionControlChanged);
    tb->addWidget(xCoordSpinBox_);

    tb->addWidget(new QLabel(tr("Y:"), mainWindow_));
    yCoordSpinBox_ = new QSpinBox(mainWindow_);
    yCoordSpinBox_->setRange(0, 65535);
    yCoordSpinBox_->setValue(1000);
    yCoordSpinBox_->setToolTip(tr("Y coordinate"));
    connect(yCoordSpinBox_, qOverload<int>(&QSpinBox::valueChanged),
            this, &ToolBarManager::positionControlChanged);
    tb->addWidget(yCoordSpinBox_);

    tb->addWidget(new QLabel(tr("Z:"), mainWindow_));
    zCoordSpinBox_ = new QSpinBox(mainWindow_);
    zCoordSpinBox_->setRange(0, 15);
    zCoordSpinBox_->setValue(7);
    zCoordSpinBox_->setToolTip(tr("Z coordinate (floor)"));
    connect(zCoordSpinBox_, qOverload<int>(&QSpinBox::valueChanged),
            this, &ToolBarManager::positionControlChanged);
    tb->addWidget(zCoordSpinBox_);

    // Go button
    QAction* goAction = createActionWithId(MenuBar::TOOLBAR_POSITION_GO, "Go", 
                                          QIcon::fromTheme("go-jump"), "", 
                                          "Navigate to specified position");
    tb->addAction(goAction);

    return tb;
}

QToolBar* ToolBarManager::createSizesToolBar()
{
    QToolBar* tb = new QToolBar(tr("Sizes"), mainWindow_);
    tb->setObjectName(QStringLiteral("SizesToolBar"));

    // Shape group
    shapeGroup_ = new QActionGroup(this);
    shapeGroup_->setExclusive(true);

    QAction* rectAction = createActionWithId(MenuBar::TOOLBAR_SIZES_RECTANGULAR, "Rectangular", 
                                           QIcon::fromTheme("draw-rectangle"), "", 
                                           "Rectangular brush shape", true, true);
    tb->addAction(rectAction);
    shapeGroup_->addAction(rectAction);

    QAction* circAction = createActionWithId(MenuBar::TOOLBAR_SIZES_CIRCULAR, "Circular", 
                                           QIcon::fromTheme("draw-circle"), "", 
                                           "Circular brush shape", true);
    tb->addAction(circAction);
    shapeGroup_->addAction(circAction);

    tb->addSeparator();

    // Size group
    sizeGroup_ = new QActionGroup(this);
    sizeGroup_->setExclusive(true);

    for (int i = 1; i <= 7; ++i) {
        MenuBar::ActionID sizeActionId = static_cast<MenuBar::ActionID>(
            static_cast<int>(MenuBar::TOOLBAR_SIZES_1) + i - 1);
        QAction* sizeAction = createActionWithId(sizeActionId, QString::number(i), 
                                                QIcon(), "", 
                                                QString("Brush size %1").arg(i), true, i == 1);
        tb->addAction(sizeAction);
        sizeGroup_->addAction(sizeAction);
    }

    return tb;
}

QAction* ToolBarManager::createActionWithId(MenuBar::ActionID actionId, const QString& text, 
                                           const QIcon& icon, const QString& shortcut, 
                                           const QString& statusTip, bool checkable, bool checked)
{
    if (!mainWindow_) return nullptr;
    return mainWindow_->createActionWithId(actionId, text, icon, shortcut, statusTip, checkable, checked);
}

void ToolBarManager::updateToolbarStates()
{
    updateStandardToolbarStates();
    updateBrushToolbarStates();
    qDebug() << "ToolBarManager::updateToolbarStates: All toolbar states updated";
}

void ToolBarManager::updateStandardToolbarStates()
{
    if (!mainWindow_) return;

    // Update standard toolbar actions based on current application state
    bool hasEditor = mainWindow_->getCurrentMap() != nullptr;
    bool canUndo = false; // TODO: Check if editor has undo actions
    bool canRedo = false; // TODO: Check if editor has redo actions
    bool canPaste = mainWindow_->canPaste();

    // Update action states
    if (QAction* undoAction = mainWindow_->getAction(MenuBar::UNDO)) {
        undoAction->setEnabled(canUndo);
    }
    if (QAction* redoAction = mainWindow_->getAction(MenuBar::REDO)) {
        redoAction->setEnabled(canRedo);
    }
    if (QAction* pasteAction = mainWindow_->getAction(MenuBar::PASTE)) {
        pasteAction->setEnabled(canPaste);
    }
    if (QAction* saveAction = mainWindow_->getAction(MenuBar::SAVE)) {
        saveAction->setEnabled(hasEditor);
    }
    if (QAction* saveAsAction = mainWindow_->getAction(MenuBar::SAVE_AS)) {
        saveAsAction->setEnabled(hasEditor);
    }

    qDebug() << "ToolBarManager::updateStandardToolbarStates: hasEditor=" << hasEditor
             << "canUndo=" << canUndo << "canRedo=" << canRedo << "canPaste=" << canPaste;
}

void ToolBarManager::updateBrushToolbarStates()
{
    // Update brush toolbar states based on current brush selection
    bool hasActiveBrush = true; // TODO: Check if any brush is currently active

    // Example: Update border brush state (placeholder)
    if (QAction* borderAction = mainWindow_->getAction(MenuBar::PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL)) {
        // TODO: Check if current brush is border brush
        // borderAction->setChecked(currentBrush == borderBrush);
    }

    qDebug() << "ToolBarManager::updateBrushToolbarStates: hasActiveBrush=" << hasActiveBrush;
}

// Task 46: Enhanced state management and interactions
void ToolBarManager::saveToolBarState()
{
    if (!mainWindow_) return;

    // Save toolbar visibility states
    QSettings settings;
    settings.beginGroup("ToolBars");

    if (standardToolBar_) {
        settings.setValue("StandardToolBar/visible", standardToolBar_->isVisible());
        settings.setValue("StandardToolBar/floating", standardToolBar_->isFloating());
        settings.setValue("StandardToolBar/area", static_cast<int>(mainWindow_->toolBarArea(standardToolBar_)));
    }

    if (brushesToolBar_) {
        settings.setValue("BrushesToolBar/visible", brushesToolBar_->isVisible());
        settings.setValue("BrushesToolBar/floating", brushesToolBar_->isFloating());
        settings.setValue("BrushesToolBar/area", static_cast<int>(mainWindow_->toolBarArea(brushesToolBar_)));
    }

    if (positionToolBar_) {
        settings.setValue("PositionToolBar/visible", positionToolBar_->isVisible());
        settings.setValue("PositionToolBar/floating", positionToolBar_->isFloating());
        settings.setValue("PositionToolBar/area", static_cast<int>(mainWindow_->toolBarArea(positionToolBar_)));
    }

    if (sizesToolBar_) {
        settings.setValue("SizesToolBar/visible", sizesToolBar_->isVisible());
        settings.setValue("SizesToolBar/floating", sizesToolBar_->isFloating());
        settings.setValue("SizesToolBar/area", static_cast<int>(mainWindow_->toolBarArea(sizesToolBar_)));
    }

    // Save control values
    if (zoomSpinBox_) {
        settings.setValue("Controls/zoomLevel", zoomSpinBox_->value());
    }

    if (layerComboBox_) {
        settings.setValue("Controls/currentLayer", layerComboBox_->currentIndex());
    }

    settings.endGroup();
    qDebug() << "ToolBarManager::saveToolBarState: Toolbar state saved";
}

void ToolBarManager::restoreToolBarState()
{
    if (!mainWindow_) return;

    QSettings settings;
    settings.beginGroup("ToolBars");

    // Restore toolbar visibility and positions
    if (standardToolBar_) {
        bool visible = settings.value("StandardToolBar/visible", true).toBool();
        bool floating = settings.value("StandardToolBar/floating", false).toBool();
        int area = settings.value("StandardToolBar/area", static_cast<int>(Qt::TopToolBarArea)).toInt();

        standardToolBar_->setVisible(visible);
        standardToolBar_->setFloating(floating);
        if (!floating) {
            mainWindow_->addToolBar(static_cast<Qt::ToolBarArea>(area), standardToolBar_);
        }
    }

    if (brushesToolBar_) {
        bool visible = settings.value("BrushesToolBar/visible", true).toBool();
        bool floating = settings.value("BrushesToolBar/floating", false).toBool();
        int area = settings.value("BrushesToolBar/area", static_cast<int>(Qt::TopToolBarArea)).toInt();

        brushesToolBar_->setVisible(visible);
        brushesToolBar_->setFloating(floating);
        if (!floating) {
            mainWindow_->addToolBar(static_cast<Qt::ToolBarArea>(area), brushesToolBar_);
        }
    }

    if (positionToolBar_) {
        bool visible = settings.value("PositionToolBar/visible", true).toBool();
        bool floating = settings.value("PositionToolBar/floating", false).toBool();
        int area = settings.value("PositionToolBar/area", static_cast<int>(Qt::TopToolBarArea)).toInt();

        positionToolBar_->setVisible(visible);
        positionToolBar_->setFloating(floating);
        if (!floating) {
            mainWindow_->addToolBar(static_cast<Qt::ToolBarArea>(area), positionToolBar_);
        }
    }

    if (sizesToolBar_) {
        bool visible = settings.value("SizesToolBar/visible", true).toBool();
        bool floating = settings.value("SizesToolBar/floating", false).toBool();
        int area = settings.value("SizesToolBar/area", static_cast<int>(Qt::TopToolBarArea)).toInt();

        sizesToolBar_->setVisible(visible);
        sizesToolBar_->setFloating(floating);
        if (!floating) {
            mainWindow_->addToolBar(static_cast<Qt::ToolBarArea>(area), sizesToolBar_);
        }
    }

    // Restore control values
    if (zoomSpinBox_) {
        int zoomLevel = settings.value("Controls/zoomLevel", 100).toInt();
        zoomSpinBox_->setValue(zoomLevel);
    }

    if (layerComboBox_) {
        int currentLayer = settings.value("Controls/currentLayer", 0).toInt();
        if (currentLayer < layerComboBox_->count()) {
            layerComboBox_->setCurrentIndex(currentLayer);
        }
    }

    settings.endGroup();
    qDebug() << "ToolBarManager::restoreToolBarState: Toolbar state restored";
}

void ToolBarManager::connectToolBarSignals()
{
    if (!mainWindow_) return;

    // Connect zoom control signals
    if (zoomSpinBox_) {
        connect(zoomSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this](int value) {
                    qDebug() << "ToolBarManager: Zoom level changed to" << value;
                    // TODO: Update MapView zoom level
                    // mainWindow_->getMapView()->setZoomLevel(value);
                });
    }

    // Connect layer selection signals
    if (layerComboBox_) {
        connect(layerComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int index) {
                    qDebug() << "ToolBarManager: Layer changed to" << index;
                    // TODO: Update MapView current layer
                    // mainWindow_->getMapView()->setCurrentLayer(index);
                });
    }

    // Connect position control signals
    if (xCoordSpinBox_) {
        connect(xCoordSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this](int value) {
                    qDebug() << "ToolBarManager: X coordinate changed to" << value;
                    // TODO: Update MapView position
                });
    }

    if (yCoordSpinBox_) {
        connect(yCoordSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this](int value) {
                    qDebug() << "ToolBarManager: Y coordinate changed to" << value;
                    // TODO: Update MapView position
                });
    }

    if (zCoordSpinBox_) {
        connect(zCoordSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this](int value) {
                    qDebug() << "ToolBarManager: Z coordinate changed to" << value;
                    // TODO: Update MapView floor level
                });
    }

    // Connect brush group signals
    if (brushGroup_) {
        connect(brushGroup_, &QActionGroup::triggered,
                this, [this](QAction* action) {
                    qDebug() << "ToolBarManager: Brush changed to" << action->text();
                    // TODO: Update current brush in BrushManager
                    // mainWindow_->getBrushManager()->setCurrentBrush(action->data().toString());
                });
    }

    // Connect size group signals
    if (sizeGroup_) {
        connect(sizeGroup_, &QActionGroup::triggered,
                this, [this](QAction* action) {
                    qDebug() << "ToolBarManager: Brush size changed to" << action->text();
                    // TODO: Update brush size
                });
    }

    // Connect shape group signals
    if (shapeGroup_) {
        connect(shapeGroup_, &QActionGroup::triggered,
                this, [this](QAction* action) {
                    qDebug() << "ToolBarManager: Brush shape changed to" << action->text();
                    // TODO: Update brush shape
                });
    }

    qDebug() << "ToolBarManager::connectToolBarSignals: All toolbar signals connected";
}

void ToolBarManager::setupHotkeys()
{
    if (!mainWindow_) return;

    // Setup hotkeys for toolbar actions that don't have menu equivalents

    // Brush size hotkeys (1-7)
    for (int i = 1; i <= 7; ++i) {
        QShortcut* sizeShortcut = new QShortcut(QKeySequence(QString::number(i)), mainWindow_);
        connect(sizeShortcut, &QShortcut::activated, this, [this, i]() {
            if (sizeGroup_) {
                QList<QAction*> actions = sizeGroup_->actions();
                if (i <= actions.size()) {
                    actions[i-1]->trigger();
                }
            }
        });
    }

    // Brush shape hotkeys
    QShortcut* rectShortcut = new QShortcut(QKeySequence("R"), mainWindow_);
    connect(rectShortcut, &QShortcut::activated, this, [this]() {
        if (shapeGroup_) {
            QList<QAction*> actions = shapeGroup_->actions();
            if (!actions.isEmpty()) {
                actions[0]->trigger(); // Rectangular
            }
        }
    });

    QShortcut* circShortcut = new QShortcut(QKeySequence("C"), mainWindow_);
    connect(circShortcut, &QShortcut::activated, this, [this]() {
        if (shapeGroup_) {
            QList<QAction*> actions = shapeGroup_->actions();
            if (actions.size() > 1) {
                actions[1]->trigger(); // Circular
            }
        }
    });

    // Layer navigation hotkeys
    QShortcut* layerUpShortcut = new QShortcut(QKeySequence("Shift+Up"), mainWindow_);
    connect(layerUpShortcut, &QShortcut::activated, this, [this]() {
        if (layerComboBox_) {
            int current = layerComboBox_->currentIndex();
            if (current > 0) {
                layerComboBox_->setCurrentIndex(current - 1);
            }
        }
    });

    QShortcut* layerDownShortcut = new QShortcut(QKeySequence("Shift+Down"), mainWindow_);
    connect(layerDownShortcut, &QShortcut::activated, this, [this]() {
        if (layerComboBox_) {
            int current = layerComboBox_->currentIndex();
            if (current < layerComboBox_->count() - 1) {
                layerComboBox_->setCurrentIndex(current + 1);
            }
        }
    });

    qDebug() << "ToolBarManager::setupHotkeys: Toolbar hotkeys configured";
}

void ToolBarManager::updateMapContext(const QString& mapName, int currentLayer, int zoomLevel)
{
    // Update toolbar controls based on map context
    if (layerComboBox_ && currentLayer >= 0 && currentLayer < layerComboBox_->count()) {
        layerComboBox_->setCurrentIndex(currentLayer);
    }

    if (zoomSpinBox_) {
        zoomSpinBox_->setValue(zoomLevel);
    }

    // Update position controls if map is available
    if (xCoordSpinBox_ && yCoordSpinBox_ && zCoordSpinBox_) {
        // TODO: Set position controls based on current map view center
        // For now, just enable them
        xCoordSpinBox_->setEnabled(!mapName.isEmpty());
        yCoordSpinBox_->setEnabled(!mapName.isEmpty());
        zCoordSpinBox_->setEnabled(!mapName.isEmpty());
    }

    qDebug() << "ToolBarManager::updateMapContext: Map=" << mapName
             << "Layer=" << currentLayer << "Zoom=" << zoomLevel;
}

void ToolBarManager::updateBrushContext(const QString& brushName, int brushSize, bool isCircular)
{
    // Update brush-related toolbar controls
    if (sizeGroup_ && brushSize >= 1 && brushSize <= 7) {
        QList<QAction*> actions = sizeGroup_->actions();
        if (brushSize <= actions.size()) {
            actions[brushSize-1]->setChecked(true);
        }
    }

    if (shapeGroup_) {
        QList<QAction*> actions = shapeGroup_->actions();
        if (actions.size() >= 2) {
            if (isCircular) {
                actions[1]->setChecked(true); // Circular
            } else {
                actions[0]->setChecked(true); // Rectangular
            }
        }
    }

    // Update brush selection in brush group
    if (brushGroup_) {
        // TODO: Find and select the appropriate brush action
        // This would require mapping brush names to action IDs
    }

    qDebug() << "ToolBarManager::updateBrushContext: Brush=" << brushName
             << "Size=" << brushSize << "Circular=" << isCircular;
}

#include "ToolBarManager.moc"
