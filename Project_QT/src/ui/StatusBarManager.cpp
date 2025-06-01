#include "StatusBarManager.h"
#include "Item.h"
#include <QDebug>
#include <QtMath>

StatusBarManager::StatusBarManager(QStatusBar* statusBar, QObject* parent)
    : QObject(parent)
    , statusBar_(statusBar)
    , mouseCoordsLabel_(nullptr)
    , itemInfoLabel_(nullptr)
    , zoomLevelLabel_(nullptr)
    , currentLayerLabel_(nullptr)
    , brushInfoLabel_(nullptr)
    , actionIdLabel_(nullptr)
    , drawingModeLabel_(nullptr)
    , toolbarStateLabel_(nullptr)
{
    Q_ASSERT(statusBar_);
}

void StatusBarManager::setupStatusBar()
{
    if (!statusBar_) {
        qWarning("StatusBarManager: StatusBar is null!");
        return;
    }

    setupStatusBarFields();
    setupStatusBarStyling();
    initializeValues();

    statusBar_->showMessage(tr("Ready"), 2000);
    qDebug() << "StatusBarManager: Enhanced status bar setup complete.";
}

void StatusBarManager::setupStatusBarFields()
{
    // Initialize and add labels as permanent widgets
    // Permanent widgets are added from right to left in Qt
    
    // Current layer/floor (rightmost)
    currentLayerLabel_ = new QLabel(statusBar_);
    currentLayerLabel_->setText(tr("Floor: 7"));
    currentLayerLabel_->setToolTip(tr("Current map floor/layer (PageUp/PageDown to change)"));
    currentLayerLabel_->setMinimumWidth(80);
    statusBar_->addPermanentWidget(currentLayerLabel_);
    
    // Add separator
    QFrame* separator1 = createStatusBarSeparator();
    statusBar_->addPermanentWidget(separator1);

    // Zoom level
    zoomLevelLabel_ = new QLabel(statusBar_);
    zoomLevelLabel_->setText(tr("Zoom: 100%"));
    zoomLevelLabel_->setToolTip(tr("Current map zoom level (Ctrl+Plus/Minus to change)"));
    zoomLevelLabel_->setMinimumWidth(90);
    statusBar_->addPermanentWidget(zoomLevelLabel_);
    
    // Add separator
    QFrame* separator2 = createStatusBarSeparator();
    statusBar_->addPermanentWidget(separator2);

    // Brush information
    brushInfoLabel_ = new QLabel(statusBar_);
    brushInfoLabel_->setText(tr("Brush: None"));
    brushInfoLabel_->setToolTip(tr("Currently selected brush tool"));
    brushInfoLabel_->setMinimumWidth(120);
    statusBar_->addPermanentWidget(brushInfoLabel_);
    
    // Add separator
    QFrame* separator3 = createStatusBarSeparator();
    statusBar_->addPermanentWidget(separator3);

    // Task 77: Action ID information
    actionIdLabel_ = new QLabel(statusBar_);
    actionIdLabel_->setText(tr("AID: Off"));
    actionIdLabel_->setToolTip(tr("Action ID for placed items (from palette)"));
    actionIdLabel_->setMinimumWidth(80);
    statusBar_->addPermanentWidget(actionIdLabel_);

    // Add separator
    QFrame* separator4a = createStatusBarSeparator();
    statusBar_->addPermanentWidget(separator4a);

    // Task 77: Drawing mode information
    drawingModeLabel_ = new QLabel(statusBar_);
    drawingModeLabel_->setText(tr("Mode: None"));
    drawingModeLabel_->setToolTip(tr("Current drawing/editing mode"));
    drawingModeLabel_->setMinimumWidth(100);
    statusBar_->addPermanentWidget(drawingModeLabel_);

    // Add separator
    QFrame* separator4b = createStatusBarSeparator();
    statusBar_->addPermanentWidget(separator4b);

    // Enhanced item information
    itemInfoLabel_ = new QLabel(statusBar_);
    itemInfoLabel_->setText(tr("Item: None"));
    itemInfoLabel_->setToolTip(tr("Information about item under cursor or selected"));
    itemInfoLabel_->setMinimumWidth(200);
    statusBar_->addPermanentWidget(itemInfoLabel_);
    
    // Add separator
    QFrame* separator4 = createStatusBarSeparator();
    statusBar_->addPermanentWidget(separator4);

    // Mouse coordinates (leftmost of permanent widgets)
    mouseCoordsLabel_ = new QLabel(statusBar_);
    mouseCoordsLabel_->setText(tr("X: -, Y: -, Z: -"));
    mouseCoordsLabel_->setToolTip(tr("Current map coordinates under mouse cursor"));
    mouseCoordsLabel_->setMinimumWidth(150);
    statusBar_->addPermanentWidget(mouseCoordsLabel_);
}

