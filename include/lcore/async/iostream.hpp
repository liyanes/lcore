#pragma once
#include "base.hpp"
#include "lcore/class.hpp"
#include "lcore/iostream.hpp"
#include "task.hpp"

/**
 * Asynchronous streams have something different from regular streams:
 * - They are not blocking, so they can be used in coroutines.
 * - They need to be opened and closed explicitly, as they may not be used in a blocking manner.
 * 
 * In below classes, we will not check the openflag, as it is the responsibility of the user to ensure that the stream is open before using it.
 * If the stream is not open, the behavior is undefined.
 */

LCORE_ASYNC_NAMESPACE_BEGIN

class AsyncStreamException : public StreamException {};
class AsyncStreamClosedException : public AsyncStreamException {};
class AsyncStreamNotClosedException : public AsyncStreamException {};

// Stream buffer

template <
    typename CharT,
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicIStreamBuffer : public BasicIStreamBufferBase<CharT> {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;

    template <typename T>
    using TaskType = TaskT<T>;
private:
    bool m_openflag = false; ///< Flag to indicate if the stream is finalized
protected:
    BasicIStreamBuffer() = default;
public:
    BasicIStreamBuffer(BufferPointer<CharType> buf):
        BasicIStreamBufferBase<CharT>(std::move(buf)) {}
    ~BasicIStreamBuffer() override {
        if (m_openflag)
            // This stream is not closed properly, throw an exception
            throw AsyncStreamNotClosedException();
    }
public:
    // Virtual function to manage the stream buffer life cycle
    /// @brief Open the stream buffer
    virtual TaskType<void> Open() { m_openflag = true; co_return; }
    /// @brief Close the stream buffer
    virtual TaskType<void> Close() { m_openflag = false; co_return; }
    /// @brief Check if the stream buffer is open
    bool IsOpen() const { return m_openflag; }

    /// Virtual functions for stream buffer management
    // Seek
    /// @brief Seek to a specific position in the stream
    virtual TaskType<PosType> SeekPos(PosType pos) { LCORE_NOTIMPLEMENTED(); }
    /// @brief Seek to a specific offset from the current position in the stream
    virtual TaskType<PosType> SeekOff(OffType off, IOSBase::SeekDir dir) { LCORE_NOTIMPLEMENTED(); }
    // Get
    /// @brief Put back the last character read from the stream, if possible
    virtual TaskType<IntType> Pbackfail(IntType c = Traits::eof()) {
        co_return Traits::eof();
        // if (this->buffer.current == this->buffer.begin) co_return Traits::eof(); // No character to put back
        // --this->buffer.current; // Move back in the buffer
        // if (c != Traits::eof()) {
        //     *this->buffer.current = Traits::to_char_type(c); // Put back the character
        // }
        // co_return Traits::to_int_type(*this->buffer.current);
    }
    /// @brief Get multiple characters from the stream
    virtual TaskType<IntType> Showmanyc() const { co_return Traits::eof();}
protected:
    /// @brief Read a character from the stream without removing it
    virtual TaskType<IntType> Underflow() {
        // Default implementation, should be overridden by derived classes
        co_return Traits::eof();
    }
    /// @brief Read a character from the stream and remove it
    virtual TaskType<IntType> Uflow() {
        // assert(this->buffer.current == this->buffer.end, "Uflow called on an empty buffer");
        if (co_await this->Underflow() == Traits::eof()) co_return Traits::eof(); // No more data to read
        co_return Traits::to_int_type(*this->buffer.current++);
    }
public:
    /// @brief Synchronize the stream buffer, ensuring all buffered input is read
    virtual TaskType<int> Sync() {
        // Default implementation, should be overridden by derived classes
        co_return 0;
    }
    /// @brief Get multiple characters from the stream
    virtual TaskType<std::streamsize> GetN(CharType* s, std::streamsize n) {
        std::streamsize count = 0;
        while (count < n) {
            if (this->buffer.current == this->buffer.end) {
                if (co_await this->Underflow() == Traits::eof()) break; // No more data to read
            }
            *s++ = *this->buffer.current++;
            ++count;
        }
        co_return count; // Return the number of characters read
    }

    // Implementation details
    // Peek a character from the stream without removing it
    TaskType<IntType> SBumpC() {
        if (this->buffer.current == this->buffer.end) co_await this->Underflow();
        co_return Traits::to_int_type(*this->buffer.current);
    }
    // Get a character from the stream and remove it
    TaskType<IntType> SGetC() {
        if (this->buffer.current == this->buffer.end) co_return co_await this->Uflow();
        co_return Traits::to_int_type(*this->buffer.current++);
    }
};

