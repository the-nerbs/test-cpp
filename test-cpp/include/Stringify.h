#pragma once
#include <string>

template <typename T>
struct Stringify
{
    static std::string convert(const T&);
};

template <typename T>
std::string stringify(const T& v)
{
    return Stringify<T>::convert(v);
}

// specializes template Stringify by delegating to a function named `to_string`
#define STRINGIFY_TO_STRING(type)                               \
    template <>                                                 \
    inline std::string Stringify<type>::convert(const type& i)  \
    {                                                           \
        using namespace std;                                    \
        return to_string(i);                                    \
    }

STRINGIFY_TO_STRING(short);
STRINGIFY_TO_STRING(unsigned short);
STRINGIFY_TO_STRING(int);
STRINGIFY_TO_STRING(unsigned int);
STRINGIFY_TO_STRING(long);
STRINGIFY_TO_STRING(unsigned long);
STRINGIFY_TO_STRING(long long);
STRINGIFY_TO_STRING(unsigned long long);
STRINGIFY_TO_STRING(float);
STRINGIFY_TO_STRING(double);
STRINGIFY_TO_STRING(long double);

template <>
inline std::string Stringify<const char*>::convert(const char* const& psz)
{
    return psz;
}

template <>
inline std::string Stringify<std::string>::convert(const std::string& str)
{
    return str;
}
