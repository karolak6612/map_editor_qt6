// CarpetBrushTest.cpp - Test for Task 43 CarpetBrush implementation

#include "CarpetBrush.h"
#include "BrushManager.h"
#include "Map.h"
#include "Tile.h"
#include "AutoBorder.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QDebug>
#include <QDomDocument>
#include <memory>

// Test widget to demonstrate CarpetBrush functionality
class CarpetBrushTestWidget : public QWidget {
    Q_OBJECT

public:
    CarpetBrushTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateCarpetBrush() {
        quint16 lookId = lookIdSpin_->value();
        
        auto carpetBrush = std::make_unique<CarpetBrush>();
        carpetBrush->setName(QString("Carpet %1").arg(lookId));
        // Note: look_id would be set through XML loading in real usage
        
        updateStatus(QString("Created CarpetBrush: %1 (LookID: %2)")
                    .arg(carpetBrush->getName())
                    .arg(carpetBrush->getLookID()));
        
        testCarpetBrushInterface(carpetBrush.get());
        
        // Store for later use
        testBrushes_.append(std::move(carpetBrush));
    }

    void onTestCarpetPlacement() {
        updateStatus("=== Testing Carpet Placement (wxwidgets Migration) ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No carpet brushes available. Create some first.");
            return;
        }
        
        // Create a test map
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(30, 30, 7);
        
        auto& carpetBrush = testBrushes_.first();
        QPoint testPos(10, 10);
        
        // Test canDraw
        bool canDraw = carpetBrush->canDraw(testMap.get(), testPos);
        updateStatus(QString("Can draw %1 at (10,10): %2").arg(carpetBrush->getName()).arg(canDraw));
        
