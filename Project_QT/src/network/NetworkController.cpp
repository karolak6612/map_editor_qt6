#include "NetworkController.h"
#include "../Position.h"
#include <QDebug>
#include <QHostAddress>

NetworkController::NetworkController(QObject* parent)
    : QObject(parent)
    , m_connectionState(LiveConnectionState::DISCONNECTED)
    , m_connectionTimeout(LiveNetworkConstants::CONNECTION_TIMEOUT)
    , m_heartbeatInterval(LiveNetworkConstants::HEARTBEAT_INTERVAL)
    , m_nextClientId(1)
    , m_expectedMessageSize(-1)
{
    m_heartbeatTimer = std::make_unique<QTimer>(this);
    connect(m_heartbeatTimer.get(), &QTimer::timeout, this, &NetworkController::onHeartbeatTimer);
}

NetworkController::~NetworkController() {
    disconnect();
}

bool NetworkController::connectToServer(const QString& address, quint16 port, 
                                       const QString& username, const QString& password) {
    if (m_connectionState != LiveConnectionState::DISCONNECTED) {
        setLastError("Already connected or connecting");
        return false;
    }

    m_username = username;
    m_password = password;

    m_clientSocket = std::make_unique<QTcpSocket>(this);
    connect(m_clientSocket.get(), &QTcpSocket::connected, this, &NetworkController::onSocketConnected);
    connect(m_clientSocket.get(), &QTcpSocket::disconnected, this, &NetworkController::onSocketDisconnected);
    connect(m_clientSocket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &NetworkController::onSocketError);
    connect(m_clientSocket.get(), &QTcpSocket::readyRead, this, &NetworkController::onSocketReadyRead);

    setConnectionState(LiveConnectionState::CONNECTING);
    m_clientSocket->connectToHost(address, port);

    qDebug() << "Attempting to connect to" << address << ":" << port;
    return true;
}

bool NetworkController::startServer(quint16 port, const QString& password) {
    if (m_connectionState != LiveConnectionState::DISCONNECTED) {
        setLastError("Already connected or server running");
        return false;
    }

    m_password = password;
    m_server = std::make_unique<QTcpServer>(this);
    connect(m_server.get(), &QTcpServer::newConnection, this, &NetworkController::onNewConnection);

    if (!m_server->listen(QHostAddress::Any, port)) {
        setLastError(QString("Failed to start server: %1").arg(m_server->errorString()));
        m_server.reset();
        return false;
    }

    setConnectionState(LiveConnectionState::CONNECTED);
    emit serverStarted();
    
    qDebug() << "Server started on port" << port;
    return true;
}

void NetworkController::disconnect() {
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket.reset();
    }

    if (m_server) {
        m_server->close();
        m_server.reset();
        emit serverStopped();
    }

    m_clientConnections.clear();
    m_heartbeatTimer->stop();
    setConnectionState(LiveConnectionState::DISCONNECTED);
}

bool NetworkController::isActive() const {
    return m_connectionState == LiveConnectionState::CONNECTED;
}

bool NetworkController::isServer() const {
    return m_server != nullptr && m_server->isListening();
}

bool NetworkController::isClient() const {
    return m_clientSocket != nullptr && m_clientSocket->state() == QAbstractSocket::ConnectedState;
}

LiveConnectionState NetworkController::getConnectionState() const {
    return m_connectionState;
}

QString NetworkController::getLastError() const {
    return m_lastError;
}

void NetworkController::sendChatMessage(const QString& message) {
    if (!isActive()) {
        qWarning() << "Cannot send chat message: not connected";
        return;
    }

    NetworkMessage msg;
    msg.writeU8(static_cast<quint8>(LivePacketType::PACKET_CLIENT_TALK));
    msg.writeString(message);
    msg.prepareForSending();

    if (isClient() && m_clientSocket) {
        m_clientSocket->write(msg.buffer());
        qDebug() << "Sent chat message:" << message;
    } else if (isServer()) {
        // Broadcast to all clients
        for (auto& client : m_clientConnections) {
            client->write(msg.buffer());
        }
        qDebug() << "Broadcasted chat message:" << message;
    }
}

