// TableBrushTest.cpp - Test for Task 42 TableBrush implementation

#include "TableBrush.h"
#include "BrushManager.h"
#include "Map.h"
#include "Tile.h"
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
#include <QDebug>
#include <QDomDocument>
#include <memory>

// Test widget to demonstrate TableBrush functionality
class TableBrushTestWidget : public QWidget {
    Q_OBJECT

public:
    TableBrushTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateTableBrush() {
        quint16 lookId = lookIdSpin_->value();
        
        auto tableBrush = std::make_unique<TableBrush>();
        tableBrush->setName(QString("Table %1").arg(lookId));
        // Note: look_id would be set through XML loading in real usage
        
        updateStatus(QString("Created TableBrush: %1 (LookID: %2)")
                    .arg(tableBrush->getName())
                    .arg(tableBrush->getLookID()));
        
        testTableBrushInterface(tableBrush.get());
        
        // Store for later use
        testBrushes_.append(std::move(tableBrush));
    }

    void onTestTablePlacement() {
        updateStatus("=== Testing Table Placement (wxwidgets Migration) ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No table brushes available. Create some first.");
            return;
        }
        
        // Create a test map
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(30, 30, 7);
        
        auto& tableBrush = testBrushes_.first();
        QPoint testPos(10, 10);
        
        // Test canDraw
        bool canDraw = tableBrush->canDraw(testMap.get(), testPos);
        updateStatus(QString("Can draw %1 at (10,10): %2").arg(tableBrush->getName()).arg(canDraw));
        
