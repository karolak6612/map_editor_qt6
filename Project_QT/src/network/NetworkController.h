#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>
#include <QHash>
#include <QString>
#include <QHostAddress>
#include <memory>

#include "LivePackets.h"
#include "NetworkMessage.h"
#include "LiveCursor.h"

// Forward declarations
class LiveClient;
class LiveServer;
class LivePeer;

/**
 * @brief Main network controller for LiveSocket functionality
 * 
 * This class serves as the central coordinator for network operations in the Qt6
 * map editor. It manages both client and server modes, handles connection lifecycle,
 * and provides a unified interface for network messaging.
 * 
 * Key responsibilities:
 * - Connection management (client/server modes)
 * - Message routing and handling
 * - Error handling and recovery
 * - Network state management
 * - Integration with the main application
 */
class NetworkController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Construct a new NetworkController
     * @param parent Parent QObject
     */
    explicit NetworkController(QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~NetworkController() override;

    // Connection management
    /**
     * @brief Connect to a server as a client
     * @param address Server address
     * @param port Server port
     * @param username Client username
     * @param password Connection password
     * @return True if connection attempt started successfully
     */
    bool connectToServer(const QString& address, quint16 port, 
                        const QString& username, const QString& password);

    /**
     * @brief Start server mode
     * @param port Port to listen on
     * @param password Server password
     * @return True if server started successfully
     */
    bool startServer(quint16 port, const QString& password);

    /**
     * @brief Disconnect from server or stop server
     */
    void disconnect();

    /**
     * @brief Check if currently connected (client mode) or running (server mode)
     * @return True if active
     */
    bool isActive() const;

    /**
     * @brief Check if running in server mode
     * @return True if server mode
     */
    bool isServer() const;

    /**
     * @brief Check if running in client mode
     * @return True if client mode
     */
    bool isClient() const;

    /**
     * @brief Get current connection state
     * @return Current state
     */
    LiveConnectionState getConnectionState() const;

    /**
     * @brief Get last error message
     * @return Error message
     */
    QString getLastError() const;

    // Message sending (stubs for now)
    /**
     * @brief Send chat message
     * @param message Message text
     */
    void sendChatMessage(const QString& message);

    /**
     * @brief Update cursor position
     * @param position New cursor position
     */
    void updateCursorPosition(const Position& position);

    /**
     * @brief Send map changes to server (client mode) or broadcast to clients (server mode)
     * @param changes Serialized change data
     */
    void sendMapChanges(const QByteArray& changes);

    /**
     * @brief Request map nodes from server
     * @param nodeList List of node coordinates to request
     */
    void requestMapNodes(const QList<QPair<qint32, qint32>>& nodeList);

    // Server-specific operations
    /**
     * @brief Start a long operation (server mode)
     * @param operationName Operation description
     */
    void startOperation(const QString& operationName);

    /**
     * @brief Update operation progress (server mode)
     * @param percent Progress percentage (0-100)
     */
    void updateOperationProgress(qint32 percent);

    /**
     * @brief Broadcast cursor update to all clients (server mode)
     * @param cursor Cursor data to broadcast
     */
    void broadcastCursorUpdate(const LiveCursor& cursor);

    // Configuration
    /**
     * @brief Set connection timeout
     * @param milliseconds Timeout in milliseconds
     */
    void setConnectionTimeout(quint32 milliseconds);

    /**
     * @brief Set heartbeat interval
     * @param milliseconds Interval in milliseconds
     */
    void setHeartbeatInterval(quint32 milliseconds);

signals:
    // Connection events
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    void serverStarted();
    void serverStopped();
    void clientConnected(quint32 clientId);
    void clientDisconnected(quint32 clientId);

    // Message events
    void chatMessageReceived(const QString& sender, const QString& message);
    void cursorUpdateReceived(const LiveCursor& cursor);
    void mapChangesReceived(const QByteArray& changes);
    void operationStarted(const QString& operationName);
    void operationProgressUpdated(qint32 percent);

    // State events
    void connectionStateChanged(LiveConnectionState newState);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketReadyRead();
    void onNewConnection();
    void onHeartbeatTimer();

private:
    void setConnectionState(LiveConnectionState newState);
    void setLastError(const QString& error);
    void processIncomingData();
    void handlePacket(const NetworkMessage& message);
    
    // Packet handlers (stubs)
    void handleHelloFromServer(NetworkMessage& message);
    void handleChatMessage(NetworkMessage& message);
    void handleCursorUpdate(NetworkMessage& message);
    void handleMapNode(NetworkMessage& message);
    void handleOperationStart(NetworkMessage& message);
    void handleOperationUpdate(NetworkMessage& message);

    LiveConnectionState m_connectionState;
    QString m_lastError;
    
    // Network components
    std::unique_ptr<QTcpSocket> m_clientSocket;
    std::unique_ptr<QTcpServer> m_server;
    QHash<quint32, std::unique_ptr<QTcpSocket>> m_clientConnections;
    
    // Configuration
    quint32 m_connectionTimeout;
    quint32 m_heartbeatInterval;
    std::unique_ptr<QTimer> m_heartbeatTimer;
    
    // Connection info
    QString m_username;
    QString m_password;
    quint32 m_nextClientId;
    
    // Message handling
    QByteArray m_incomingBuffer;
    qint32 m_expectedMessageSize;
};

#endif // NETWORKCONTROLLER_H
