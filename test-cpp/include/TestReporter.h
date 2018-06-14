#pragma once
#include "TestResult.h"
#include "TestInfo.h"

namespace test
{
    class TestReporter
    {
    public:
        virtual ~TestReporter() = default;

        virtual void runStarting() = 0;
        virtual void runFinished() = 0;

        virtual void testStarting(const TestInfo& testInfo) = 0;
        virtual void testFinished(const TestInfo& testInfo, const TestResult& result) = 0;
    };
}
