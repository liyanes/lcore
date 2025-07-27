#pragma once
#include "iostream.hpp"
#include <fstream>

LCORE_NAMESPACE_BEGIN

class String;

// Simple wrapper for std::ifstream
template <typename CharT = char, typename Traits = std::char_traits<CharT>>
class BasicIFStream: public BasicIStream<CharT, Traits>, public std::basic_ifstream<CharT, Traits> {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;

    BasicIFStream(const char* path, std::ios_base::openmode mode = std::ios_base::in)
        : BasicIStream<CharT, Traits>(), std::basic_ifstream<CharT, Traits>(path, mode) {}

    BasicIFStream(const String& path, std::ios_base::openmode mode = std::ios_base::in)
        : BasicIStream<CharT, Traits>(), std::basic_ifstream<CharT, Traits>(path, mode) {}

    BasicIFStream(const std::filesystem::path& path, std::ios_base::openmode mode = std::ios_base::in)
        : BasicIStream<CharT, Traits>(), std::basic_ifstream<CharT, Traits>(path, mode) {}

    void get(CharType& c) override {
        std::basic_ifstream<CharT, Traits>::get(c);
    }

    void get(CharType* buffer, OffType size) override {
        std::basic_ifstream<CharT, Traits>::read(buffer, size);
    }

    void ignore(OffType count = 1, int delim = EOF) override {
        std::basic_ifstream<CharT, Traits>::ignore(count, delim);
    }

    PosType tellg() override {
        return std::basic_ifstream<CharT, Traits>::tellg();
    }

    void seekg(PosType pos) override {
        std::basic_ifstream<CharT, Traits>::seekg(pos);
    }

    void clear(std::ios_base::iostate state = std::ios_base::goodbit) override {
        std::basic_ifstream<CharT, Traits>::clear(state);
    }

    void set_state(std::ios_base::iostate state) override {
        std::basic_ifstream<CharT, Traits>::setstate(state);
    }

    std::ios_base::iostate get_state() const override {
        return std::basic_ifstream<CharT, Traits>::rdstate();
    }

    void swap(IFStream& other) override {
        std::basic_ifstream<CharT, Traits>::swap(other);
        BasicIStream<CharT, Traits>::swap(other);
    }
};

template <typename CharT = char, typename Traits = std::char_traits<CharT>>
class BasicOFStream: public BasicOStream<CharT, Traits>, public std::basic_ofstream<CharT, Traits> {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;

    BasicOFStream(const char* path, std::ios_base::openmode mode = std::ios_base::out | std::ios_base::trunc)
        : BasicOStream<CharT, Traits>(), std::basic_ofstream<CharT, Traits>(path, mode) {}

    BasicOFStream(const String& path, std::ios_base::openmode mode = std::ios_base::out | std::ios_base::trunc)
        : BasicOStream<CharT, Traits>(), std::basic_ofstream<CharT, Traits>(path, mode) {}

    BasicOFStream(const std::filesystem::path& path, std::ios_base::openmode mode = std::ios_base::out | std::ios_base::trunc)
        : BasicOStream<CharT, Traits>(), std::basic_ofstream<CharT, Traits>(path, mode) {}

    void put(CharType c) override {
        std::basic_ofstream<CharT, Traits>::put(c);
    }
    void put(const CharType* buffer, OffType size) override {
        std::basic_ofstream<CharT, Traits>::write(buffer, size);
    }
    void flush() override {
        std::basic_ofstream<CharT, Traits>::flush();
    }
    PosType tellp() override {
        return std::basic_ofstream<CharT, Traits>::tellp();
    }
    void seekp(PosType pos) override {
        std::basic_ofstream<CharT, Traits>::seekp(pos);
    }
    void clear(std::ios_base::iostate state = std::ios_base::goodbit) override {
        std::basic_ofstream<CharT, Traits>::clear(state);
    }
    void set_state(std::ios_base::iostate state) override {
        std::basic_ofstream<CharT, Traits>::setstate(state);
    }
    std::ios_base::iostate get_state() const override {
        return std::basic_ofstream<CharT, Traits>::rdstate();
    }
    void swap(OFStream& other) override {
        std::basic_ofstream<CharT, Traits>::swap(other);
        BasicOStream<CharT, Traits>::swap(other);
    }
};

template <typename CharT = char, typename Traits = std::char_traits<CharT>>
class BasicIOFStream: public IFStream<CharT, Traits>, public OFStream<CharT, Traits> {
public:
    using CharType = CharT;
    using TraitsType = Traits;
    using IntType = typename Traits::int_type;
    using PosType = typename Traits::pos_type;
    using OffType = typename Traits::off_type;

    BasicIOFStream(const char* path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::trunc)
        : BasicIOStream<CharT, Traits>(), std::basic_iostream<CharT, Traits>(new IFStream<CharT, Traits>(path, mode), new OFStream<CharT, Traits>(path, mode)) {}

    BasicIOFStream(const String& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::trunc)
        : BasicIOStream<CharT, Traits>(), std::basic_iostream<CharT, Traits>(new IFStream<CharT, Traits>(path, mode), new OFStream<CharT, Traits>(path, mode)) {}

    BasicIOFStream(const std::filesystem::path& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::trunc)
        : BasicIOStream<CharT, Traits>(), std::basic_iostream<CharT, Traits>(new IFStream<CharT, Traits>(path, mode), new OFStream<CharT, Traits>(path, mode)) {}

    void clear(std::ios_base::iostate state = std::ios_base::goodbit) override {
        BasicIOStream<CharT, Traits>::clear(state);
        std::basic_iostream<CharT, Traits>::clear(state);
    }
};

using IFStream = BasicIFStream<char>;
using OFStream = BasicOFStream<char>;
using IOFStream = BasicIOFStream<char>;

LCORE_NAMESPACE_END
