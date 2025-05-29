#include "BrushSizePanel.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QButtonGroup>
#include <QDebug> // For placeholder event handlers

const int NUM_SIZE_BUTTONS = 7;

BrushSizePanel::BrushSizePanel(QWidget *parent) : 
    QWidget(parent), 
    m_largeIcons(true), // Default to large icons layout
    m_row1Widget(nullptr),
    m_row2Widget(nullptr) 
{
    setupUi();
    updateLayout(); // Initial layout construction

    // Connections
    // Using QButtonGroup's idClicked signal
    connect(m_shapeButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &BrushSizePanel::onShapeButtonClicked);
    connect(m_sizeButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &BrushSizePanel::onSizeButtonClicked);
}

BrushSizePanel::~BrushSizePanel() {
    // Child widgets and layouts managed by Qt's ownership
}

void BrushSizePanel::setupUi() {
    m_mainLayout = new QVBoxLayout(this); // Set main layout on this widget

    m_shapeButtonGroup = new QButtonGroup(this);
    m_shapeButtonGroup->setExclusive(true); // Ensure only one shape is selected
    m_sizeButtonGroup = new QButtonGroup(this);
    m_sizeButtonGroup->setExclusive(true); // Only one size can be active

    m_brushshapeSquareButton = new QPushButton(tr("Sq")); 
    m_brushshapeSquareButton->setCheckable(true);
    m_brushshapeSquareButton->setToolTip(tr("Square brush")); 
    m_shapeButtonGroup->addButton(m_brushshapeSquareButton, 0); 

    m_brushshapeCircleButton = new QPushButton(tr("Ci")); 
    m_brushshapeCircleButton->setCheckable(true);
    m_brushshapeCircleButton->setToolTip(tr("Circle brush")); 
    m_shapeButtonGroup->addButton(m_brushshapeCircleButton, 1); 
    
    m_sizeButtons.reserve(NUM_SIZE_BUTTONS);
    const char* sizeTooltips[NUM_SIZE_BUTTONS] = {
        "Brush size 1x1", "Brush size 3x3", "Brush size 5x5",
        "Brush size 7x7", "Brush size 9x9", "Brush size 15x15",
        "Brush size 19x19"
    };
    for (int i = 0; i < NUM_SIZE_BUTTONS; ++i) {
        QPushButton* button = new QPushButton(QString::number(i + 1)); 
        button->setCheckable(true);
        button->setToolTip(tr(sizeTooltips[i]));
        m_sizeButtons.append(button);
        m_sizeButtonGroup->addButton(button, i); 
    }

    if(m_brushshapeSquareButton) m_brushshapeSquareButton->setChecked(true);
    if (!m_sizeButtons.isEmpty()) {
        m_sizeButtons[0]->setChecked(true);
    }
}

void BrushSizePanel::setLargeIcons(bool largeIcons) {
    if (m_largeIcons == largeIcons) {
        return;
    }
    m_largeIcons = largeIcons;
    updateLayout();
}

void BrushSizePanel::updateLayout() {
    if (m_row1Widget) {
        m_mainLayout->removeWidget(m_row1Widget);
        delete m_row1Widget;
        m_row1Widget = nullptr;
    }
    if (m_row2Widget) {
        m_mainLayout->removeWidget(m_row2Widget);
        delete m_row2Widget;
        m_row2Widget = nullptr;
    }
    
    QLayoutItem* item;
    while ((item = m_mainLayout->takeAt(0)) != nullptr) {
        if (item->spacerItem()) {
            delete item->spacerItem();
        }
        delete item; 
    }

    if (m_largeIcons) {
        m_row1Widget = new QWidget(this);
        QHBoxLayout* row1Layout = new QHBoxLayout(m_row1Widget);
        row1Layout->setContentsMargins(0,0,0,0); 
        row1Layout->addWidget(m_brushshapeSquareButton);
        row1Layout->addWidget(m_brushshapeCircleButton);
        row1Layout->addSpacerItem(new QSpacerItem(36, 0, QSizePolicy::Fixed, QSizePolicy::Minimum)); // Fixed spacer
        if (NUM_SIZE_BUTTONS > 0) row1Layout->addWidget(m_sizeButtons[0]);
        if (NUM_SIZE_BUTTONS > 1) row1Layout->addWidget(m_sizeButtons[1]);
        m_mainLayout->addWidget(m_row1Widget);

        if (NUM_SIZE_BUTTONS > 2) {
            m_row2Widget = new QWidget(this);
            QHBoxLayout* row2Layout = new QHBoxLayout(m_row2Widget);
            row2Layout->setContentsMargins(0,0,0,0);
            for (int i = 2; i < NUM_SIZE_BUTTONS; ++i) {
                row2Layout->addWidget(m_sizeButtons[i]);
            }
            row2Layout->addStretch(1); 
            m_mainLayout->addWidget(m_row2Widget);
        }
    } else {
        m_row1Widget = new QWidget(this);
        QHBoxLayout* rowLayout = new QHBoxLayout(m_row1Widget);
        rowLayout->setContentsMargins(0,0,0,0);
        rowLayout->addWidget(m_brushshapeSquareButton);
        rowLayout->addWidget(m_brushshapeCircleButton);
        rowLayout->addSpacerItem(new QSpacerItem(18, 0, QSizePolicy::Fixed, QSizePolicy::Minimum)); // Fixed spacer
        for (int i = 0; i < NUM_SIZE_BUTTONS; ++i) {
            rowLayout->addWidget(m_sizeButtons[i]);
        }
        rowLayout->addStretch(1); 
        m_mainLayout->addWidget(m_row1Widget);
    }
    m_mainLayout->addStretch(1); 
}

void BrushSizePanel::onShapeButtonClicked(int id) {
    qDebug() << "BrushSizePanel: Shape button clicked, id:" << id;
    // QButtonGroup with setExclusive(true) handles unchecking.
    // If it was not exclusive, you'd do:
    // if (id == 0 && m_brushshapeCircleButton) m_brushshapeCircleButton->setChecked(false);
    // if (id == 1 && m_brushshapeSquareButton) m_brushshapeSquareButton->setChecked(false);
}

void BrushSizePanel::onSizeButtonClicked(int id) {
    qDebug() << "BrushSizePanel: Size button clicked, id (0-6):" << id;
    // QButtonGroup handles unchecking others.
}
