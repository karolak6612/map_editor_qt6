#ifndef LIVEPACKETS_H
#define LIVEPACKETS_H

#include <QtGlobal>

/**
 * @brief Live packet type definitions for Qt6 network messaging
 * 
 * This file defines all packet types used in the LiveSocket network communication
 * system, providing 1:1 compatibility with the original wxwidgets implementation.
 * 
 * The packet system supports multi-user live editing with the following features:
 * - Client-server authentication and handshaking
 * - Real-time map data synchronization
 * - Cursor position updates for collaborative editing
 * - Chat messaging between connected users
 * - Operation progress updates (e.g., for large map operations)
 * - House and spawn management commands
 */

/**
 * @brief Packet types for client-to-server communication
 */
enum class LivePacketType : quint8 {
    // Client authentication and setup packets (0x10-0x1F)
    PACKET_HELLO_FROM_CLIENT = 0x10,    ///< Initial client hello with credentials
    PACKET_READY_CLIENT = 0x11,         ///< Client ready to receive data

    // Map data request and modification packets (0x20-0x2F)
    PACKET_REQUEST_NODES = 0x20,        ///< Request map nodes from server
    PACKET_CHANGE_LIST = 0x21,          ///< Send map changes to server
    PACKET_ADD_HOUSE = 0x23,            ///< Add new house to map
    PACKET_EDIT_HOUSE = 0x24,           ///< Modify existing house
    PACKET_REMOVE_HOUSE = 0x25,         ///< Remove house from map

    // Client interaction packets (0x30-0x3F)
    PACKET_CLIENT_TALK = 0x30,          ///< Chat message from client
    PACKET_CLIENT_UPDATE_CURSOR = 0x31, ///< Client cursor position update
    PACKET_CLIENT_COLOR_UPDATE = 0x32,  ///< Client cursor color change

    // Server response and broadcast packets (0x80-0x9F)
    PACKET_HELLO_FROM_SERVER = 0x80,    ///< Server hello response
    PACKET_KICK = 0x81,                 ///< Server kicks client
    PACKET_ACCEPTED_CLIENT = 0x82,      ///< Client accepted by server
    PACKET_CHANGE_CLIENT_VERSION = 0x83, ///< Server requests client version change
    PACKET_SERVER_TALK = 0x84,          ///< Server chat message
    PACKET_COLOR_UPDATE = 0x85,         ///< Server broadcasts color update

    // Server data broadcast packets (0x90-0x9F)
    PACKET_NODE = 0x90,                 ///< Map node data from server
    PACKET_CURSOR_UPDATE = 0x91,        ///< Cursor position broadcast
    PACKET_START_OPERATION = 0x92,      ///< Operation start notification
    PACKET_UPDATE_OPERATION = 0x93,     ///< Operation progress update
    PACKET_CHAT_MESSAGE = 0x94          ///< Chat message broadcast
};

/**
 * @brief Network protocol constants
 */
namespace LiveNetworkConstants {
    constexpr quint16 DEFAULT_PORT = 7171;          ///< Default server port
    constexpr quint32 PROTOCOL_VERSION = 1;        ///< Current protocol version
    constexpr quint32 MAX_PACKET_SIZE = 65536;     ///< Maximum packet size in bytes
    constexpr quint32 HEADER_SIZE = 4;             ///< Packet header size (length field)
    constexpr quint32 MAX_MESSAGE_LENGTH = 1024;   ///< Maximum chat message length
    constexpr quint32 MAX_USERNAME_LENGTH = 32;    ///< Maximum username length
    constexpr quint32 MAX_PASSWORD_LENGTH = 64;    ///< Maximum password length
    constexpr quint32 CONNECTION_TIMEOUT = 30000;  ///< Connection timeout in milliseconds
    constexpr quint32 HEARTBEAT_INTERVAL = 10000;  ///< Heartbeat interval in milliseconds
}

/**
 * @brief Network error codes
 */
enum class LiveNetworkError : quint8 {
    NO_ERROR = 0,
    INVALID_CREDENTIALS = 1,
    SERVER_FULL = 2,
    PROTOCOL_MISMATCH = 3,
    CONNECTION_LOST = 4,
    TIMEOUT = 5,
    INVALID_PACKET = 6,
    PERMISSION_DENIED = 7,
    MAP_LOCKED = 8,
    UNKNOWN_ERROR = 255
};

/**
 * @brief Connection states for network components
 */
enum class LiveConnectionState : quint8 {
    DISCONNECTED = 0,
    CONNECTING = 1,
    AUTHENTICATING = 2,
    CONNECTED = 3,
    DISCONNECTING = 4,
    ERROR_STATE = 5
};

/**
 * @brief Client types for permission management
 */
enum class LiveClientType : quint8 {
    VIEWER = 0,     ///< Read-only access
    EDITOR = 1,     ///< Can edit map
    ADMIN = 2       ///< Full administrative access
};

#endif // LIVEPACKETS_H
