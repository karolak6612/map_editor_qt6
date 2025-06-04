// NetworkTest.cpp - Test application for Task 99 Network Implementation

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QGroupBox>
#include <QSplitter>
#include <QMessageBox>
#include <QDebug>

#include "network/NetworkController.h"
#include "network/LivePackets.h"
#include "network/NetworkMessage.h"
#include "network/LiveCursor.h"

/**
 * @brief Test widget for network functionality
 * 
 * This test application demonstrates the basic network features implemented
 * for Task 99, including:
 * - Client/server connection management
 * - Chat messaging
 * - Cursor position updates
 * - Operation progress updates
 * - Basic packet handling
 */
class NetworkTestWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit NetworkTestWidget(QWidget* parent = nullptr) : QMainWindow(parent) {
        setupUI();
        setupNetworking();
        connectSignals();
    }

private slots:
    void onStartServer() {
        quint16 port = static_cast<quint16>(m_portSpinBox->value());
        QString password = m_passwordEdit->text();
        
        if (m_networkController->startServer(port, password)) {
            m_logEdit->append(QString("Server started on port %1").arg(port));
            m_startServerBtn->setEnabled(false);
            m_stopBtn->setEnabled(true);
            m_connectBtn->setEnabled(false);
        } else {
            QMessageBox::warning(this, "Error", m_networkController->getLastError());
        }
    }

    void onConnectToServer() {
        QString address = m_addressEdit->text();
        quint16 port = static_cast<quint16>(m_portSpinBox->value());
        QString username = m_usernameEdit->text();
        QString password = m_passwordEdit->text();
        
        if (m_networkController->connectToServer(address, port, username, password)) {
            m_logEdit->append(QString("Connecting to %1:%2 as %3").arg(address).arg(port).arg(username));
            m_connectBtn->setEnabled(false);
            m_stopBtn->setEnabled(true);
            m_startServerBtn->setEnabled(false);
        } else {
            QMessageBox::warning(this, "Error", m_networkController->getLastError());
        }
    }

    void onDisconnect() {
        m_networkController->disconnect();
        m_logEdit->append("Disconnected");
        m_startServerBtn->setEnabled(true);
        m_connectBtn->setEnabled(true);
        m_stopBtn->setEnabled(false);
    }

    void onSendChat() {
        QString message = m_chatEdit->text();
        if (!message.isEmpty()) {
            m_networkController->sendChatMessage(message);
            m_chatEdit->clear();
            m_logEdit->append(QString("Sent: %1").arg(message));
        }
    }

    void onSendCursorUpdate() {
        Position pos(
            static_cast<quint16>(m_cursorXSpinBox->value()),
            static_cast<quint16>(m_cursorYSpinBox->value()),
            static_cast<quint8>(m_cursorZSpinBox->value())
        );
        m_networkController->updateCursorPosition(pos);
        m_logEdit->append(QString("Sent cursor update: (%1, %2, %3)")
                         .arg(pos.x).arg(pos.y).arg(pos.z));
    }

    void onStartOperation() {
        QString operation = m_operationEdit->text();
        if (!operation.isEmpty()) {
            m_networkController->startOperation(operation);
            m_operationEdit->clear();
        }
    }

    void onUpdateProgress() {
        qint32 progress = m_progressSpinBox->value();
        m_networkController->updateOperationProgress(progress);
    }

    // Network event handlers
    void onConnected() {
        m_logEdit->append("Connected to server");
    }

    void onDisconnected() {
        m_logEdit->append("Disconnected from server");
        onDisconnect(); // Reset UI state
    }

    void onConnectionError(const QString& error) {
        m_logEdit->append(QString("Connection error: %1").arg(error));
        QMessageBox::critical(this, "Connection Error", error);
    }

    void onServerStarted() {
        m_logEdit->append("Server started successfully");
    }

    void onClientConnected(quint32 clientId) {
        m_logEdit->append(QString("Client %1 connected").arg(clientId));
    }

    void onClientDisconnected(quint32 clientId) {
        m_logEdit->append(QString("Client %1 disconnected").arg(clientId));
    }

    void onChatMessageReceived(const QString& sender, const QString& message) {
        m_logEdit->append(QString("[%1]: %2").arg(sender, message));
    }

    void onCursorUpdateReceived(const LiveCursor& cursor) {
        m_logEdit->append(QString("Cursor update from %1: (%2, %3, %4)")
                         .arg(cursor.id).arg(cursor.pos.x).arg(cursor.pos.y).arg(cursor.pos.z));
    }

    void onOperationStarted(const QString& operationName) {
        m_logEdit->append(QString("Operation started: %1").arg(operationName));
    }

    void onOperationProgressUpdated(qint32 percent) {
        m_logEdit->append(QString("Operation progress: %1%").arg(percent));
    }

