#include "BrushManager.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDebug>
#include <QTimer>

// Simple test application to verify BrushManager functionality
class BrushManagerTestWidget : public QWidget {
    Q_OBJECT

public:
    BrushManagerTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onCreateBrush() {
        Brush::Type type = static_cast<Brush::Type>(brushTypeCombo_->currentData().toInt());
        
        QVariantMap parameters;
        if (type == Brush::Type::Ground) {
            parameters["groundId"] = groundIdSpin_->value();
        } else if (type == Brush::Type::Door) {
            parameters["doorType"] = static_cast<int>(DoorType::Normal);
        } else if (type == Brush::Type::Pixel) {
            parameters["color"] = QColor(Qt::red);
        }
        
        Brush* brush = brushManager_.createBrush(type, parameters);
        if (brush) {
            updateStatus(QString("Created brush: %1").arg(brush->name()));
            updateBrushList();
        } else {
            updateStatus("Failed to create brush");
        }
    }

    void onCreateSharedBrush() {
        Brush::Type type = static_cast<Brush::Type>(brushTypeCombo_->currentData().toInt());
        
        QVariantMap parameters;
        parameters["size"] = brushSizeSpin_->value();
        
        auto sharedBrush = brushManager_.createBrushShared(type, parameters);
        if (sharedBrush) {
            updateStatus(QString("Created shared brush: %1").arg(sharedBrush->name()));
            updateBrushList();
        } else {
            updateStatus("Failed to create shared brush");
        }
    }

    void onSetCurrentBrush() {
        QString brushName = currentBrushCombo_->currentText();
        if (!brushName.isEmpty()) {
            brushManager_.setCurrentBrush(brushName);
            updateStatus(QString("Set current brush to: %1").arg(brushName));
        }
    }

    void onUpdateBrushSize() {
        int newSize = brushSizeSpin_->value();
        brushManager_.updateBrushSize(newSize);
        updateStatus(QString("Updated brush size to: %1").arg(newSize));
    }

    void onUpdateBrushShape() {
        Brush::BrushShape shape = static_cast<Brush::BrushShape>(brushShapeCombo_->currentData().toInt());
        brushManager_.updateBrushShape(shape);
        updateStatus(QString("Updated brush shape to: %1").arg(shape == Brush::BrushShape::Circle ? "Circle" : "Square"));
    }

    void onTestBrushContext() {
        brushManager_.setBrushContext("testKey", "testValue");
        QVariant value = brushManager_.getBrushContext("testKey");
        updateStatus(QString("Brush context test - Set: testValue, Got: %1").arg(value.toString()));
    }

    void onClearCache() {
        int sizeBefore = brushManager_.getBrushes().size();
        // Note: We don't have a clearAll method, so we'll just show current size
        updateStatus(QString("Current brush count: %1").arg(sizeBefore));
    }

    void onCurrentBrushChanged(Brush* newBrush, Brush* previousBrush) {
        QString newName = newBrush ? newBrush->name() : "None";
        QString prevName = previousBrush ? previousBrush->name() : "None";
        updateStatus(QString("Brush changed: %1 -> %2").arg(prevName).arg(newName));
    }

    void onBrushCreated(Brush* brush) {
        updateStatus(QString("Brush created signal: %1").arg(brush ? brush->name() : "Unknown"));
    }

    void onBrushSizeChanged(int newSize) {
        updateStatus(QString("Brush size changed signal: %1").arg(newSize));
    }

