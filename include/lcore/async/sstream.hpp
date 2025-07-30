#pragma once
#include "iostream.hpp"
#include "lcore/sstream.hpp"

LCORE_ASYNC_NAMESPACE_BEGIN

template <
    typename CharT,
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>>
class BasicIStringViewBuffer : public BasicIStreamBuffer<CharT, TaskT, Traits> {
public:
    using StringViewType = std::basic_string_view<CharT, Traits>;
    BasicIStringViewBuffer(StringViewType str)
        : BasicIStreamBuffer<CharT, Traits>(BufferPointer<CharT>{str.data(), str.data(), str.data() + str.size()}) {}
    /// @brief Get the string from the buffer
    StringViewType str() const {
        return StringViewType(this->buffer.begin, this->buffer.end - this->buffer.begin);
    }
    // Default implementation will read from the buffer, do not override
};

template <
    typename CharT, 
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>, 
    typename Allocator = std::allocator<CharT>>
class BasicIStringBuffer : public BasicIStreamBuffer<CharT, TaskT, Traits> {
public:
    using StringType = std::basic_string<CharT, Traits, Allocator>;
    using StringViewType = std::basic_string_view<CharT, Traits>;
    using CharType = typename BasicIStreamBuffer<CharT, Traits>::CharType;
    using TraitsType = typename BasicIStreamBuffer<CharT, Traits>::TraitsType;
    using IntType = typename BasicIStreamBuffer<CharT, Traits>::IntType;
    using PosType = typename BasicIStreamBuffer<CharT, Traits>::PosType;
    using OffType = typename BasicIStreamBuffer<CharT, Traits>::OffType;
    using AllocatorType = Allocator;
    template <typename T>
    using TaskType = typename BasicIStreamBuffer<CharT, TaskT, Traits>::TaskType<T>;
private:
    _BasicStringBufferData<CharT, Allocator>* _buffer; ///< Pointer to
    /// the string buffer data
public:
    BasicIStringBuffer(_BasicStringBufferData<CharT, Allocator>* bufferdata) : _buffer(bufferdata) {
        this->_buffer->_onChanged[this->_buffer->_onChangedCount++] = &this->buffer;
        this->buffer.begin = this->_buffer->_data;
        this->buffer.current = this->_buffer->_data;
        this->buffer.end = this->_buffer->_data + this->_buffer->_size - 1; // Leave space for null terminator
    }

    // Seek functions
    TaskType<PosType> SeekPos(PosType pos) override {
        if (pos < 0 || pos >= static_cast<PosType>(this->_buffer->_size)) {
            throw SeekOutOfRangeException(); // Out of bounds
        }
        this->buffer.current = this->buffer.begin + pos;
        co_return pos;
    }
    TaskType<PosType> SeekOff(OffType off, IOSBase::SeekDir dir) override {
        PosType newPos = 0;
        switch (dir) {
            case IOSBase::SeekDir::Begin:
                newPos = off;
                break;
            case IOSBase::SeekDir::Current:
                newPos = this->buffer.current - this->buffer.begin + off;
                break;
            case IOSBase::SeekDir::End:
                newPos = this->buffer.end - this->buffer.begin + off;
                break;
        }
        if (newPos < 0 || newPos >= static_cast<PosType>(this->buffer.end - this->buffer.begin)) {
            throw SeekOutOfRangeException(); // Out of bounds
        }
        this->buffer.current = this->buffer.begin + newPos;
        co_return newPos;
    }
    TaskType<IntType> Showmanyc() const override {
        co_return static_cast<IntType>(this->buffer.end - this->buffer.current);
    }
    StringViewType View() const {
        return StringViewType(this->buffer.begin, this->buffer.end);
    }
};

template <
    typename CharT, 
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>, 
    typename Allocator = std::allocator<CharT>>
class BasicOStringBuffer : public BasicOStreamBuffer<CharT, TaskT, Traits> {
public:
    using StringType = std::basic_string<CharT, Traits, Allocator>;
    using StringViewType = std::basic_string_view<CharT, Traits>;
    using CharType = typename BasicOStreamBuffer<CharT, Traits>::CharType;
    using TraitsType = typename BasicOStreamBuffer<CharT, Traits>::TraitsType;
    using IntType = typename BasicOStreamBuffer<CharT, Traits>::IntType;
    using PosType = typename BasicOStreamBuffer<CharT, Traits>::PosType;
    using OffType = typename BasicOStreamBuffer<CharT, Traits>::OffType;
    using AllocatorType = Allocator;
    template <typename T>
    using TaskType = typename BasicOStreamBuffer<CharT, TaskT, Traits>::TaskType<T>;
private:
    _BasicStringBufferData<CharT, Allocator>* _buffer; ///< Pointer to
    /// the string buffer data
public:
    BasicOStringBuffer(_BasicStringBufferData<CharT, Allocator>* bufferdata) : _buffer(bufferdata) {
        this->_buffer->_onChanged[this->_buffer->_onChangedCount++] = &this->buffer;
        this->buffer.begin = this->_buffer->_data;
        this->buffer.current = this->_buffer->_data;
        this->buffer.end = this->_buffer->_data + this->_buffer->_size - 1; // Leave space for null terminator
    }
    
