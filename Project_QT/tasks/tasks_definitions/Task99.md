**Task99: Migrate any features that network messaging in `LiveSocket` requires in Qt (Data Structures, Packets, Basic Send/Receive Stubs)**
- Task: **If `LiveSocket` in `wxwidgets` was used for network messaging (e.g., for multi-user live editing, server synchronization, chat, or telemetry as hinted by `StartPasting`, `updateProgress`), begin migrating its core features to `project_qt` using Qt's network module (`QTcpSocket`, `QUdpSocket`, or `QNetworkAccessManager` if HTTP-based). This initial task focuses on data structures, packet definitions, and basic send/receive stubs.**
    - **Analyze `LiveSocket` Functionality:** Understand the purpose of `LiveSocket` from `wxwidgets` source. What kind of messages were sent/received? What protocols were used (TCP/UDP, custom binary/text)?
    - **Qt Network Class Selection:** Choose the appropriate Qt network class:
        -   `QTcpSocket` for reliable, connection-oriented communication.
        -   `QUdpSocket` for datagrams.
        -   `QNetworkAccessManager` for HTTP-based communication.
    - **Data Structures & Packet Definitions:**
        -   Migrate or redefine C++ structs/classes that represent the messages (packets) sent over `LiveSocket`. Use Qt data types (`QString`, `QByteArray`, `QVariantMap`, etc.).
        -   If packets had headers (ID, length), define these.
    - **Serialization/Deserialization:** Implement logic to serialize these message objects into a `QByteArray` for sending, and deserialize incoming `QByteArray`s back into message objects. `QDataStream` is often used for binary packet formats. JSON might be used for text-based.
    - **Basic Send/Receive Stubs:**
        -   Create a new Qt network manager class (e.g., `NetworkController : public QObject`).
        -   Implement basic connection logic (`connectToServer(host, port)`).
        -   Implement placeholder `sendXxxMessage(const XxxPacket& packet)` methods that serialize the packet and attempt to write it to the socket (e.g., `m_socket->write(byteArray)`).
        -   Implement placeholder slots connected to socket signals like `readyRead()` which would attempt to read data, buffer it, and try to deserialize packets.
    - **Logging and UI Feedback:**
        -   Log network activity (connection attempts, messages sent/received - stubs for actual content) to a log window (if one exists).
        -   If operations like `updateProgress` or `StartPasting` were triggered by network messages, implement placeholder handlers in `NetworkController` that would eventually signal `MainWindow` or other components to display progress or start conceptual pasting.
    - **Scope:** This task is foundational. It sets up the data structures and basic communication stubs. Full implementation of robust error handling, reconnection logic, complex message processing, and UI integration for network features is deferred unless `Task99.md` details simple, specific features.
    - **`Task99.md` must specify the protocol used by `LiveSocket`, the structure of all messages/packets, their purpose (e.g., what `update of operation in server/main window/logs` entailed), how messages were serialized, and the core logic of `sendXXX` methods from `wxwidgets`.**
