// WaypointCreationDialog.cpp - Dialog for creating new waypoints (Task 71)

#include "WaypointEditorPanel.h"
#include "Map.h"
#include "Waypoint.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QToolButton>
#include <QTextEdit>
#include <QColorDialog>
#include <QMessageBox>
#include <QRegExpValidator>

// WaypointCreationDialog implementation
WaypointCreationDialog::WaypointCreationDialog(Map* map, const MapPos& position, QWidget* parent)
    : QDialog(parent)
    , map_(map)
    , initialPosition_(position)
    , selectedColor_(Qt::red) {
    
    setWindowTitle("Create New Waypoint");
    setModal(true);
    setMinimumSize(400, 350);
    
    setupUI();
    connectSignals();
    
    // Set initial values
    if (position.isValid()) {
        setWaypointPosition(position);
    }
    
    // Generate default name
    if (map_) {
        QString defaultName = map_->generateUniqueWaypointName("Waypoint");
        setWaypointName(defaultName);
    }
    
    onValidateInput();
}

WaypointCreationDialog::~WaypointCreationDialog() {
    // Qt handles cleanup automatically
}

void WaypointCreationDialog::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(10);
    
    // Title
    QLabel* titleLabel = new QLabel("Create New Waypoint", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout_->addWidget(titleLabel);
    
    // Form layout
    QWidget* formWidget = new QWidget(this);
    formLayout_ = new QGridLayout(formWidget);
    formLayout_->setContentsMargins(0, 0, 0, 0);
    formLayout_->setSpacing(8);
    
    int row = 0;
    
    // Name
    formLayout_->addWidget(new QLabel("Name:"), row, 0);
    nameEdit_ = new QLineEdit(formWidget);
    nameEdit_->setPlaceholderText("Enter waypoint name");
    nameEdit_->setMaxLength(50);
    
    // Add name validation
    QRegExpValidator* nameValidator = new QRegExpValidator(QRegExp("[A-Za-z0-9 _-]+"), this);
    nameEdit_->setValidator(nameValidator);
    
    formLayout_->addWidget(nameEdit_, row, 1, 1, 2);
    row++;
    
    // Type
    formLayout_->addWidget(new QLabel("Type:"), row, 0);
    typeCombo_ = new QComboBox(formWidget);
    typeCombo_->setEditable(true);
    typeCombo_->addItems({"temple", "town", "depot", "custom"});
    typeCombo_->setCurrentText("temple");
    formLayout_->addWidget(typeCombo_, row, 1, 1, 2);
    row++;
    
    // Position
    formLayout_->addWidget(new QLabel("Position:"), row, 0);
    
    xSpinBox_ = new QSpinBox(formWidget);
    xSpinBox_->setRange(0, 65535);
    xSpinBox_->setPrefix("X: ");
    xSpinBox_->setValue(initialPosition_.x);
    formLayout_->addWidget(xSpinBox_, row, 1);
    
    ySpinBox_ = new QSpinBox(formWidget);
    ySpinBox_->setRange(0, 65535);
    ySpinBox_->setPrefix("Y: ");
    ySpinBox_->setValue(initialPosition_.y);
    formLayout_->addWidget(ySpinBox_, row, 2);
    row++;
    
    // Floor and Radius
    formLayout_->addWidget(new QLabel("Floor:"), row, 0);
    
    zSpinBox_ = new QSpinBox(formWidget);
    zSpinBox_->setRange(0, 15);
    zSpinBox_->setPrefix("Z: ");
    zSpinBox_->setValue(initialPosition_.z);
    formLayout_->addWidget(zSpinBox_, row, 1);
    
    radiusSpinBox_ = new QSpinBox(formWidget);
    radiusSpinBox_->setRange(1, 100);
    radiusSpinBox_->setPrefix("Radius: ");
    radiusSpinBox_->setSuffix(" tiles");
    radiusSpinBox_->setValue(3);
    formLayout_->addWidget(radiusSpinBox_, row, 2);
    row++;
    
    // Color
    formLayout_->addWidget(new QLabel("Color:"), row, 0);
    colorButton_ = new QToolButton(formWidget);
    colorButton_->setMinimumSize(60, 30);
    colorButton_->setStyleSheet("background-color: #FF0000; border: 1px solid gray;");
    colorButton_->setToolTip("Click to change waypoint color");
    colorButton_->setText("Choose...");
    formLayout_->addWidget(colorButton_, row, 1);
    
    // Preview
    previewLabel_ = new QLabel("Preview: Red waypoint", formWidget);
    previewLabel_->setStyleSheet("color: gray; font-style: italic;");
    formLayout_->addWidget(previewLabel_, row, 2);
    row++;
    
    // Script
    formLayout_->addWidget(new QLabel("Script:"), row, 0, Qt::AlignTop);
    scriptEdit_ = new QTextEdit(formWidget);
    scriptEdit_->setMaximumHeight(80);
    scriptEdit_->setPlaceholderText("Enter waypoint script (optional)");
    formLayout_->addWidget(scriptEdit_, row, 1, 1, 2);
    row++;
    
    mainLayout_->addWidget(formWidget);
    
    // Button box
    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox_->button(QDialogButtonBox::Ok)->setText("Create Waypoint");
    buttonBox_->button(QDialogButtonBox::Ok)->setEnabled(false);
    mainLayout_->addWidget(buttonBox_);
    
    updatePreview();
}