template <
    typename CharT,
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicOStreamBuffer : public BasicOStreamBufferBase<CharT> {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;

    template <typename T>
    using TaskType = TaskT<T>;
private:
    bool m_openflag = false; ///< Flag to indicate if the stream is finalized
protected:
    BasicOStreamBuffer() = default;
public:
    BasicOStreamBuffer(BufferPointer<CharType> buf):
        BasicOStreamBufferBase<CharT>(std::move(buf)) {}
    ~BasicOStreamBuffer() override {
        if (m_openflag)
            // This stream is not closed properly, throw an exception
            throw AsyncStreamNotClosedException();
    }
    
    // Virtual function to manage the stream buffer life cycle
    /// @brief Open the stream buffer
    virtual TaskType<void> Open() { m_openflag = true; co_return; }
    /// @brief Close the stream buffer
    virtual TaskType<void> Close() { m_openflag = false; co_return; }
    /// @brief Check if the stream buffer is open
    bool IsOpen() const { return m_openflag; }

    /// Virtual functions for stream buffer management
    // Seek
    /// @brief Seek to a specific position in the stream
    virtual TaskType<PosType> SeekPos(PosType pos) { LCORE_NOTIMPLEMENTED(); }
    /// @brief Seek to a specific offset from the current position in the stream
    virtual TaskType<PosType> SeekOff(OffType off, IOSBase::SeekDir dir) { LCORE_NOTIMPLEMENTED(); }
    // Put
    /// @brief Put multiple characters into the stream
    virtual TaskType<std::streamsize> SPutN(const CharType* s, std::streamsize n) {
        std::streamsize count = 0;
        while (count < n) {
            if (this->buffer.current == this->buffer.end) {
                if (co_await this->Overflow() == Traits::eof()) break; // No more space in the buffer
            }
            *this->buffer.current++ = *s++;
            ++count;
        }
        co_return count; // Return the number of characters written
    }
protected:
    /// @brief The buffer is overflowed, this function is called to write a character to the stream
    virtual TaskType<IntType> Overflow(IntType c = Traits::eof()) {
        // assert(this->buffer.current == this->buffer.end, "Buffer not overflowed when calling Overflow");
        co_return Traits::eof(); // Defaultly return a failure
    }
public:
    /// @brief Synchronize the stream buffer with the device
    virtual TaskType<int> Sync() {
        // Default implementation, should be overridden by derived classes
        co_return 0;
    }

    // Implementation details
    /// @brief Put a character into the stream
    TaskType<IntType> SPutC(CharType c) {
        if (this->buffer.current == this->buffer.end) {
            if (co_await this->Overflow(Traits::to_int_type(c)) == Traits::eof())
                co_return Traits::eof(); // No more space in the buffer
        } else {
            *this->buffer.current++ = c; // Put the character into the buffer
        }
        co_return Traits::to_int_type(c);
    }
};

// Standard IO

template <
    typename CharT,
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicIOS : public IOSBase {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;

    template <typename T>
    using TaskType = TaskT<T>;

    using IStreamBuffer = BasicIStreamBuffer<CharType, TaskT, Traits>;
    using OStreamBuffer = BasicOStreamBuffer<CharType, TaskT, Traits>;
private:
    IStreamBuffer* m_inputBuffer = nullptr; ///< Input stream buffer
    OStreamBuffer* m_outputBuffer = nullptr; ///< Output stream buffer
public:
    BasicIOS() = default;
    virtual ~BasicIOS() = default;

    RawPtr<IStreamBuffer> InputBuffer() const { return m_inputBuffer; }
    RawPtr<IStreamBuffer> InputBuffer(IStreamBuffer* inBuf) { std::swap(m_inputBuffer, inBuf); return inBuf; }
    RawPtr<OStreamBuffer> OutputBuffer() const { return m_outputBuffer; }
    RawPtr<OStreamBuffer> OutputBuffer(OStreamBuffer* outBuf) { std::swap(m_outputBuffer, outBuf); return outBuf; }
};

// Basic Input/Output Streams

template <
    typename CharT,
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicIStream;

template <
    typename CharT,
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicOStream;

template <
    typename CharT,
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicIStream : public virtual BasicIOS<CharT, TaskT, Traits> {
public:
    using CharType = typename BasicIOS<CharT, TaskT, Traits>::CharType;
    using TraitsType = typename BasicIOS<CharT, TaskT, Traits>::TraitsType;
    using IntType = typename BasicIOS<CharT, TaskT, Traits>::IntType;
    using PosType = typename BasicIOS<CharT, TaskT, Traits>::PosType;
    using OffType = typename BasicIOS<CharT, TaskT, Traits>::OffType

    template <typename T>
    using TaskType = TaskT<T>;
protected:
    BasicIStream() = default;
public:
    BasicIStream(BasicIStreamBuffer<CharType, TaskT, Traits>* inBuf) { this->InputBuffer(inBuf); }

    /// @brief Open the input stream
    virtual TaskType<void> Open() { return this->InputBuffer()->Open(); }
    /// @brief Close the input stream
    virtual TaskType<void> Close() { return this->InputBuffer()->Close(); }
    /// @brief Check if the input stream is open
    bool IsOpen() const { return this->InputBuffer()->IsOpen(); }
    /// @brief Read a character from the input stream without removing it
    virtual TaskType<IntType> PeekCh() { return this->InputBuffer()->SBumpC(); }
    /// @brief Read a character from the input stream and remove it
    virtual TaskType<IntType> GetCh() { return this->InputBuffer()->SGetC(); }
    /// @brief Put back the last character read from the input stream, if possible
    virtual TaskType<IntType> UngetCh(IntType c = Traits::eof()) { return this->InputBuffer()->Pbackfail(c); }
    /// @brief Read multiple characters from the input stream
    virtual TaskType<std::streamsize> GetN(CharType* s, std::streamsize n) { return this->InputBuffer()->SGetN(s, n); }
    /// @brief Flush the input stream, ensuring all buffered input is read
    virtual TaskType<int> Flush() { return this->InputBuffer()->Sync(); }
    /// @brief Seek to a specific position in the input stream
    virtual TaskType<PosType> SeekPos(PosType pos) { return this->InputBuffer()->SeekPos(pos); }
    /// @brief Seek to a specific offset from the current position in the input stream
    virtual TaskType<PosType> SeekOff(OffType off, IOSBase::SeekDir dir) { return this->InputBuffer()->SeekOff(off, dir); }

    /// Implement for high-level stream operations
    /// @brief Read a line from the input stream into a string
    virtual TaskType<std::basic_string<CharType, Traits>> GetLine(CharType delimiter = '\n') {
        std::basic_string<CharType, Traits> line;
        IntType c;
        while ((c = Traits::to_char_type(co_await this->GetCh())) != Traits::eof() && c != Traits::to_char_type(delimiter)) {
            line.push_back(c);
        }
        co_return line;
    }
};

