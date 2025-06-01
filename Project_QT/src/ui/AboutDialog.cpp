#include "AboutDialog.h"
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QSysInfo>
#include <QThread>
#include <QDebug>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , tabWidget_(nullptr)
    , mainLayout_(nullptr)
    , buttonLayout_(nullptr)
    , websiteButton_(nullptr)
    , bugReportButton_(nullptr)
    , updateButton_(nullptr)
    , copyInfoButton_(nullptr)
    , closeButton_(nullptr)
    , logoLabel_(nullptr)
    , titleLabel_(nullptr)
    , versionLabel_(nullptr)
    , descriptionLabel_(nullptr)
    , copyrightLabel_(nullptr)
    , infoTextEdit_(nullptr)
    , creditsTextEdit_(nullptr)
    , licenseTextEdit_(nullptr)
    , systemInfoTextEdit_(nullptr)
{
    setWindowTitle(tr("About %1").arg(QApplication::applicationName()));
    setWindowIcon(QIcon(":/icons/about.png"));
    resize(600, 500);
    setModal(true);

    setupUI();
    connectSignals();
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::setupUI() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(15, 15, 15, 15);
    mainLayout_->setSpacing(15);

    setupTabWidget();
    setupButtonBox();

    mainLayout_->addWidget(tabWidget_);
    mainLayout_->addLayout(buttonLayout_);
}

void AboutDialog::setupTabWidget() {
    tabWidget_ = new QTabWidget(this);
    tabWidget_->setTabPosition(QTabWidget::North);
    tabWidget_->setMovable(false);
    tabWidget_->setTabsClosable(false);

    // Add all tabs
    tabWidget_->addTab(createAboutTab(), tr("About"));
    tabWidget_->addTab(createCreditsTab(), tr("Credits"));
    tabWidget_->addTab(createLicenseTab(), tr("License"));
    tabWidget_->addTab(createSystemInfoTab(), tr("System Info"));
}

void AboutDialog::setupButtonBox() {
    buttonLayout_ = new QHBoxLayout();
    buttonLayout_->setSpacing(10);

    websiteButton_ = new QPushButton(tr("&Website"), this);
    websiteButton_->setToolTip(tr("Visit the project website"));
    websiteButton_->setIcon(QIcon(":/icons/web.png"));

    bugReportButton_ = new QPushButton(tr("&Report Bug"), this);
    bugReportButton_->setToolTip(tr("Report a bug or issue"));
    bugReportButton_->setIcon(QIcon(":/icons/bug.png"));

    updateButton_ = new QPushButton(tr("Check &Updates"), this);
    updateButton_->setToolTip(tr("Check for application updates"));
    updateButton_->setIcon(QIcon(":/icons/update.png"));

    copyInfoButton_ = new QPushButton(tr("&Copy Info"), this);
    copyInfoButton_->setToolTip(tr("Copy system information to clipboard"));
    copyInfoButton_->setIcon(QIcon(":/icons/copy.png"));

    buttonLayout_->addWidget(websiteButton_);
    buttonLayout_->addWidget(bugReportButton_);
    buttonLayout_->addWidget(updateButton_);
    buttonLayout_->addWidget(copyInfoButton_);
    buttonLayout_->addStretch();

    closeButton_ = new QPushButton(tr("&Close"), this);
    closeButton_->setToolTip(tr("Close this dialog"));
    closeButton_->setDefault(true);
    closeButton_->setIcon(QIcon(":/icons/close.png"));

    buttonLayout_->addWidget(closeButton_);
}
