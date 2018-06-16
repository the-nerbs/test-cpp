#include "TestRunner.h"
#include "TestResult.h"
#include "Test.h"
#include "Stringify.h"
#include "AssertExceptions.h"

#include <vector>
#include <unordered_set>
#include <cstdint>
#include <cstdio>

#include <io.h>

#include <Windows.h>
#undef min
#undef max

using namespace test::details;

namespace test
{
    struct FullTestInfo
    {
        const TestFunc* testFnInfo;
        std::unordered_set<std::string> tags;
    };

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
            // 28199:   MSVC\prefast - possibly uninitialized memory.
#pragma warning(suppress : 28199)
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

        // 4996:    MSVC suppress deprecated (unsafe) CRT function.
        // 6031:    MSVC\prefast - return value ignored.
#pragma warning(suppress: 4996 6031)
        freopen(file.c_str(), "w", f);

        return orig;
    }

    static std::string readAndRestoreStream(FILE* f, const std::string& file, int origFd)
    {
        // flush and restore the std stream
        fflush(f);

        // 6031:    MSVC\prefast - return value ignored.
#pragma warning(suppress : 6031)
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

    static std::vector<FullTestInfo> discoverTests()
    {
        const uint8_t* pBase = (const uint8_t*)GetModuleHandle(NULL);
        auto pDosHeader = (const IMAGE_DOS_HEADER*)pBase;
        auto pNtHeader = (const IMAGE_NT_HEADERS*)(pBase + pDosHeader->e_lfanew);

        auto rvaExports = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        auto pExports = (const IMAGE_EXPORT_DIRECTORY*)(pBase + rvaExports);

        const DWORD* pFuncs = (const DWORD*)(pBase + pExports->AddressOfFunctions);
        const DWORD* pNames = (const DWORD*)(pBase + pExports->AddressOfNames);

        std::vector<const TestFunc*> infos;
        std::vector<const TestTag*> tags;

        for (DWORD i = 0; i < pExports->NumberOfFunctions; i++)
        {
            const void* pfn = (const void*)(pBase + pFuncs[i]);
            const char* pszName = (const char*)(pBase + pNames[i]);

            static constexpr const char TestInfoNamePrefix[] = "getTestInfo_";
            static constexpr const char TagInfoNamePrefix[] = "getTestTag_";
            if (strncmp(pszName, TestInfoNamePrefix, _countof(TestInfoNamePrefix) - 1) == 0)
            {
                using GetInfoFn = const TestFunc* (*)();
                auto pfnGetInfo = (GetInfoFn)pfn;
                infos.push_back(pfnGetInfo());
            }
            else if (strncmp(pszName, TagInfoNamePrefix, _countof(TagInfoNamePrefix) - 1) == 0)
            {
                using GetTagFn = const TestTag* (*)();
                auto pfnGetTag = (GetTagFn)pfn;
                tags.push_back(pfnGetTag());
            }
        }

        // link up all the infos
        std::vector<FullTestInfo> fullInfos;
        for (const TestFunc* pTestFn : infos)
        {
            if (pTestFn != nullptr)
            {
                FullTestInfo fti;
                fti.testFnInfo = pTestFn;

                // tags
                for (const TestTag* pTag : tags)
                {
                    if (strcmp(pTestFn->pszName, pTag->pszTestName) == 0
                        && strcmp(pTestFn->pszFile, pTag->pszTestFile) == 0)
                    {
                        fti.tags.emplace(pTag->pszTag);
                    }
                }

                fullInfos.push_back(std::move(fti));
            }
        }

        return fullInfos;
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


    static void runTest(TestReporter& reporter, const FullTestInfo& fullTestInfo, const TestFilter& filter)
    {
        const TestFunc& testInfo = *fullTestInfo.testFnInfo;

        TestInfo info{
            testInfo.pszName,
            testInfo.pszFile,
            testInfo.count,
            0,
            fullTestInfo.tags
        };

        uintptr_t argsAddr = reinterpret_cast<uintptr_t>(testInfo.arg);

        for (int n = 0;
             n < testInfo.count;
             n++, argsAddr += testInfo.argSize)
        {
            info.rowNumber = 1 + n;
            reporter.testStarting(info);
            TestResult result;

            if (filter.shouldRunTest(info))
            {
                result = runTest(testInfo.pfnTest, (void*)argsAddr);
            }
            else
            {
                result.outcome = TestOutcome::NotRun;
                result.message = "Test did not pass filters.";
            }

            result.pszTestName = testInfo.pszName;
            result.rowNumber = info.rowNumber;
            reporter.testFinished(info, result);
        }
    }

    void runAllTests(TestReporter& reporter)
    {
        runAllTests(reporter, NoFilter::Instance);
    }

    void runAllTests(TestReporter& reporter, const TestFilter& filter)
    {
        // discover tests
        std::vector<FullTestInfo> tests = discoverTests();

        // run tests
        reporter.runStarting();

        for (const auto& testInfo : tests)
        {
            runTest(reporter, testInfo, filter);
        }

        reporter.runFinished();
    }
}
