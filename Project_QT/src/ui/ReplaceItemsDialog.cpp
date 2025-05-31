#include "ReplaceItemsDialog.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout> // For action buttons layout if not using QDialogButtonBox for them
#include <QDialogButtonBox>
#include <QDebug>
#include <QVariant> // For QComboBox item data

ReplaceItemsDialog::ReplaceItemsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Find and Replace Items"));
    setMinimumSize(500, 400); // Set a reasonable minimum size

    setupUi(); // Call helper to create UI elements

    // Connect signals to slots
    if (m_findButton) {
        connect(m_findButton, &QPushButton::clicked, this, &ReplaceItemsDialog::onFindClicked);
    }
    if (m_replaceSelectedButton) {
        connect(m_replaceSelectedButton, &QPushButton::clicked, this, &ReplaceItemsDialog::onReplaceSelectedClicked);
    }
    if (m_replaceAllButton) {
        connect(m_replaceAllButton, &QPushButton::clicked, this, &ReplaceItemsDialog::onReplaceAllClicked);
    }
    if (m_pickFindItemButton) {
        connect(m_pickFindItemButton, &QPushButton::clicked, this, &ReplaceItemsDialog::onPickFindItemClicked);
    }
    if (m_pickReplaceItemButton) {
        connect(m_pickReplaceItemButton, &QPushButton::clicked, this, &ReplaceItemsDialog::onPickReplaceItemClicked);
    }

    if (m_dialogButtonBox) {
        // Assuming QDialogButtonBox::Close was used, it emits rejected()
        connect(m_dialogButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        // If using Ok/Cancel:
        // connect(m_dialogButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        // connect(m_dialogButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
    qDebug() << "ReplaceItemsDialog created and UI setup.";
}

ReplaceItemsDialog::~ReplaceItemsDialog() {
    qDebug() << "ReplaceItemsDialog destroyed.";
    // Child widgets and layouts are automatically deleted by Qt's parent-child mechanism.
}

void ReplaceItemsDialog::setupUi() {
    m_mainLayout = new QVBoxLayout(this); // Main layout for the dialog

    // --- Find Criteria Group ---
    m_findCriteriaGroup = new QGroupBox(tr("Find Criteria"));
    QFormLayout* findLayout = new QFormLayout();

    m_findItemIdLineEdit = new QLineEdit();
    m_findItemIdLineEdit->setPlaceholderText(tr("Enter Item ID (e.g., 3031)"));
    findLayout->addRow(tr("Item ID:"), m_findItemIdLineEdit);

    m_findNameLineEdit = new QLineEdit();
    m_findNameLineEdit->setPlaceholderText(tr("Enter part of item name (case-insensitive)"));
    findLayout->addRow(tr("Name contains:"), m_findNameLineEdit);

    m_findIsMoveableCheckBox = new QCheckBox(tr("Is Moveable"));
    findLayout->addRow(m_findIsMoveableCheckBox);

    m_findIsBlockingCheckBox = new QCheckBox(tr("Is Blocking"));
    findLayout->addRow(m_findIsBlockingCheckBox);

    // Add more common property checkboxes here if desired later (e.g. stackable, pickupable)
    // m_findIsStackableCheckBox = new QCheckBox(tr("Is Stackable"));
    // findLayout->addRow(m_findIsStackableCheckBox);

    m_findItemTypeComboBox = new QComboBox();
    m_findItemTypeComboBox->addItem(tr("Any Type"), QVariant(-1)); // -1 or specific enum for 'any'
    m_findItemTypeComboBox->addItem(tr("Ground Tile"), QVariant(0)); // Placeholder values
    m_findItemTypeComboBox->addItem(tr("Border"), QVariant(1));
    m_findItemTypeComboBox->addItem(tr("Wall"), QVariant(2));
    m_findItemTypeComboBox->addItem(tr("Generic Item"), QVariant(3));
    m_findItemTypeComboBox->addItem(tr("Container"), QVariant(4));
    m_findItemTypeComboBox->addItem(tr("Creature"), QVariant(5)); // If searching creatures
    m_findItemTypeComboBox->addItem(tr("Spawn"), QVariant(6));    // If searching spawns
    findLayout->addRow(tr("Item Type:"), m_findItemTypeComboBox);

    m_pickFindItemButton = new QPushButton(tr("Pick Item Properties..."));
    m_pickFindItemButton->setToolTip(tr("Use an item property editor to define find criteria (placeholder)"));
    findLayout->addRow(m_pickFindItemButton);

    m_findCriteriaGroup->setLayout(findLayout);
    m_mainLayout->addWidget(m_findCriteriaGroup);

    // --- Replacement Rule Group ---
    m_replaceRuleGroup = new QGroupBox(tr("Replacement Rule"));
    QFormLayout* replaceLayout = new QFormLayout();

    m_replaceItemIdLineEdit = new QLineEdit();
    m_replaceItemIdLineEdit->setPlaceholderText(tr("Enter Item ID to replace with"));
    replaceLayout->addRow(tr("Replace with Item ID:"), m_replaceItemIdLineEdit);

    m_deleteFoundItemsCheckBox = new QCheckBox(tr("Or, Delete Found Items"));
    replaceLayout->addRow(m_deleteFoundItemsCheckBox);

    m_pickReplaceItemButton = new QPushButton(tr("Pick Replacement Item..."));
    m_pickReplaceItemButton->setToolTip(tr("Use an item property editor to define replacement item (placeholder)"));
    replaceLayout->addRow(m_pickReplaceItemButton);

    m_replaceRuleGroup->setLayout(replaceLayout);
    m_mainLayout->addWidget(m_replaceRuleGroup);

    // --- Results Group (Placeholder) ---
    m_resultsGroup = new QGroupBox(tr("Found Items / Tiles (Placeholder)"));
    QVBoxLayout* resultsLayout = new QVBoxLayout();
    m_resultsListWidget = new QListWidget();
    m_resultsListWidget->setMinimumHeight(100);
    m_resultsListWidget->addItem(tr("(Search results will appear here)")); // Placeholder item
    resultsLayout->addWidget(m_resultsListWidget);
    m_resultsGroup->setLayout(resultsLayout);
    m_mainLayout->addWidget(m_resultsGroup);
    m_resultsGroup->setVisible(false); // Initially hidden, shown after "Find"

    // --- Actions Group / Buttons ---
    // Using QHBoxLayout for main action buttons for now
    m_actionsGroup = new QGroupBox(tr("Actions")); // Optional grouping
    QHBoxLayout* actionsButtonLayout = new QHBoxLayout();
    m_findButton = new QPushButton(tr("Find"));
    m_replaceSelectedButton = new QPushButton(tr("Replace Selected"));
    m_replaceAllButton = new QPushButton(tr("Replace All"));
    m_replaceSelectedButton->setEnabled(false); // Initially disabled
    m_replaceAllButton->setEnabled(false);     // Initially disabled

    actionsButtonLayout->addWidget(m_findButton);
    actionsButtonLayout->addStretch();
    actionsButtonLayout->addWidget(m_replaceSelectedButton);
    actionsButtonLayout->addWidget(m_replaceAllButton);
    m_actionsGroup->setLayout(actionsButtonLayout);
    m_mainLayout->addWidget(m_actionsGroup);

    // --- Dialog Button Box (for Close) ---
    m_dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    m_mainLayout->addWidget(m_dialogButtonBox);

    setLayout(m_mainLayout); // Set the main layout for the dialog
}

// Placeholder implementations for slots (to be created in Step 3)
void ReplaceItemsDialog::onFindClicked() {
    QString findItemId = m_findItemIdLineEdit->text().trimmed();
    QString findName = m_findNameLineEdit->text().trimmed();
    bool findIsMoveable = m_findIsMoveableCheckBox->isChecked();
    bool findIsBlocking = m_findIsBlockingCheckBox->isChecked();
    QString findItemType = m_findItemTypeComboBox->currentText();
    QVariant findItemTypeData = m_findItemTypeComboBox->currentData();

    qDebug() << "Find Button Clicked:";
    qDebug() << "  Find Item ID:" << findItemId;
    qDebug() << "  Find Name Contains:" << findName;
    qDebug() << "  Is Moveable:" << findIsMoveable;
    qDebug() << "  Is Blocking:" << findIsBlocking;
    qDebug() << "  Item Type:" << findItemType << "(Data:" << findItemTypeData << ")";

    // Simulate finding items: show results group, enable replace buttons, add dummy result
    if (m_resultsGroup) m_resultsGroup->setVisible(true);
    if (m_resultsListWidget) {
        m_resultsListWidget->clear(); // Clear previous dummy results
        m_resultsListWidget->addItem(QString("Found: Placeholder Item 1 (ID: %1) at (10,10,7)").arg(findItemId.isEmpty() ? "Any" : findItemId));
        m_resultsListWidget->addItem(QString("Found: Placeholder Item 2 (Name: %1) at (20,20,7)").arg(findName.isEmpty() ? "Any" : findName));
    }
    if (m_replaceSelectedButton) m_replaceSelectedButton->setEnabled(true);
    if (m_replaceAllButton) m_replaceAllButton->setEnabled(true);
}

void ReplaceItemsDialog::onReplaceSelectedClicked() {
    // Log Find Criteria
    QString findItemId = m_findItemIdLineEdit->text().trimmed();
    QString findName = m_findNameLineEdit->text().trimmed();
    bool findIsMoveable = m_findIsMoveableCheckBox->isChecked();
    bool findIsBlocking = m_findIsBlockingCheckBox->isChecked();
    QString findItemType = m_findItemTypeComboBox->currentText();
    QVariant findItemTypeData = m_findItemTypeComboBox->currentData();

    // Log Replacement Rule
    QString replaceItemId = m_replaceItemIdLineEdit->text().trimmed();
    bool deleteFound = m_deleteFoundItemsCheckBox->isChecked();

    qDebug() << "Replace Selected Button Clicked:";
    qDebug() << "  -- Find Criteria --";
    qDebug() << "  Find Item ID:" << findItemId;
    qDebug() << "  Find Name Contains:" << findName;
    qDebug() << "  Is Moveable:" << findIsMoveable;
    qDebug() << "  Is Blocking:" << findIsBlocking;
    qDebug() << "  Item Type:" << findItemType << "(Data:" << findItemTypeData << ")";
    qDebug() << "  -- Replacement Rule --";
    qDebug() << "  Replace with Item ID:" << replaceItemId;
    qDebug() << "  Delete Found Items:" << deleteFound;

    if(m_resultsListWidget && m_resultsListWidget->currentItem()){
        qDebug() << "  Replacing selected item in list:" << m_resultsListWidget->currentItem()->text();
    } else {
        qDebug() << "  No item selected in results list to replace.";
    }
}

void ReplaceItemsDialog::onReplaceAllClicked() {
    // Log Find Criteria
    QString findItemId = m_findItemIdLineEdit->text().trimmed();
    QString findName = m_findNameLineEdit->text().trimmed();
    bool findIsMoveable = m_findIsMoveableCheckBox->isChecked();
    bool findIsBlocking = m_findIsBlockingCheckBox->isChecked();
    QString findItemType = m_findItemTypeComboBox->currentText();
    QVariant findItemTypeData = m_findItemTypeComboBox->currentData();

    // Log Replacement Rule
    QString replaceItemId = m_replaceItemIdLineEdit->text().trimmed();
    bool deleteFound = m_deleteFoundItemsCheckBox->isChecked();

    qDebug() << "Replace All Button Clicked:";
    qDebug() << "  -- Find Criteria --";
    qDebug() << "  Find Item ID:" << findItemId;
    qDebug() << "  Find Name Contains:" << findName;
    qDebug() << "  Is Moveable:" << findIsMoveable;
    qDebug() << "  Is Blocking:" << findIsBlocking;
    qDebug() << "  Item Type:" << findItemType << "(Data:" << findItemTypeData << ")";
    qDebug() << "  -- Replacement Rule --";
    qDebug() << "  Replace with Item ID:" << replaceItemId;
    qDebug() << "  Delete Found Items:" << deleteFound;
    qDebug() << "  Replacing all items in (simulated) results list.";
}

void ReplaceItemsDialog::onPickFindItemClicked() {
    qDebug() << "Pick Find Item button clicked. (ItemPropertyEditor integration placeholder)";
}

void ReplaceItemsDialog::onPickReplaceItemClicked() {
    qDebug() << "Pick Replace Item button clicked. (ItemPropertyEditor integration placeholder)";
}
