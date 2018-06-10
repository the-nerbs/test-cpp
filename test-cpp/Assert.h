#pragma once
#include "Stringify.h"
#include <string>

namespace test
{
    class Assert
    {
    public:
        static void fail(const std::string& message);

        static void inconclusive(const std::string& message);

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


    private:
        static void fail(
            const std::string& message,
            const char* assertFunc,
            const std::string& expected,
            const std::string& actual);
    };
}
