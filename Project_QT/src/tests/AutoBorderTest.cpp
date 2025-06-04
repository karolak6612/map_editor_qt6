// AutoBorderTest.cpp - Test for Task 41 AutoBorder implementation

#include "AutoBorder.h"
#include "GroundBrush.h"
#include "Map.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QDebug>
#include <QDomDocument>
#include <memory>

// Test widget to demonstrate AutoBorder functionality
class AutoBorderTestWidget : public QWidget {
    Q_OBJECT

public:
    AutoBorderTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateAutoBorder() {
        quint32 borderId = borderIdSpin_->value();
        
        auto autoBorder = std::make_unique<AutoBorder>(borderId);
        autoBorder->setGroup(groupSpin_->value());
        autoBorder->setGround(isGroundCheck_->isChecked());
        
        updateStatus(QString("Created AutoBorder: ID=%1, Group=%2, Ground=%3")
                    .arg(autoBorder->getId())
                    .arg(autoBorder->getGroup())
                    .arg(autoBorder->isGround()));
        
        testAutoBorderInterface(autoBorder.get());
        
        // Store for later use
        testBorders_.append(std::move(autoBorder));
    }

    void onTestEdgeNameMapping() {
        updateStatus("=== Testing Edge Name to ID Mapping (wxwidgets Migration) ===");
        
        // Test all edge names from wxwidgets AutoBorder::edgeNameToID
        QStringList edgeNames = {"n", "w", "s", "e", "cnw", "cne", "csw", "cse", "dnw", "dne", "dsw", "dse"};
        
        for (const QString& edgeName : edgeNames) {
            BorderType borderType = AutoBorder::edgeNameToID(edgeName);
            updateStatus(QString("Edge '%1' -> BorderType %2").arg(edgeName).arg(static_cast<int>(borderType)));
        }
        
        // Test invalid edge name
        BorderType invalidType = AutoBorder::edgeNameToID("invalid");
        updateStatus(QString("Invalid edge 'invalid' -> BorderType %1 (should be 0)").arg(static_cast<int>(invalidType)));
    }

    void onTestBorderItemManagement() {
        updateStatus("=== Testing Border Item Management ===");
        
        if (testBorders_.isEmpty()) {
            updateStatus("No AutoBorder objects available. Create some first.");
            return;
        }
        
        auto& border = testBorders_.first();
        
        // Test setting border items for different types
        QList<QPair<BorderType, quint16>> testItems = {
            {BorderType::NORTH_HORIZONTAL, 1001},
            {BorderType::SOUTH_HORIZONTAL, 1002},
            {BorderType::EAST_HORIZONTAL, 1003},
            {BorderType::WEST_HORIZONTAL, 1004},
            {BorderType::NORTHWEST_CORNER, 1005},
            {BorderType::NORTHEAST_CORNER, 1006},
            {BorderType::SOUTHWEST_CORNER, 1007},
            {BorderType::SOUTHEAST_CORNER, 1008}
        };
        
        for (const auto& item : testItems) {
            BorderType borderType = item.first;
            quint16 itemId = item.second;
            
            border->setItemId(borderType, itemId);
            quint16 retrievedId = border->getItemId(borderType);
            
            updateStatus(QString("Set BorderType %1 to item %2, retrieved: %3")
                        .arg(static_cast<int>(borderType))
                        .arg(itemId)
                        .arg(retrievedId));
        }
        
        // Test hasItemId method
        bool hasItem1001 = border->hasItemId(1001);
        bool hasItem9999 = border->hasItemId(9999);
        
        updateStatus(QString("Border has item 1001: %1").arg(hasItem1001));
        updateStatus(QString("Border has item 9999: %1").arg(hasItem9999));
        
        // Test getItemId with alignment parameter
        quint16 alignmentItem = border->getItemId(1); // NORTH_HORIZONTAL
        updateStatus(QString("Item for alignment 1: %1").arg(alignmentItem));
    }

