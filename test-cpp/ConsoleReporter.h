#pragma once
#include "TestReporter.h"

#include <vector>

namespace test
{
    class ConsoleReporter :
        public TestReporter
    {
    public:
        void runStarting() override;
        void runFinished() override;

        void testStarting(const TestInfo& testInfo) override;
        void testFinished(const TestInfo& testInfo, const TestResult& result) override;


    private:
        std::vector<TestResult> _results;
        TestOutcome _outcome;
    };
}
