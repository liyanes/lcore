#pragma once
#include "base.hpp"
#include "lcore/class.hpp"
#include "lcore/iostream.hpp"
#include "task.hpp"


LCORE_ASYNC_NAMESPACE_BEGIN

template <
    typename CharT,
    template <typename> typename TaskType = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicIStream: public AbstractClass, public virtual IOSBase {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;

    // Input operations
    virtual TaskType<CharType> get(CharType& c) = 0;
    virtual TaskType<CharType> get(CharType* buffer, OffType size) = 0;
    virtual TaskType<CharType> ignore(OffType count = 1, int delim = EOF) = 0;

    // Positioning operations
    virtual TaskType<PosType> tellg() = 0;
    virtual TaskType<void> seekg(PosType pos) = 0;
    // State management
    virtual TaskType<void> clear(std::ios_base::iostate state = std::ios_base::goodbit) = 0;
};

template <
    typename CharT,
    template <typename> typename TaskType = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicOStream: public AbstractClass, public virtual IOSBase {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;

    // Output operations
    virtual TaskType<void> put(CharType c) = 0;
    virtual TaskType<void> put(const CharType* buffer, OffType size) = 0;
    virtual TaskType<void> flush() = 0;

    // Positioning operations
    virtual TaskType<PosType> tellp() = 0;
    virtual TaskType<void> seekp(PosType pos) = 0;

    // State management
    virtual TaskType<void> clear(std::ios_base::iostate state = std::ios_base::goodbit) = 0;
};

template <
    typename CharT,
    template <typename> typename TaskType = DefaultTaskWrapper,
    typename Traits = std::char_traits<CharT>
    >
class BasicIOStream: public BasicIStream<CharT, TaskType, Traits>, public BasicOStream<CharT, TaskType, Traits> {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;
    virtual TaskType<void> clear(std::ios_base::iostate state = std::ios_base::goodbit) override {
        co_await BasicIStream<CharType, TaskType, Traits>::clear(state);
        co_await BasicOStream<CharType, TaskType, Traits>::clear(state);
        co_return;
    }
};

using IStream = BasicIStream<char>;
using OStream = BasicOStream<char>;
using IOStream = BasicIOStream<char>;

LCORE_ASYNC_NAMESPACE_END
