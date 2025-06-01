// ReplaceItemsDialog.cpp - Enhanced Find and Replace Items Dialog for Task 50

#include "ReplaceItemsDialog.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QSplitter>
#include <QSpinBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QProgressBar>
#include <QLabel>
#include <QTextEdit>
#include <QDebug>
#include <QVariant>

ReplaceItemsDialog::ReplaceItemsDialog(QWidget *parent)
    : QDialog(parent)
    , m_searchInProgress(false)
    , m_totalResultsFound(0) {

    setWindowTitle(tr("Find and Replace Items - Enhanced"));
    setMinimumSize(800, 600);
    resize(1000, 700);

    setupUi();
    connectSignals();
    updateButtonStates();
    populateItemTypeComboBox();
    populatePresetComboBox();

    qDebug() << "Enhanced ReplaceItemsDialog created (Task 50).";
}

ReplaceItemsDialog::~ReplaceItemsDialog() {
    qDebug() << "Enhanced ReplaceItemsDialog destroyed.";
}

// Task 50: Public interface methods
void ReplaceItemsDialog::setSearchScope(bool selectionOnly) {
    if (selectionOnly) {
        m_scopeSelectionRadio->setChecked(true);
    } else {
        m_scopeWholeMapRadio->setChecked(true);
    }
}

void ReplaceItemsDialog::setInitialFindItemId(int itemId) {
    m_findItemIdLineEdit->setText(QString::number(itemId));
}

void ReplaceItemsDialog::setInitialReplaceItemId(int itemId) {
    m_replaceItemIdLineEdit->setText(QString::number(itemId));
}

// Task 50: Enhanced UI setup
void ReplaceItemsDialog::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);

    // Create main splitter for layout
    m_mainSplitter = new QSplitter(Qt::Vertical, this);

    // Create tab widget for organized input
    m_tabWidget = new QTabWidget();

    // Setup individual tabs
    setupFindCriteriaTab();
    setupReplacementTab();
    setupSearchScopeTab();
    setupAdvancedOptionsTab();

    // Add tabs to widget
    m_tabWidget->addTab(m_findCriteriaTab, tr("Find Criteria"));
    m_tabWidget->addTab(m_replacementTab, tr("Replacement"));
    m_tabWidget->addTab(m_searchScopeTab, tr("Search Scope"));
    m_tabWidget->addTab(m_advancedOptionsTab, tr("Advanced"));

    m_mainSplitter->addWidget(m_tabWidget);

    // Setup results area
    setupResultsArea();
    m_mainSplitter->addWidget(m_resultsGroup);

    // Setup preset management
    setupPresetManagement();

    // Set splitter proportions
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 1);

    m_mainLayout->addWidget(m_mainSplitter);
    m_mainLayout->addWidget(m_presetGroup);

    // Setup action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_findButton = new QPushButton(tr("Find"));
    m_replaceSelectedButton = new QPushButton(tr("Replace Selected"));
    m_replaceAllButton = new QPushButton(tr("Replace All"));
    m_cancelButton = new QPushButton(tr("Cancel"));

    buttonLayout->addWidget(m_findButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_replaceSelectedButton);
    buttonLayout->addWidget(m_replaceAllButton);
    buttonLayout->addWidget(m_cancelButton);

    m_mainLayout->addLayout(buttonLayout);

    // Dialog button box
    m_dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    m_mainLayout->addWidget(m_dialogButtonBox);
}