template <
    typename CharT,
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicOStream : public virtual BasicIOS<CharT, TaskT, Traits> {
public:
    using CharType = typename BasicIOS<CharT, TaskT, Traits>::CharType;
    using TraitsType = typename BasicIOS<CharT, TaskT, Traits>::TraitsType;
    using IntType = typename BasicIOS<CharT, TaskT, Traits>::IntType;
    using PosType = typename BasicIOS<CharT, TaskT, Traits>::PosType;
    using OffType = typename BasicIOS<CharT, TaskT, Traits>::OffType;

    template <typename T>
    using TaskType = TaskT<T>;
protected:
    BasicOStream() = default;
public:
    BasicOStream(BasicOStreamBuffer<CharType, TaskT, Traits>* outBuf) { this->OutputBuffer(outBuf); }

    /// @brief Open the output stream
    virtual TaskType<void> Open() { return this->OutputBuffer()->Open(); }
    /// @brief Close the output stream
    virtual TaskType<void> Close() { return this->OutputBuffer()->Close(); }
    /// @brief Check if the output stream is open
    bool IsOpen() const { return this->OutputBuffer()->IsOpen(); }
    /// @brief Write a character to the output stream
    virtual TaskType<IntType> PutC(CharType c) { return this->OutputBuffer()->SPutC(c); }
    /// @brief Write multiple characters to the output stream
    virtual TaskType<std::streamsize> PutN(const CharType* s, std::streamsize n) { return this->OutputBuffer()->SPutN(s, n); }
    /// @brief Flush the output stream, ensuring all buffered output is written
    virtual TaskType<int> Flush() { return this->OutputBuffer()->Sync(); }
    /// @brief Seek to a specific position in the output stream
    virtual TaskType<PosType> SeekPos(PosType pos) { return this->OutputBuffer()->SeekPos(pos); }
    /// @brief Seek to a specific offset from the current position
    virtual TaskType<PosType> SeekOff(OffType off, IOSBase::SeekDir dir) { return this->OutputBuffer()->SeekOff(off, dir); }
};

template <
    typename CharT,
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicIOStream : public BasicIStream<CharT, TaskT, Traits>, public BasicOStream<CharT, TaskT, Traits> {
public:
    using CharType = typename BasicIOS<CharT, TaskT, Traits>::CharType;
    using TraitsType = typename BasicIOS<CharT, TaskT, Traits>::TraitsType;
    using IntType = typename BasicIOS<CharT, TaskT, Traits>::IntType;
    using PosType = typename BasicIOS<CharT, TaskT, Traits>::PosType;
    using OffType = typename BasicIOS<CharT, TaskT, Traits>::OffType;

    template <typename T>
    using TaskType = TaskT<T>;
protected:
    BasicIOStream() = default;
public:
    BasicIOStream(BasicIStreamBuffer<CharType, TaskT, Traits>* inBuf, BasicOStreamBuffer<CharType, TaskT, Traits>* outBuf)
        : BasicIStream<CharT, TaskT, Traits>(inBuf), BasicOStream<CharT, TaskT, Traits>(outBuf) {}

    /// @brief Open the input and output streams
    virtual TaskType<void> Open() { co_await this->InputBuffer()->Open(); co_await this->OutputBuffer()->Open(); }
    /// @brief Close the input and output streams
    virtual TaskType<void> Close() { co_await this->InputBuffer()->Close(); co_await this->OutputBuffer()->Close(); }
    /// @brief Check if the input and output streams are open
    bool IsOpen() const { return this->InputBuffer()->IsOpen() && this->OutputBuffer()->IsOpen(); }
    /// @brief Synchronize the input and output buffers
    virtual TaskType<int> Flush() { co_return co_await this->InputBuffer()->Sync() || co_await this->OutputBuffer()->Sync(); }
};

LCORE_ASYNC_NAMESPACE_END
