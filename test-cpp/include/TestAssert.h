#pragma once
#include "Stringify.h"
#include <string>
#include <memory>

namespace test
{
    class Assert
    {
    public:
        static void fail(const std::string& message);

        static void inconclusive(const std::string& message);

        static void isTrue(bool condition, const std::string& message = "");

        static void isFalse(bool condition, const std::string& message = "");


        template <typename T, typename U>
        static void areEqual(const T& expected, const U& actual, const std::string& message = "")
        {
            if (!(expected == (T)actual))
            {
                fail(message,
                    __func__,
                    stringify(expected),
                    stringify(actual)
                );
            }
        }

        static void areEqual(double expected, double actual, double tolerance, const std::string& message = "");


        template <typename T, typename U>
        static void areNotEqual(const T& expected, const U& actual, const std::string& message = "")
        {
            if (expected == (T)actual)
            {
                fail(message,
                    __func__,
                    stringify(expected),
                    stringify(actual)
                );
            }
        }

        static void areNotEqual(double expected, double actual, double tolerance, const std::string& message = "");


        static void areSame(const void* expected, const void* actual, const std::string& message = "");

        template <typename T>
        static void areSame(const T& expected, const T& actual, const std::string& message = "")
        {
            areSame(
                (const void*)std::addressof(expected),
                (const void*)std::addressof(actual),
                message
            );
        }


        static void areNotSame(const void* expected, const void* actual, const std::string& message = "");

        template <typename T>
        static void areNotSame(const T& expected, const T& actual, const std::string& message = "")
        {
            areNotSame(
                (const void*)std::addressof(expected),
                (const void*)std::addressof(actual),
                message
            );
        }


        static void isNull(const void* ptr, const std::string& message = "");

        template <typename T>
        static void isNull(const T* ptr, const std::string& message = "")
        {
            isNull((const void*)ptr, message);
        }


        static void isNotNull(const void* ptr, const std::string& message = "");

        template <typename T>
        static void isNotNull(const T* ptr, const std::string& message = "")
        {
            isNotNull((const void*)ptr, message);
        }


    private:
        static void fail(
            const std::string& message,
            const char* assertFunc,
            const std::string& expected,
            const std::string& actual);
    };
}
