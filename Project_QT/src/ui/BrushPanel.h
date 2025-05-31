#ifndef BRUSHPANEL_H
#define BRUSHPANEL_H

#include <QWidget>
#include <QList> // For QList<SpriteButton*>

// Forward declarations
class SpriteButton;
class QGridLayout; // Or another layout like QFlowLayout if available/created
class QScrollArea;

class BrushPanel : public QWidget {
    Q_OBJECT

public:
    explicit BrushPanel(QWidget *parent = nullptr);
    ~BrushPanel() override;

    void addBrushButton(SpriteButton* button);
    void clearButtons(); // Removes and deletes all buttons

private:
    QGridLayout* m_gridLayout;      // Layout for the buttons
    QWidget* m_buttonContainerWidget; // Widget to hold the gridLayout, placed in scrollArea
    QScrollArea* m_scrollArea;        // To make the panel scrollable
    QList<SpriteButton*> m_buttons; // To keep track of buttons for easy clearing
};

#endif // BRUSHPANEL_H
