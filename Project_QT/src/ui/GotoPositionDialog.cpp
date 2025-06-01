#include "GotoPositionDialog.h"
#include <QTextEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpression>

GotoPositionDialog::GotoPositionDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Go To Position"));
    setModal(true);
    resize(400, 300);
    
    setupUi();
    
    // Connect signals
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &GotoPositionDialog::onAccepted);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

GotoPositionDialog::~GotoPositionDialog() {
    // Qt's parent-child ownership handles cleanup
}

void GotoPositionDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Input group
    QGroupBox* inputGroup = new QGroupBox(tr("Enter Position"), this);
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
    
    // Format hint label (matching wxWidgets version)
    formatHintLabel_ = new QLabel(this);
    formatHintLabel_->setText(tr("Supported formats:\n"
                                "{x = 0, y = 0, z = 0}\n"
                                "{\"x\":0,\"y\":0,\"z\":0}\n"
                                "x, y, z\n"
                                "(x, y, z)\n"
                                "Position(x, y, z)"));
    formatHintLabel_->setWordWrap(true);
    formatHintLabel_->setStyleSheet("QLabel { color: gray; font-size: 9pt; }");
    
    inputLayout->addWidget(formatHintLabel_);
    
    // Position input text edit
    positionTextEdit_ = new QTextEdit(this);
    positionTextEdit_->setMaximumHeight(60); // Keep it relatively small
    positionTextEdit_->setPlaceholderText(tr("Enter position coordinates..."));
    
    inputLayout->addWidget(positionTextEdit_);
    
    mainLayout->addWidget(inputGroup);
    
    // Add stretch to push buttons to bottom
    mainLayout->addStretch();
    
    // Create standard dialog buttons
    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox_);
    
    setLayout(mainLayout);
    
    // Set focus to the text edit
    positionTextEdit_->setFocus();
}

void GotoPositionDialog::onAccepted() {
    QString input = positionTextEdit_->toPlainText().trimmed();
    
    if (input.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter a position."));
        return; // Don't close the dialog
    }
    
    position_ = parsePosition(input);
    
    if (!position_.isValid()) {
        QMessageBox::warning(this, tr("Error"), 
                           tr("Invalid position format or coordinates.\n"
                              "Please check the supported formats and try again."));
        return; // Don't close the dialog
    }
    
    // Accept the dialog
    accept();
}

Position GotoPositionDialog::parsePosition(const QString& input) const {
    Position pos;
    
    // Try different parsing patterns (matching wxWidgets functionality)
    QRegularExpression patterns[] = {
        // {x = 123, y = 456, z = 7}
        QRegularExpression(R"(\{\s*x\s*=\s*(\d+)\s*,\s*y\s*=\s*(\d+)\s*,\s*z\s*=\s*(\d+)\s*\})"),
        // {"x":123,"y":456,"z":7}
        QRegularExpression(R"(\{\s*"x"\s*:\s*(\d+)\s*,\s*"y"\s*:\s*(\d+)\s*,\s*"z"\s*:\s*(\d+)\s*\})"),
        // 123, 456, 7
        QRegularExpression(R"((\d+)\s*,\s*(\d+)\s*,\s*(\d+))"),
        // (123, 456, 7)
        QRegularExpression(R"(\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\))"),
        // Position(123, 456, 7)
        QRegularExpression(R"(Position\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\))")
    };
    
    for (const auto& pattern : patterns) {
        QRegularExpressionMatch match = pattern.match(input);
        if (match.hasMatch()) {
            bool ok1, ok2, ok3;
            int x = match.captured(1).toInt(&ok1);
            int y = match.captured(2).toInt(&ok2);
            int z = match.captured(3).toInt(&ok3);
            
            if (ok1 && ok2 && ok3) {
                pos.x = x;
                pos.y = y;
                pos.z = z;
                break;
            }
        }
    }
    
    return pos;
}

// Getter for parsed position
Position GotoPositionDialog::getPosition() const {
    return position_;
}

// Setter for default position
void GotoPositionDialog::setPosition(const Position& position) {
    position_ = position;
    positionTextEdit_->setPlainText(QString("%1, %2, %3").arg(position.x).arg(position.y).arg(position.z));
}