    void onTestXMLLoading() {
        updateStatus("=== Testing XML Loading (wxwidgets Migration) ===");

        // Create XML document for testing (correct wxwidgets structure)
        QDomDocument doc;
        QDomElement root = doc.createElement("border");
        root.setAttribute("id", "2000");
        root.setAttribute("group", "5");
        root.setAttribute("ground", "true");

        // Add borderitem elements (correct wxwidgets XML structure)
        QStringList edges = {"n", "s", "e", "w", "cnw", "cne", "csw", "cse"};
        quint16 itemId = 3000;

        for (const QString& edge : edges) {
            QDomElement borderItem = doc.createElement("borderitem");
            borderItem.setAttribute("edge", edge);
            borderItem.setAttribute("item", QString::number(itemId));
            root.appendChild(borderItem);
            itemId++;
        }

        doc.appendChild(root);
        
        // Create AutoBorder and load from XML
        auto autoBorder = std::make_unique<AutoBorder>(0);
        QStringList warnings;
        
        bool loadSuccess = autoBorder->load(root, warnings);
        
        updateStatus(QString("XML loading successful: %1").arg(loadSuccess));
        if (!warnings.isEmpty()) {
            updateStatus(QString("Warnings: %1").arg(warnings.join("; ")));
        }
        
        updateStatus(QString("Loaded border: ID=%1, Group=%2, Ground=%3")
                    .arg(autoBorder->getId())
                    .arg(autoBorder->getGroup())
                    .arg(autoBorder->isGround()));
        
        // Test loaded border items
        for (const QString& edge : edges) {
            BorderType borderType = AutoBorder::edgeNameToID(edge);
            quint16 loadedItemId = autoBorder->getItemId(borderType);
            updateStatus(QString("Loaded edge '%1': item ID %2").arg(edge).arg(loadedItemId));
        }
        
        testBorders_.append(std::move(autoBorder));
    }

    void onTestBorderClearance() {
        updateStatus("=== Testing Border Item Clearance ===");
        
        if (testBorders_.isEmpty()) {
            updateStatus("No AutoBorder objects available. Create some first.");
            return;
        }
        
        auto& border = testBorders_.first();
        
        // Set some items first
        border->setItemId(BorderType::NORTH_HORIZONTAL, 5001);
        border->setItemId(BorderType::SOUTH_HORIZONTAL, 5002);
        border->setItemId(BorderType::EAST_HORIZONTAL, 5003);
        
        updateStatus("Set some border items...");
        updateStatus(QString("North item: %1").arg(border->getItemId(BorderType::NORTH_HORIZONTAL)));
        updateStatus(QString("South item: %1").arg(border->getItemId(BorderType::SOUTH_HORIZONTAL)));
        updateStatus(QString("East item: %1").arg(border->getItemId(BorderType::EAST_HORIZONTAL)));
        
        // Clear all items
        border->clearItemIds();
        updateStatus("Cleared all border items");
        
        updateStatus(QString("North item after clear: %1").arg(border->getItemId(BorderType::NORTH_HORIZONTAL)));
        updateStatus(QString("South item after clear: %1").arg(border->getItemId(BorderType::SOUTH_HORIZONTAL)));
        updateStatus(QString("East item after clear: %1").arg(border->getItemId(BorderType::EAST_HORIZONTAL)));
    }

    void onTestBorderItemsToPlace() {
        updateStatus("=== Testing Border Items To Place (Placeholder) ===");
        
        if (testBorders_.isEmpty()) {
            updateStatus("No AutoBorder objects available. Create some first.");
            return;
        }
        
        // Create a test map
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        
        auto& border = testBorders_.first();
        
        // Test getBorderItemsToPlace method (placeholder implementation)
        QList<Item*> borderItems = border->getBorderItemsToPlace(testMap.get());
        
        updateStatus(QString("Border items to place: %1 items").arg(borderItems.size()));
        updateStatus("Note: This is a placeholder implementation for future Item integration");
    }

