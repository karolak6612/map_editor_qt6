#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QProgressDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QElapsedTimer>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QApplication>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMovie>
#include <QPixmap>
#include <QIcon>

/**
 * @brief Task 88: Enhanced ProgressDialog for threaded long operations
 * 
 * Complete progress dialog implementation:
 * - Wrapper around QProgressDialog with enhanced features
 * - Support for threaded operations with cancellation
 * - Consistent look and feel across the application
 * - Automatic UI responsiveness with QApplication::processEvents()
 * - Integration with worker threads and async operations
 * - 1:1 replacement for wxProgressDialog functionality
 */

class ProgressDialog : public QProgressDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget* parent = nullptr);
    explicit ProgressDialog(const QString& labelText, const QString& cancelButtonText, 
                           int minimum, int maximum, QWidget* parent = nullptr);
    ~ProgressDialog() override;

    // Enhanced API (1:1 wxProgressDialog compatibility)
    void setProgress(int value);
    void setProgress(int value, const QString& newLabelText);
    void setLabelText(const QString& text);
    void setTitle(const QString& title);
    void setCancelButtonText(const QString& text);
    
    // Extended functionality
    void setDetailText(const QString& text);
    QString getDetailText() const;
    
    void setShowElapsedTime(bool show);
    bool isShowElapsedTime() const;
    
    void setShowRemainingTime(bool show);
    bool isShowRemainingTime() const;
    
    void setShowProgressRate(bool show);
    bool isShowProgressRate() const;
    
    void setAutoClose(bool autoClose);
    bool isAutoClose() const;
    
    void setAutoReset(bool autoReset);
    bool isAutoReset() const;
    
    // Animation and visual enhancements
    void setShowAnimation(bool show);
    bool isShowAnimation() const;
    
    void setAnimationFile(const QString& filePath);
    void setProgressIcon(const QIcon& icon);
    
    // Threading support
    void setThreadSafe(bool threadSafe);
    bool isThreadSafe() const;
    
    // Statistics
    qint64 getElapsedTime() const;
    qint64 getEstimatedRemainingTime() const;
    double getProgressRate() const; // items per second
    
    // Utility methods
    void reset();
    void forceShow();
    void processEvents();
    
    // Static convenience methods
    static ProgressDialog* create(const QString& title, const QString& labelText, 
                                 int minimum = 0, int maximum = 100, QWidget* parent = nullptr);
    static void showProgress(const QString& title, const QString& labelText, 
                           std::function<void(ProgressDialog*)> operation, QWidget* parent = nullptr);

public slots:
    void updateProgress(int value);
    void updateProgress(int value, const QString& labelText);
    void updateDetailText(const QString& text);
    void finish();
    void cancel();

signals:
    void progressUpdated(int value);
    void labelTextChanged(const QString& text);
    void detailTextChanged(const QString& text);
    void finished();
    void cancelled();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private slots:
    void onUpdateTimer();
    void onCancelClicked();

private:
    void setupUI();
    void updateTimeDisplays();
    void updateProgressRate();
    void updateEstimatedTime();
    void startTimers();
    void stopTimers();
    
    // Thread-safe update methods
    void safeUpdateProgress(int value);
    void safeUpdateLabelText(const QString& text);
    void safeUpdateDetailText(const QString& text);

private:
    // Enhanced UI components
    QLabel* detailLabel_;
    QLabel* elapsedTimeLabel_;
    QLabel* remainingTimeLabel_;
    QLabel* progressRateLabel_;
    QLabel* animationLabel_;
    QMovie* animationMovie_;
    
    // Settings
    bool showElapsedTime_;
    bool showRemainingTime_;
    bool showProgressRate_;
    bool showAnimation_;
    bool autoClose_;
    bool autoReset_;
    bool threadSafe_;
    
    // Timing and statistics
    QElapsedTimer elapsedTimer_;
    QTimer* updateTimer_;
    qint64 lastUpdateTime_;
    int lastProgressValue_;
    double progressRate_;
    
    // Thread safety
    mutable QMutex mutex_;
    
    // Detail text
    QString detailText_;
};

