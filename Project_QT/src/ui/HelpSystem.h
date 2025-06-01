#ifndef HELPSYSTEM_H
#define HELPSYSTEM_H

#include <QDialog>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QScrollArea>
#include <QGroupBox>
#include <QFrame>
#include <QUrl>
#include <QTextDocument>
#include <QTextCursor>
#include <QSyntaxHighlighter>

// Forward declarations
class QNetworkAccessManager;
class QNetworkReply;

/**
 * @brief Comprehensive help system with documentation and tutorials
 * 
 * Task 78: Complete migration of wxwidgets HelpSystem with enhanced features:
 * - Interactive help browser with navigation
 * - Searchable documentation with full-text search
 * - Keyboard shortcuts reference with customizable bindings
 * - Tool explanations with interactive demonstrations
 * - Tutorial system with step-by-step guides
 * - Context-sensitive help integration
 * - Online help updates and synchronization
 * - Bookmark and history management
 */
class HelpSystem : public QMainWindow
{
    Q_OBJECT

public:
    explicit HelpSystem(QWidget* parent = nullptr);
    ~HelpSystem() override;

    // Static access methods
    static HelpSystem* getInstance();
    static void showHelp(const QString& topic = QString());
    static void showKeyboardShortcuts();
    static void showToolHelp(const QString& toolName);
    static void showTutorial(const QString& tutorialName);
    static void showContextHelp(const QString& context);

    // Help content management
    void loadHelpContent();
    void refreshContent();
    void setCurrentTopic(const QString& topic);
    QString getCurrentTopic() const;

public slots:
    void showTopic(const QString& topic);
    void searchHelp(const QString& query);
    void showPreviousTopic();
    void showNextTopic();
    void showHomePage();

private slots:
    // Navigation slots
    void onTopicSelected();
    void onSearchTextChanged(const QString& text);
    void onSearchRequested();
    void onBackClicked();
    void onForwardClicked();
    void onHomeClicked();
    void onRefreshClicked();
    void onPrintClicked();
    
    // Content slots
    void onLinkClicked(const QUrl& url);
    void onAnchorClicked(const QUrl& url);
    void onContentLoaded();
    void onContentError();
    
    // Bookmark slots
    void onAddBookmark();
    void onRemoveBookmark();
    void onBookmarkSelected();
    void onManageBookmarks();
    
    // Tutorial slots
    void onStartTutorial();
    void onNextTutorialStep();
    void onPreviousTutorialStep();
    void onFinishTutorial();
    void onTutorialSelectionChanged();
    
    // Settings slots
    void onFontSizeChanged(int size);
    void onThemeChanged(const QString& theme);
    void onLanguageChanged(const QString& language);
    void onUpdateCheckToggled(bool enabled);

private:
    // UI setup methods
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupDockWidgets();
    void setupCentralWidget();
    void connectSignals();
    
    // Content management
    void loadTopicContent(const QString& topic);
    void loadSearchResults(const QString& query);
    void loadTutorialContent(const QString& tutorial);
    void loadKeyboardShortcuts();
    void updateNavigationButtons();
    void updateBookmarksList();
    void updateTutorialsList();
    
    // Help content creation
    QString createTopicHtml(const QString& topic) const;
    QString createSearchResultsHtml(const QStringList& results) const;
    QString createKeyboardShortcutsHtml() const;
    QString createToolHelpHtml(const QString& toolName) const;
    QString createTutorialHtml(const QString& tutorial) const;
    
    // Navigation management
    void addToHistory(const QString& topic);
    void navigateToHistoryItem(int index);
    bool canGoBack() const;
    bool canGoForward() const;
    
    // Bookmark management
    void loadBookmarks();
    void saveBookmarks();
    void addBookmark(const QString& topic, const QString& title);
    void removeBookmark(const QString& topic);
    bool isBookmarked(const QString& topic) const;
    
    // Tutorial management
    void loadTutorials();
    void startTutorial(const QString& tutorialName);
    void nextTutorialStep();
    void previousTutorialStep();
    void finishTutorial();
    void updateTutorialProgress();
    
    // Search functionality
    QStringList searchTopics(const QString& query) const;
    QStringList getRelatedTopics(const QString& topic) const;
    void highlightSearchTerms(const QString& query);
    
    // Settings management
    void loadSettings();
    void saveSettings();
    void applyTheme(const QString& theme);
    void applyFontSize(int size);
    
    // Static instance
    static HelpSystem* instance_;
    
    // UI components
    QWidget* centralWidget_;
    QSplitter* mainSplitter_;
    QVBoxLayout* mainLayout_;
    
    // Menu and toolbar
    QMenuBar* menuBar_;
    QToolBar* toolBar_;
    QStatusBar* statusBar_;
    
    // Navigation toolbar
    QPushButton* backButton_;
    QPushButton* forwardButton_;
    QPushButton* homeButton_;
    QPushButton* refreshButton_;
    QPushButton* printButton_;
    QLineEdit* searchLineEdit_;
    QPushButton* searchButton_;
    
    // Dock widgets
    QDockWidget* topicsDock_;
    QDockWidget* bookmarksDock_;
    QDockWidget* tutorialsDock_;
    QDockWidget* searchDock_;
    
    // Content widgets
    QTreeWidget* topicsTreeWidget_;
    QListWidget* bookmarksListWidget_;
    QListWidget* tutorialsListWidget_;
    QListWidget* searchResultsListWidget_;
    QTextBrowser* contentBrowser_;
    
    // Tutorial widgets
    QLabel* tutorialTitleLabel_;
    QLabel* tutorialStepLabel_;
    QProgressBar* tutorialProgressBar_;
    QPushButton* previousStepButton_;
    QPushButton* nextStepButton_;
    QPushButton* finishTutorialButton_;
    
    // Settings widgets
    QComboBox* fontSizeComboBox_;
    QComboBox* themeComboBox_;
    QComboBox* languageComboBox_;
    QCheckBox* updateCheckCheckBox_;
    
    // Data management
    QStringList navigationHistory_;
    int currentHistoryIndex_;
    QStringList bookmarks_;
    QStringList tutorials_;
    QString currentTopic_;
    QString currentTutorial_;
    int currentTutorialStep_;
    int totalTutorialSteps_;
    
    // Settings
    int fontSize_;
    QString theme_;
    QString language_;
    bool updateCheckEnabled_;
    
    // Network management
    QNetworkAccessManager* networkManager_;
    
    // Content data
    QMap<QString, QString> topicContent_;
    QMap<QString, QStringList> topicKeywords_;
    QMap<QString, QString> tutorialContent_;
    QMap<QString, QStringList> tutorialSteps_;
};

/**
 * @brief Simple help dialog for quick help display
 */
class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(const QString& topic, QWidget* parent = nullptr);
    ~HelpDialog() override;

private:
    void setupUI();
    void loadContent(const QString& topic);
    
    QVBoxLayout* mainLayout_;
    QTextBrowser* contentBrowser_;
    QPushButton* closeButton_;
    QPushButton* moreHelpButton_;
    QString topic_;
};

#endif // HELPSYSTEM_H