    // Seek functions
    TaskType<PosType> SeekPos(PosType pos) override {
        if (pos < 0 || pos >= static_cast<PosType>(this->_buffer->_size)) {
            throw SeekOutOfRangeException(); // Out of bounds
        }
        this->buffer.current = this->buffer.begin + pos;
        co_return pos;
    }
    TaskType<PosType> SeekOff(OffType off, IOSBase::SeekDir dir) override {
        PosType newPos = 0;
        switch (dir) {
            case IOSBase::SeekDir::Begin:
                newPos = off;
                break;
            case IOSBase::SeekDir::Current:
                newPos = this->buffer.current - this->buffer.begin + off;
                break;
            case IOSBase::SeekDir::End:
                newPos = this->buffer.end - this->buffer.begin + off;
                break;
        }
        if (newPos < 0 || newPos >= static_cast<PosType>(this->buffer.end - this->buffer.begin)) {
            throw SeekOutOfRangeException(); // Out of bounds
        }
        this->buffer.current = this->buffer.begin + newPos;
        co_return newPos;
    }
protected:
    TaskType<IntType> Overflow(IntType c = Traits::eof()) override {
        this->_buffer->ReAlloc();
        if (c != Traits::eof()) {
            *this->buffer.current++ = Traits::to_char_type(c);
            this->_buffer->OnLengthChanged(this->buffer.current - this->buffer.begin);
        }
        co_return Traits::to_int_type(c);
    }
public:
    TaskType<std::streamsize> SPutN(const CharType* s, std::streamsize n) override {
        if (this->buffer.current + n > this->buffer.end) {
            // Not enough space, reallocate
            this->_buffer->ReAlloc(this->buffer.current - this->buffer.begin + n + 1);
        }
        std::copy(s, s + n, this->buffer.current);
        this->buffer.current += n;
        if (this->buffer.current > this->buffer.end)
            this->_buffer->OnLengthChanged(this->buffer.current - this->buffer.begin);
        co_return n;
    }
    StringViewType View() const {
        return StringViewType(this->buffer.begin, this->buffer.end);
    }
    StringType Str() const {
        return StringType(this->buffer.begin, this->buffer.end);
    }
};

// Stream

template <
    typename CharT, 
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>, 
    typename Allocator = std::allocator<CharT>>
class BasicIStringStream : public BasicIStream<CharT, TaskT, Traits> {
public:
    using StringType = std::basic_string<CharT, Traits, Allocator>;
    using StringViewType = std::basic_string_view<CharT, Traits>;
    using CharType = typename BasicIStream<CharT, Traits>::CharType;
    using TraitsType = typename BasicIStream<CharT, Traits>::TraitsType;
    using IntType = typename BasicIStream<CharT, Traits>::IntType;
    using PosType = typename BasicIStream<CharT, Traits>::PosType;
    using OffType = typename BasicIStream<CharT, Traits>::OffType;
    using AllocatorType = Allocator;
    template <typename T>
    using TaskType = typename BasicIStream<CharT, TaskT, Traits>::TaskType<T>;

    using StringBufferType = BasicIStringBuffer<CharT, TaskT, Traits, Allocator>;
private:
    _BasicStringBufferData<CharT, Allocator> _bufferdata;
    StringBufferType _buffer;
public:
    BasicIStringStream(): _buffer(&_bufferdata) { this->InputBuffer(&_buffer); }
    BasicIStringStream(StringViewType str): _bufferdata(str.size()), _buffer(&_bufferdata) {
        std::copy(str.data(), str.data() + str.size(), _bufferdata._data);
        _bufferdata._data[str.size()] = '\0'; // Null-terminate the string
        this->InputBuffer(&_buffer);
    }

    /// @brief Get the string view from the input stream
    StringViewType View() const { return this->InputBuffer().template Cast<StringBufferType>()->View(); }
};