void NetworkController::updateCursorPosition(const Position& position) {
    if (!isActive()) {
        return;
    }

    NetworkMessage msg;
    msg.writeU8(static_cast<quint8>(LivePacketType::PACKET_CLIENT_UPDATE_CURSOR));
    msg.writePosition(position);
    msg.prepareForSending();

    if (isClient() && m_clientSocket) {
        m_clientSocket->write(msg.buffer());
    }
}

void NetworkController::sendMapChanges(const QByteArray& changes) {
    if (!isActive()) {
        qWarning() << "Cannot send map changes: not connected";
        return;
    }

    NetworkMessage msg;
    msg.writeU8(static_cast<quint8>(LivePacketType::PACKET_CHANGE_LIST));
    // TODO: Implement proper change serialization
    msg.writeString(QString::fromUtf8(changes));
    msg.prepareForSending();

    if (isClient() && m_clientSocket) {
        m_clientSocket->write(msg.buffer());
        qDebug() << "Sent map changes, size:" << changes.size();
    }
}

void NetworkController::requestMapNodes(const QList<QPair<qint32, qint32>>& nodeList) {
    if (!isClient() || !m_clientSocket) {
        qWarning() << "Cannot request nodes: not connected as client";
        return;
    }

    NetworkMessage msg;
    msg.writeU8(static_cast<quint8>(LivePacketType::PACKET_REQUEST_NODES));
    msg.writeU32(static_cast<quint32>(nodeList.size()));
    
    for (const auto& node : nodeList) {
        msg.writeI32(node.first);
        msg.writeI32(node.second);
    }
    
    msg.prepareForSending();
    m_clientSocket->write(msg.buffer());
    qDebug() << "Requested" << nodeList.size() << "map nodes";
}

void NetworkController::startOperation(const QString& operationName) {
    if (!isServer()) {
        return;
    }

    NetworkMessage msg;
    msg.writeU8(static_cast<quint8>(LivePacketType::PACKET_START_OPERATION));
    msg.writeString(operationName);
    msg.prepareForSending();

    for (auto& client : m_clientConnections) {
        client->write(msg.buffer());
    }
    
    emit operationStarted(operationName);
    qDebug() << "Started operation:" << operationName;
}

void NetworkController::updateOperationProgress(qint32 percent) {
    if (!isServer()) {
        return;
    }

    NetworkMessage msg;
    msg.writeU8(static_cast<quint8>(LivePacketType::PACKET_UPDATE_OPERATION));
    msg.writeI32(percent);
    msg.prepareForSending();

    for (auto& client : m_clientConnections) {
        client->write(msg.buffer());
    }
    
    emit operationProgressUpdated(percent);
}

void NetworkController::broadcastCursorUpdate(const LiveCursor& cursor) {
    if (!isServer()) {
        return;
    }

    NetworkMessage msg;
    msg.writeU8(static_cast<quint8>(LivePacketType::PACKET_CURSOR_UPDATE));
    msg.writeU32(cursor.id);
    msg.writeColor(cursor.color);
    msg.writePosition(cursor.pos);
    msg.prepareForSending();

    for (auto& client : m_clientConnections) {
        client->write(msg.buffer());
    }
    
    emit cursorUpdateReceived(cursor);
}

void NetworkController::setConnectionTimeout(quint32 milliseconds) {
    m_connectionTimeout = milliseconds;
}

void NetworkController::setHeartbeatInterval(quint32 milliseconds) {
    m_heartbeatInterval = milliseconds;
    if (m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->setInterval(static_cast<int>(milliseconds));
    }
}

void NetworkController::setConnectionState(LiveConnectionState newState) {
    if (m_connectionState != newState) {
        m_connectionState = newState;
        emit connectionStateChanged(newState);
    }
}

void NetworkController::setLastError(const QString& error) {
    m_lastError = error;
    qWarning() << "Network error:" << error;
}