private:
    void setupUI() {
        auto* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        auto* mainLayout = new QVBoxLayout(centralWidget);
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        mainLayout->addWidget(splitter);

        // Left panel - Controls
        auto* controlsWidget = new QWidget();
        auto* controlsLayout = new QVBoxLayout(controlsWidget);

        // Connection group
        auto* connectionGroup = new QGroupBox("Connection");
        auto* connectionLayout = new QVBoxLayout(connectionGroup);

        auto* addressLayout = new QHBoxLayout();
        addressLayout->addWidget(new QLabel("Address:"));
        m_addressEdit = new QLineEdit("127.0.0.1");
        addressLayout->addWidget(m_addressEdit);

        auto* portLayout = new QHBoxLayout();
        portLayout->addWidget(new QLabel("Port:"));
        m_portSpinBox = new QSpinBox();
        m_portSpinBox->setRange(1, 65535);
        m_portSpinBox->setValue(LiveNetworkConstants::DEFAULT_PORT);
        portLayout->addWidget(m_portSpinBox);

        auto* usernameLayout = new QHBoxLayout();
        usernameLayout->addWidget(new QLabel("Username:"));
        m_usernameEdit = new QLineEdit("TestUser");
        usernameLayout->addWidget(m_usernameEdit);

        auto* passwordLayout = new QHBoxLayout();
        passwordLayout->addWidget(new QLabel("Password:"));
        m_passwordEdit = new QLineEdit("test123");
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        passwordLayout->addWidget(m_passwordEdit);

        connectionLayout->addLayout(addressLayout);
        connectionLayout->addLayout(portLayout);
        connectionLayout->addLayout(usernameLayout);
        connectionLayout->addLayout(passwordLayout);

        m_startServerBtn = new QPushButton("Start Server");
        m_connectBtn = new QPushButton("Connect to Server");
        m_stopBtn = new QPushButton("Disconnect/Stop");
        m_stopBtn->setEnabled(false);

        connectionLayout->addWidget(m_startServerBtn);
        connectionLayout->addWidget(m_connectBtn);
        connectionLayout->addWidget(m_stopBtn);

        controlsLayout->addWidget(connectionGroup);

        // Chat group
        auto* chatGroup = new QGroupBox("Chat");
        auto* chatLayout = new QVBoxLayout(chatGroup);

        auto* chatInputLayout = new QHBoxLayout();
        m_chatEdit = new QLineEdit();
        m_chatEdit->setPlaceholderText("Enter chat message...");
        auto* sendChatBtn = new QPushButton("Send");
        chatInputLayout->addWidget(m_chatEdit);
        chatInputLayout->addWidget(sendChatBtn);

        chatLayout->addLayout(chatInputLayout);
        controlsLayout->addWidget(chatGroup);

        // Cursor group
        auto* cursorGroup = new QGroupBox("Cursor Position");
        auto* cursorLayout = new QVBoxLayout(cursorGroup);

        auto* cursorPosLayout = new QHBoxLayout();
        cursorPosLayout->addWidget(new QLabel("X:"));
        m_cursorXSpinBox = new QSpinBox();
        m_cursorXSpinBox->setRange(0, 65535);
        m_cursorXSpinBox->setValue(100);
        cursorPosLayout->addWidget(m_cursorXSpinBox);

        cursorPosLayout->addWidget(new QLabel("Y:"));
        m_cursorYSpinBox = new QSpinBox();
        m_cursorYSpinBox->setRange(0, 65535);
        m_cursorYSpinBox->setValue(100);
        cursorPosLayout->addWidget(m_cursorYSpinBox);

        cursorPosLayout->addWidget(new QLabel("Z:"));
        m_cursorZSpinBox = new QSpinBox();
        m_cursorZSpinBox->setRange(0, 15);
        m_cursorZSpinBox->setValue(7);
        cursorPosLayout->addWidget(m_cursorZSpinBox);

        auto* sendCursorBtn = new QPushButton("Send Cursor Update");

        cursorLayout->addLayout(cursorPosLayout);
        cursorLayout->addWidget(sendCursorBtn);
        controlsLayout->addWidget(cursorGroup);

        // Operations group (server only)
        auto* operationsGroup = new QGroupBox("Operations (Server)");
        auto* operationsLayout = new QVBoxLayout(operationsGroup);

        auto* operationLayout = new QHBoxLayout();
        m_operationEdit = new QLineEdit();
        m_operationEdit->setPlaceholderText("Operation name...");
        auto* startOpBtn = new QPushButton("Start Operation");
        operationLayout->addWidget(m_operationEdit);
        operationLayout->addWidget(startOpBtn);

        auto* progressLayout = new QHBoxLayout();
        progressLayout->addWidget(new QLabel("Progress:"));
        m_progressSpinBox = new QSpinBox();
        m_progressSpinBox->setRange(0, 100);
        m_progressSpinBox->setSuffix("%");
        auto* updateProgressBtn = new QPushButton("Update Progress");
        progressLayout->addWidget(m_progressSpinBox);
        progressLayout->addWidget(updateProgressBtn);

        operationsLayout->addLayout(operationLayout);
        operationsLayout->addLayout(progressLayout);
        controlsLayout->addWidget(operationsGroup);

        controlsLayout->addStretch();
        splitter->addWidget(controlsWidget);

        // Right panel - Log
        m_logEdit = new QTextEdit();
        m_logEdit->setReadOnly(true);
        m_logEdit->setMinimumWidth(400);
        splitter->addWidget(m_logEdit);

        splitter->setSizes({300, 400});

        // Connect UI signals
        connect(m_startServerBtn, &QPushButton::clicked, this, &NetworkTestWidget::onStartServer);
        connect(m_connectBtn, &QPushButton::clicked, this, &NetworkTestWidget::onConnectToServer);
        connect(m_stopBtn, &QPushButton::clicked, this, &NetworkTestWidget::onDisconnect);
        connect(sendChatBtn, &QPushButton::clicked, this, &NetworkTestWidget::onSendChat);
        connect(m_chatEdit, &QLineEdit::returnPressed, this, &NetworkTestWidget::onSendChat);
        connect(sendCursorBtn, &QPushButton::clicked, this, &NetworkTestWidget::onSendCursorUpdate);
        connect(startOpBtn, &QPushButton::clicked, this, &NetworkTestWidget::onStartOperation);
        connect(updateProgressBtn, &QPushButton::clicked, this, &NetworkTestWidget::onUpdateProgress);

        setWindowTitle("Network Test - Task 99");
        resize(800, 600);
    }

    void setupNetworking() {
        m_networkController = new NetworkController(this);
    }

    void connectSignals() {
        connect(m_networkController, &NetworkController::connected, this, &NetworkTestWidget::onConnected);
        connect(m_networkController, &NetworkController::disconnected, this, &NetworkTestWidget::onDisconnected);
        connect(m_networkController, &NetworkController::connectionError, this, &NetworkTestWidget::onConnectionError);
        connect(m_networkController, &NetworkController::serverStarted, this, &NetworkTestWidget::onServerStarted);
        connect(m_networkController, &NetworkController::clientConnected, this, &NetworkTestWidget::onClientConnected);
        connect(m_networkController, &NetworkController::clientDisconnected, this, &NetworkTestWidget::onClientDisconnected);
        connect(m_networkController, &NetworkController::chatMessageReceived, this, &NetworkTestWidget::onChatMessageReceived);
        connect(m_networkController, &NetworkController::cursorUpdateReceived, this, &NetworkTestWidget::onCursorUpdateReceived);
        connect(m_networkController, &NetworkController::operationStarted, this, &NetworkTestWidget::onOperationStarted);
        connect(m_networkController, &NetworkController::operationProgressUpdated, this, &NetworkTestWidget::onOperationProgressUpdated);
    }

    // UI components
    QLineEdit* m_addressEdit;
    QSpinBox* m_portSpinBox;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_startServerBtn;
    QPushButton* m_connectBtn;
    QPushButton* m_stopBtn;
    QLineEdit* m_chatEdit;
    QSpinBox* m_cursorXSpinBox;
    QSpinBox* m_cursorYSpinBox;
    QSpinBox* m_cursorZSpinBox;
    QLineEdit* m_operationEdit;
    QSpinBox* m_progressSpinBox;
    QTextEdit* m_logEdit;

    // Network controller
    NetworkController* m_networkController;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    NetworkTestWidget window;
    window.show();

    return app.exec();
}

#include "NetworkTest.moc"
