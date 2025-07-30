#pragma once
#include "iostream.hpp"
#include "string.hpp"
#include <filesystem>
#include <fstream>

LCORE_NAMESPACE_BEGIN

class String;

class FileException : public StreamException {
public:
    int errno_;
    FileException(int errno_): errno_(errno_) {}
    inline const char* what() const noexcept override { return "File operation failed"; }
};
class FileOpenException : public FileException {
public:
    String filePath;
    IOSBase::OpenMode mode;
    FileOpenException(StringView path, IOSBase::OpenMode openMode, int errno_): FileException(errno_), filePath(path), mode(openMode) {}
    inline const char* what() const noexcept override { return "Failed to open file"; }
};

class FileStreamHandler {
    std::FILE *filePtr;
public:
    IOSBase::IOMode lastOperation = IOSBase::IOMode::In;
    FileStreamHandler(const char* path, IOSBase::OpenMode mode = IOSBase::OpenMode::Read | IOSBase::OpenMode::Write | IOSBase::OpenMode::Truncate) {
        char modeStr[6] = {0}, *p = modeStr;
        if (bool(mode & IOSBase::OpenMode::Read)) *p++ = 'r';
        if (bool(mode & IOSBase::OpenMode::Write)) *p++ = 'w';
        if (bool(mode & IOSBase::OpenMode::Append)) *p++ = 'a';
        if (bool(mode & IOSBase::OpenMode::Binary)) *p++ = 'b';
        if (bool(mode & IOSBase::OpenMode::Truncate)) *p++ = 't';
        if (p == modeStr) *p++ = 'r'; // Default to read mode if no mode is specified
        *p = '\0';
        filePtr = std::fopen(path, modeStr);
        if (!filePtr) {
            throw FileOpenException(path, mode, errno);
        }
    }
    ~FileStreamHandler() { std::fclose(filePtr); }
    
    void Seek(std::streampos pos, IOSBase::SeekDir dir = IOSBase::SeekDir::Begin) {
        if (std::fseek(filePtr, pos, (int)dir) != 0) { throw FileException(errno); }
    }

    std::streampos Tell() const {
        auto pos = std::ftell(filePtr);
        if (pos == -1) throw FileException(errno);
        return pos;
    }

    std::streamsize Read(void* buffer, std::streamsize size) {
        auto bytesRead = std::fread(buffer, 1, size, filePtr);
        if (bytesRead < 0) throw FileException(errno);
        return bytesRead;
    }

    std::streamsize Write(const void* buffer, std::streamsize size) {
        auto bytesWritten = std::fwrite(buffer, 1, size, filePtr);
        if (bytesWritten < 0) throw FileException(errno);
        return bytesWritten;
    }

