#include "QtBinaryFile.h"
#include <QDebug>

QtBinaryFile::QtBinaryFile(QObject* parent)
    : QObject(parent)
    , file_(nullptr)
    , stream_(nullptr)
{
}

QtBinaryFile::QtBinaryFile(const QString& fileName, QObject* parent)
    : QObject(parent)
    , fileName_(fileName)
    , file_(nullptr)
    , stream_(nullptr)
{
}

QtBinaryFile::~QtBinaryFile() {
    close();
}

bool QtBinaryFile::open(QIODevice::OpenMode mode) {
    if (isOpen()) {
        qWarning() << "QtBinaryFile::open - File is already open";
        return false;
    }

    if (fileName_.isEmpty()) {
        setError("No file name specified");
        return false;
    }

    file_ = new QFile(fileName_, this);
    if (!file_->open(mode)) {
        setError(QString("Failed to open file: %1 - %2").arg(fileName_).arg(file_->errorString()));
        delete file_;
        file_ = nullptr;
        return false;
    }

    stream_ = new QDataStream(file_);
    stream_->setByteOrder(QDataStream::LittleEndian);

    qDebug() << "QtBinaryFile::open - Opened file:" << fileName_ << "Mode:" << mode;
    return true;
}

void QtBinaryFile::close() {
    if (stream_) {
        delete stream_;
        stream_ = nullptr;
    }

    if (file_) {
        if (file_->isOpen()) {
            file_->close();
        }
        delete file_;
        file_ = nullptr;
    }

    clearError();
}

bool QtBinaryFile::isOpen() const {
    return file_ && file_->isOpen();
}

bool QtBinaryFile::isReadable() const {
    return isOpen() && file_->isReadable();
}

bool QtBinaryFile::isWritable() const {
    return isOpen() && file_->isWritable();
}

qint64 QtBinaryFile::size() const {
    return file_ ? file_->size() : 0;
}

qint64 QtBinaryFile::pos() const {
    return file_ ? file_->pos() : 0;
}

bool QtBinaryFile::seek(qint64 pos) {
    if (!file_) {
        setError("File not open");
        return false;
    }

    return file_->seek(pos);
}

bool QtBinaryFile::atEnd() const {
    return file_ ? file_->atEnd() : true;
}

QDataStream* QtBinaryFile::stream() {
    return stream_;
}

const QDataStream* QtBinaryFile::stream() const {
    return stream_;
}

QString QtBinaryFile::fileName() const {
    return fileName_;
}

void QtBinaryFile::setFileName(const QString& fileName) {
    if (isOpen()) {
        qWarning() << "QtBinaryFile::setFileName - Cannot change file name while file is open";
        return;
    }

    fileName_ = fileName;
}

// Read operations
quint8 QtBinaryFile::readUInt8() {
    quint8 value = 0;
    if (stream_) {
        *stream_ >> value;
        if (stream_->status() != QDataStream::Ok) {
            setError("Failed to read UInt8");
        }
    } else {
        setError("Stream not available");
    }
    return value;
}

quint16 QtBinaryFile::readUInt16() {
    quint16 value = 0;
    if (stream_) {
        *stream_ >> value;
        if (stream_->status() != QDataStream::Ok) {
            setError("Failed to read UInt16");
        }
    } else {
        setError("Stream not available");
    }
    return value;
}

quint32 QtBinaryFile::readUInt32() {
    quint32 value = 0;
    if (stream_) {
        *stream_ >> value;
        if (stream_->status() != QDataStream::Ok) {
            setError("Failed to read UInt32");
        }
    } else {
        setError("Stream not available");
    }
    return value;
}

qint8 QtBinaryFile::readInt8() {
    qint8 value = 0;
    if (stream_) {
        *stream_ >> value;
        if (stream_->status() != QDataStream::Ok) {
            setError("Failed to read Int8");
        }
    } else {
        setError("Stream not available");
    }
    return value;
}

qint16 QtBinaryFile::readInt16() {
    qint16 value = 0;
    if (stream_) {
        *stream_ >> value;
        if (stream_->status() != QDataStream::Ok) {
            setError("Failed to read Int16");
        }
    } else {
        setError("Stream not available");
    }
    return value;
}

