#pragma once
#include "base.hpp"
#include "class.hpp"
#include "enum.hpp"
#include <iostream>

LCORE_NAMESPACE_BEGIN

class IOSBase {
public:
    enum class IOState {
        Good = 0,
        EndOfFile = 1,
        Fail = 2,
        Bad = 4
    };
    enum class SeekDir {
        Begin = std::ios_base::beg,
        Current = std::ios_base::cur,
        End = std::ios_base::end
    };
    enum class IOMode {
        In = std::ios_base::in,
        Out = std::ios_base::out
    };
};

LCORE_ENUM_BITWISE_OPERATORS(IOSBase::IOState);

template <typename T>
struct BufferPointer {
    T* begin;
    T* current;
    T* end;

    void swap(BufferPointer& other) {
        std::swap(begin, other.begin);
        std::swap(current, other.current);
        std::swap(end, other.end);
    }
};

template <typename CharT>
class BasicStreamBufferBase {
public:
    using CharType = CharT;
    using BufferType = BufferPointer<CharType>;

    /// @brief Swap the current buffer with another
    void swap(BasicStreamBufferBase& other) {
        buffer.swap(other.buffer);
    }
protected:
    BufferPointer<CharType> buffer; ///< The buffer for the stream
public:
    BasicStreamBufferBase(BufferPointer<CharType> buf)
        : buffer(std::move(buf)) {}
};

template <
    typename CharT,
    typename Traits = std::char_traits<CharT>>
class BasicIStreamBuffer: public BasicStreamBufferBase {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;
public:
    BasicStreamBufferBase(BufferPointer<CharType> buf)
        : BasicStreamBufferBase(std::move(buf)) {}
public:
    // Virtual functions for stream buffer management
    // Seek
    /// @brief Seek to a specific position in the stream
    virtual PosType SeekPos(PosType pos) = 0;
    /// @brief Seek to a specific offset from the current position in the stream
    virtual PosType SeekOff(OffType off, IOSBase::SeekDir dir) = 0;
    // Get
    /// @brief Put back the last character read from the stream, if possible 
    virtual IntType Pbackfail(IntType c = Traits::eof()) {
        if (buffer.current == buffer.begin) return Traits::eof(); // No character to put back
        --buffer.current; // Move back in the buffer
        if (c != Traits::eof()) {
            *buffer.current = Traits::to_char_type(c); // Put back the character
        }
        return Traits::to_int_type(*buffer.current);
    }
    /// @brief Obtain the number of characters that can be read, if known
    virtual IntType Showmanyc() const { return Traits::eof(); }
protected:
    /// @brief The buffer is underflowed, this function is called to read more data into the buffer
    virtual IntType Underflow() {
        // assert(buffer.current == buffer.end, "Buffer not underflowed when calling Underflow");
        return Traits::eof(); // Defaultly return end-of-file
    }
    /// @brief The buffer is underflowed, this function is called to read a character from the stream
    virtual IntType Uflow() {
        // assert(buffer.current == buffer.end, "Buffer not underflowed when calling Uflow");
        if (Underflow() == Traits::eof()) return Traits::eof(); // No more data to read
        return Traits::to_int_type(*buffer.current++);
    }
public:
    /// @brief Synchronize the stream buffer with the device
    virtual int Sync() { return 0; }
    /// @brief Get multiple characters from the stream
    /// @return std::streamsize The number of characters read, if this value is less than n, it means the end of the stream is reached.
    /// If a error is encountered, it will throw an exception.
    virtual std::streamsize SGetN(CharType* s, std::streamsize n) {
        std::streamsize count = 0;
        while (count < n) {
            if (buffer.current == buffer.end) {
                if (Underflow() == Traits::eof()) break; // No more data to read
            }
            *s++ = *buffer.current++;
            ++count;
        }
        return count; // Return the number of characters read
    }


    // Implementation details
    // Peek a character from the stream without removing it
    IntType SBumpC() {
        if (buffer.current == buffer.end) this->Underflow();
        return Traits::to_int_type(*buffer.current);
    }
    // Get a character from the stream and remove it
    IntType SGetC() {
        if (buffer.current == buffer.end) return this->Uflow();
        return Traits::to_int_type(*buffer.current++);
    }
};

template <
    typename CharT,
    typename Traits = std::char_traits<CharT>>
class BasicOStreamBuffer: public BasicStreamBufferBase {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;
public:
    BasicStreamBufferBase(BufferPointer<CharType> buf)
        : BasicStreamBufferBase(std::move(buf)) {}

