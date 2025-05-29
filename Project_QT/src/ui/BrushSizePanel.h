#ifndef BRUSHSIZEPANEL_H
#define BRUSHSIZEPANEL_H

#include <QWidget>
#include <QVector> // For storing buttons

// Forward declarations
class QPushButton;
class QVBoxLayout;
class QButtonGroup; // To manage size buttons

class BrushSizePanel : public QWidget {
    Q_OBJECT

public:
    explicit BrushSizePanel(QWidget *parent = nullptr);
    ~BrushSizePanel() override;

    void setLargeIcons(bool largeIcons);

private slots:
    void onShapeButtonClicked(int id);
    void onSizeButtonClicked(int id);

private:
    void setupUi();
    void updateLayout(); // Helper to change layout based on m_largeIcons

    bool m_largeIcons;

    QPushButton* m_brushshapeSquareButton;
    QPushButton* m_brushshapeCircleButton;
    
    QVector<QPushButton*> m_sizeButtons; // To hold the 7 size buttons

    QButtonGroup* m_shapeButtonGroup;
    QButtonGroup* m_sizeButtonGroup;

    QVBoxLayout* m_mainLayout;
    // Keep pointers to the row layouts to remove/add them
    QWidget* m_row1Widget; // Using QWidget as a container for QHBoxLayout
    QWidget* m_row2Widget; // Using QWidget as a container for QHBoxLayout
};

#endif // BRUSHSIZEPANEL_H