qint32 QtBinaryFile::readInt32() {
    qint32 value = 0;
    if (stream_) {
        *stream_ >> value;
        if (stream_->status() != QDataStream::Ok) {
            setError("Failed to read Int32");
        }
    } else {
        setError("Stream not available");
    }
    return value;
}

QString QtBinaryFile::readString() {
    if (!stream_) {
        setError("Stream not available");
        return QString();
    }

    quint16 length = readUInt16();
    if (hasError()) {
        return QString();
    }

    if (length == 0) {
        return QString();
    }

    QByteArray data(length, 0);
    int bytesRead = stream_->readRawData(data.data(), length);
    if (bytesRead != length) {
        setError("Failed to read string data");
        return QString();
    }

    return QString::fromUtf8(data);
}

QByteArray QtBinaryFile::readBytes(int count) {
    if (!stream_) {
        setError("Stream not available");
        return QByteArray();
    }

    QByteArray data(count, 0);
    int bytesRead = stream_->readRawData(data.data(), count);
    if (bytesRead != count) {
        setError("Failed to read bytes");
        return QByteArray();
    }

    return data;
}

// Write operations
bool QtBinaryFile::writeUInt8(quint8 value) {
    if (!stream_) {
        setError("Stream not available");
        return false;
    }

    *stream_ << value;
    if (stream_->status() != QDataStream::Ok) {
        setError("Failed to write UInt8");
        return false;
    }
    return true;
}

bool QtBinaryFile::writeUInt16(quint16 value) {
    if (!stream_) {
        setError("Stream not available");
        return false;
    }

    *stream_ << value;
    if (stream_->status() != QDataStream::Ok) {
        setError("Failed to write UInt16");
        return false;
    }
    return true;
}

bool QtBinaryFile::writeUInt32(quint32 value) {
    if (!stream_) {
        setError("Stream not available");
        return false;
    }

    *stream_ << value;
    if (stream_->status() != QDataStream::Ok) {
        setError("Failed to write UInt32");
        return false;
    }
    return true;
}

bool QtBinaryFile::writeInt8(qint8 value) {
    if (!stream_) {
        setError("Stream not available");
        return false;
    }

    *stream_ << value;
    if (stream_->status() != QDataStream::Ok) {
        setError("Failed to write Int8");
        return false;
    }
    return true;
}

bool QtBinaryFile::writeInt16(qint16 value) {
    if (!stream_) {
        setError("Stream not available");
        return false;
    }

    *stream_ << value;
    if (stream_->status() != QDataStream::Ok) {
        setError("Failed to write Int16");
        return false;
    }
    return true;
}

bool QtBinaryFile::writeInt32(qint32 value) {
    if (!stream_) {
        setError("Stream not available");
        return false;
    }

    *stream_ << value;
    if (stream_->status() != QDataStream::Ok) {
        setError("Failed to write Int32");
        return false;
    }
    return true;
}

bool QtBinaryFile::writeString(const QString& value) {
    if (!stream_) {
        setError("Stream not available");
        return false;
    }

    QByteArray data = value.toUtf8();
    if (!writeUInt16(data.size())) {
        return false;
    }

    if (data.size() > 0) {
        int bytesWritten = stream_->writeRawData(data.constData(), data.size());
        if (bytesWritten != data.size()) {
            setError("Failed to write string data");
            return false;
        }
    }

    return true;
}

bool QtBinaryFile::writeBytes(const QByteArray& data) {
    if (!stream_) {
        setError("Stream not available");
        return false;
    }

    int bytesWritten = stream_->writeRawData(data.constData(), data.size());
    if (bytesWritten != data.size()) {
        setError("Failed to write bytes");
        return false;
    }

    return true;
}

// Error handling
bool QtBinaryFile::hasError() const {
    return !lastError_.isEmpty();
}

QString QtBinaryFile::getLastError() const {
    return lastError_;
}

void QtBinaryFile::clearError() {
    lastError_.clear();
}

void QtBinaryFile::setError(const QString& error) {
    lastError_ = error;
    qWarning() << "QtBinaryFile Error:" << error;
}

// QtNodeFileWriteHandle implementation
bool QtNodeFileWriteHandle::flush() {
    if (file_.isOpen()) {
        return file_.flush();
    }
    return false;
}