    // Virtual functions for stream buffer management
    // Seek
    /// @brief Seek to a specific position in the stream
    virtual PosType SeekPos(PosType pos) = 0;
    /// @brief Seek to a specific offset from the current position in the stream
    virtual PosType SeekOff(OffType off, IOSBase::SeekDir dir) = 0;
    // Put
    /// @brief Put multiple characters into the stream
    virtual std::streamsize SPutN(const CharType* s, std::streamsize n) { return 0; }
protected:
    /// @brief The buffer is overflowed, this function is called to write a character to the stream
    virtual IntType Overflow(IntType c = Traits::eof()) {
        // assert(buffer.current == buffer.end, "Buffer not overflowed when calling Overflow");
        return Traits::eof(); // Defaultly return a failure
    }
public:
    /// @brief Synchronize the stream buffer with the device
    virtual int Sync() { return 0; }

    // Implementation details
    // Put a character into the stream
    IntType SPutC(CharType c) {
        if (buffer.current == buffer.end) {
            if (Overflow(Traits::to_int_type(c)) == Traits::eof())
                return Traits::eof(); // No more space in the buffer, overflow
        } else {
            *buffer.current++ = c; // Put the character into the buffer
        }
        return Traits::to_int_type(c);
    }
};

// Standard IO

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicIOS: public IOSBase, public AbstractClass {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;

    using IStreamBuffer = BasicIStreamBuffer<CharType, Traits>;
    using OStreamBuffer = BasicOStreamBuffer<CharType, Traits>;
private:
    IStreamBuffer* inputBuffer = nullptr; ///< Input stream buffer
    OStreamBuffer* outputBuffer = nullptr; ///< Output stream buffer
public:
    BasicIOS() {}

    IStreamBuffer* InputBuffer() const { return inputBuffer; }
    IStreamBuffer* InputBuffer(IStreamBuffer* inBuf) { std::swap(inputBuffer, inBuf); return inBuf; }
    OStreamBuffer* OutputBuffer() const { return outputBuffer; }
    OStreamBuffer* OutputBuffer(OStreamBuffer* outBuf) { std::swap(outputBuffer, outBuf); return outBuf; }
};

