#ifndef QTBINARYFILE_H
#define QTBINARYFILE_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QString>
#include <QStack>
#include <QHash>
#include <QMutex>
#include <QElapsedTimer>

/**
 * @brief Qt-based BinaryFile I/O replacement for Task 84
 * 
 * Complete replacement of wxwidgets BinaryFile system with Qt:
 * - QFile and QDataStream-based implementation
 * - 1:1 compatibility with wxwidgets FileHandle system
 * - Node-based binary file structure support
 * - Memory and disk-based file handling
 * - Performance optimization and error handling
 * - Thread-safe operations with proper locking
 */

/**
 * @brief File handle error enumeration (1:1 wxwidgets compatibility)
 */
enum class FileHandleError {
    NO_ERROR = 0,
    COULD_NOT_OPEN = 1,
    INVALID_IDENTIFIER = 2,
    STRING_TOO_LONG = 3,
    READ_ERROR = 4,
    WRITE_ERROR = 5,
    SYNTAX_ERROR = 6,
    PREMATURE_END = 7,
    INVALID_FORMAT = 8,
    UNSUPPORTED_VERSION = 9,
    MEMORY_ERROR = 10
};

/**
 * @brief Node type enumeration (1:1 wxwidgets compatibility)
 */
enum class NodeType : quint8 {
    NODE_START = 0xFE,
    NODE_END = 0xFF,
    ESCAPE_CHAR = 0xFD
};

/**
 * @brief Base Qt file handle class
 */
class QtFileHandle : public QObject
{
    Q_OBJECT

public:
    explicit QtFileHandle(QObject* parent = nullptr);
    virtual ~QtFileHandle();

    // File operations
    virtual bool open(const QString& fileName, QIODevice::OpenMode mode);
    virtual void close();
    virtual bool isOpen() const;
    virtual bool isOk() const;
    virtual qint64 size() const;
    virtual qint64 pos() const;
    virtual bool seek(qint64 pos);
    virtual bool atEnd() const;
    
    // Error handling
    FileHandleError getErrorCode() const { return errorCode_; }
    QString getErrorMessage() const;
    void clearError();
    
    // Status information
    QString getFileName() const { return fileName_; }
    QIODevice::OpenMode getOpenMode() const { return openMode_; }

signals:
    void errorOccurred(FileHandleError error, const QString& message);

protected:
    void setError(FileHandleError error, const QString& message = QString());
    
    QFile file_;
    QString fileName_;
    QIODevice::OpenMode openMode_;
    FileHandleError errorCode_;
    QString errorMessage_;
    mutable QMutex mutex_;
};

/**
 * @brief Qt file read handle (1:1 wxwidgets FileReadHandle compatibility)
 */
class QtFileReadHandle : public QtFileHandle
{
    Q_OBJECT

public:
    explicit QtFileReadHandle(QObject* parent = nullptr);
    explicit QtFileReadHandle(const QString& fileName, QObject* parent = nullptr);
    ~QtFileReadHandle() override;

    // Basic read operations (1:1 wxwidgets compatibility)
    bool getU8(quint8& value);
    bool getByte(quint8& value) { return getU8(value); }
    bool getSByte(qint8& value);
    bool getU16(quint16& value);
    bool getU32(quint32& value);
    bool getU64(quint64& value);
    bool get32(qint32& value);
    bool get64(qint64& value);
    
    // String operations
    bool getString(QString& str);
    bool getLongString(QString& str);
    bool getRAW(QByteArray& data, qint64 size);
    bool getRAW(QString& str, qint64 size);
    
    // Advanced operations
    bool skip(qint64 bytes);
    bool seekRelative(qint64 offset);
    QByteArray readAll();
    QByteArray read(qint64 maxSize);

private:
    template<typename T>
    bool getType(T& value);
    
    QDataStream stream_;
};

/**
 * @brief Qt file write handle (1:1 wxwidgets FileWriteHandle compatibility)
 */
class QtFileWriteHandle : public QtFileHandle
{
    Q_OBJECT

public:
    explicit QtFileWriteHandle(QObject* parent = nullptr);
    explicit QtFileWriteHandle(const QString& fileName, QObject* parent = nullptr);
    ~QtFileWriteHandle() override;