/**
 * @brief Scoped Progress Dialog for RAII-style usage
 */
class ScopedProgressDialog
{
public:
    explicit ScopedProgressDialog(const QString& title, const QString& labelText, 
                                 int minimum = 0, int maximum = 100, QWidget* parent = nullptr);
    ~ScopedProgressDialog();

    // Progress dialog access
    ProgressDialog* dialog() const { return dialog_; }
    ProgressDialog* operator->() const { return dialog_; }
    ProgressDialog& operator*() const { return *dialog_; }

    // Convenience methods
    void setProgress(int value) { dialog_->setProgress(value); }
    void setProgress(int value, const QString& text) { dialog_->setProgress(value, text); }
    void setLabelText(const QString& text) { dialog_->setLabelText(text); }
    bool wasCanceled() const { return dialog_->wasCanceled(); }

private:
    ProgressDialog* dialog_;
};

/**
 * @brief Progress Dialog Manager for application-wide progress tracking
 */
class ProgressDialogManager : public QObject
{
    Q_OBJECT

public:
    static ProgressDialogManager* instance();
    
    // Global progress dialog management
    ProgressDialog* createProgressDialog(const QString& title, const QString& labelText,
                                       int minimum = 0, int maximum = 100, QWidget* parent = nullptr);
    void destroyProgressDialog(ProgressDialog* dialog);
    
    // Active dialog tracking
    QList<ProgressDialog*> getActiveDialogs() const;
    int getActiveDialogCount() const;
    bool hasActiveDialogs() const;
    
    // Global settings
    void setDefaultAutoClose(bool autoClose);
    bool getDefaultAutoClose() const;
    
    void setDefaultShowElapsedTime(bool show);
    bool getDefaultShowElapsedTime() const;
    
    void setDefaultThreadSafe(bool threadSafe);
    bool getDefaultThreadSafe() const;
    
    // Convenience methods for common operations
    ProgressDialog* showMapLoadingProgress(QWidget* parent = nullptr);
    ProgressDialog* showMapSavingProgress(QWidget* parent = nullptr);
    ProgressDialog* showSearchProgress(QWidget* parent = nullptr);
    ProgressDialog* showReplaceProgress(QWidget* parent = nullptr);
    ProgressDialog* showValidationProgress(QWidget* parent = nullptr);
    ProgressDialog* showSpriteLoadingProgress(QWidget* parent = nullptr);

signals:
    void dialogCreated(ProgressDialog* dialog);
    void dialogDestroyed(ProgressDialog* dialog);
    void activeDialogCountChanged(int count);

private:
    explicit ProgressDialogManager(QObject* parent = nullptr);
    ~ProgressDialogManager() override;

    static ProgressDialogManager* instance_;
    QList<ProgressDialog*> activeDialogs_;
    
    // Default settings
    bool defaultAutoClose_;
    bool defaultShowElapsedTime_;
    bool defaultThreadSafe_;
    
    mutable QMutex mutex_;
};

/**
 * @brief Progress Dialog Styles for consistent appearance
 */
class ProgressDialogStyles
{
public:
    // Style presets
    static void applyDefaultStyle(ProgressDialog* dialog);
    static void applyLoadingStyle(ProgressDialog* dialog);
    static void applyProcessingStyle(ProgressDialog* dialog);
    static void applySearchStyle(ProgressDialog* dialog);
    static void applyValidationStyle(ProgressDialog* dialog);
    
    // Color schemes
    static void applyLightTheme(ProgressDialog* dialog);
    static void applyDarkTheme(ProgressDialog* dialog);
    
    // Animation presets
    static void setLoadingAnimation(ProgressDialog* dialog);
    static void setProcessingAnimation(ProgressDialog* dialog);
    static void setSearchAnimation(ProgressDialog* dialog);

private:
    ProgressDialogStyles() = default; // Static utility class
};

#endif // PROGRESSDIALOG_H
