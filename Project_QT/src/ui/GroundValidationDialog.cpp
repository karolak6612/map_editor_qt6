#include "GroundValidationDialog.h"
#include <QCheckBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>

GroundValidationDialog::GroundValidationDialog(QWidget *parent)
    : QDialog(parent),
      validateGroundStack_(false),
      generateEmptySurroundedGrounds_(false),
      removeDuplicateGrounds_(false)
{
    setWindowTitle(tr("Ground Tile Validation"));
    setFixedSize(350, 250); // Similar to wxWidgets version (300, 200) but slightly larger for better spacing
    
    setupUi();
    
    // Connect signals
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &GroundValidationDialog::onAccepted);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

GroundValidationDialog::~GroundValidationDialog() {
    // Qt's parent-child ownership handles cleanup
}

void GroundValidationDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create checkboxes with tooltips (matching wxWidgets functionality)
    validateGroundStackBox_ = new QCheckBox(tr("Validate ground stack order"), this);
    validateGroundStackBox_->setToolTip(tr("Move ground tiles to bottom of stack if they are above other items"));
    
    generateEmptySurroundedGroundsBox_ = new QCheckBox(tr("Generate empty surrounded grounds"), this);
    generateEmptySurroundedGroundsBox_->setToolTip(tr("Fill empty ground tiles that are surrounded by other ground tiles"));
    
    removeDuplicateGroundsBox_ = new QCheckBox(tr("Remove duplicate grounds"), this);
    removeDuplicateGroundsBox_->setToolTip(tr("Remove duplicate ground tiles from the same position"));
    
    // Add checkboxes to layout
    mainLayout->addWidget(validateGroundStackBox_);
    mainLayout->addWidget(generateEmptySurroundedGroundsBox_);
    mainLayout->addWidget(removeDuplicateGroundsBox_);
    
    // Add warning text (matching wxWidgets red color)
    warningLabel_ = new QLabel(tr("Warning: This operation cannot be undone!\nPlease save your map before proceeding."), this);
    warningLabel_->setStyleSheet("QLabel { color: red; }");
    warningLabel_->setAlignment(Qt::AlignCenter);
    warningLabel_->setWordWrap(true);
    
    mainLayout->addWidget(warningLabel_);
    
    // Add some spacing
    mainLayout->addStretch();
    
    // Create standard dialog buttons
    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox_);
    
    setLayout(mainLayout);
}

void GroundValidationDialog::onAccepted() {
    // Store the checkbox states
    validateGroundStack_ = validateGroundStackBox_->isChecked();
    generateEmptySurroundedGrounds_ = generateEmptySurroundedGroundsBox_->isChecked();
    removeDuplicateGrounds_ = removeDuplicateGroundsBox_->isChecked();
    
    // Validation: at least one option must be selected (matching wxWidgets behavior)
    if (!validateGroundStack_ && !generateEmptySurroundedGrounds_ && !removeDuplicateGrounds_) {
        QMessageBox::warning(this, tr("Error"), tr("Please select at least one validation option!"));
        return; // Don't close the dialog
    }
    
    // Accept the dialog
    accept();
}

// Getters for validation options
bool GroundValidationDialog::shouldValidateGroundStack() const {
    return validateGroundStack_;
}

bool GroundValidationDialog::shouldGenerateEmptySurroundedGrounds() const {
    return generateEmptySurroundedGrounds_;
}

bool GroundValidationDialog::shouldRemoveDuplicateGrounds() const {
    return removeDuplicateGrounds_;
}
