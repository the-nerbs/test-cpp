#pragma once
#include <string>

namespace test
{
    class AssertException
    {
    public:
        AssertException(std::string message);
        virtual ~AssertException() = default;

    public:
        const std::string& message() const { return _msg; }
        virtual std::string toString() const = 0;

    private:
        std::string _msg;
    };


    class AssertFailedException final :
        public AssertException
    {
    public:
        AssertFailedException(
            std::string message,
            const char* assertFunc);

        AssertFailedException(
            std::string message,
            const char* assertFunc,
            std::string expected,
            std::string actual);

    public:
        std::string toString() const override;

    private:
        const char* _assertFunc;
        std::string _expected;
        std::string _actual;
    };


    class AssertInconclusiveException final :
        public AssertException
    {
    public:
        AssertInconclusiveException(std::string message);

    public:
        std::string toString() const override;
    };
}