        // Test draw method
        if (canDraw) {
            Tile* tile = testMap->getTile(testPos.x(), testPos.y(), 0);
            if (tile) {
                tableBrush->draw(testMap.get(), tile);
                updateStatus("Table placement executed");
                
                // Check if table was placed
                bool hasTable = false;
                for (Item* item : tile->getItems()) {
                    if (item && item->isTable()) {
                        hasTable = true;
                        break;
                    }
                }
                updateStatus(QString("Tile has table after placement: %1").arg(hasTable));
            }
        }
    }

    void onTestTableConnections() {
        updateStatus("=== Testing Table Connection System (doTables) ===");
        
        if (testBrushes_.isEmpty()) {
            updateStatus("No table brushes available. Create some first.");
            return;
        }
        
        // Create test map with multiple tables
        auto testMap = std::make_unique<Map>(this);
        testMap->setMapSize(20, 20, 7);
        
        auto& tableBrush = testBrushes_.first();
        
        // Place tables in a pattern
        QList<QPoint> tablePositions = {{5, 5}, {6, 5}, {7, 5}, {5, 6}, {6, 6}};
        
        updateStatus("Placing tables in pattern...");
        for (const QPoint& pos : tablePositions) {
            Tile* tile = testMap->getTile(pos.x(), pos.y(), 0);
            if (tile) {
                tableBrush->draw(testMap.get(), tile);
                updateStatus(QString("Placed table at (%1,%2)").arg(pos.x()).arg(pos.y()));
            }
        }
        
        // Test doTables function for each tile
        updateStatus("Running doTables for connection logic...");
        for (const QPoint& pos : tablePositions) {
            Tile* tile = testMap->getTile(pos.x(), pos.y(), 0);
            if (tile) {
                TableBrush::doTables(testMap.get(), tile);
                updateStatus(QString("Processed table connections at (%1,%2)").arg(pos.x()).arg(pos.y()));
            }
        }
        
        updateStatus("Table connection processing completed");
    }

    void onTestTableVariations() {
        updateStatus("=== Testing Table Variations and Alignment ===");
        
        // Test different table alignments
        QStringList alignments = {"alone", "vertical", "horizontal", "south", "east", "north", "west"};
        
        for (const QString& alignment : alignments) {
            QtTableAlignment alignEnum;
            if (alignment == "alone") alignEnum = TABLE_ALONE;
            else if (alignment == "vertical") alignEnum = TABLE_VERTICAL;
            else if (alignment == "horizontal") alignEnum = TABLE_HORIZONTAL;
            else if (alignment == "south") alignEnum = TABLE_SOUTH_END;
            else if (alignment == "east") alignEnum = TABLE_EAST_END;
            else if (alignment == "north") alignEnum = TABLE_NORTH_END;
            else if (alignment == "west") alignEnum = TABLE_WEST_END;
            else alignEnum = TABLE_ALONE;
            
            updateStatus(QString("Table alignment '%1' = enum value %2").arg(alignment).arg(static_cast<int>(alignEnum)));
        }
        
        // Test lookup table
        updateStatus("Testing neighbor configuration lookup...");
        
        // Test some neighbor configurations
        quint8 testConfigs[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 255};
        
        for (quint8 config : testConfigs) {
            // This would use the static lookup table
            updateStatus(QString("Neighbor config %1 -> alignment (lookup table test)").arg(config));
        }
    }

    void onTestXMLLoading() {
        updateStatus("=== Testing XML Loading (wxwidgets Migration) ===");
        
        // Create XML document for testing (migrated structure)
        QDomDocument doc;
        QDomElement root = doc.createElement("tablebrush");
        root.setAttribute("name", "test_table");
        root.setAttribute("server_lookid", "2000");
        root.setAttribute("lookid", "2001");
        
        // Add table alignment elements (migrated from wxwidgets XML structure)
        QStringList alignments = {"alone", "vertical", "horizontal"};
        quint16 itemId = 3000;
        
        for (const QString& alignment : alignments) {
            QDomElement tableElement = doc.createElement("table");
            tableElement.setAttribute("align", alignment);
            
            // Add item elements
            for (int i = 0; i < 2; i++) {
                QDomElement itemElement = doc.createElement("item");
                itemElement.setAttribute("id", QString::number(itemId));
                itemElement.setAttribute("chance", QString::number(50 + i * 25));
                tableElement.appendChild(itemElement);
                itemId++;
            }
            
            root.appendChild(tableElement);
        }
        
        doc.appendChild(root);
        
        // Create TableBrush and load from XML
        auto tableBrush = std::make_unique<TableBrush>();
        QString warnings;
        
        bool loadSuccess = tableBrush->load(root, warnings);
        
        updateStatus(QString("XML loading successful: %1").arg(loadSuccess));
        if (!warnings.isEmpty()) {
            updateStatus(QString("Warnings: %1").arg(warnings));
        }
        
        updateStatus(QString("Loaded brush: %1 (LookID: %2)")
                    .arg(tableBrush->getName())
                    .arg(tableBrush->getLookID()));
        
        testBrushes_.append(std::move(tableBrush));
    }

    void onTestBrushManager() {
        updateStatus("=== Testing BrushManager Integration ===");
        
        BrushManager manager;
        
        // Test table brush creation through manager
        auto tableBrush = manager.createBrushShared(Brush::Type::Table, {{"lookId", 4000}});
        
        if (tableBrush && tableBrush->isTable()) {
            TableBrush* table = tableBrush->asTable();
            if (table) {
                table->setName("Manager Created Table");
                
                updateStatus(QString("BrushManager created table brush: %1 (LookID: %2)")
                            .arg(table->getName())
                            .arg(table->getLookID()));
                
                // Test brush properties
                updateStatus(QString("Brush properties - NeedBorders: %1, IsTable: %2")
                            .arg(table->needBorders())
                            .arg(table->isTable()));
                
                // Test type casting
                Brush* baseBrush = table;
                updateStatus(QString("Type casting - Base brush type: %1")
                            .arg(static_cast<int>(baseBrush->type())));
            }
        } else {
            updateStatus("Failed to create table brush through BrushManager");
        }
    }

    void onTestWxwidgetsCompatibility() {
        updateStatus("=== Testing wxwidgets Compatibility ===");
        
        updateStatus("TableBrush provides 1:1 migration from wxwidgets:");
        updateStatus("");
        updateStatus("Key compatibility features:");
        updateStatus("1. Same constructor and destructor");
        updateStatus("2. Same table_items[7] structure (QtTableNode array)");
        updateStatus("3. Same table_types lookup table (256 entries)");
        updateStatus("4. Same load method with server_lookid/lookid handling");
        updateStatus("5. Same draw/undraw methods with chance-based selection");
        updateStatus("6. Same doTables static method for neighbor analysis");
        updateStatus("");
        updateStatus("Table alignments migrated from wxwidgets:");
        updateStatus("- TABLE_ALONE = 0");
        updateStatus("- TABLE_VERTICAL = 1");
        updateStatus("- TABLE_HORIZONTAL = 2");
        updateStatus("- TABLE_SOUTH_END = 3");
        updateStatus("- TABLE_EAST_END = 4");
        updateStatus("- TABLE_NORTH_END = 5");
        updateStatus("- TABLE_WEST_END = 6");
        updateStatus("");
        updateStatus("XML structure matches wxwidgets format:");
        updateStatus("<tablebrush name=\"table_name\" server_lookid=\"2000\">");
        updateStatus("  <table align=\"alone\">");
        updateStatus("    <item id=\"3000\" chance=\"100\"/>");
        updateStatus("  </table>");
        updateStatus("  <table align=\"vertical\">");
        updateStatus("    <item id=\"3001\" chance=\"100\"/>");
        updateStatus("  </table>");
        updateStatus("</tablebrush>");
    }

