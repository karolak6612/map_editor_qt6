#include "ExportMiniMapDialog.h"
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>

ExportMiniMapDialog::ExportMiniMapDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Export Minimap"));
    setModal(true);
    resize(450, 350);
    
    setupUi();
    
    // Connect signals
    connect(browseButton_, &QPushButton::clicked, this, &ExportMiniMapDialog::onBrowseClicked);
    connect(exportTypeComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ExportMiniMapDialog::onExportTypeChanged);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &ExportMiniMapDialog::onAccepted);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ExportMiniMapDialog::~ExportMiniMapDialog() {
    // Qt's parent-child ownership handles cleanup
}

void ExportMiniMapDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Output directory group
    QGroupBox* outputGroup = new QGroupBox(tr("Output Location"), this);
    QVBoxLayout* outputLayout = new QVBoxLayout(outputGroup);
    
    QHBoxLayout* directoryLayout = new QHBoxLayout();
    outputDirectoryLineEdit_ = new QLineEdit(this);
    outputDirectoryLineEdit_->setPlaceholderText(tr("Select output directory..."));
    browseButton_ = new QPushButton(tr("Browse..."), this);
    
    directoryLayout->addWidget(outputDirectoryLineEdit_);
    directoryLayout->addWidget(browseButton_);
    outputLayout->addLayout(directoryLayout);
    
    // File name
    QFormLayout* fileLayout = new QFormLayout();
    fileNameLineEdit_ = new QLineEdit(this);
    fileNameLineEdit_->setText("minimap"); // Default filename
    fileNameLineEdit_->setToolTip(tr("Base filename for exported minimap files"));
    fileLayout->addRow(tr("File Name:"), fileNameLineEdit_);
    outputLayout->addLayout(fileLayout);
    
    mainLayout->addWidget(outputGroup);
    
    // Floor range group
    QGroupBox* floorGroup = new QGroupBox(tr("Floor Range"), this);
    QFormLayout* floorLayout = new QFormLayout(floorGroup);
    
    startFloorSpinBox_ = new QSpinBox(this);
    startFloorSpinBox_->setRange(0, 15); // Typical floor range for Tibia maps
    startFloorSpinBox_->setValue(7); // Ground floor
    startFloorSpinBox_->setToolTip(tr("Starting floor to export (0 = highest floor)"));
    
    endFloorSpinBox_ = new QSpinBox(this);
    endFloorSpinBox_->setRange(0, 15);
    endFloorSpinBox_->setValue(7); // Ground floor
    endFloorSpinBox_->setToolTip(tr("Ending floor to export (0 = highest floor)"));
    
    floorLayout->addRow(tr("Start Floor:"), startFloorSpinBox_);
    floorLayout->addRow(tr("End Floor:"), endFloorSpinBox_);
    
    mainLayout->addWidget(floorGroup);
    
    // Export options group
    QGroupBox* optionsGroup = new QGroupBox(tr("Export Options"), this);
    QFormLayout* optionsLayout = new QFormLayout(optionsGroup);
    
    exportTypeComboBox_ = new QComboBox(this);
    exportTypeComboBox_->addItems({
        tr("PNG Images"),
        tr("BMP Images"),
        tr("Single Combined Image"),
        tr("Tiled Images")
    });
    exportTypeComboBox_->setToolTip(tr("Format for exported minimap"));
    
    exportEmptyFloorsCheckBox_ = new QCheckBox(tr("Export empty floors"), this);
    exportEmptyFloorsCheckBox_->setToolTip(tr("Include floors with no tiles in the export"));
    exportEmptyFloorsCheckBox_->setChecked(false);
    
    optionsLayout->addRow(tr("Export Type:"), exportTypeComboBox_);
    optionsLayout->addRow(exportEmptyFloorsCheckBox_);
    
    mainLayout->addWidget(optionsGroup);
    
    // Add stretch to push buttons to bottom
    mainLayout->addStretch();
    
    // Create standard dialog buttons
    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox_);
    
    setLayout(mainLayout);
}

void ExportMiniMapDialog::onBrowseClicked() {
    QString directory = QFileDialog::getExistingDirectory(
        this,
        tr("Select Output Directory"),
        outputDirectoryLineEdit_->text()
    );
    
    if (!directory.isEmpty()) {
        outputDirectoryLineEdit_->setText(directory);
    }
}

void ExportMiniMapDialog::onExportTypeChanged() {
    // Could add logic here to enable/disable certain options based on export type
    // For now, just a placeholder for future functionality
}

void ExportMiniMapDialog::onAccepted() {
    // Validate that an output directory has been selected
    if (outputDirectoryLineEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please select an output directory."));
        return; // Don't close the dialog
    }
    
    // Validate that a filename has been provided
    if (fileNameLineEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please provide a filename."));
        return; // Don't close the dialog
    }
    
    // Validate floor range
    if (startFloorSpinBox_->value() > endFloorSpinBox_->value()) {
        QMessageBox::warning(this, tr("Error"), tr("Start floor cannot be higher than end floor."));
        return; // Don't close the dialog
    }
    
    // Accept the dialog
    accept();
}

// Getters for export settings
QString ExportMiniMapDialog::getOutputDirectory() const {
    return outputDirectoryLineEdit_->text().trimmed();
}

QString ExportMiniMapDialog::getFileName() const {
    return fileNameLineEdit_->text().trimmed();
}

int ExportMiniMapDialog::getStartFloor() const {
    return startFloorSpinBox_->value();
}

int ExportMiniMapDialog::getEndFloor() const {
    return endFloorSpinBox_->value();
}

int ExportMiniMapDialog::getExportType() const {
    return exportTypeComboBox_->currentIndex();
}

bool ExportMiniMapDialog::shouldExportEmptyFloors() const {
    return exportEmptyFloorsCheckBox_->isChecked();
}

// Setters for default values
void ExportMiniMapDialog::setOutputDirectory(const QString& directory) {
    outputDirectoryLineEdit_->setText(directory);
}

void ExportMiniMapDialog::setFileName(const QString& fileName) {
    fileNameLineEdit_->setText(fileName);
}

void ExportMiniMapDialog::setFloorRange(int startFloor, int endFloor) {
    startFloorSpinBox_->setValue(startFloor);
    endFloorSpinBox_->setValue(endFloor);
}