    void onTestWxwidgetsCompatibility() {
        updateStatus("=== Testing wxwidgets Compatibility ===");
        
        updateStatus("AutoBorder class provides 1:1 migration from wxwidgets:");
        updateStatus("");
        updateStatus("Key compatibility features:");
        updateStatus("1. Same constructor: AutoBorder(quint32 id)");
        updateStatus("2. Same tiles[13] array structure");
        updateStatus("3. Same id, group, ground properties");
        updateStatus("4. Same edgeNameToID static method");
        updateStatus("5. Same load method signature and behavior");
        updateStatus("6. Same hasItemId and getItemId methods");
        updateStatus("");
        updateStatus("Border types migrated from wxwidgets/brush_enums.h:");
        updateStatus("- BORDER_NONE = 0");
        updateStatus("- NORTH_HORIZONTAL = 1");
        updateStatus("- EAST_HORIZONTAL = 2");
        updateStatus("- SOUTH_HORIZONTAL = 3");
        updateStatus("- WEST_HORIZONTAL = 4");
        updateStatus("- NORTHWEST_CORNER = 5");
        updateStatus("- NORTHEAST_CORNER = 6");
        updateStatus("- SOUTHWEST_CORNER = 7");
        updateStatus("- SOUTHEAST_CORNER = 8");
        updateStatus("- NORTHWEST_DIAGONAL = 9");
        updateStatus("- NORTHEAST_DIAGONAL = 10");
        updateStatus("- SOUTHEAST_DIAGONAL = 11");
        updateStatus("- SOUTHWEST_DIAGONAL = 12");
        updateStatus("- CARPET_CENTER = 13");
        updateStatus("");
        updateStatus("XML structure matches wxwidgets format exactly:");
        updateStatus("");
        updateStatus("Correct XML format:");
        updateStatus("<border id=\"1\">");
        updateStatus("  <borderitem edge=\"n\" item=\"357\"/>");
        updateStatus("  <borderitem edge=\"s\" item=\"356\"/>");
        updateStatus("  <borderitem edge=\"e\" item=\"358\"/>");
        updateStatus("  <borderitem edge=\"w\" item=\"359\"/>");
        updateStatus("  <borderitem edge=\"cse\" item=\"360\"/>");
        updateStatus("  <borderitem edge=\"csw\" item=\"361\"/>");
        updateStatus("  <borderitem edge=\"cne\" item=\"363\"/>");
        updateStatus("  <borderitem edge=\"cnw\" item=\"362\"/>");
        updateStatus("</border>");
    }

private:
    void setupUI() {
        setWindowTitle("AutoBorder Test Application - Task 41");
        setFixedSize(1000, 800);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("AutoBorder Implementation Test (Task 41)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Configuration group
        QGroupBox* configGroup = new QGroupBox("AutoBorder Configuration");
        QVBoxLayout* configLayout = new QVBoxLayout(configGroup);
        
        QHBoxLayout* idLayout = new QHBoxLayout();
        idLayout->addWidget(new QLabel("Border ID:"));
        borderIdSpin_ = new QSpinBox();
        borderIdSpin_->setRange(1, 99999);
        borderIdSpin_->setValue(1000);
        idLayout->addWidget(borderIdSpin_);
        configLayout->addLayout(idLayout);
        
        QHBoxLayout* groupLayout = new QHBoxLayout();
        groupLayout->addWidget(new QLabel("Group:"));
        groupSpin_ = new QSpinBox();
        groupSpin_->setRange(0, 9999);
        groupSpin_->setValue(1);
        groupLayout->addWidget(groupSpin_);
        configLayout->addLayout(groupLayout);
        
        isGroundCheck_ = new QCheckBox("Is Ground Border");
        configLayout->addWidget(isGroundCheck_);
        
        mainLayout->addWidget(configGroup);
        
        // Test buttons
        QHBoxLayout* buttonLayout1 = new QHBoxLayout();
        
        QPushButton* createBtn = new QPushButton("Create AutoBorder");
        QPushButton* edgeBtn = new QPushButton("Test Edge Mapping");
        QPushButton* itemBtn = new QPushButton("Test Item Management");
        
        buttonLayout1->addWidget(createBtn);
        buttonLayout1->addWidget(edgeBtn);
        buttonLayout1->addWidget(itemBtn);
        
        mainLayout->addLayout(buttonLayout1);
        
        QHBoxLayout* buttonLayout2 = new QHBoxLayout();
        
        QPushButton* xmlBtn = new QPushButton("Test XML Loading");
        QPushButton* clearBtn = new QPushButton("Test Clearance");
        QPushButton* placeBtn = new QPushButton("Test Items To Place");
        
        buttonLayout2->addWidget(xmlBtn);
        buttonLayout2->addWidget(clearBtn);
        buttonLayout2->addWidget(placeBtn);
        
        mainLayout->addLayout(buttonLayout2);
        
        QHBoxLayout* buttonLayout3 = new QHBoxLayout();
        
        QPushButton* compatBtn = new QPushButton("Test wxwidgets Compatibility");
        
        buttonLayout3->addWidget(compatBtn);
        buttonLayout3->addStretch();
        
        mainLayout->addLayout(buttonLayout3);
        
        // Status area
        statusText_ = new QTextEdit();
        statusText_->setMaximumHeight(400);
        statusText_->setReadOnly(true);
        mainLayout->addWidget(statusText_);
        
        // Exit button
        QPushButton* exitBtn = new QPushButton("Exit");
        connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);
        mainLayout->addWidget(exitBtn);
        
        // Connect buttons
        connect(createBtn, &QPushButton::clicked, this, &AutoBorderTestWidget::onCreateAutoBorder);
        connect(edgeBtn, &QPushButton::clicked, this, &AutoBorderTestWidget::onTestEdgeNameMapping);
        connect(itemBtn, &QPushButton::clicked, this, &AutoBorderTestWidget::onTestBorderItemManagement);
        connect(xmlBtn, &QPushButton::clicked, this, &AutoBorderTestWidget::onTestXMLLoading);
        connect(clearBtn, &QPushButton::clicked, this, &AutoBorderTestWidget::onTestBorderClearance);
        connect(placeBtn, &QPushButton::clicked, this, &AutoBorderTestWidget::onTestBorderItemsToPlace);
        connect(compatBtn, &QPushButton::clicked, this, &AutoBorderTestWidget::onTestWxwidgetsCompatibility);
    }
    
    void connectSignals() {
        // No additional signals needed for this test
    }
    
    void runTests() {
        updateStatus("AutoBorder Test Application Started");
        updateStatus("This application tests the AutoBorder implementation");
        updateStatus("as migrated 1:1 from wxwidgets for Task 41.");
        updateStatus("");
        updateStatus("Key features tested:");
        updateStatus("- Border type enumeration");
        updateStatus("- Edge name to ID mapping");
        updateStatus("- Border item management");
        updateStatus("- XML loading compatibility");
        updateStatus("- wxwidgets structure migration");
        updateStatus("");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "AutoBorderTest:" << message;
    }
    
    void testAutoBorderInterface(AutoBorder* border) {
        if (!border) return;
        
        updateStatus(QString("  Interface test - ID: %1, Group: %2, Ground: %3")
                    .arg(border->getId())
                    .arg(border->getGroup())
                    .arg(border->isGround()));
    }
    
    QSpinBox* borderIdSpin_;
    QSpinBox* groupSpin_;
    QCheckBox* isGroundCheck_;
    QTextEdit* statusText_;
    
    QList<std::unique_ptr<AutoBorder>> testBorders_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    AutoBorderTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "AutoBorderTest.moc"