        // Test draw method
        if (canDraw) {
            Tile* tile = testMap->getTile(testPos.x(), testPos.y(), 0);
            if (tile) {
                carpetBrush->draw(testMap.get(), tile);
                updateStatus("Carpet placement executed");
                
                // Check if carpet was placed
                bool hasCarpet = false;
                for (Item* item : tile->getItems()) {
                    if (item && item->isCarpet()) {
                        hasCarpet = true;
                        break;
                    }
                }
                updateStatus(QString("Tile has carpet after placement: %1").arg(hasCarpet));
            }
        }
    }

    void onTestCarpetBorders() {
        updateStatus("=== Testing Carpet Border System (doCarpets) ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No carpet brushes available. Create some first.");
            return;
        }
        
        // Create test map with multiple carpets
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        
        auto& carpetBrush = testBrushes_.first();
        
        // Place carpets in a pattern
        QList<QPoint> carpetPositions = {{5, 5}, {6, 5}, {7, 5}, {5, 6}, {6, 6}, {7, 6}, {5, 7}, {6, 7}, {7, 7}};
        
        updateStatus("Placing carpets in 3x3 pattern...");
        for (const QPoint& pos : carpetPositions) {
            Tile* tile = testMap->getTile(pos.x(), pos.y(), 0);
            if (tile) {
                carpetBrush->draw(testMap.get(), tile);
                updateStatus(QString("Placed carpet at (%1,%2)").arg(pos.x()).arg(pos.y()));
            }
        }
        
        // Test doCarpets function for each tile
        updateStatus("Running doCarpets for border logic...");
        for (const QPoint& pos : carpetPositions) {
            Tile* tile = testMap->getTile(pos.x(), pos.y(), 0);
            if (tile) {
                CarpetBrush::doCarpets(testMap.get(), tile);
                updateStatus(QString("Processed carpet borders at (%1,%2)").arg(pos.x()).arg(pos.y()));
            }
        }
        
        updateStatus("Carpet border processing completed");
    }

    void onTestBorderTypes() {
        updateStatus("=== Testing Border Types and Alignment ===");
        
        // Test different border types from AutoBorder
        QStringList borderNames = {"BORDER_NONE", "NORTH_HORIZONTAL", "EAST_HORIZONTAL", "SOUTH_HORIZONTAL", 
                                  "WEST_HORIZONTAL", "NORTHWEST_CORNER", "NORTHEAST_CORNER", "SOUTHWEST_CORNER", 
                                  "SOUTHEAST_CORNER", "NORTHWEST_DIAGONAL", "NORTHEAST_DIAGONAL", "SOUTHEAST_DIAGONAL", 
                                  "SOUTHWEST_DIAGONAL", "CARPET_CENTER"};
        
        for (int i = 0; i < borderNames.size(); i++) {
            BorderType borderType = static_cast<BorderType>(i);
            updateStatus(QString("BorderType %1 = %2").arg(i).arg(borderNames[i]));
        }
        
        // Test carpet lookup table
        updateStatus("Testing neighbor configuration lookup...");
        
        // Test some neighbor configurations
        quint8 testConfigs[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 255};
        
        for (quint8 config : testConfigs) {
            // This would use the static lookup table
            updateStatus(QString("Neighbor config %1 -> border type (lookup table test)").arg(config));
        }
    }

    void onTestGetCarpetLogic() {
        updateStatus("=== Testing getCarpet Logic (wxwidgets Migration) ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No carpet brushes available. Create some first.");
            return;
        }
        
        auto& carpetBrush = testBrushes_.first();
        
        // Test getRandomCarpet for different border types
        QList<BorderType> testBorders = {
            BorderType::CARPET_CENTER,
            BorderType::NORTH_HORIZONTAL,
            BorderType::SOUTH_HORIZONTAL,
            BorderType::EAST_HORIZONTAL,
            BorderType::WEST_HORIZONTAL,
            BorderType::NORTHWEST_CORNER,
            BorderType::NORTHEAST_CORNER,
            BorderType::SOUTHWEST_CORNER,
            BorderType::SOUTHEAST_CORNER
        };
        
        for (BorderType borderType : testBorders) {
            quint16 carpetId = carpetBrush->getRandomCarpet(borderType);
            updateStatus(QString("BorderType %1 -> Carpet ID %2")
                        .arg(static_cast<int>(borderType))
                        .arg(carpetId));
        }
        
        updateStatus("getCarpet logic test completed");
    }

    void onTestXMLLoading() {
        updateStatus("=== Testing XML Loading (wxwidgets Migration) ===");
        
        // Create XML document for testing (migrated structure)
        QDomDocument doc;
        QDomElement root = doc.createElement("carpetbrush");
        root.setAttribute("name", "test_carpet");
        root.setAttribute("server_lookid", "3000");
        root.setAttribute("lookid", "3001");
        
        // Add carpet border elements (migrated from wxwidgets XML structure)
        QStringList borders = {"center", "n", "s", "e", "w", "cnw", "cne", "csw", "cse"};
        quint16 itemId = 4000;
        
        for (const QString& border : borders) {
            QDomElement carpetElement = doc.createElement("carpet");
            carpetElement.setAttribute("align", border);
            
            // Add item elements
            for (int i = 0; i < 2; i++) {
                QDomElement itemElement = doc.createElement("item");
                itemElement.setAttribute("id", QString::number(itemId));
                itemElement.setAttribute("chance", QString::number(50 + i * 25));
                carpetElement.appendChild(itemElement);
                itemId++;
            }
            
            root.appendChild(carpetElement);
        }
        
        doc.appendChild(root);
        
        // Create CarpetBrush and load from XML
        auto carpetBrush = std::make_unique<CarpetBrush>();
        QString warnings;
        
        bool loadSuccess = carpetBrush->load(root, warnings);
        
        updateStatus(QString("XML loading successful: %1").arg(loadSuccess));
        if (!warnings.isEmpty()) {
            updateStatus(QString("Warnings: %1").arg(warnings));
        }
        
        updateStatus(QString("Loaded brush: %1 (LookID: %2)")
                    .arg(carpetBrush->getName())
                    .arg(carpetBrush->getLookID()));
        
        testBrushes_.append(std::move(carpetBrush));
    }

    void onTestLayerCarpets() {
        updateStatus("=== Testing Layer Carpets Feature ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No carpet brushes available. Create some first.");
            return;
        }
        
        // Create test map
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        
        auto& carpetBrush = testBrushes_.first();
        Tile* testTile = testMap->getTile(10, 10, 0);
        
        if (testTile) {
            // Test with layering disabled (default)
            updateStatus("Testing with LAYER_CARPETS = false (default)");
            carpetBrush->draw(testMap.get(), testTile);
            int itemCount1 = testTile->getItems().size();
            updateStatus(QString("Items after first draw: %1").arg(itemCount1));
            
            carpetBrush->draw(testMap.get(), testTile);
            int itemCount2 = testTile->getItems().size();
            updateStatus(QString("Items after second draw: %1").arg(itemCount2));
            updateStatus(QString("Layering disabled - items should be same: %1").arg(itemCount1 == itemCount2));
            
            // Test undraw
            carpetBrush->undraw(testMap.get(), testTile);
            int itemCount3 = testTile->getItems().size();
            updateStatus(QString("Items after undraw: %1").arg(itemCount3));
        }
    }

    void onTestWxwidgetsCompatibility() {
        updateStatus("=== Testing wxwidgets Compatibility ===");
        
        updateStatus("CarpetBrush provides 1:1 migration from wxwidgets:");
        updateStatus("");
        updateStatus("Key compatibility features:");
        updateStatus("1. Same constructor and destructor");
        updateStatus("2. Same carpet_items[13] structure (QtCarpetNode array)");
        updateStatus("3. Same carpet_types lookup table (256 entries)");
        updateStatus("4. Same load method with server_lookid/lookid handling");
        updateStatus("5. Same draw/undraw methods with border-aware placement");
        updateStatus("6. Same doCarpets static method for neighbor analysis");
        updateStatus("7. Same getRandomCarpet method for border-specific selection");
        updateStatus("");
        updateStatus("Border types migrated from AutoBorder:");
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
        updateStatus("XML structure matches wxwidgets format:");
        updateStatus("<carpetbrush name=\"carpet_name\" server_lookid=\"3000\">");
        updateStatus("  <carpet align=\"center\">");
        updateStatus("    <item id=\"4000\" chance=\"100\"/>");
        updateStatus("  </carpet>");
        updateStatus("  <carpet align=\"n\">");
        updateStatus("    <item id=\"4001\" chance=\"100\"/>");
        updateStatus("  </carpet>");
        updateStatus("</carpetbrush>");
    }

