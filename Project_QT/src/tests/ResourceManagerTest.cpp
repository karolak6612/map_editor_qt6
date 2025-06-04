#include "ResourceManager.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QTimer>

// Simple test application to verify ResourceManager functionality
class ResourceManagerTestWidget : public QWidget {
    Q_OBJECT

public:
    ResourceManagerTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
        runTests();
    }

private slots:
    void onTestBasicLoading() {
        qDebug() << "=== Testing Basic Loading ===";
        
        // Test loading from Qt resources
        QPixmap pixmap1 = ResourceManager::instance().getPixmap(":/images/brush.png", ResourceCategory::BRUSHES);
        updateStatus(QString("Loaded brush.png: %1x%2").arg(pixmap1.width()).arg(pixmap1.height()));
        
        // Test categorized access
        QPixmap iconPixmap = ResourceManager::instance().getIconPixmap("brush");
        updateStatus(QString("Loaded icon via category: %1x%2").arg(iconPixmap.width()).arg(iconPixmap.height()));
        
        // Test fallback functionality
        QPixmap fallback = ResourceManager::instance().getFallbackPixmap(QSize(32, 32));
        updateStatus(QString("Generated fallback: %1x%2").arg(fallback.width()).arg(fallback.height()));
        
        // Display loaded images
        if (!pixmap1.isNull()) {
            imageLabel1_->setPixmap(pixmap1.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        if (!fallback.isNull()) {
            imageLabel2_->setPixmap(fallback.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    void onTestCaching() {
        qDebug() << "=== Testing Caching ===";
        
        QString testPath = ":/images/brush.png";
        
        // First load
        auto start = QDateTime::currentMSecsSinceEpoch();
        QPixmap pixmap1 = ResourceManager::instance().getPixmap(testPath, ResourceCategory::BRUSHES);
        auto firstLoadTime = QDateTime::currentMSecsSinceEpoch() - start;
        
        // Second load (should be from cache)
        start = QDateTime::currentMSecsSinceEpoch();
        QPixmap pixmap2 = ResourceManager::instance().getPixmap(testPath, ResourceCategory::BRUSHES);
        auto secondLoadTime = QDateTime::currentMSecsSinceEpoch() - start;
        
        updateStatus(QString("First load: %1ms, Second load: %2ms").arg(firstLoadTime).arg(secondLoadTime));
        updateStatus(QString("Cache hit improvement: %1x faster").arg(double(firstLoadTime) / qMax(1.0, double(secondLoadTime))));
        
        // Test cache status
        bool isCached = ResourceManager::instance().isPixmapCached(testPath);
        int cacheSize = ResourceManager::instance().cacheSize();
        updateStatus(QString("Is cached: %1, Cache size: %2").arg(isCached ? "Yes" : "No").arg(cacheSize));
    }

    void onTestCategories() {
        qDebug() << "=== Testing Categories ===";
        
        // Test different category placeholders
        QPixmap iconPlaceholder = ResourceManager::instance().getPlaceholderPixmap(ResourceCategory::ICONS, QSize(24, 24));
        QPixmap brushPlaceholder = ResourceManager::instance().getPlaceholderPixmap(ResourceCategory::BRUSHES, QSize(24, 24));
        QPixmap itemPlaceholder = ResourceManager::instance().getPlaceholderPixmap(ResourceCategory::ITEMS, QSize(24, 24));
        
        updateStatus(QString("Generated placeholders: Icons(%1x%2), Brushes(%3x%4), Items(%5x%6)")
                    .arg(iconPlaceholder.width()).arg(iconPlaceholder.height())
                    .arg(brushPlaceholder.width()).arg(brushPlaceholder.height())
                    .arg(itemPlaceholder.width()).arg(itemPlaceholder.height()));
        
        // Display category placeholders
        if (!iconPlaceholder.isNull()) {
            imageLabel3_->setPixmap(iconPlaceholder.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        if (!brushPlaceholder.isNull()) {
            imageLabel4_->setPixmap(brushPlaceholder.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        
        // Test supported formats
        QStringList formats = ResourceManager::instance().getSupportedFormats();
        updateStatus(QString("Supported formats: %1").arg(formats.join(", ")));
    }

    void onTestSpriteSheet() {
        qDebug() << "=== Testing Sprite Sheet ===";
        
        // Create a test sprite sheet
        QPixmap testSheet = createTestSpriteSheet();
        QString sheetPath = "test_sheet";
        
        // Manually add to cache for testing
        // Note: In real usage, this would be loaded from file
        updateStatus("Created test sprite sheet for extraction testing");
        
        // Test extraction (using fallback since we don't have real sheet)
        QPixmap extracted = ResourceManager::instance().getPixmapFromSheet(":/images/brush.png", QRect(0, 0, 16, 16), ResourceCategory::MISC);
        updateStatus(QString("Extracted sprite: %1x%2").arg(extracted.width()).arg(extracted.height()));
        
        if (!extracted.isNull()) {
            imageLabel5_->setPixmap(extracted.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    void onClearCache() {
        qDebug() << "=== Clearing Cache ===";
        int sizeBefore = ResourceManager::instance().cacheSize();
        ResourceManager::instance().clearCache();
        int sizeAfter = ResourceManager::instance().cacheSize();
        updateStatus(QString("Cache cleared: %1 -> %2 items").arg(sizeBefore).arg(sizeAfter));
    }

private:
    void setupUI() {
        setWindowTitle("ResourceManager Test Application");
        setFixedSize(600, 500);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Title
        QLabel* titleLabel = new QLabel("ResourceManager Test Application");
        titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Test buttons
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        
        QPushButton* basicTestBtn = new QPushButton("Test Basic Loading");
        QPushButton* cacheTestBtn = new QPushButton("Test Caching");
        QPushButton* categoryTestBtn = new QPushButton("Test Categories");
        QPushButton* spriteTestBtn = new QPushButton("Test Sprite Sheet");
        QPushButton* clearCacheBtn = new QPushButton("Clear Cache");
        
        buttonLayout->addWidget(basicTestBtn);
        buttonLayout->addWidget(cacheTestBtn);
        buttonLayout->addWidget(categoryTestBtn);
        buttonLayout->addWidget(spriteTestBtn);
        buttonLayout->addWidget(clearCacheBtn);
        
        mainLayout->addLayout(buttonLayout);
        
        // Image display area
        QHBoxLayout* imageLayout = new QHBoxLayout();
        
        imageLabel1_ = new QLabel("Image 1");
        imageLabel2_ = new QLabel("Image 2");
        imageLabel3_ = new QLabel("Image 3");
        imageLabel4_ = new QLabel("Image 4");
        imageLabel5_ = new QLabel("Image 5");
        
        imageLabel1_->setFixedSize(64, 64);
        imageLabel2_->setFixedSize(64, 64);
        imageLabel3_->setFixedSize(48, 48);
        imageLabel4_->setFixedSize(48, 48);
        imageLabel5_->setFixedSize(48, 48);
        
        imageLabel1_->setStyleSheet("border: 1px solid gray;");
        imageLabel2_->setStyleSheet("border: 1px solid gray;");
        imageLabel3_->setStyleSheet("border: 1px solid gray;");
        imageLabel4_->setStyleSheet("border: 1px solid gray;");
        imageLabel5_->setStyleSheet("border: 1px solid gray;");
        
        imageLabel1_->setAlignment(Qt::AlignCenter);
        imageLabel2_->setAlignment(Qt::AlignCenter);
        imageLabel3_->setAlignment(Qt::AlignCenter);
        imageLabel4_->setAlignment(Qt::AlignCenter);
        imageLabel5_->setAlignment(Qt::AlignCenter);
        
        imageLayout->addWidget(imageLabel1_);
        imageLayout->addWidget(imageLabel2_);
        imageLayout->addWidget(imageLabel3_);
        imageLayout->addWidget(imageLabel4_);
        imageLayout->addWidget(imageLabel5_);
        
        mainLayout->addLayout(imageLayout);
        
        // Status area
        statusLabel_ = new QLabel("Ready to test ResourceManager functionality");
        statusLabel_->setStyleSheet("border: 1px solid gray; padding: 5px; margin: 10px;");
        statusLabel_->setWordWrap(true);
        statusLabel_->setMaximumHeight(150);
        mainLayout->addWidget(statusLabel_);
        
        // Exit button
        QPushButton* exitBtn = new QPushButton("Exit");
        connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);
        mainLayout->addWidget(exitBtn);
        
        // Connect test buttons
        connect(basicTestBtn, &QPushButton::clicked, this, &ResourceManagerTestWidget::onTestBasicLoading);
        connect(cacheTestBtn, &QPushButton::clicked, this, &ResourceManagerTestWidget::onTestCaching);
        connect(categoryTestBtn, &QPushButton::clicked, this, &ResourceManagerTestWidget::onTestCategories);
        connect(spriteTestBtn, &QPushButton::clicked, this, &ResourceManagerTestWidget::onTestSpriteSheet);
        connect(clearCacheBtn, &QPushButton::clicked, this, &ResourceManagerTestWidget::onClearCache);
    }
    
    void connectSignals() {
        // Auto-run basic test after a short delay
        QTimer::singleShot(1000, this, &ResourceManagerTestWidget::onTestBasicLoading);
    }
    
    void runTests() {
        updateStatus("ResourceManager test application started");
        updateStatus("Click buttons to run different tests");
    }
    
    void updateStatus(const QString& message) {
        QString currentText = statusLabel_->text();
        if (currentText == "Ready to test ResourceManager functionality") {
            currentText.clear();
        }
        
        if (!currentText.isEmpty()) {
            currentText += "\n";
        }
        currentText += message;
        
        statusLabel_->setText(currentText);
        qDebug() << "ResourceManagerTest:" << message;
    }
    
    QPixmap createTestSpriteSheet() {
        QPixmap sheet(64, 64);
        sheet.fill(Qt::transparent);
        
        QPainter painter(&sheet);
        painter.setRenderHint(QPainter::Antialiasing, false);
        
        // Draw a 4x4 grid of colored squares
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                QColor color = QColor::fromHsv((x + y * 4) * 24, 255, 255);
                painter.fillRect(x * 16, y * 16, 16, 16, color);
                painter.setPen(Qt::black);
                painter.drawRect(x * 16, y * 16, 15, 15);
            }
        }
        
        return sheet;
    }
    
    QLabel* statusLabel_;
    QLabel* imageLabel1_;
    QLabel* imageLabel2_;
    QLabel* imageLabel3_;
    QLabel* imageLabel4_;
    QLabel* imageLabel5_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    ResourceManagerTestWidget testWidget;
    testWidget.show();
    
    return app.exec();
}

#include "ResourceManagerTest.moc"
