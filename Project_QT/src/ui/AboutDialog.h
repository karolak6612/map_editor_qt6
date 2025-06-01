#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QScrollArea>
#include <QGroupBox>
#include <QFrame>
#include <QPixmap>
#include <QIcon>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

/**
 * @brief Enhanced About dialog with comprehensive application information
 *
 * Task 78: Complete migration of wxwidgets AboutWindow with enhanced features:
 * - Application information with version, build date, Qt version
 * - Credits tab with developer information and acknowledgments
 * - License tab with complete license text
 * - System information tab with technical details
 * - Professional appearance with application logo and styling
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override;

private slots:
    void onVisitWebsite();
    void onReportBug();
    void onCheckUpdates();
    void onCopySystemInfo();

private:
    // UI setup methods
    void setupUI();
    void setupTabWidget();
    void setupButtonBox();
    void connectSignals();

    // Tab creation methods
    QWidget* createAboutTab();
    QWidget* createCreditsTab();
    QWidget* createLicenseTab();
    QWidget* createSystemInfoTab();

    // Helper methods
    QString getApplicationInfo() const;
    QString getSystemInfo() const;
    QString getBuildInfo() const;
    QString getCreditsText() const;
    QString getLicenseText() const;
    QPixmap getApplicationLogo() const;

    // UI components
    QTabWidget* tabWidget_;
    QVBoxLayout* mainLayout_;
    QHBoxLayout* buttonLayout_;

    // Buttons
    QPushButton* websiteButton_;
    QPushButton* bugReportButton_;
    QPushButton* updateButton_;
    QPushButton* copyInfoButton_;
    QPushButton* closeButton_;

    // About tab components
    QLabel* logoLabel_;
    QLabel* titleLabel_;
    QLabel* versionLabel_;
    QLabel* descriptionLabel_;
    QLabel* copyrightLabel_;
    QTextEdit* infoTextEdit_;

    // Credits tab components
    QTextEdit* creditsTextEdit_;

    // License tab components
    QTextEdit* licenseTextEdit_;

    // System info tab components
    QTextEdit* systemInfoTextEdit_;
};

#endif // ABOUTDIALOG_H