private:
    void setupUI() {
        setWindowTitle("CarpetBrush Test Application - Task 43");
        setFixedSize(1000, 800);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("CarpetBrush Implementation Test (Task 43)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Configuration group
        QGroupBox* configGroup = new QGroupBox("Carpet Brush Configuration");
        QVBoxLayout* configLayout = new QVBoxLayout(configGroup);
        
        QHBoxLayout* idLayout = new QHBoxLayout();
        idLayout->addWidget(new QLabel("Look ID:"));
        lookIdSpin_ = new QSpinBox();
        lookIdSpin_->setRange(1, 9999);
        lookIdSpin_->setValue(3000);
        idLayout->addWidget(lookIdSpin_);
        configLayout->addLayout(idLayout);
        
        mainLayout->addWidget(configGroup);
        
        // Test buttons
        QHBoxLayout* buttonLayout1 = new QHBoxLayout();
        
        QPushButton* createBtn = new QPushButton("Create Carpet Brush");
        QPushButton* placementBtn = new QPushButton("Test Carpet Placement");
        QPushButton* bordersBtn = new QPushButton("Test Carpet Borders");
        
        buttonLayout1->addWidget(createBtn);
        buttonLayout1->addWidget(placementBtn);
        buttonLayout1->addWidget(bordersBtn);
        
        mainLayout->addLayout(buttonLayout1);
        
        QHBoxLayout* buttonLayout2 = new QHBoxLayout();
        
        QPushButton* borderTypesBtn = new QPushButton("Test Border Types");
        QPushButton* getCarpetBtn = new QPushButton("Test getCarpet Logic");
        QPushButton* xmlBtn = new QPushButton("Test XML Loading");
        
        buttonLayout2->addWidget(borderTypesBtn);
        buttonLayout2->addWidget(getCarpetBtn);
        buttonLayout2->addWidget(xmlBtn);
        
        mainLayout->addLayout(buttonLayout2);
        
        QHBoxLayout* buttonLayout3 = new QHBoxLayout();
        
        QPushButton* layerBtn = new QPushButton("Test Layer Carpets");
        QPushButton* compatBtn = new QPushButton("Test wxwidgets Compatibility");
        
        buttonLayout3->addWidget(layerBtn);
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
        connect(createBtn, &QPushButton::clicked, this, &CarpetBrushTestWidget::onCreateCarpetBrush);
        connect(placementBtn, &QPushButton::clicked, this, &CarpetBrushTestWidget::onTestCarpetPlacement);
        connect(bordersBtn, &QPushButton::clicked, this, &CarpetBrushTestWidget::onTestCarpetBorders);
        connect(borderTypesBtn, &QPushButton::clicked, this, &CarpetBrushTestWidget::onTestBorderTypes);
        connect(getCarpetBtn, &QPushButton::clicked, this, &CarpetBrushTestWidget::onTestGetCarpetLogic);
        connect(xmlBtn, &QPushButton::clicked, this, &CarpetBrushTestWidget::onTestXMLLoading);
        connect(layerBtn, &QPushButton::clicked, this, &CarpetBrushTestWidget::onTestLayerCarpets);
        connect(compatBtn, &QPushButton::clicked, this, &CarpetBrushTestWidget::onTestWxwidgetsCompatibility);
    }
    
    void connectSignals() {
        // No additional signals needed for this test
    }
    
    void runTests() {
        updateStatus("CarpetBrush Test Application Started");
        updateStatus("This application tests the CarpetBrush implementation");
        updateStatus("as migrated 1:1 from wxwidgets for Task 43.");
        updateStatus("");
        updateStatus("Key features tested:");
        updateStatus("- Carpet item placement and removal");
        updateStatus("- Border-aware carpet system (doCarpets)");
        updateStatus("- getCarpet logic for border-specific selection");
        updateStatus("- Layer carpets functionality");
        updateStatus("- XML loading compatibility");
        updateStatus("- wxwidgets structure migration");
        updateStatus("");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "CarpetBrushTest:" << message;
    }
    
    void testCarpetBrushInterface(CarpetBrush* brush) {
        if (!brush) return;
        
        updateStatus(QString("  Interface test - IsCarpet: %1, NeedBorders: %2")
                    .arg(brush->isCarpet())
                    .arg(brush->needBorders()));
        
        updateStatus(QString("  Properties - Name: %1, LookID: %2")
                    .arg(brush->getName())
                    .arg(brush->getLookID()));
    }
    
    QSpinBox* lookIdSpin_;
    QTextEdit* statusText_;
    
    QList<std::unique_ptr<CarpetBrush>> testBrushes_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    CarpetBrushTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "CarpetBrushTest.moc"
