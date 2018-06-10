#pragma once
#include "Stringify.h"
#include <string>

namespace test
{
    enum class TestOutcome
    {
        NotRun,         // Test was not run
        Error,          // An error occurred trying to run the test.
        Failed,         // The test failed.
        Inconclusive,   // The test was inconclusive.
        Passed,         // The test passed.
    };

    struct TestResult
    {
        const char* pszTestName;
        int rowNumber;
        TestOutcome outcome;
        std::string message;
        std::string testStdout;
        std::string testStderr;


        TestResult() :
            outcome{ TestOutcome::NotRun },
            testStdout{},
            testStderr{}
        { }
    };

}

template<>
inline std::string Stringify<test::TestOutcome>::convert(const test::TestOutcome& v)
{
    using namespace test;
    switch (v)
    {
    case TestOutcome::NotRun:       return "Not Run";
    case TestOutcome::Error:        return "Error";
    case TestOutcome::Failed:       return "Failed";
    case TestOutcome::Inconclusive: return "Inconclusive";
    case TestOutcome::Passed:       return "Passed";
    default:                        return "Error";
    }
}
