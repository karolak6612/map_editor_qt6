// BuildSystemTest.cpp - Test file to verify build system functionality

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QDebug>
#include <QMessageBox>

// Test core includes
#include "Position.h"
#include "SettingsManager.h"
#include "ResourceManager.h"

// Test network includes (Task 99)
#include "network/NetworkController.h"
#include "network/LivePackets.h"
#include "network/NetworkMessage.h"
#include "network/LiveCursor.h"

// Test UI includes
#include "ui/MainWindow.h"

/**
 * @brief Build system test widget
 * 
 * This test widget verifies that all major components can be included
 * and instantiated correctly, ensuring the build system is working.
 */
class BuildSystemTestWidget : public QWidget {
    Q_OBJECT

public:
    explicit BuildSystemTestWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        runTests();
    }

private slots:
    void runTests() {
        QString results;
        
        // Test Position class
        try {
            Position pos(100, 200, 7);
            Position pos2 = pos.moved(10, -5, 1);
            results += QString("✓ Position class: Created (%1) and moved to (%2)\n")
                      .arg(pos.toString(), pos2.toString());
        } catch (...) {
            results += "✗ Position class: Failed\n";
        }
        
        // Test NetworkMessage
        try {
            NetworkMessage msg;
            msg.writeU32(12345);
            msg.writeString("Test message");
            msg.prepareForSending();
            results += QString("✓ NetworkMessage: Created and wrote %1 bytes\n").arg(msg.size());
        } catch (...) {
            results += "✗ NetworkMessage: Failed\n";
        }
        
        // Test LiveCursor
        try {
            Position cursorPos(50, 75, 7);
            LiveCursor cursor(1, QColor(255, 0, 0), cursorPos);
            results += QString("✓ LiveCursor: Created cursor ID %1 at %2\n")
                      .arg(cursor.id).arg(cursor.pos.toString());
        } catch (...) {
            results += "✗ LiveCursor: Failed\n";
        }
        
        // Test NetworkController
        try {
            NetworkController* controller = new NetworkController(this);
            results += QString("✓ NetworkController: Created, state = %1\n")
                      .arg(static_cast<int>(controller->getConnectionState()));
        } catch (...) {
            results += "✗ NetworkController: Failed\n";
        }
        
        // Test SettingsManager
        try {
            SettingsManager& settings = SettingsManager::getInstance();
            results += "✓ SettingsManager: Singleton access successful\n";
        } catch (...) {
            results += "✗ SettingsManager: Failed\n";
        }
        
        // Test ResourceManager
        try {
            ResourceManager& resources = ResourceManager::getInstance();
            results += "✓ ResourceManager: Singleton access successful\n";
        } catch (...) {
            results += "✗ ResourceManager: Failed\n";
        }
        
        // Test packet types
        try {
            LivePacketType packetType = LivePacketType::PACKET_HELLO_FROM_CLIENT;
            quint8 packetValue = static_cast<quint8>(packetType);
            results += QString("✓ LivePackets: Packet type enum = %1\n").arg(packetValue);
        } catch (...) {
            results += "✗ LivePackets: Failed\n";
        }
        
        m_resultsEdit->setPlainText(results);
        
        // Count successes
        int successes = results.count("✓");
        int failures = results.count("✗");
        
        QString summary = QString("\n=== BUILD SYSTEM TEST SUMMARY ===\n"
                                 "Successes: %1\n"
                                 "Failures: %2\n"
                                 "Total Tests: %3\n")
                         .arg(successes).arg(failures).arg(successes + failures);
        
        m_resultsEdit->append(summary);
        
        if (failures == 0) {
            m_statusLabel->setText("✓ All tests passed! Build system is working correctly.");
            m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
        } else {
            m_statusLabel->setText(QString("✗ %1 test(s) failed. Check build configuration.").arg(failures));
            m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
        }
        
        qDebug() << "Build system test completed:" << successes << "successes," << failures << "failures";
    }

    void showMainWindow() {
        try {
            MainWindow* window = new MainWindow();
            window->show();
            m_resultsEdit->append("\n✓ MainWindow created and shown successfully");
        } catch (const std::exception& e) {
            m_resultsEdit->append(QString("\n✗ MainWindow failed: %1").arg(e.what()));
        } catch (...) {
            m_resultsEdit->append("\n✗ MainWindow failed: Unknown error");
        }
    }

private:
    void setupUI() {
        setWindowTitle("Build System Test - Task 100");
        setMinimumSize(600, 400);
        
        auto* layout = new QVBoxLayout(this);
        
        auto* titleLabel = new QLabel("Build System Verification Test");
        titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);
        
        auto* descLabel = new QLabel("This test verifies that all major components can be included and instantiated correctly.");
        descLabel->setWordWrap(true);
        descLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(descLabel);
        
        m_statusLabel = new QLabel("Running tests...");
        m_statusLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_statusLabel);
        
        m_resultsEdit = new QTextEdit();
        m_resultsEdit->setReadOnly(true);
        m_resultsEdit->setFont(QFont("Courier", 9));
        layout->addWidget(m_resultsEdit);
        
        auto* buttonLayout = new QHBoxLayout();
        
        auto* retestBtn = new QPushButton("Re-run Tests");
        connect(retestBtn, &QPushButton::clicked, this, &BuildSystemTestWidget::runTests);
        buttonLayout->addWidget(retestBtn);
        
        auto* mainWindowBtn = new QPushButton("Test MainWindow");
        connect(mainWindowBtn, &QPushButton::clicked, this, &BuildSystemTestWidget::showMainWindow);
        buttonLayout->addWidget(mainWindowBtn);
        
        auto* closeBtn = new QPushButton("Close");
        connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
        buttonLayout->addWidget(closeBtn);
        
        layout->addLayout(buttonLayout);
    }

    QLabel* m_statusLabel;
    QTextEdit* m_resultsEdit;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    BuildSystemTestWidget test;
    test.show();
    
    return app.exec();
}

#include "BuildSystemTest.moc"
