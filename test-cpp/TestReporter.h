#pragma once
#include "TestResult.h"
#include "TestInfo.h"

namespace test
{
    class TestReporter
    {
    public:
        virtual ~TestReporter() = default;

        virtual void RunStarting() = 0;
        virtual void RunFinished() = 0;

        virtual void TestStarting(const TestInfo& testInfo) = 0;
        virtual void TestFinished(const TestInfo& testInfo, const TestResult& result) = 0;
    };
}
