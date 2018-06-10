#pragma once
#include "TestReporter.h"

#include <vector>

namespace test
{
    class ConsoleReporter :
        public TestReporter
    {
    public:
        void RunStarting() override;
        void RunFinished() override;

        void TestStarting(const TestInfo& testInfo) override;
        void TestFinished(const TestInfo& testInfo, const TestResult& result) override;


    private:
        std::vector<TestResult> _results;
        TestOutcome _outcome;
    };
}