    // Basic write operations (1:1 wxwidgets compatibility)
    bool addU8(quint8 value);
    bool addByte(quint8 value) { return addU8(value); }
    bool addSByte(qint8 value);
    bool addU16(quint16 value);
    bool addU32(quint32 value);
    bool addU64(quint64 value);
    bool add32(qint32 value);
    bool add64(qint64 value);
    
    // String operations
    bool addString(const QString& str);
    bool addLongString(const QString& str);
    bool addRAW(const QByteArray& data);
    bool addRAW(const QString& str);
    bool addRAW(const char* data, qint64 size);
    
    // Advanced operations
    bool flush();
    void setBufferSize(qint64 size);

private:
    template<typename T>
    bool addType(const T& value);
    
    QDataStream stream_;
    qint64 bufferSize_;
};

/**
 * @brief Binary node for structured file reading (1:1 wxwidgets BinaryNode compatibility)
 */
class QtBinaryNode : public QObject
{
    Q_OBJECT

public:
    explicit QtBinaryNode(QtNodeFileReadHandle* fileHandle, QtBinaryNode* parent = nullptr);
    ~QtBinaryNode() override;

    // Basic read operations (1:1 wxwidgets compatibility)
    bool getU8(quint8& value);
    bool getByte(quint8& value) { return getU8(value); }
    bool getU16(quint16& value);
    bool getU32(quint32& value);
    bool getU64(quint64& value);
    bool skip(qint64 size);
    
    // String operations
    bool getString(QString& str);
    bool getLongString(QString& str);
    bool getRAW(QByteArray& data, qint64 size);
    bool getRAW(QString& str, qint64 size);
    
    // Node navigation
    QtBinaryNode* getChild();
    QtBinaryNode* advance();
    bool hasMoreData() const;
    qint64 getRemainingSize() const;
    
    // Node information
    quint8 getNodeType() const { return nodeType_; }
    QtBinaryNode* getParent() const { return parent_; }
    bool isValid() const { return isValid_; }

private:
    template<typename T>
    bool getType(T& value);
    
    void loadNodeData();
    bool validateNode();
    
    QtNodeFileReadHandle* fileHandle_;
    QtBinaryNode* parent_;
    QtBinaryNode* child_;
    QByteArray nodeData_;
    qint64 readOffset_;
    quint8 nodeType_;
    bool isValid_;
    bool dataLoaded_;
};

/**
 * @brief Node-based file read handle (1:1 wxwidgets NodeFileReadHandle compatibility)
 */
class QtNodeFileReadHandle : public QtFileHandle
{
    Q_OBJECT

public:
    explicit QtNodeFileReadHandle(QObject* parent = nullptr);
    explicit QtNodeFileReadHandle(const QString& fileName, const QStringList& acceptableIdentifiers, QObject* parent = nullptr);
    ~QtNodeFileReadHandle() override;

    // Node operations
    QtBinaryNode* getRootNode();
    void freeNode(QtBinaryNode* node);
    
    // File operations
    bool open(const QString& fileName, const QStringList& acceptableIdentifiers);
    void close() override;
    
    // Cache management
    bool renewCache();
    void clearCache();
    qint64 getCacheSize() const { return cacheSize_; }
    void setCacheSize(qint64 size);

private:
    friend class QtBinaryNode;
    
    QtBinaryNode* createNode(QtBinaryNode* parent);
    bool readNodeHeader(quint8& nodeType, qint32& nodeSize);
    bool validateIdentifier(const QStringList& acceptableIdentifiers);
    
    QtBinaryNode* rootNode_;
    QByteArray cache_;
    qint64 cacheSize_;
    qint64 cacheLength_;
    qint64 localReadIndex_;
    bool lastWasStart_;
    QStack<QtBinaryNode*> nodeStack_;
    QStringList acceptableIdentifiers_;
    
    static const qint64 DEFAULT_CACHE_SIZE;
};

/**
 * @brief Node-based file write handle (1:1 wxwidgets NodeFileWriteHandle compatibility)
 */
class QtNodeFileWriteHandle : public QtFileHandle
{
    Q_OBJECT

public:
    explicit QtNodeFileWriteHandle(QObject* parent = nullptr);
    explicit QtNodeFileWriteHandle(const QString& fileName, const QString& identifier, QObject* parent = nullptr);
    ~QtNodeFileWriteHandle() override;

