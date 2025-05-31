#include "BrushPanel.h"
#include "SpriteButton.h" // Assuming SpriteButton.h is available
#include <QGridLayout>
#include <QScrollArea>
#include <QDebug>
#include <QVBoxLayout> // Added for mainLayout

BrushPanel::BrushPanel(QWidget *parent) : QWidget(parent) {
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true); // Important for the inner widget to resize correctly
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_buttonContainerWidget = new QWidget(); // This widget will contain the grid of buttons
    m_gridLayout = new QGridLayout(m_buttonContainerWidget);
    m_gridLayout->setSpacing(2); // Small spacing between buttons
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft); // Align buttons to top-left

    m_buttonContainerWidget->setLayout(m_gridLayout);
    m_scrollArea->setWidget(m_buttonContainerWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(this); // Main layout for BrushPanel itself
    mainLayout->addWidget(m_scrollArea);
    mainLayout->setContentsMargins(0,0,0,0);
    setLayout(mainLayout);

    // Set a distinct background for debugging if needed
    // setAutoFillBackground(true);
    // QPalette pal = palette();
    // pal.setColor(QPalette::Window, Qt::darkCyan);
    // setPalette(pal);
    // m_buttonContainerWidget->setAutoFillBackground(true);
    // QPalette pal2 = m_buttonContainerWidget->palette();
    // pal2.setColor(QPalette::Window, Qt::cyan);
    // m_buttonContainerWidget->setPalette(pal2);


    qDebug() << "BrushPanel created.";
}

BrushPanel::~BrushPanel() {
    clearButtons(); // Ensure all buttons are deleted
    // m_gridLayout is owned by m_buttonContainerWidget
    // m_buttonContainerWidget is owned by m_scrollArea (if set as widget) or needs manual deletion if not
    // m_scrollArea is owned by BrushPanel (this)
    // Qt's parent-child hierarchy should handle deletion of layouts and child widgets.
    // Explicitly deleting m_buttonContainerWidget if it's not parented to m_scrollArea by setWidget:
    // if(m_scrollArea->widget() != m_buttonContainerWidget) delete m_buttonContainerWidget;
    // However, setWidget does reparent it.
    qDebug() << "BrushPanel destroyed.";
}

void BrushPanel::addBrushButton(SpriteButton* button) {
    if (!button) return;

    // Example: add to grid layout, 5 columns
    int colCount = 5;
    int currentButtonCount = m_buttons.count();
    int row = currentButtonCount / colCount;
    int col = currentButtonCount % colCount;

    m_gridLayout->addWidget(button, row, col);
    m_buttons.append(button);
    // button->setParent(m_buttonContainerWidget); // addWidget should handle parenting
}

void BrushPanel::clearButtons() {
    for (SpriteButton* button : m_buttons) {
        if (button) {
            m_gridLayout->removeWidget(button); // Remove from layout
            button->setParent(nullptr); // Decouple from parent before deleting
            delete button;              // Delete the button
        }
    }
    m_buttons.clear();
    qDebug() << "BrushPanel: All buttons cleared and deleted.";
}