void NetworkController::onSocketConnected() {
    setConnectionState(LiveConnectionState::AUTHENTICATING);
    m_heartbeatTimer->start(static_cast<int>(m_heartbeatInterval));

    // Send hello packet
    NetworkMessage msg;
    msg.writeU8(static_cast<quint8>(LivePacketType::PACKET_HELLO_FROM_CLIENT));
    msg.writeString(m_username);
    msg.writeString(m_password);
    msg.writeU32(LiveNetworkConstants::PROTOCOL_VERSION);
    msg.prepareForSending();

    m_clientSocket->write(msg.buffer());
    emit connected();
    qDebug() << "Connected to server, sent hello packet";
}

void NetworkController::onSocketDisconnected() {
    m_heartbeatTimer->stop();
    setConnectionState(LiveConnectionState::DISCONNECTED);
    emit disconnected();
    qDebug() << "Disconnected from server";
}

void NetworkController::onSocketError(QAbstractSocket::SocketError error) {
    QString errorString;
    switch (error) {
        case QAbstractSocket::ConnectionRefusedError:
            errorString = "Connection refused";
            break;
        case QAbstractSocket::RemoteHostClosedError:
            errorString = "Remote host closed connection";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorString = "Host not found";
            break;
        case QAbstractSocket::SocketTimeoutError:
            errorString = "Connection timeout";
            break;
        default:
            errorString = m_clientSocket ? m_clientSocket->errorString() : "Unknown error";
            break;
    }

    setLastError(errorString);
    setConnectionState(LiveConnectionState::ERROR_STATE);
    emit connectionError(errorString);
}

void NetworkController::onSocketReadyRead() {
    if (!m_clientSocket) {
        return;
    }

    m_incomingBuffer.append(m_clientSocket->readAll());
    processIncomingData();
}

void NetworkController::onNewConnection() {
    if (!m_server) {
        return;
    }

    while (m_server->hasPendingConnections()) {
        QTcpSocket* clientSocket = m_server->nextPendingConnection();
        quint32 clientId = m_nextClientId++;

        m_clientConnections[clientId] = std::unique_ptr<QTcpSocket>(clientSocket);

        connect(clientSocket, &QTcpSocket::disconnected, [this, clientId]() {
            m_clientConnections.remove(clientId);
            emit clientDisconnected(clientId);
            qDebug() << "Client" << clientId << "disconnected";
        });

        connect(clientSocket, &QTcpSocket::readyRead, [this, clientSocket]() {
            // TODO: Handle client data
            QByteArray data = clientSocket->readAll();
            qDebug() << "Received data from client, size:" << data.size();
        });

        emit clientConnected(clientId);
        qDebug() << "New client connected, ID:" << clientId;
    }
}

void NetworkController::onHeartbeatTimer() {
    // TODO: Implement heartbeat/ping mechanism
    qDebug() << "Heartbeat timer triggered";
}

void NetworkController::processIncomingData() {
    while (m_incomingBuffer.size() >= static_cast<int>(LiveNetworkConstants::HEADER_SIZE)) {
        if (m_expectedMessageSize == -1) {
            // Read message size from header
            QDataStream stream(m_incomingBuffer);
            stream.setByteOrder(QDataStream::LittleEndian);
            quint32 messageSize;
            stream >> messageSize;

            if (messageSize > LiveNetworkConstants::MAX_PACKET_SIZE) {
                setLastError("Received packet too large");
                disconnect();
                return;
            }

            m_expectedMessageSize = static_cast<qint32>(messageSize);
        }

        qint32 totalSize = m_expectedMessageSize + LiveNetworkConstants::HEADER_SIZE;
        if (m_incomingBuffer.size() >= totalSize) {
            // We have a complete message
            QByteArray messageData = m_incomingBuffer.left(totalSize);
            m_incomingBuffer.remove(0, totalSize);

            NetworkMessage message;
            message.setBuffer(messageData);
            handlePacket(message);

            m_expectedMessageSize = -1;
        } else {
            // Wait for more data
            break;
        }
    }
}

