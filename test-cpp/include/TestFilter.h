#pragma once
#include "TestInfo.h"

namespace test
{
    class TestFilter
    {
    public:
        virtual ~TestFilter() = default;

        virtual bool shouldRunTest(const TestInfo& test) const = 0;
    };


    class NoFilter :
        public TestFilter
    {
    public:
        static const NoFilter Instance;

    public:
        bool shouldRunTest(const TestInfo&) const override { return true; }
    };
}