template <
    typename CharT, 
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>, 
    typename Allocator = std::allocator<CharT>>
class BasicOStringStream : public BasicOStream<CharT, TaskT, Traits> {
public:
    using StringType = std::basic_string<CharT, Traits, Allocator>;
    using StringViewType = std::basic_string_view<CharT, Traits>;
    using CharType = typename BasicOStream<CharT, Traits>::CharType;
    using TraitsType = typename BasicOStream<CharT, Traits>::TraitsType;
    using IntType = typename BasicOStream<CharT, Traits>::IntType;
    using PosType = typename BasicOStream<CharT, Traits>::PosType;
    using OffType = typename BasicOStream<CharT, Traits>::OffType;
    using AllocatorType = Allocator;
    template <typename T>
    using TaskType = typename BasicOStream<CharT, TaskT, Traits>::TaskType<T>;
    using StringBufferType = BasicOStringBuffer<CharT, TaskT, Traits, Allocator>;
private:
    _BasicStringBufferData<CharT, Allocator> _bufferdata;
    StringBufferType _buffer;
public:
    BasicOStringStream(): _buffer(&_bufferdata) { this->OutputBuffer(&_buffer); }
    BasicOStringStream(StringViewType str): _bufferdata(str.size()), _buffer(&_bufferdata) {
        std::copy(str.data(), str.data() + str.size(), _bufferdata._data);
        _bufferdata._data[str.size()] = '\0'; // Null-terminate the string
        this->OutputBuffer(&_buffer);
    }

    /// @brief Get the string view from the output stream
    StringViewType View() const { return this->OutputBuffer().template Cast<StringBufferType>()->View(); }
    /// @brief Get the string from the output stream
    StringType Str() const { return this->OutputBuffer().template Cast<StringBufferType>()->Str(); }
};

template <
    typename CharT, 
    template <typename> typename TaskT = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>, 
    typename Allocator = std::allocator<CharT>>
class BasicStringStream : public BasicIStringStream<CharT, TaskT, Traits, Allocator>,
                           public BasicOStringStream<CharT, TaskT, Traits, Allocator> {
public:
    using StringType = std::basic_string<CharT, Traits, Allocator>;
    using StringViewType = std::basic_string_view<CharT, Traits>;
    using CharType = typename BasicIStringStream<CharT, TaskT, Traits, Allocator>::CharType;
    using TraitsType = typename BasicIStringStream<CharT, TaskT, Traits, Allocator>::TraitsType;
    using IntType = typename BasicIStringStream<CharT, TaskT, Traits, Allocator>::IntType;
    using PosType = typename BasicIStringStream<CharT, TaskT, Traits, Allocator>::PosType;
    using OffType = typename BasicIStringStream<CharT, TaskT, Traits, Allocator>::OffType;
    using AllocatorType = Allocator;
    template <typename T>
    using TaskType = typename BasicIStringStream<CharT, TaskT, Traits, Allocator>::TaskType<T>;
    using IStreamBufferType = BasicIStringStream<CharT, TaskT, Traits, Allocator>;
    using OStreamBufferType = BasicOStringStream<CharT, TaskT, Traits, Allocator>;
private:
    _BasicStringBufferData<CharT, Allocator> _bufferdata; ///< Pointer to the string buffer data
    IStreamBufferType _inBuffer; ///< Input buffer for the string stream
    OStreamBufferType _outBuffer; ///< Output buffer for the string stream
public:
    BasicStringStream() : _inBuffer(&_bufferdata), _outBuffer(&_bufferdata) {
        this->InputBuffer(&_inBuffer);
        this->OutputBuffer(&_outBuffer);
    }
    BasicStringStream(StringViewType str) : _bufferdata(str.size()), _inBuffer(&_bufferdata), _outBuffer(&_bufferdata) {
        std::copy(str.data(), str.data() + str.size(), _bufferdata._data);
        _bufferdata._data[str.size()] = '\0'; // Null-terminate the string
        this->InputBuffer(&_inBuffer);
        this->OutputBuffer(&_outBuffer);
    }
    /// @brief Get the string view from the string stream
    StringViewType View() const {
        return this->OutputBuffer().template Cast<OStreamBufferType>()->View();
    }
    /// @brief Get the string from the string stream
    StringType Str() const {
        return this->OutputBuffer().template Cast<OStreamBufferType>()->Str();
    }
};

using IStringStream = BasicIStringStream<char>;
using OStringStream = BasicOStringStream<char>;
using StringStream = BasicStringStream<char>;

LCORE_ASYNC_NAMESPACE_END
