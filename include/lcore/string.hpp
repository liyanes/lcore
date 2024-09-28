#pragma once
#include "base.hpp"
#include <string>
#include <string_view>
#include <sstream>

LCORE_NAMESPACE_BEGIN

class String;
class StringStream;

class StringView: public std::string_view {
public:
    using std::string_view::basic_string_view;
    inline StringView(const std::string& string);
    inline StringView(std::string_view&& view);
    inline StringView(const char* str);

    inline String operator+(StringView rhs) const;
    inline bool isdigit() const noexcept;
};

class String: public std::string {
public:
    using std::string::basic_string;
    inline String(const StringView view);
    inline String(std::string&& str);
    
    inline String& operator=(const std::string& str);
    inline String& operator=(const StringView view);
    inline String& operator=(const char* str);

    inline String operator+(StringView view);
    inline StringView substr(size_t pos, size_t n = npos) const;
    inline StringView trim(size_t lpos, size_t rpos) const;
    inline StringView center(size_t lslice, size_t rslice) const;
    inline bool isdigit() const noexcept;
};

class StringStream: public std::stringstream {
public:
    String str() const {
        return std::stringstream::str();
    };
};

inline StringView::StringView(const std::string& string): std::string_view(string.begin(), string.end()) {}
inline StringView::StringView(std::string_view&& view): std::string_view(view) {}
inline StringView::StringView(const char* str): std::string_view(str) {}

inline String StringView::operator+(StringView rhs) const{
    StringStream ss;
    ss << *this << rhs;
    return ss.str();
};

inline bool StringView::isdigit() const noexcept {
    for (auto c: *this){
        if (!std::isdigit(c)) return false;
    }
    return true;
};

inline String::String(const StringView view): std::string(view.begin(), view.end()) {}
inline String::String(std::string&& str): std::string(str) {}
inline String& String::operator=(const std::string& str){
    std::string::operator=(str);
    return *this;
};
inline String& String::operator=(const StringView view){
    std::string::operator=(view);
    return *this;
};
inline String& String::operator=(const char* str){
    std::string::operator=(str);
    return *this;
};

inline String String::operator+(StringView view){
    StringStream ss;
    ss << *this << view;
    return ss.str();
};
inline StringView String::substr(size_t pos, size_t n) const {
    if (n == npos) return StringView(this->data() + pos, size() - pos);
    return StringView(this->data() + pos, n);
};
inline StringView String::trim(size_t lpos, size_t rpos) const {
    return StringView(this->data() + lpos, this->data() + rpos);
};
inline StringView String::center(size_t lslice, size_t rslice) const {
    return StringView(this->data() + lslice, size() - lslice - rslice);
};
inline bool String::isdigit() const noexcept {
    for (auto c: *this){
        if (!std::isdigit(c)) return false;
    }
    return true;
};

LCORE_NAMESPACE_END
