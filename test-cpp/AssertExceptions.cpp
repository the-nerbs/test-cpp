#pragma once
#include "AssertExceptions.h"

#include <utility>

namespace test
{
    AssertException::AssertException(std::string message) :
        _msg{ std::move(message) }
    { }


    AssertFailedException::AssertFailedException(std::string message, const char* assertFunc) :
        AssertException{ std::move(message) },
        _assertFunc{ assertFunc },
        _expected{},
        _actual{}
    { }

    AssertFailedException::AssertFailedException(std::string message, const char* assertFunc, std::string expected, std::string actual) :
        AssertException{ std::move(message) },
        _assertFunc{ assertFunc },
        _expected{ std::move(expected) },
        _actual{ std::move(actual) }
    { }

    std::string AssertFailedException::toString() const
    {

        const std::string& msg = message();
        std::string str;

        if (_expected.length() > 0 || _actual.length() > 0)
        {
            static constexpr const char MsgPart1[] = " failed. Expected=<";
            static constexpr const char MsgPart2[] = ">  Actual=<";
            static constexpr const char MsgPart3[] = ">. ";

            size_t len = strlen(_assertFunc)
                + _countof(MsgPart1)
                + _expected.length()
                + _countof(MsgPart2)
                + _actual.length()
                + _countof(MsgPart3)
                + msg.length();

            str.reserve(len);

            str += _assertFunc;
            str += MsgPart1;
            str += _expected;
            str += MsgPart2;
            str += _actual;
            str += MsgPart3;
            str += msg;
        }
        else
        {
            static constexpr const char MsgPart1[] = " failed. ";

            size_t len = strlen(_assertFunc)
                + _countof(MsgPart1)
                + msg.length();

            str.reserve(len);

            str += _assertFunc;
            str += MsgPart1;
            str += msg;
        }

        return str;
    }


    AssertInconclusiveException::AssertInconclusiveException(std::string message) :
        AssertException{ std::move(message) }
    {}

    std::string AssertInconclusiveException::toString() const
    {
        static constexpr const char MsgPart1[] = "Test was inconclusive. ";

        const std::string& msg = message();

        size_t len = _countof(MsgPart1)
            + msg.length();

        std::string str;
        str.reserve(len);

        str += MsgPart1;
        str += msg;

        return str;
    }
}
