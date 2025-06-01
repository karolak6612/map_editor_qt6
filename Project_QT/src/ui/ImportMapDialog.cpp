#include "ImportMapDialog.h"
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>

ImportMapDialog::ImportMapDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Import Map"));
    setModal(true);
    resize(400, 300);
    
    setupUi();
    
    // Connect signals
    connect(browseButton_, &QPushButton::clicked, this, &ImportMapDialog::onBrowseClicked);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &ImportMapDialog::onAccepted);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ImportMapDialog::~ImportMapDialog() {
    // Qt's parent-child ownership handles cleanup
}

void ImportMapDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // File selection group
    QGroupBox* fileGroup = new QGroupBox(tr("Map File"), this);
    QHBoxLayout* fileLayout = new QHBoxLayout(fileGroup);
    
    filePathLineEdit_ = new QLineEdit(this);
    filePathLineEdit_->setPlaceholderText(tr("Select map file to import..."));
    browseButton_ = new QPushButton(tr("Browse..."), this);
    
    fileLayout->addWidget(filePathLineEdit_);
    fileLayout->addWidget(browseButton_);
    
    mainLayout->addWidget(fileGroup);
    
    // Offset settings group
    QGroupBox* offsetGroup = new QGroupBox(tr("Position Offset"), this);
    QFormLayout* offsetLayout = new QFormLayout(offsetGroup);
    
    xOffsetSpinBox_ = new QSpinBox(this);
    xOffsetSpinBox_->setRange(-32768, 32767); // Reasonable range for map coordinates
    xOffsetSpinBox_->setValue(0);
    xOffsetSpinBox_->setToolTip(tr("X coordinate offset for imported map"));
    
    yOffsetSpinBox_ = new QSpinBox(this);
    yOffsetSpinBox_->setRange(-32768, 32767);
    yOffsetSpinBox_->setValue(0);
    yOffsetSpinBox_->setToolTip(tr("Y coordinate offset for imported map"));
    
    offsetLayout->addRow(tr("X Offset:"), xOffsetSpinBox_);
    offsetLayout->addRow(tr("Y Offset:"), yOffsetSpinBox_);
    
    mainLayout->addWidget(offsetGroup);
    
    // Import options group
    QGroupBox* optionsGroup = new QGroupBox(tr("Import Options"), this);
    QFormLayout* optionsLayout = new QFormLayout(optionsGroup);
    
    houseOptionsComboBox_ = new QComboBox(this);
    houseOptionsComboBox_->addItems({
        tr("Don't import houses"),
        tr("Import houses"),
        tr("Merge with existing houses")
    });
    houseOptionsComboBox_->setToolTip(tr("How to handle house data during import"));
    
    spawnOptionsComboBox_ = new QComboBox(this);
    spawnOptionsComboBox_->addItems({
        tr("Don't import spawns"),
        tr("Import spawns"),
        tr("Merge with existing spawns")
    });
    spawnOptionsComboBox_->setToolTip(tr("How to handle spawn data during import"));
    
    optionsLayout->addRow(tr("Houses:"), houseOptionsComboBox_);
    optionsLayout->addRow(tr("Spawns:"), spawnOptionsComboBox_);
    
    mainLayout->addWidget(optionsGroup);
    
    // Add stretch to push buttons to bottom
    mainLayout->addStretch();
    
    // Create standard dialog buttons
    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox_);
    
    setLayout(mainLayout);
}

void ImportMapDialog::onBrowseClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select Map File"),
        QString(), // Default directory
        tr("Map Files (*.otbm *.xml);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        filePathLineEdit_->setText(fileName);
    }
}

void ImportMapDialog::onAccepted() {
    // Validate that a file has been selected
    if (filePathLineEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please select a map file to import."));
        return; // Don't close the dialog
    }
    
    // Accept the dialog
    accept();
}

// Getters for import settings
QString ImportMapDialog::getFilePath() const {
    return filePathLineEdit_->text().trimmed();
}

int ImportMapDialog::getXOffset() const {
    return xOffsetSpinBox_->value();
}

int ImportMapDialog::getYOffset() const {
    return yOffsetSpinBox_->value();
}

int ImportMapDialog::getHouseOption() const {
    return houseOptionsComboBox_->currentIndex();
}

int ImportMapDialog::getSpawnOption() const {
    return spawnOptionsComboBox_->currentIndex();
}

// Setters for default values
void ImportMapDialog::setFilePath(const QString& path) {
    filePathLineEdit_->setText(path);
}

void ImportMapDialog::setXOffset(int offset) {
    xOffsetSpinBox_->setValue(offset);
}

void ImportMapDialog::setYOffset(int offset) {
    yOffsetSpinBox_->setValue(offset);
}
