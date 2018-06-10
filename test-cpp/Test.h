#pragma once

namespace test
{
    namespace details
    {
        using TestFn = void (__cdecl *)(void*);

        struct TestFunc
        {
            TestFn pfnTest;
            const char* pszName;
            const char* pszFile;
            void* arg;
            int count;
            int argSize;
        };
    }
}

#define GTI_FN_NAME(t)      getTestInfo_ ## t
#define COUNT(arr)          (sizeof(arr) / sizeof(arr[0]))
#define ARGSIZE(arr)        (sizeof(arr[0]))
#define PPSTRING2(x)        # x
#define PPSTRING(x)         PPSTRING2(x)

#define TEST(fn)                                        \
    extern "C" __declspec(dllexport)                    \
    const ::test::details::TestFunc* GTI_FN_NAME(fn)()  \
    {                                                   \
        static const ::test::details::TestFunc info{    \
            (::test::details::TestFn)fn,                \
            PPSTRING(fn),                               \
            __FILE__,                                   \
            nullptr,                                    \
            1, 0                                        \
        };                                              \
        return &info;                                   \
    }


#define ROW_TEST(fn, args)                              \
    extern "C" __declspec(dllexport)                    \
    const ::test::details::TestFunc* GTI_FN_NAME(fn)()  \
    {                                                   \
        static const ::test::details::TestFunc info{    \
            (::test::details::TestFn)fn,                \
            PPSTRING(fn),                               \
            __FILE__,                                   \
            (void*)&args,                               \
            COUNT(args), ARGSIZE(args)                  \
        };                                              \
        return &info;                                   \
    }