void WaypointCreationDialog::connectSignals() {
    // Form validation
    connect(nameEdit_, &QLineEdit::textChanged, this, &WaypointCreationDialog::onNameChanged);
    connect(xSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &WaypointCreationDialog::onPositionChanged);
    connect(ySpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &WaypointCreationDialog::onPositionChanged);
    connect(zSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &WaypointCreationDialog::onPositionChanged);
    connect(radiusSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, &WaypointCreationDialog::onValidateInput);
    connect(typeCombo_, &QComboBox::currentTextChanged, this, &WaypointCreationDialog::onValidateInput);
    
    // Color button
    connect(colorButton_, &QToolButton::clicked, this, &WaypointCreationDialog::onColorButtonClicked);
    
    // Button box
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// Get methods
QString WaypointCreationDialog::getWaypointName() const {
    return nameEdit_->text().trimmed();
}

QString WaypointCreationDialog::getWaypointType() const {
    return typeCombo_->currentText().trimmed();
}

MapPos WaypointCreationDialog::getWaypointPosition() const {
    return MapPos(xSpinBox_->value(), ySpinBox_->value(), zSpinBox_->value());
}

int WaypointCreationDialog::getWaypointRadius() const {
    return radiusSpinBox_->value();
}

QColor WaypointCreationDialog::getWaypointColor() const {
    return selectedColor_;
}

QString WaypointCreationDialog::getWaypointScript() const {
    return scriptEdit_->toPlainText().trimmed();
}

// Set methods
void WaypointCreationDialog::setWaypointName(const QString& name) {
    nameEdit_->setText(name);
}

void WaypointCreationDialog::setWaypointType(const QString& type) {
    typeCombo_->setCurrentText(type);
}

void WaypointCreationDialog::setWaypointPosition(const MapPos& position) {
    xSpinBox_->setValue(position.x);
    ySpinBox_->setValue(position.y);
    zSpinBox_->setValue(position.z);
}

// Event handlers
void WaypointCreationDialog::onNameChanged() {
    onValidateInput();
    updatePreview();
}

void WaypointCreationDialog::onPositionChanged() {
    onValidateInput();
    updatePreview();
}

void WaypointCreationDialog::onColorButtonClicked() {
    QColor newColor = QColorDialog::getColor(selectedColor_, this, "Select Waypoint Color");
    if (newColor.isValid()) {
        selectedColor_ = newColor;
        colorButton_->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(newColor.name()));
        updatePreview();
    }
}

void WaypointCreationDialog::onValidateInput() {
    bool valid = validateInput();
    buttonBox_->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

bool WaypointCreationDialog::validateInput() const {
    // Check name
    QString name = nameEdit_->text().trimmed();
    if (name.isEmpty()) {
        return false;
    }
    
    // Check if name already exists
    if (map_ && map_->hasWaypoint(name)) {
        return false;
    }
    
    // Check position bounds
    if (map_) {
        int x = xSpinBox_->value();
        int y = ySpinBox_->value();
        if (x < 0 || x >= map_->getWidth() || y < 0 || y >= map_->getHeight()) {
            return false;
        }
    }
    
    return true;
}

void WaypointCreationDialog::updatePreview() {
    QString name = getWaypointName();
    QString type = getWaypointType();
    MapPos pos = getWaypointPosition();
    
    if (name.isEmpty()) {
        previewLabel_->setText("Preview: Enter a name");
        previewLabel_->setStyleSheet("color: red; font-style: italic;");
        return;
    }
    
    if (map_ && map_->hasWaypoint(name)) {
        previewLabel_->setText("Preview: Name already exists!");
        previewLabel_->setStyleSheet("color: red; font-style: italic;");
        return;
    }
    
    QString colorName = selectedColor_.name();
    QString preview = QString("Preview: %1 %2 waypoint at [%3, %4, %5]")
                     .arg(colorName)
                     .arg(type.isEmpty() ? "custom" : type)
                     .arg(pos.x).arg(pos.y).arg(pos.z);
    
    previewLabel_->setText(preview);
    previewLabel_->setStyleSheet("color: green; font-style: italic;");
}