    void onBrushShapeChanged(Brush::BrushShape newShape) {
        updateStatus(QString("Brush shape changed signal: %1").arg(newShape == Brush::BrushShape::Circle ? "Circle" : "Square"));
    }

private:
    void setupUI() {
        setWindowTitle("BrushManager Test Application");
        setFixedSize(600, 500);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("BrushManager Test Application");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Brush creation controls
        QHBoxLayout* creationLayout = new QHBoxLayout();
        
        creationLayout->addWidget(new QLabel("Type:"));
        brushTypeCombo_ = new QComboBox();
        brushTypeCombo_->addItem("Ground", static_cast<int>(Brush::Type::Ground));
        brushTypeCombo_->addItem("Door", static_cast<int>(Brush::Type::Door));
        brushTypeCombo_->addItem("Pixel", static_cast<int>(Brush::Type::Pixel));
        creationLayout->addWidget(brushTypeCombo_);
        
        creationLayout->addWidget(new QLabel("Ground ID:"));
        groundIdSpin_ = new QSpinBox();
        groundIdSpin_->setRange(100, 999);
        groundIdSpin_->setValue(100);
        creationLayout->addWidget(groundIdSpin_);
        
        QPushButton* createBtn = new QPushButton("Create Brush");
        QPushButton* createSharedBtn = new QPushButton("Create Shared");
        creationLayout->addWidget(createBtn);
        creationLayout->addWidget(createSharedBtn);
        
        mainLayout->addLayout(creationLayout);
        
        // Brush management controls
        QHBoxLayout* managementLayout = new QHBoxLayout();
        
        managementLayout->addWidget(new QLabel("Current:"));
        currentBrushCombo_ = new QComboBox();
        managementLayout->addWidget(currentBrushCombo_);
        
        QPushButton* setCurrentBtn = new QPushButton("Set Current");
        managementLayout->addWidget(setCurrentBtn);
        
        mainLayout->addLayout(managementLayout);
        
        // Brush property controls
        QHBoxLayout* propertyLayout = new QHBoxLayout();
        
        propertyLayout->addWidget(new QLabel("Size:"));
        brushSizeSpin_ = new QSpinBox();
        brushSizeSpin_->setRange(1, 10);
        brushSizeSpin_->setValue(1);
        propertyLayout->addWidget(brushSizeSpin_);
        
        propertyLayout->addWidget(new QLabel("Shape:"));
        brushShapeCombo_ = new QComboBox();
        brushShapeCombo_->addItem("Square", static_cast<int>(Brush::BrushShape::Square));
        brushShapeCombo_->addItem("Circle", static_cast<int>(Brush::BrushShape::Circle));
        propertyLayout->addWidget(brushShapeCombo_);
        
        QPushButton* updateSizeBtn = new QPushButton("Update Size");
        QPushButton* updateShapeBtn = new QPushButton("Update Shape");
        propertyLayout->addWidget(updateSizeBtn);
        propertyLayout->addWidget(updateShapeBtn);
        
        mainLayout->addLayout(propertyLayout);
        
        // Test buttons
        QHBoxLayout* testLayout = new QHBoxLayout();
        
        QPushButton* contextTestBtn = new QPushButton("Test Context");
        QPushButton* clearBtn = new QPushButton("Show Count");
        
        testLayout->addWidget(contextTestBtn);
        testLayout->addWidget(clearBtn);
        
        mainLayout->addLayout(testLayout);
        
        // Status area
        statusLabel_ = new QLabel("Ready to test BrushManager functionality");
        statusLabel_->setStyleSheet("border: 1px solid gray; padding: 5px; margin: 10px;");
        statusLabel_->setWordWrap(true);
        statusLabel_->setMaximumHeight(150);
        mainLayout->addWidget(statusLabel_);
        
        // Exit button
        QPushButton* exitBtn = new QPushButton("Exit");
        connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);
        mainLayout->addWidget(exitBtn);
        
        // Connect buttons
        connect(createBtn, &QPushButton::clicked, this, &BrushManagerTestWidget::onCreateBrush);
        connect(createSharedBtn, &QPushButton::clicked, this, &BrushManagerTestWidget::onCreateSharedBrush);
        connect(setCurrentBtn, &QPushButton::clicked, this, &BrushManagerTestWidget::onSetCurrentBrush);
        connect(updateSizeBtn, &QPushButton::clicked, this, &BrushManagerTestWidget::onUpdateBrushSize);
        connect(updateShapeBtn, &QPushButton::clicked, this, &BrushManagerTestWidget::onUpdateBrushShape);
        connect(contextTestBtn, &QPushButton::clicked, this, &BrushManagerTestWidget::onTestBrushContext);
        connect(clearBtn, &QPushButton::clicked, this, &BrushManagerTestWidget::onClearCache);
    }
    
    void connectSignals() {
        // Connect BrushManager signals
        connect(&brushManager_, &BrushManager::currentBrushChanged, this, &BrushManagerTestWidget::onCurrentBrushChanged);
        connect(&brushManager_, &BrushManager::brushCreated, this, &BrushManagerTestWidget::onBrushCreated);
        connect(&brushManager_, &BrushManager::brushSizeChanged, this, &BrushManagerTestWidget::onBrushSizeChanged);
        connect(&brushManager_, &BrushManager::brushShapeChanged, this, &BrushManagerTestWidget::onBrushShapeChanged);
    }
    
    void runTests() {
        updateStatus("BrushManager test application started");
        updateStatus("Available brush types: " + brushManager_.getAvailableBrushTypes().join(", "));
        
        // Auto-create a test brush after a short delay
        QTimer::singleShot(1000, [this]() {
            QVariantMap params;
            params["groundId"] = 100;
            Brush* testBrush = brushManager_.createBrush(Brush::Type::Ground, params);
            if (testBrush) {
                updateStatus("Auto-created test ground brush");
                updateBrushList();
            }
        });
    }
    
    void updateStatus(const QString& message) {
        QString currentText = statusLabel_->text();
        if (currentText == "Ready to test BrushManager functionality") {
            currentText.clear();
        }
        
        if (!currentText.isEmpty()) {
            currentText += "\n";
        }
        currentText += message;
        
        statusLabel_->setText(currentText);
        qDebug() << "BrushManagerTest:" << message;
    }
    
    void updateBrushList() {
        currentBrushCombo_->clear();
        QMap<QString, Brush*> brushes = brushManager_.getBrushes();
        for (auto it = brushes.begin(); it != brushes.end(); ++it) {
            currentBrushCombo_->addItem(it.key());
        }
    }
    
    BrushManager brushManager_;
    QComboBox* brushTypeCombo_;
    QComboBox* currentBrushCombo_;
    QComboBox* brushShapeCombo_;
    QSpinBox* groundIdSpin_;
    QSpinBox* brushSizeSpin_;
    QLabel* statusLabel_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    BrushManagerTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "BrushManagerTest.moc"