void NetworkController::handlePacket(const NetworkMessage& message) {
    NetworkMessage msg = message; // Make a copy for processing
    msg.setPosition(LiveNetworkConstants::HEADER_SIZE);

    if (!msg.hasMoreData()) {
        qWarning() << "Received empty packet";
        return;
    }

    quint8 packetType = msg.readU8();
    LivePacketType type = static_cast<LivePacketType>(packetType);

    switch (type) {
        case LivePacketType::PACKET_HELLO_FROM_SERVER:
            handleHelloFromServer(msg);
            break;
        case LivePacketType::PACKET_CHAT_MESSAGE:
            handleChatMessage(msg);
            break;
        case LivePacketType::PACKET_CURSOR_UPDATE:
            handleCursorUpdate(msg);
            break;
        case LivePacketType::PACKET_NODE:
            handleMapNode(msg);
            break;
        case LivePacketType::PACKET_START_OPERATION:
            handleOperationStart(msg);
            break;
        case LivePacketType::PACKET_UPDATE_OPERATION:
            handleOperationUpdate(msg);
            break;
        default:
            qWarning() << "Received unknown packet type:" << packetType;
            break;
    }
}

void NetworkController::handleHelloFromServer(NetworkMessage& message) {
    try {
        quint32 protocolVersion = message.readU32();
        QString serverMessage = message.readString();

        if (protocolVersion != LiveNetworkConstants::PROTOCOL_VERSION) {
            setLastError(QString("Protocol version mismatch. Server: %1, Client: %2")
                        .arg(protocolVersion).arg(LiveNetworkConstants::PROTOCOL_VERSION));
            disconnect();
            return;
        }

        setConnectionState(LiveConnectionState::CONNECTED);
        qDebug() << "Server hello received:" << serverMessage;

        // Send ready packet
        NetworkMessage readyMsg;
        readyMsg.writeU8(static_cast<quint8>(LivePacketType::PACKET_READY_CLIENT));
        readyMsg.prepareForSending();

        if (m_clientSocket) {
            m_clientSocket->write(readyMsg.buffer());
        }

    } catch (const std::exception& e) {
        setLastError(QString("Error processing hello packet: %1").arg(e.what()));
    }
}

void NetworkController::handleChatMessage(NetworkMessage& message) {
    try {
        QString sender = message.readString();
        QString chatMessage = message.readString();

        emit chatMessageReceived(sender, chatMessage);
        qDebug() << "Chat message from" << sender << ":" << chatMessage;

    } catch (const std::exception& e) {
        qWarning() << "Error processing chat message:" << e.what();
    }
}

void NetworkController::handleCursorUpdate(NetworkMessage& message) {
    try {
        LiveCursor cursor;
        cursor.id = message.readU32();
        cursor.color = message.readColor();
        cursor.pos = message.readPosition();

        emit cursorUpdateReceived(cursor);

    } catch (const std::exception& e) {
        qWarning() << "Error processing cursor update:" << e.what();
    }
}

void NetworkController::handleMapNode(NetworkMessage& message) {
    try {
        // TODO: Implement proper map node handling
        // For now, just emit the raw data
        QByteArray nodeData = message.buffer().mid(message.position());
        emit mapChangesReceived(nodeData);
        qDebug() << "Received map node data, size:" << nodeData.size();

    } catch (const std::exception& e) {
        qWarning() << "Error processing map node:" << e.what();
    }
}

void NetworkController::handleOperationStart(NetworkMessage& message) {
    try {
        QString operationName = message.readString();
        emit operationStarted(operationName);
        qDebug() << "Operation started:" << operationName;

    } catch (const std::exception& e) {
        qWarning() << "Error processing operation start:" << e.what();
    }
}

void NetworkController::handleOperationUpdate(NetworkMessage& message) {
    try {
        qint32 percent = message.readI32();
        emit operationProgressUpdated(percent);

    } catch (const std::exception& e) {
        qWarning() << "Error processing operation update:" << e.what();
    }
}