    // Node operations
    bool addNode(quint8 nodeType);
    bool endNode();
    
    // Basic write operations
    bool addU8(quint8 value);
    bool addByte(quint8 value) { return addU8(value); }
    bool addU16(quint16 value);
    bool addU32(quint32 value);
    bool addU64(quint64 value);
    
    // String operations
    bool addString(const QString& str);
    bool addLongString(const QString& str);
    bool addRAW(const QByteArray& data);
    bool addRAW(const QString& str);
    
    // File operations
    bool open(const QString& fileName, const QString& identifier);
    void close() override;
    bool flush();

private:
    void renewCache();
    void writeBytes(const QByteArray& data);
    void writeEscapedByte(quint8 byte);
    bool writeNodeHeader(quint8 nodeType, qint32 nodeSize);
    
    QByteArray cache_;
    qint64 cacheSize_;
    qint64 localWriteIndex_;
    QStack<qint64> nodeSizeStack_;
    QString identifier_;
    
    static const qint64 DEFAULT_CACHE_SIZE;
};

/**
 * @brief Memory-based node file read handle for in-memory operations
 */
class QtMemoryNodeFileReadHandle : public QtNodeFileReadHandle
{
    Q_OBJECT

public:
    explicit QtMemoryNodeFileReadHandle(QObject* parent = nullptr);
    explicit QtMemoryNodeFileReadHandle(const QByteArray& data, QObject* parent = nullptr);
    ~QtMemoryNodeFileReadHandle() override;

    // Memory operations
    void assign(const QByteArray& data);
    QByteArray getData() const { return memoryData_; }
    
    // File operations
    bool open(const QByteArray& data, const QStringList& acceptableIdentifiers);
    void close() override;
    qint64 size() const override { return memoryData_.size(); }
    qint64 pos() const override { return memoryIndex_; }
    bool seek(qint64 pos) override;
    bool atEnd() const override { return memoryIndex_ >= memoryData_.size(); }

private:
    bool renewCache() override;
    
    QByteArray memoryData_;
    qint64 memoryIndex_;
};

/**
 * @brief Memory-based node file write handle for in-memory operations
 */
class QtMemoryNodeFileWriteHandle : public QtNodeFileWriteHandle
{
    Q_OBJECT

public:
    explicit QtMemoryNodeFileWriteHandle(QObject* parent = nullptr);
    ~QtMemoryNodeFileWriteHandle() override;

    // Memory operations
    QByteArray getData() const { return memoryData_; }
    void reset();
    qint64 getDataSize() const { return memoryData_.size(); }
    
    // File operations
    void close() override;
    bool flush() override;

private:
    void renewCache() override;
    
    QByteArray memoryData_;
};

/**
 * @brief Utility class for binary file operations
 */
class QtBinaryFileUtils : public QObject
{
    Q_OBJECT

public:
    explicit QtBinaryFileUtils(QObject* parent = nullptr);

    // Format detection
    static bool isOTBMFile(const QString& fileName);
    static bool isOTMMFile(const QString& fileName);
    static QString detectFileFormat(const QString& fileName);
    static QByteArray readFileHeader(const QString& fileName, qint64 headerSize = 1024);
    
    // File validation
    static bool validateOTBMFile(const QString& fileName);
    static bool validateOTMMFile(const QString& fileName);
    static bool validateFileStructure(const QString& fileName);
    
    // Conversion utilities
    static QByteArray convertToLittleEndian(const QByteArray& data);
    static QByteArray convertToBigEndian(const QByteArray& data);
    static QString bytesToHexString(const QByteArray& data);
    static QByteArray hexStringToBytes(const QString& hexString);
    
    // Performance utilities
    static qint64 getOptimalBufferSize(qint64 fileSize);
    static void optimizeFileAccess(QFile& file);
    static QByteArray compressData(const QByteArray& data);
    static QByteArray decompressData(const QByteArray& compressedData);

private:
    static const QStringList OTBM_SIGNATURES;
    static const QStringList OTMM_SIGNATURES;
    static const qint64 MIN_BUFFER_SIZE;
    static const qint64 MAX_BUFFER_SIZE;
    static const qint64 DEFAULT_HEADER_SIZE;
};

#endif // QTBINARYFILE_H