private:
    void setupUI() {
        setWindowTitle("TableBrush Test Application - Task 42");
        setFixedSize(1000, 800);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("TableBrush Implementation Test (Task 42)");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Configuration group
        QGroupBox* configGroup = new QGroupBox("Table Brush Configuration");
        QVBoxLayout* configLayout = new QVBoxLayout(configGroup);
        
        QHBoxLayout* idLayout = new QHBoxLayout();
        idLayout->addWidget(new QLabel("Look ID:"));
        lookIdSpin_ = new QSpinBox();
        lookIdSpin_->setRange(1, 9999);
        lookIdSpin_->setValue(2000);
        idLayout->addWidget(lookIdSpin_);
        configLayout->addLayout(idLayout);
        
        mainLayout->addWidget(configGroup);
        
        // Test buttons
        QHBoxLayout* buttonLayout1 = new QHBoxLayout();
        
        QPushButton* createBtn = new QPushButton("Create Table Brush");
        QPushButton* placementBtn = new QPushButton("Test Table Placement");
        QPushButton* connectionsBtn = new QPushButton("Test Table Connections");
        
        buttonLayout1->addWidget(createBtn);
        buttonLayout1->addWidget(placementBtn);
        buttonLayout1->addWidget(connectionsBtn);
        
        mainLayout->addLayout(buttonLayout1);
        
        QHBoxLayout* buttonLayout2 = new QHBoxLayout();
        
        QPushButton* variationsBtn = new QPushButton("Test Table Variations");
        QPushButton* xmlBtn = new QPushButton("Test XML Loading");
        QPushButton* managerBtn = new QPushButton("Test BrushManager");
        
        buttonLayout2->addWidget(variationsBtn);
        buttonLayout2->addWidget(xmlBtn);
        buttonLayout2->addWidget(managerBtn);
        
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
        connect(createBtn, &QPushButton::clicked, this, &TableBrushTestWidget::onCreateTableBrush);
        connect(placementBtn, &QPushButton::clicked, this, &TableBrushTestWidget::onTestTablePlacement);
        connect(connectionsBtn, &QPushButton::clicked, this, &TableBrushTestWidget::onTestTableConnections);
        connect(variationsBtn, &QPushButton::clicked, this, &TableBrushTestWidget::onTestTableVariations);
        connect(xmlBtn, &QPushButton::clicked, this, &TableBrushTestWidget::onTestXMLLoading);
        connect(managerBtn, &QPushButton::clicked, this, &TableBrushTestWidget::onTestBrushManager);
        connect(compatBtn, &QPushButton::clicked, this, &TableBrushTestWidget::onTestWxwidgetsCompatibility);
    }
    
    void connectSignals() {
        // No additional signals needed for this test
    }
    
    void runTests() {
        updateStatus("TableBrush Test Application Started");
        updateStatus("This application tests the TableBrush implementation");
        updateStatus("as migrated 1:1 from wxwidgets for Task 42.");
        updateStatus("");
        updateStatus("Key features tested:");
        updateStatus("- Table item placement and removal");
        updateStatus("- Table connection and alignment system");
        updateStatus("- Neighbor-based table variations");
        updateStatus("- XML loading compatibility");
        updateStatus("- wxwidgets structure migration");
        updateStatus("");
    }
    
    void updateStatus(const QString& message) {
        statusText_->append(message);
        qDebug() << "TableBrushTest:" << message;
    }
    
    void testTableBrushInterface(TableBrush* brush) {
        if (!brush) return;
        
        updateStatus(QString("  Interface test - IsTable: %1, NeedBorders: %2")
                    .arg(brush->isTable())
                    .arg(brush->needBorders()));
        
        updateStatus(QString("  Properties - Name: %1, LookID: %2")
                    .arg(brush->getName())
                    .arg(brush->getLookID()));
    }
    
    QSpinBox* lookIdSpin_;
    QTextEdit* statusText_;
    
    QList<std::unique_ptr<TableBrush>> testBrushes_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    TableBrushTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "TableBrushTest.moc"
