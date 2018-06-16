#pragma once
#include <string>
#include <unordered_set>

namespace test
{
    struct TestInfo
    {
        const char* testName;
        const char* testFile;
        int totalRowCount;
        int rowNumber;
        std::unordered_set<std::string> tags;
    };
}
