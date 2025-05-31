#ifndef BRUSHPALETTEPANEL_H
#define BRUSHPALETTEPANEL_H

#include <QWidget>
#include <QList> // For QList<Brush*>

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

signals:
    void brushSelected(Brush* brush);

private slots:
    void onBrushButtonClicked();

private:
    // QLabel *label_; // Removed as it's no longer used
    BrushPanel* m_brushPanel;
};

#endif // BRUSHPALETTEPANEL_H