void StatusBarManager::setupStatusBarStyling()
{
    // Apply consistent styling to status bar
    statusBar_->setStyleSheet(
        "QStatusBar {"
        "    border-top: 1px solid #c0c0c0;"
        "    background-color: #f0f0f0;"
        "}"
        "QStatusBar QLabel {"
        "    padding: 2px 4px;"
        "    border: none;"
        "    background-color: transparent;"
        "}"
        "QStatusBar QFrame {"
        "    color: #c0c0c0;"
        "}"
    );
}

void StatusBarManager::initializeValues()
{
    // Initialize status bar with default values
    updateMouseMapCoordinates(QPointF(-1, -1), 7);
    updateZoomLevel(1.0);
    updateCurrentLayer(7);
    updateCurrentBrush(tr("None"));
    updateSelectedItemInfo(tr("None"));
}

QFrame* StatusBarManager::createStatusBarSeparator()
{
    QFrame* separator = new QFrame(statusBar_);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedWidth(1);
    separator->setFixedHeight(16);
    return separator;
}

void StatusBarManager::updateMouseMapCoordinates(const QPointF& mapPos, int floor)
{
    if (mouseCoordsLabel_) {
        // Enhanced coordinate display with validation
        if (mapPos.x() < 0 || mapPos.y() < 0) {
            mouseCoordsLabel_->setText(tr("X: -, Y: -, Z: %1").arg(floor));
        } else {
            mouseCoordsLabel_->setText(QString("X: %1, Y: %2, Z: %3")
                                         .arg(qRound(mapPos.x()))
                                         .arg(qRound(mapPos.y()))
                                         .arg(floor));
        }
        
        // Update tooltip with additional information
        if (mapPos.x() >= 0 && mapPos.y() >= 0) {
            mouseCoordsLabel_->setToolTip(tr("Map coordinates: (%1, %2, %3)\nClick to go to position")
                                            .arg(qRound(mapPos.x()))
                                            .arg(qRound(mapPos.y()))
                                            .arg(floor));
        } else {
            mouseCoordsLabel_->setToolTip(tr("Current map coordinates under mouse cursor"));
        }
    }
}

void StatusBarManager::updateZoomLevel(double zoom)
{
    if (zoomLevelLabel_) {
        int zoomPercent = static_cast<int>(zoom * 100);
        zoomLevelLabel_->setText(QString("Zoom: %1%").arg(zoomPercent));
        
        // Update tooltip with zoom information
        zoomLevelLabel_->setToolTip(tr("Current zoom level: %1%\nCtrl+Plus/Minus to zoom\nCtrl+0 to reset")
                                      .arg(zoomPercent));
    }
}

void StatusBarManager::updateCurrentLayer(int layer)
{
    if (currentLayerLabel_) {
        currentLayerLabel_->setText(QString("Floor: %1").arg(layer));
        
        // Update tooltip with layer information
        QString layerName = getLayerName(layer);
        currentLayerLabel_->setToolTip(tr("Current floor: %1 (%2)\nPageUp/PageDown to change")
                                         .arg(layer)
                                         .arg(layerName));
    }
}

void StatusBarManager::updateCurrentBrush(const QString& brushName)
{
    if (brushInfoLabel_) {
        QString displayName = brushName.isEmpty() ? "None" : brushName;
        brushInfoLabel_->setText(QString("Brush: %1").arg(displayName));
        
        // Update tooltip with brush information
        if (brushName.isEmpty()) {
            brushInfoLabel_->setToolTip(tr("No brush selected\nSelect a brush from the palette"));
        } else {
            brushInfoLabel_->setToolTip(tr("Active brush: %1\nClick to change brush settings").arg(brushName));
        }
    }
}

void StatusBarManager::updateSelectedItemInfo(const QString& itemInfo)
{
    if (itemInfoLabel_) {
        QString displayText = itemInfo.isEmpty() ? "Item: None" : itemInfo;
        itemInfoLabel_->setText(displayText);
        
        // Update tooltip with item information
        if (itemInfo.isEmpty()) {
            itemInfoLabel_->setToolTip(tr("No item selected\nHover over items to see information"));
        } else {
            itemInfoLabel_->setToolTip(tr("Item information: %1\nRight-click for properties").arg(itemInfo));
        }
    }
}

