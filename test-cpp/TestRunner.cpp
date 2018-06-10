#include "TestRunner.h"
#include "TestResult.h"
#include "Test.h"
#include "Stringify.h"
#include "AssertExceptions.h"

#include <map>
#include <vector>
#include <iostream>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <cstdint>

#include <io.h>
#include <cstdio>

#include <Windows.h>
#undef min
#undef max

using namespace test::details;

namespace test
{
    static std::string getTempFileName()
    {
        GUID uid{};
        HRESULT hr = CoCreateGuid(&uid);
        while (FAILED(hr))
        {
            hr = CoCreateGuid(&uid);
        }

        char fname[64] = { 0 };
        int idx = 0;
        const uint8_t* pBytes = (const uint8_t*)(&uid);
        const uint8_t* end = (const uint8_t*)((&uid) + 1);

        static constexpr const char HexChars[] = "0123456789ABCDEF";

        while (pBytes < end)
        {
            fname[idx++] = HexChars[*pBytes >> 4];
            fname[idx++] = HexChars[*pBytes & 0x0F];
            pBytes++;
        }
        fname[idx++] = 0;

        char path[MAX_PATH + 1];
        DWORD len = GetTempPathA(MAX_PATH, path);

        strcpy_s(path + len, MAX_PATH - len, fname);

        return path;
    }

    static int redirectStream(FILE* f, const std::string& file)
    {
        int orig = _dup(_fileno(f));

        // suppress deprecated (unsafe) CRT function.
#pragma warning(suppress: 4996)
        freopen(file.c_str(), "w", f);

        return orig;
    }

    static std::string readAndRestoreStream(FILE* f, const std::string& file, int origFd)
    {
        // flush and restore the std stream
        fflush(f);
        _dup2(origFd, _fileno(f));

        // read the redirected contents
        std::string content;

        FILE* fd = nullptr;

        if (fopen_s(&fd, file.c_str(), "r") == 0
            && fd != nullptr)
        {
            char buffer[4096];
            size_t read;
            do
            {
                read = fread_s(buffer, sizeof(buffer), 1, _countof(buffer), fd);
                if (read > 0)
                {
                    content.append(buffer, read);
                }

            } while (read > 0);

            fclose(fd);
        }
        else
        {
            content = "failed to read stream content.";
        }

        // delete the temp file.
        std::remove(file.c_str());

        return content;
    }

    static std::vector<const TestFunc*> discoverTests()
    {
        const uint8_t* pBase = (const uint8_t*)GetModuleHandle(NULL);
        auto pDosHeader = (const IMAGE_DOS_HEADER*)pBase;
        auto pNtHeader = (const IMAGE_NT_HEADERS*)(pBase + pDosHeader->e_lfanew);

        auto rvaExports = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        auto pExports = (const IMAGE_EXPORT_DIRECTORY*)(pBase + rvaExports);

        const DWORD* pFuncs = (const DWORD*)(pBase + pExports->AddressOfFunctions);
        const DWORD* pNames = (const DWORD*)(pBase + pExports->AddressOfNames);

        std::vector<const TestFunc*> infos;

        for (DWORD i = 0; i < pExports->NumberOfFunctions; i++)
        {
            const void* pfn = (const void*)(pBase + pFuncs[i]);
            const char* pszName = (const char*)(pBase + pNames[i]);

            static constexpr const char NamePrefix[] = "getTestInfo_";
            if (strncmp(pszName, NamePrefix, _countof(NamePrefix) - 1) == 0)
            {
                using GetInfoFn = const TestFunc* (*)();
                auto pfnGetInfo = (GetInfoFn)pfn;
                infos.push_back(pfnGetInfo());
            }
        }

        return infos;
    }

    static TestResult runTest(TestFn test, void* context)
    {
        TestResult result;

        std::string stdoutFile = getTempFileName();
        std::string stderrFile = getTempFileName();

        int fdStdout = redirectStream(stdout, stdoutFile);
        int fdStderr = redirectStream(stderr, stderrFile);

        try
        {
            test(context);
            result.outcome = TestOutcome::Passed;
        }
        catch (const AssertInconclusiveException& ex)
        {
            result.outcome = TestOutcome::Inconclusive;
            result.message = ex.toString();
        }
        catch (const AssertFailedException& ex)
        {
            result.outcome = TestOutcome::Failed;
            result.message = ex.toString();
        }
        catch (const std::exception& ex)
        {
            result.outcome = TestOutcome::Failed;
            result.message = ex.what();
        }
        catch (...)
        {
            result.outcome = TestOutcome::Failed;
            result.message = "unknown error";
        }

        result.testStdout = readAndRestoreStream(stdout, stdoutFile, fdStdout);
        result.testStderr = readAndRestoreStream(stderr, stderrFile, fdStderr);

        return result;
    }


    static void runTest(std::vector<TestResult>& allResults, const TestFunc& testInfo)
    {
        std::cout << std::setw(20) << std::left << testInfo.pszName << std::flush;
        int outcome = static_cast<int>(TestOutcome::Passed);
        int startIdx = static_cast<int>(allResults.size());

        if (testInfo.count == 1 && testInfo.argSize == 0)
        {
            TestResult result = runTest(testInfo.pfnTest, nullptr);
            result.pszTestName = testInfo.pszName;
            result.rowNumber = 0;
            allResults.push_back(result);

            outcome = std::min(outcome, (int)result.outcome);
        }
        else if (testInfo.argSize > 0)
        {
            uintptr_t argsAddr = reinterpret_cast<uintptr_t>(testInfo.arg);

            for (int n = 0;
                n < testInfo.count;
                n++, argsAddr += testInfo.argSize)
            {
                TestResult result = runTest(testInfo.pfnTest, (void*)argsAddr);
                result.pszTestName = testInfo.pszName;
                result.rowNumber = n;
                allResults.push_back(result);

                outcome = std::min(outcome, static_cast<int>(result.outcome));
            }
        }

        std::cout << stringify((TestOutcome)outcome) << std::endl;

        if (outcome != (int)TestOutcome::Passed)
        {
            if (testInfo.count > 1)
            {
                for (int n = 0; n < testInfo.count; n++)
                {
                    const TestResult& result = allResults[startIdx + n];
                    if (result.outcome != TestOutcome::Passed)
                    {
                        std::cout
                            << "  Row "
                            << std::left << std::setw(14) << n
                            << stringify(result.outcome) << "\n"
                            << "    " << result.message
                            << std::endl;
                    }
                }
            }
            else
            {
                const TestResult& result = allResults[startIdx];
                std::cout << "    " << result.message << std::endl;
            }
        }
    }

    void runAllTests()
    {
        // discover tests
        std::vector<const TestFunc*> testTokenRows = discoverTests();

        // run tests
        std::vector<TestResult> results;
        for (const auto* pTestInfo : testTokenRows)
        {
            if (pTestInfo != nullptr)
            {
                runTest(results, *pTestInfo);
            }
        }

        // print summary
        std::map<TestOutcome, size_t> outcomes;

        for (const auto& res : results)
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
        std::cout << std::setw(14) << "Total" << results.size();
        std::cout << "\n\n";

        if (outcomes[TestOutcome::Passed] == results.size())
        {
            std::cout << "Test run passed.\n\n";
        }
        else
        {
            std::cout << "Test run failed.\n\n";
        }
    }
}
