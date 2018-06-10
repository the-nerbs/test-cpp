#include "Assert.h"
#include "AssertExceptions.h"

#include <cmath>

static bool areEqualWithTolerance(double expected, double actual, double tolerance)
{
    return (std::abs(expected - actual) <= tolerance);
}

namespace test
{
    void Assert::fail(const std::string& message)
    {
        throw AssertFailedException{ message, __func__ };
    }


    void Assert::inconclusive(const std::string& message)
    {
        throw AssertInconclusiveException{ message };
    }


    void Assert::areEqual(double expected, double actual, double tolerance, const std::string& message)
    {
        if (!areEqualWithTolerance(expected, actual, tolerance))
        {
            fail(
                message,
                __func__,
                Stringify<double>::convert(expected),
                Stringify<double>::convert(actual)
            );
        }
    }


    void Assert::areNotEqual(double expected, double actual, double tolerance, const std::string& message)
    {
        if (areEqualWithTolerance(expected, actual, tolerance))
        {
            fail(
                message,
                __func__,
                Stringify<double>::convert(expected),
                Stringify<double>::convert(actual)
            );
        }
    }


    void Assert::fail(
        const std::string& message,
        const char* assertFunc,
        const std::string& expected,
        const std::string& actual)
    {
        throw AssertFailedException{ message, assertFunc, expected, actual };
    }
}