// Basic Input/Output Streams

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicIStream;

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicOStream;

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicIStream: public virtual BasicIOS<CharT, Traits>, public AbstractClass {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;
protected:
    BasicIStream() {}
public:
    BasicIStream(BasicIStreamBuffer<CharType, Traits>* inBuf) {InputBuffer(inBuf);}

    /// @brief Read a character from the input stream without removing it
    IntType FetchCh() { return this->InputBuffer()->Underflow(); }
    /// @brief Read a character from the input stream and remove it
    IntType GetCh() { return this->InputBuffer()->Uflow(); }
    /// @brief Put back the last character read from the input stream, if possible
    IntType UngetCh(IntType c = Traits::eof()) { return this->InputBuffer()->Pbackfail(c); }
    /// @brief Get multiple characters from the input stream
    std::streamsize GetN(CharType* s, std::streamsize n) { return this->InputBuffer()->SGetN(s, n); }
    /// @brief Seek to a specific position in the input stream
    PosType SeekPos(PosType pos) { return this->InputBuffer()->SeekPos(pos); }
    /// @brief Seek to a specific offset from the current position in the input stream
    PosType SeekOff(OffType off, IOSBase::SeekDir dir) { return this->InputBuffer()->SeekOff(off, dir); }
    
    BasicIFStream<CharT, Traits>& operator>>(CharType& c) {
        IntType ch = GetCh();
        if (ch != Traits::eof()) {
            c = static_cast<CharType>(ch);
        }
        return *this;
    }
    BasicIFStream<CharT, Traits>& operator>>(CharType* buffer) {
        this->InputBuffer()->SGetN(buffer, std::char_traits<CharType>::length(buffer));
        return *this;
    }
    BasicIFStream<CharT, Traits>& operator>>(std::basic_string<CharT>& str) {
        CharType c;
        while (this->InputBuffer()->Underflow() != Traits::eof()) {
            c = GetCh();
            if (c == Traits::eof()) break;
            str.push_back(c);
        }
        return *this;
    }
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicOStream: public virtual BasicIOS<CharT, Traits>, public AbstractClass {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;
protected:
    BasicOStream() {}
public:
    BasicOStream(BasicOStreamBuffer<CharType, Traits>* outBuf) {OutputBuffer(outBuf);}

    /// @brief Write a character to the output stream
    IntType PutCh(CharType c) { return this->OutputBuffer()->Overflow(Traits::to_int_type(c)); }
    /// @brief Write multiple characters to the output stream
    std::streamsize PutN(const CharType* s, std::streamsize n) { return this->OutputBuffer()->SPutN(s, n);}
    /// @brief Flush the output stream, ensuring all buffered output is written
    int Flush() { return this->OutputBuffer()->Sync(); }
    /// @brief Seek to a specific position in the output stream
    PosType SeekPos(PosType pos) { return this->OutputBuffer()->SeekPos(pos); }
    /// @brief Seek to a specific offset from the current position in the output stream
    PosType SeekOff(OffType off, IOSBase::SeekDir dir) { return this->OutputBuffer()->SeekOff(off, dir); }
    BasicOStream& operator<<(CharType c) {
        PutCh(c);
        return *this;
    }
    BasicOStream& operator<<(const CharType* buffer) {
        this->OutputBuffer()->SPutN(buffer, std::char_traits<CharType>::length(buffer));
        return *this;
    }
    BasicOStream& operator<<(const std::basic_string<CharT>& str) {
        for (const CharType& c : str) {
            PutCh(c);
        }
        return *this;
    }
    BasicOStream& operator<<(const std::basic_string_view<CharT>& str)
    {
        this->OutputBuffer()->SPutN(str.data(), str.size());
        return *this;
    }
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicIOStream: public BasicIStream<CharT, Traits>, public BasicOStream<CharT, Traits> {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;
protected:
    BasicIOStream() {}
public:
    BasicIOStream(BasicIStreamBuffer<CharType, Traits>* inBuf, BasicOStreamBuffer<CharType, Traits>* outBuf)
        : BasicIStream<CharT, Traits>(inBuf), BasicOStream<CharT, Traits>(outBuf) {}
};

// Stream redirectors

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicIStreamRedirector {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;
private:
    BasicIStream<CharType, Traits>* target_inputStream;
    BasicIStreamBuffer<CharType, Traits>* cachedBuffer;
public:
    BasicIStreamRedirector(BasicIStream<CharType, Traits>* source, BasicIStream<CharType, Traits>* target)
        : target_inputStream(target) {
        cachedBuffer = target->InputBuffer(source->InputBuffer());
    }
    BasicIStreamRedirector(const BasicIStreamRedirector&) = delete; // Disable copy constructor
    BasicIStreamRedirector& operator=(const BasicIStreamRedirector&) = delete;

    ~BasicIStreamRedirector() {
        target_inputStream->InputBuffer(cachedBuffer);
    }
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicOStreamRedirector {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;
private:
    BasicOStream<CharType, Traits>* target_outputStream;
    BasicOStreamBuffer<CharType, Traits>* cachedBuffer;
public:
    BasicOStreamRedirector(BasicOStream<CharType, Traits>* source, BasicOStream<CharType, Traits>* target)
        : target_outputStream(target) {
        cachedBuffer = target->OutputBuffer(source->OutputBuffer());
    }
    BasicOStreamRedirector(const BasicOStreamRedirector&) = delete; // Disable copy constructor
    BasicOStreamRedirector& operator=(const BasicOStreamRedirector&) = delete;
    ~BasicOStreamRedirector() {
        target_outputStream->OutputBuffer(cachedBuffer);
    }
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class BasicIOStreamRedirector {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;
private:
    BasicIOStream<CharType, Traits>* target_ioStream;
    BasicIStreamBuffer<CharType, Traits>* cachedInputBuffer;
    BasicOStreamBuffer<CharType, Traits>* cachedOutputBuffer;
public:
    BasicIOStreamRedirector(BasicIOStream<CharType, Traits>* source, BasicIOStream<CharType, Traits>* target)
        : target_ioStream(target) {
        cachedInputBuffer = target->InputBuffer(source->InputBuffer());
        cachedOutputBuffer = target->OutputBuffer(source->OutputBuffer());
    }
    BasicIOStreamRedirector(const BasicIOStreamRedirector&) = delete; // Disable copy
    BasicIOStreamRedirector& operator=(const BasicIOStreamRedirector&) = delete;
    ~BasicIOStreamRedirector() {
        target_ioStream->InputBuffer(cachedInputBuffer);
        target_ioStream->OutputBuffer(cachedOutputBuffer);
    }
};

// Type aliases for standard character streams

using IStream = BasicIStream<char>;
using OStream = BasicOStream<char>;
using IOStream = BasicIOStream<char>;
using IStreamRedirector = BasicIStreamRedirector<char>;
using OStreamRedirector = BasicOStreamRedirector<char>;
using IOStreamRedirector = BasicIOStreamRedirector<char>;

LCORE_NAMESPACE_END
