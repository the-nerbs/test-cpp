#include "TestRunner.h"
#include "TestResult.h"
#include "Test.h"
#include "Stringify.h"
#include "AssertExceptions.h"

#include <vector>
#include <cstdint>
#include <cstdio>

#include <io.h>

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


    static void runTest(TestReporter& reporter, const TestFunc& testInfo)
    {
        TestInfo info{
            testInfo.pszName,
            testInfo.pszFile,
            testInfo.count,
            0
        };

        if (testInfo.count == 1 && testInfo.argSize == 0)
        {
            info.rowNumber = 1;
            reporter.TestStarting(info);

            TestResult result = runTest(testInfo.pfnTest, nullptr);
            result.pszTestName = testInfo.pszName;
            result.rowNumber = info.rowNumber;

            reporter.TestFinished(info, result);
        }
        else if (testInfo.argSize > 0)
        {
            uintptr_t argsAddr = reinterpret_cast<uintptr_t>(testInfo.arg);

            for (int n = 0;
                n < testInfo.count;
                n++, argsAddr += testInfo.argSize)
            {
                info.rowNumber = 1+n;
                reporter.TestStarting(info);

                TestResult result = runTest(testInfo.pfnTest, (void*)argsAddr);
                result.pszTestName = testInfo.pszName;
                result.rowNumber = info.rowNumber;

                reporter.TestFinished(info, result);
            }
        }
    }

    void runAllTests(TestReporter& reporter)
    {
        // discover tests
        std::vector<const TestFunc*> tests = discoverTests();

        // run tests
        reporter.RunStarting();

        for (const auto* pTestInfo : tests)
        {
            if (pTestInfo != nullptr)
            {
                runTest(reporter, *pTestInfo);
            }
        }

        reporter.RunFinished();
    }
}
