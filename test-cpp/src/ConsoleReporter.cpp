#include "ConsoleReporter.h"

#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace test
{
    void ConsoleReporter::runStarting()
    {
        _results.clear();
    }

    void ConsoleReporter::runFinished()
    {
        // print summary
        std::map<TestOutcome, size_t> outcomes;

        for (const auto& res : _results)
        {
            outcomes[res.outcome]++;
        }

        std::cout << "\n\n";
        std::cout << "Test results\n";
        std::cout << "------------------\n";
        for (const auto& kvp : outcomes)
        {
            std::cout << std::setw(14) << std::left << stringify(kvp.first);
            std::cout << kvp.second << '\n';
        }
        std::cout << '\n';
        std::cout << std::setw(14) << "Total" << _results.size();
        std::cout << "\n\n";

        size_t passedOrNotRun = outcomes[TestOutcome::Passed]
                              + outcomes[TestOutcome::NotRun];

        if (passedOrNotRun == _results.size())
        {
            std::cout << "Test run passed.\n\n";
        }
        else
        {
            std::cout << "Test run failed.\n\n";
        }
    }

    void ConsoleReporter::testStarting(const TestInfo& testInfo)
    {
        if (testInfo.rowNumber == 1)
        {
            std::cout << std::setw(20) << std::left << testInfo.testName << std::flush;
            _outcome = TestOutcome::Passed;
        }
    }

    void ConsoleReporter::testFinished(const TestInfo& testInfo, const TestResult& result)
    {
        _outcome = (TestOutcome)std::min((int)_outcome, static_cast<int>(result.outcome));
        _results.push_back(result);

        if (result.rowNumber == testInfo.totalRowCount)
        {
            std::cout << std::setw(14) << std::left << stringify((TestOutcome)_outcome);
            if (testInfo.totalRowCount > 1)
            {
                std::cout << ' ' << testInfo.totalRowCount << " Rows";
            }
            std::cout << std::endl;

            if (_outcome != TestOutcome::Passed)
            {
                size_t startIdx = _results.size() - testInfo.totalRowCount;

                if (testInfo.totalRowCount > 1)
                {
                    for (int n = 0; n < testInfo.totalRowCount; n++)
                    {
                        const TestResult& rowResult = _results[startIdx + n];
                        if (rowResult.outcome != TestOutcome::Passed)
                        {
                            std::cout
                                << "  Row "
                                << std::left << std::setw(14) << n
                                << std::left << std::setw(15) << stringify(rowResult.outcome)
                                << rowResult.message
                                << std::endl;
                        }
                    }
                }
                else
                {
                    const TestResult& rowResult = _results[startIdx];
                    std::cout << "    " << rowResult.message << std::endl;
                }
            }
        }
    }
}
