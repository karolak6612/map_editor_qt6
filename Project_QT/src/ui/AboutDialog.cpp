#include "AboutDialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout> // For button layout
#include <QScreen> 
#include <QApplication> 
#include <QSysInfo> // For QSysInfo::prettyProductName()

// Placeholder version info - actual values might come from a config file or build system
const QString RME_VERSION_PLACEHOLDER = "Dev Version"; 
// const QString COMPILED_DATE_PLACEHOLDER = __DATE__; // This macro might not be ideal for Qt builds
// const QString COMPILED_TIME_PLACEHOLDER = __TIME__; // This macro might not be ideal for Qt builds

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("About Idler Map Editor")); // Original was "About"

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    infoLabel_ = new QLabel(this);
    infoLabel_->setWordWrap(true);
    infoLabel_->setTextFormat(Qt::PlainText); // Original used wxStaticText which is plain
    infoLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse); // Allow text selection

    // Construct the about text, similar to the wxwidgets version
    QString aboutText;
    aboutText += "Idler Map Editor\n"; // Use \n for newlines in QLabel with PlainText
    aboutText += "(based on OTA Remere's Map Editor)\n\n";
    aboutText += "This program is a map editor for game servers\n";
    aboutText += "that derivied from OpenTibia project.\n\n";
    aboutText += "Brought to you by Idler enhanced using Cursor.com\n\n"; // As per original

    // Qt version instead of wxWidgets version
    // Use QApplication::applicationVersion() if set, otherwise placeholder.
    QString appVersion = QApplication::applicationVersion();
    if (appVersion.isEmpty()) {
        appVersion = RME_VERSION_PLACEHOLDER;
    }
    aboutText += QString("Version %1 for %2\n\n")
                     .arg(appVersion) 
                     .arg(QSysInfo::prettyProductName());

    aboutText += QString("Using Qt version %1\n").arg(QT_VERSION_STR);
    // OpenGL version would require an active GL context to query, omit for this simple dialog for now.
    // aboutText += "OpenGL version ...\n\n"; 

    aboutText += "This program comes with ABSOLUTELY NO WARRANTY;\n";
    aboutText += "for details see the LICENSE file (not shown in this basic dialog).\n";
    aboutText += "This is free software, and you are welcome to redistribute it\n";
    aboutText += "under certain conditions.\n";
    aboutText += "Just make sure that you include the invite link to discord.\n\n"; // As per original

    // Compilation date/time and compiler are harder to get generically in Qt
    // without build system integration. Use placeholders or omit.
    // aboutText += QString("Compiled on: %1 : %2\n").arg(COMPILED_DATE_PLACEHOLDER).arg(COMPILED_TIME_PLACEHOLDER);
    // aboutText += QString("Compiled with: %1\n").arg("Compiler Info Placeholder");

    infoLabel_->setText(aboutText);
    mainLayout->addWidget(infoLabel_);

    // OK Button
    okButton_ = new QPushButton(tr("OK"), this);
    connect(okButton_, &QPushButton::clicked, this, &QDialog::accept); // Closes dialog with QDialog::Accepted result

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton_);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // Adjust initial size - original was wxSize(300, 320)
    // Let's use a slightly wider size for better text flow.
    resize(450, 380); 
}

AboutDialog::~AboutDialog() {
    // Qt's parent-child ownership will handle deleting widgets (infoLabel_, okButton_)
    // and layouts.
}
