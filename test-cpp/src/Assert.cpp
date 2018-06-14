#include "../include/Assert.h"
#include "AssertExceptions.h"

#include <cmath>
#include <cinttypes>
#include <cstdlib>


static bool areEqualWithTolerance(double expected, double actual, double tolerance)
{
    return (std::abs(expected - actual) <= tolerance);
}

static std::string ptrToString(const void* ptr)
{
    char buffer[64] = { 0 };
    sprintf_s(buffer, "0x%" PRIXPTR, reinterpret_cast<const uintptr_t>(ptr));
    return buffer;
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

    void Assert::isTrue(bool condition, const std::string& message)
    {
        if (!condition)
        {
            throw AssertFailedException{ message, __func__ };
        }
    }

    void Assert::isFalse(bool condition, const std::string& message)
    {
        if (condition)
        {
            throw AssertFailedException{ message, __func__ };
        }
    }

    void Assert::areEqual(double expected, double actual, double tolerance, const std::string& message)
    {
        if (!areEqualWithTolerance(expected, actual, tolerance))
        {
            fail(
                message,
                __func__,
                stringify(expected),
                stringify(actual)
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
                stringify(expected),
                stringify(actual)
            );
        }
    }

    void Assert::areSame(const void* expected, const void* actual, const std::string& message)
    {
        if (expected != actual)
        {
            fail(message, __func__, ptrToString(expected), ptrToString(actual));
        }
    }

    void Assert::areNotSame(const void* expected, const void* actual, const std::string& message)
    {
        if (expected == actual)
        {
            fail(message, __func__, ptrToString(expected), ptrToString(actual));
        }
    }

    void Assert::isNull(const void* ptr, const std::string& message)
    {
        if (ptr != nullptr)
        {
            throw AssertFailedException{ message, __func__ };
        }
    }

    void Assert::isNotNull(const void* ptr, const std::string& message)
    {
        if (ptr == nullptr)
        {
            throw AssertFailedException{ message, __func__ };
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