void ReplaceItemsDialog::setupFindCriteriaTab() {
    m_findCriteriaTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_findCriteriaTab);

    // Basic criteria group
    m_findCriteriaGroup = new QGroupBox(tr("Basic Search Criteria"));
    QFormLayout* basicLayout = new QFormLayout(m_findCriteriaGroup);

    m_findItemIdLineEdit = new QLineEdit();
    m_findItemIdLineEdit->setPlaceholderText(tr("Enter Item ID (e.g., 3031)"));
    basicLayout->addRow(tr("Item ID:"), m_findItemIdLineEdit);

    m_findNameLineEdit = new QLineEdit();
    m_findNameLineEdit->setPlaceholderText(tr("Enter part of item name"));
    basicLayout->addRow(tr("Name contains:"), m_findNameLineEdit);

    m_findMatchCaseCheckBox = new QCheckBox(tr("Match Case"));
    m_findWholeWordCheckBox = new QCheckBox(tr("Whole Word Only"));
    QHBoxLayout* textOptionsLayout = new QHBoxLayout();
    textOptionsLayout->addWidget(m_findMatchCaseCheckBox);
    textOptionsLayout->addWidget(m_findWholeWordCheckBox);
    textOptionsLayout->addStretch();
    basicLayout->addRow(tr("Text Options:"), textOptionsLayout);

    m_findItemTypeComboBox = new QComboBox();
    basicLayout->addRow(tr("Item Type:"), m_findItemTypeComboBox);

    // Count range
    QHBoxLayout* countLayout = new QHBoxLayout();
    m_findMinCountSpinBox = new QSpinBox();
    m_findMinCountSpinBox->setRange(0, 999);
    m_findMinCountSpinBox->setValue(1);
    m_findMaxCountSpinBox = new QSpinBox();
    m_findMaxCountSpinBox->setRange(0, 999);
    m_findMaxCountSpinBox->setValue(999);
    countLayout->addWidget(new QLabel(tr("Min:")));
    countLayout->addWidget(m_findMinCountSpinBox);
    countLayout->addWidget(new QLabel(tr("Max:")));
    countLayout->addWidget(m_findMaxCountSpinBox);
    countLayout->addStretch();
    basicLayout->addRow(tr("Count Range:"), countLayout);

    // Action and Unique IDs
    m_findActionIdLineEdit = new QLineEdit();
    m_findActionIdLineEdit->setPlaceholderText(tr("Action ID (optional)"));
    basicLayout->addRow(tr("Action ID:"), m_findActionIdLineEdit);

    m_findUniqueIdLineEdit = new QLineEdit();
    m_findUniqueIdLineEdit->setPlaceholderText(tr("Unique ID (optional)"));
    basicLayout->addRow(tr("Unique ID:"), m_findUniqueIdLineEdit);

    mainLayout->addWidget(m_findCriteriaGroup);

    // Item properties group
    QGroupBox* propertiesGroup = new QGroupBox(tr("Item Properties"));
    QVBoxLayout* propsLayout = new QVBoxLayout(propertiesGroup);

    // Create property checkboxes in a grid
    QHBoxLayout* row1 = new QHBoxLayout();
    m_findIsMoveableCheckBox = new QCheckBox(tr("Is Moveable"));
    m_findIsBlockingCheckBox = new QCheckBox(tr("Is Blocking"));
    m_findIsStackableCheckBox = new QCheckBox(tr("Is Stackable"));
    m_findIsPickupableCheckBox = new QCheckBox(tr("Is Pickupable"));
    row1->addWidget(m_findIsMoveableCheckBox);
    row1->addWidget(m_findIsBlockingCheckBox);
    row1->addWidget(m_findIsStackableCheckBox);
    row1->addWidget(m_findIsPickupableCheckBox);

    QHBoxLayout* row2 = new QHBoxLayout();
    m_findIsReadableCheckBox = new QCheckBox(tr("Is Readable"));
    m_findIsWriteableCheckBox = new QCheckBox(tr("Is Writeable"));
    m_findIsRotateableCheckBox = new QCheckBox(tr("Is Rotateable"));
    m_findIsHangableCheckBox = new QCheckBox(tr("Is Hangable"));
    row2->addWidget(m_findIsReadableCheckBox);
    row2->addWidget(m_findIsWriteableCheckBox);
    row2->addWidget(m_findIsRotateableCheckBox);
    row2->addWidget(m_findIsHangableCheckBox);

    QHBoxLayout* row3 = new QHBoxLayout();
    m_findIsVerticalCheckBox = new QCheckBox(tr("Is Vertical"));
    m_findIsHorizontalCheckBox = new QCheckBox(tr("Is Horizontal"));
    row3->addWidget(m_findIsVerticalCheckBox);
    row3->addWidget(m_findIsHorizontalCheckBox);
    row3->addStretch();

    propsLayout->addLayout(row1);
    propsLayout->addLayout(row2);
    propsLayout->addLayout(row3);

    mainLayout->addWidget(propertiesGroup);

    // Item property editor integration
    m_pickFindItemButton = new QPushButton(tr("Pick Item Properties..."));
    m_pickFindItemButton->setToolTip(tr("Use item property editor to define find criteria"));
    mainLayout->addWidget(m_pickFindItemButton);

    mainLayout->addStretch();
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
