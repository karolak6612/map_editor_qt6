#ifndef BRUSHPALETTEPANEL_H
#define BRUSHPALETTEPANEL_H

#include <QWidget>
#include <QList> // For QList<Brush*>
#include <QHash> // For QHash<int, QPixmap>
#include <QSet>  // For QSet<int>
#include <QTimer> // For QTimer
#include <QPixmap> // For QPixmap

// Forward declarations
class QLabel; // Still here from previous, though not directly used by BrushPalettePanel's interface itself anymore
class BrushPanel;
class Brush;        // Added: Forward declaration for Brush
class SpriteButton; // Added: Forward declaration for SpriteButton (though BrushPanel.h might also have it)


class BrushPalettePanel : public QWidget {
    Q_OBJECT
public:
    explicit BrushPalettePanel(const QString& name, QWidget *parent = nullptr);
    ~BrushPalettePanel() override;
    void clearBrushDisplay();
    void populateBrushes(const QList<Brush*>& brushes);
    void resetPanelState();

    // Task 47: Enhanced resource management methods
    void clearPixmapCache();
    void optimizeMemoryUsage();
    void onMapClose();
    void onTilesetChange();
    void refreshButtonStates();

    // Task 47: Efficient button management
    void updateButtonVisibility(bool visible);
    void preloadButtonPixmaps(const QList<Brush*>& brushes);
    void releaseUnusedPixmaps();

signals:
    void brushSelected(Brush* brush);
    void memoryOptimized(int pixmapsReleased);

private slots:
    void onBrushButtonClicked();

private:
    // QLabel *label_; // Removed as it's no longer used
    BrushPanel* m_brushPanel;

    // Task 47: Resource management members
    QHash<int, QPixmap> pixmapCache_;           // Cache for brush pixmaps by lookID
    QSet<int> activePixmaps_;                   // Track currently used pixmaps
    QTimer* memoryOptimizationTimer_;           // Timer for periodic memory cleanup
    bool isVisible_;                            // Track visibility state
    int maxCacheSize_;                          // Maximum cache size
    qint64 lastOptimizationTime_;               // Last optimization timestamp

    // Task 47: State management
    QList<Brush*> currentBrushes_;              // Current brush list for state management
    Brush* selectedBrush_;                      // Currently selected brush
    bool needsRefresh_;                         // Flag for deferred refresh
};

#endif // BRUSHPALETTEPANEL_H