void StatusBarManager::updateSelectedItemInfo(const Item* item)
{
    if (!item) {
        updateSelectedItemInfo(QString());
        return;
    }
    
    // Create comprehensive item information string
    QString itemInfo = QString("Item: %1 (ID: %2)")
                         .arg(item->name().isEmpty() ? "Unknown" : item->name())
                         .arg(item->getServerId());
    
    // Add additional information if available
    QStringList additionalInfo;
    
    if (item->getCount() > 1) {
        additionalInfo << QString("Count: %1").arg(item->getCount());
    }
    
    if (item->getCharges() > 0) {
        additionalInfo << QString("Charges: %1").arg(item->getCharges());
    }
    
    if (item->getActionId() > 0) {
        additionalInfo << QString("Action: %1").arg(item->getActionId());
    }
    
    if (!additionalInfo.isEmpty()) {
        itemInfo += QString(" [%1]").arg(additionalInfo.join(", "));
    }
    
    updateSelectedItemInfo(itemInfo);
}

QString StatusBarManager::getLayerName(int layer) const
{
    // Convert layer number to descriptive name
    if (layer == 0) return tr("Sea Level");
    else if (layer == 7) return tr("Ground");
    else if (layer < 7) return tr("Underground %1").arg(7 - layer);
    else return tr("Above Ground %1").arg(layer - 7);
}

void StatusBarManager::updateProgress(const QString& operation, int progress)
{
    // Show progress for long operations
    if (progress < 0) {
        statusBar_->clearMessage();
    } else if (progress >= 100) {
        showTemporaryMessage(tr("%1 completed").arg(operation), 2000);
    } else {
        statusBar_->showMessage(tr("%1... %2%").arg(operation).arg(progress));
    }
}

void StatusBarManager::showTemporaryMessage(const QString& message, int timeout)
{
    statusBar_->showMessage(message, timeout);
}

void StatusBarManager::handleStatusUpdateRequest(const QString& type, const QVariantMap& data)
{
    // Handle status update requests from MapView and other components
    if (type == "coordinates") {
        QPointF mapPos(data.value("x").toDouble(), data.value("y").toDouble());
        int floor = data.value("z").toInt();
        updateMouseMapCoordinates(mapPos, floor);
    } else if (type == "zoom") {
        double zoomLevel = data.value("level").toDouble();
        updateZoomLevel(zoomLevel);
    } else if (type == "floor") {
        int layer = data.value("layer").toInt();
        updateCurrentLayer(layer);
    } else if (type == "brush") {
        QString brushName = data.value("name").toString();
        updateCurrentBrush(brushName);
    } else if (type == "item") {
        QString itemInfo = data.value("info").toString();
        updateSelectedItemInfo(itemInfo);
    } else if (type == "actionId") {
        quint16 actionId = data.value("id").toUInt();
        bool enabled = data.value("enabled").toBool();
        updateActionId(actionId, enabled);
    } else if (type == "drawingMode") {
        QString modeName = data.value("name").toString();
        QString description = data.value("description").toString();
        updateDrawingMode(modeName, description);
    } else if (type == "toolbarState") {
        QString toolName = data.value("tool").toString();
        bool active = data.value("active").toBool();
        updateToolbarState(toolName, active);
    } else {
        qDebug() << "StatusBarManager::handleStatusUpdateRequest: Unknown status type:" << type;
    }
}

// Task 77: Enhanced status update method implementations
void StatusBarManager::updateActionId(quint16 actionId, bool enabled)
{
    if (actionIdLabel_) {
        if (enabled && actionId > 0) {
            actionIdLabel_->setText(QString("AID: %1").arg(actionId));
            actionIdLabel_->setToolTip(tr("Action ID enabled: %1\nItems placed will have this action ID").arg(actionId));
        } else {
            actionIdLabel_->setText(tr("AID: Off"));
            actionIdLabel_->setToolTip(tr("Action ID disabled\nItems placed will not have an action ID"));
        }
    }
}

void StatusBarManager::updateDrawingMode(const QString& modeName, const QString& description)
{
    if (drawingModeLabel_) {
        QString displayName = modeName.isEmpty() ? "None" : modeName;
        drawingModeLabel_->setText(QString("Mode: %1").arg(displayName));

        if (description.isEmpty()) {
            drawingModeLabel_->setToolTip(tr("Current drawing mode: %1").arg(displayName));
        } else {
            drawingModeLabel_->setToolTip(tr("Current drawing mode: %1\n%2").arg(displayName, description));
        }
    }
}

void StatusBarManager::updateToolbarState(const QString& toolName, bool active)
{
    if (toolbarStateLabel_) {
        if (active && !toolName.isEmpty()) {
            toolbarStateLabel_->setText(QString("Tool: %1").arg(toolName));
            toolbarStateLabel_->setToolTip(tr("Active tool: %1").arg(toolName));
        } else {
            toolbarStateLabel_->setText(tr("Tool: None"));
            toolbarStateLabel_->setToolTip(tr("No active tool selected"));
        }
    }
}

#include "StatusBarManager.moc"