    int Flush() {
        if (std::fflush(filePtr) != 0) {
            throw FileException(errno);
        }
        return 0; // Return 0 on success
    }
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicIFStreamBuffer: public BasicIStreamBuffer<CharT, Traits> {
public:
    using CharType = typename BasicIStreamBuffer<CharT, Traits>::CharType;
    using TraitsType = typename BasicIStreamBuffer<CharT, Traits>::TraitsType;
    using IntType = typename BasicIStreamBuffer<CharT, Traits>::IntType;
    using PosType = typename BasicIStreamBuffer<CharT, Traits>::PosType;
    using OffType = typename BasicIStreamBuffer<CharT, Traits>::OffType;

    static constexpr size_t BufferSize = 1024; ///< Size of the read buffer
private:
    FileStreamHandler* _fileHandler;
    std::array<CharType, BufferSize> _readBuffer; ///< Buffer for reading data from the file
    std::streamsize _readPos = 0; ///< Current position in the file handler for reading

    // We don't consider the case where the file size is not an integer multiple of T
public:
    BasicIFStreamBuffer(FileStreamHandler* fileHandler)
        : BasicIStreamBuffer<CharT, Traits>(), _fileHandler(fileHandler) {
        this->buffer.begin = _readBuffer.data();
        this->buffer.current = _readBuffer.data();
        this->buffer.end = _readBuffer.data();  // Initially empty buffer
        this->_readPos = this->_fileHandler->Tell();
    }

    PosType SeekPos(PosType pos) override {
        _fileHandler->lastOperation = IOSBase::IOMode::In;
        _fileHandler->Seek(pos * sizeof(CharType), IOSBase::SeekDir::Begin);
        _readPos = _fileHandler->Tell();
        this->Underflow(); // Ensure the buffer is filled after seeking
        return pos;
    }

    PosType SeekOff(OffType off, IOSBase::SeekDir dir) override {
        _fileHandler->lastOperation = IOSBase::IOMode::In;
        _fileHandler->Seek(off * sizeof(CharType), dir);
        _readPos = _fileHandler->Tell();
        this->Underflow(); // Ensure the buffer is filled after seeking
        return _fileHandler->Tell() / sizeof(CharType);
    }

    PosType Tell() const {
        return _readPos / sizeof(CharType) - (this->buffer.end - this->buffer.current);
    }
    
    IntType Showmanyc() const override {
        if (_fileHandler->lastOperation != IOSBase::IOMode::In) {
            _fileHandler->Seek(_readPos, IOSBase::SeekDir::Begin);
            _fileHandler->lastOperation = IOSBase::IOMode::In;
        }
        _fileHandler->Seek(0, IOSBase::SeekDir::End);
        auto endPos = _fileHandler->Tell();
        _fileHandler->Seek(_readPos, IOSBase::SeekDir::Begin);
        return static_cast<IntType>(endPos - _readPos) / sizeof(CharType);
    }
protected:
    IntType Underflow() override {
        if (_fileHandler->lastOperation != IOSBase::IOMode::In) {
            _fileHandler->Seek(_readPos, IOSBase::SeekDir::Begin);
            _fileHandler->lastOperation = IOSBase::IOMode::In;
        }
        auto roff = _fileHandler->Read(_readBuffer.data(), BufferSize);
        if (roff <= 0) {
            return Traits::eof(); // No more data to read
        }
        this->buffer.current = this->buffer.begin;
        this->buffer.end = this->buffer.begin + roff / sizeof(CharType);
        _readPos += roff;
        return Traits::to_int_type(*this->buffer.current);
    }
    IntType Uflow() override {
        this->Underflow(); // Ensure the buffer is filled
        return Traits::to_int_type(*this->buffer.current++);
    }
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicOFStreamBuffer: public BasicOStreamBuffer<CharT, Traits> {
public:
    using CharType = typename BasicOStreamBuffer<CharT, Traits>::CharType;
    using TraitsType = typename BasicOStreamBuffer<CharT, Traits>::TraitsType;
    using IntType = typename BasicOStreamBuffer<CharT, Traits>::IntType;
    using PosType = typename BasicOStreamBuffer<CharT, Traits>::PosType;
    using OffType = typename BasicOStreamBuffer<CharT, Traits>::OffType;

    static constexpr size_t BufferSize = 1024; ///< Size of the write buffer
private:
    FileStreamHandler* _fileHandler;
    std::array<CharType, BufferSize> _writeBuffer; ///< Buffer for writing data to the file
    std::streamsize _writePos = 0; ///< Current position in the file handler for writing
public:
    BasicOFStreamBuffer(FileStreamHandler* fileHandler)
        : BasicOStreamBuffer<CharT, Traits>(), _fileHandler(fileHandler) {
        this->buffer.begin = _writeBuffer.data();
        this->buffer.current = _writeBuffer.data();
        this->buffer.end = _writeBuffer.data() + BufferSize;
        this->_writePos = this->_fileHandler->Tell();
    }

    PosType SeekPos(PosType pos) override {
        _fileHandler->lastOperation = IOSBase::IOMode::Out;
        _fileHandler->Seek(pos * sizeof(CharType), IOSBase::SeekDir::Begin);
        _writePos = _fileHandler->Tell();
        this->Overflow(); // Ensure the buffer is flushed after seeking
        return pos;
    }

    PosType SeekOff(OffType off, IOSBase::SeekDir dir) override {
        _fileHandler->lastOperation = IOSBase::IOMode::Out;
        _fileHandler->Seek(off * sizeof(CharType), dir);
        _writePos = _fileHandler->Tell();
        this->Overflow(); // Ensure the buffer is flushed after seeking
        return _fileHandler->Tell() / sizeof(CharType);
    }

    PosType Tell() const {
        return _writePos / sizeof(CharType) - (this->buffer.end - this->buffer.current);
    }

    std::streamsize SPutN(const CharType* s, std::streamsize n) override {
        if (_fileHandler->lastOperation != IOSBase::IOMode::Out) {
            _fileHandler->Seek(_writePos, IOSBase::SeekDir::Begin);
            _fileHandler->lastOperation = IOSBase::IOMode::Out;
        }
        std::streamsize bytesWritten = 0;
        while (bytesWritten < n) {
            if (this->buffer.current == this->buffer.end) {
                if (Overflow(Traits::to_int_type(*s)) == Traits::eof()) break; // No more space in the buffer
            }
            *this->buffer.current++ = *s++;
            ++bytesWritten;
        }
        _writePos += bytesWritten * sizeof(CharType);
        return bytesWritten;
    }
protected:
    IntType Overflow(IntType c = Traits::eof()) override {
        // The buffer content ( begin -> current ) will be written to the file
        if (_fileHandler->lastOperation != IOSBase::IOMode::Out) {
            _fileHandler->Seek(_writePos, IOSBase::SeekDir::Begin);
            _fileHandler->lastOperation = IOSBase::IOMode::Out;
        }
        IntType wsize = 0;
        if (this->buffer.current != this->buffer.begin) {
            std::streamsize bytesToWrite = this->buffer.current - this->buffer.begin;
            if (_fileHandler->Write(this->buffer.begin, bytesToWrite * sizeof(CharType)) < bytesToWrite * sizeof(CharType)) {
                return Traits::eof(); // Write failed, what happened?
            }
            this->buffer.current = this->buffer.begin; // Reset the buffer
            this->_writePos += bytesToWrite * sizeof(CharType);
            wsize += bytesToWrite;
        }
        if (c != Traits::eof()) {
            *this->buffer.current++ = Traits::to_char_type(c); // Put the character into the buffer
            wsize++;
        }
        return wsize; // Return the number of characters written
    }
public:
    int Sync() override {
        if (this->Overflow() == Traits::eof()) return int(-1); // Clear the buffer
        return this->_fileHandler->Flush(); // Ensure all buffered output is written
    }
}; 

// Simple wrapper for std::ifstream
template <typename CharT = char, typename Traits = std::char_traits<CharT>>
class BasicIFStream: public BasicIStream<CharT, Traits> {
    FileStreamHandler _fileHandler;
    BasicIFStreamBuffer<CharT, Traits> _buffer;
public:
    using CharType = typename BasicIStream<CharT, Traits>::CharType;
    using TraitsType = typename BasicIStream<CharT, Traits>::TraitsType;
    using IntType = typename BasicIStream<CharT, Traits>::IntType;
    using PosType = typename BasicIStream<CharT, Traits>::PosType;
    using OffType = typename BasicIStream<CharT, Traits>::OffType;

    BasicIFStream(const char* path, IOSBase::OpenMode mode = IOSBase::OpenMode::Read)
        : BasicIStream<CharT, Traits>(), _fileHandler(path, mode), _buffer(&_fileHandler) { this->InputBuffer(&_buffer); }

    BasicIFStream(const String& path, IOSBase::OpenMode mode = IOSBase::OpenMode::Read)
        : BasicIStream<CharT, Traits>(), _fileHandler(path.c_str(), mode), _buffer(&_fileHandler) { this->InputBuffer(&_buffer); }

    BasicIFStream(const std::filesystem::path& path, IOSBase::OpenMode mode = IOSBase::OpenMode::Read)
        : BasicIStream<CharT, Traits>(), _fileHandler(path.c_str(), mode), _buffer(&_fileHandler) { this->InputBuffer(&_buffer); }
};

template <typename CharT = char, typename Traits = std::char_traits<CharT>>
class BasicOFStream: public BasicOStream<CharT, Traits> {
    FileStreamHandler _fileHandler;
    BasicOFStreamBuffer<CharT, Traits> _buffer;
public:
    using CharType = typename BasicOStream<CharT, Traits>::CharType;
    using TraitsType = typename BasicOStream<CharT, Traits>::TraitsType;
    using IntType = typename BasicOStream<CharT, Traits>::IntType;
    using PosType = typename BasicOStream<CharT, Traits>::PosType;
    using OffType = typename BasicOStream<CharT, Traits>::OffType;

    BasicOFStream(const char* path, IOSBase::OpenMode mode = IOSBase::OpenMode::Write | IOSBase::OpenMode::Truncate)
        : BasicOStream<CharT, Traits>(), _fileHandler(path, mode), _buffer(&_fileHandler) { this->OutputBuffer(&_buffer); }

    BasicOFStream(const String& path, IOSBase::OpenMode mode = IOSBase::OpenMode::Write | IOSBase::OpenMode::Truncate)
        : BasicOStream<CharT, Traits>(), _fileHandler(path.c_str(), mode), _buffer(&_fileHandler) { this->OutputBuffer(&_buffer); }

    BasicOFStream(const std::filesystem::path& path, IOSBase::OpenMode mode = IOSBase::OpenMode::Write | IOSBase::OpenMode::Truncate)
        : BasicOStream<CharT, Traits>(), _fileHandler(path.c_str(), mode), _buffer(&_fileHandler) { this->OutputBuffer(&_buffer); }
};

template <typename CharT = char, typename Traits = std::char_traits<CharT>>
class BasicIOFStream: public BasicIOStream<CharT, Traits> {
    FileStreamHandler _fileHandler;
    BasicIFStreamBuffer<CharT, Traits> _inputBuffer;
    BasicOFStreamBuffer<CharT, Traits> _outputBuffer;
public:
    using CharType = typename BasicIOStream<CharT, Traits>::CharType;
    using TraitsType = typename BasicIOStream<CharT, Traits>::TraitsType;
    using IntType = typename BasicIOStream<CharT, Traits>::IntType;
    using PosType = typename BasicIOStream<CharT, Traits>::PosType;
    using OffType = typename BasicIOStream<CharT, Traits>::OffType;

    BasicIOFStream(const char* path, IOSBase::OpenMode mode = IOSBase::OpenMode::Read | IOSBase::OpenMode::Write)
        : BasicIOStream<CharT, Traits>(), _fileHandler(path, mode), _inputBuffer(&_fileHandler), _outputBuffer(&_fileHandler) {
        this->InputBuffer(&_inputBuffer);
        this->OutputBuffer(&_outputBuffer);
    }

    BasicIOFStream(const String& path, IOSBase::OpenMode mode = IOSBase::OpenMode::Read | IOSBase::OpenMode::Write)
        : BasicIOStream<CharT, Traits>(), _fileHandler(path.c_str(), mode), _inputBuffer(&_fileHandler), _outputBuffer(&_fileHandler) {
        this->InputBuffer(&_inputBuffer);
        this->OutputBuffer(&_outputBuffer);
    }

    BasicIOFStream(const std::filesystem::path& path, IOSBase::OpenMode mode = IOSBase::OpenMode::Read | IOSBase::OpenMode::Write)
        : BasicIOStream<CharT, Traits>(), _fileHandler(path.c_str(), mode), _inputBuffer(&_fileHandler), _outputBuffer(&_fileHandler) {
        this->InputBuffer(&_inputBuffer);
        this->OutputBuffer(&_outputBuffer);
    }
};

using IFStream = BasicIFStream<char>;
using OFStream = BasicOFStream<char>;
using IOFStream = BasicIOFStream<char>;

LCORE_NAMESPACE_END
