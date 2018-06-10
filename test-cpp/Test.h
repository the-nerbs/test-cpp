#pragma once
#include <type_traits>

#define TEST_CALLCONV   __cdecl

namespace test
{
    namespace details
    {
        using TestFn = void (TEST_CALLCONV *)(void*);

        struct TestFunc
        {
            TestFn pfnTest;
            const char* pszName;
            const char* pszFile;
            void* arg;
            int count;
            int argSize;
        };

        // calling convention checking
        // must be a caller-cleanup, non-member convention (only __cdecl for MSVC)
        // caller-cleanup needed since we always pass 1 arg, even to 0-arg tests.
        // TODO: write a wrapper for 0-arg tests so I can relax this restriction a bit.
        template <typename TFn>
        struct is_valid_test_callconv : std::false_type {};

        template <typename TRet, typename... TArgs>
        struct is_valid_test_callconv<TRet TEST_CALLCONV(TArgs...)> : std::true_type {};


        // argument checking
        // must be 0 args, or 1 const& argument.
        template <typename TFn>
        struct is_valid_test_args : std::false_type {};

        template <typename TRet>
        struct is_valid_test_args<TRet()> : std::true_type {};


        template <typename TFn>
        struct is_valid_rowtest_args : std::false_type {};

        template <typename TRet, typename TArg>
        struct is_valid_rowtest_args<TRet(const TArg&)> : std::true_type {};


        // return checking
        // must be void return
        template <typename TFn>
        struct is_valid_test_return : std::false_type {};

        template <typename TRet, typename... TArgs>
        struct is_valid_test_return<TRet(TArgs...)> : std::bool_constant<
            std::is_same<TRet, void>::value
        > {};
    }
}

#define GTI_FN_NAME(t)      getTestInfo_ ## t
#define COUNT(arr)          (sizeof(arr) / sizeof(arr[0]))
#define ARGSIZE(arr)        (sizeof(arr[0]))
#define PPSTRING2(x)        # x
#define PPSTRING(x)         PPSTRING2(x)

#define STATIC_ASSERT_VALID_TEST_FN(fn)                                         \
    static_assert(                                                              \
        ::test::details::is_valid_test_callconv<decltype(fn)>::value,           \
        PPSTRING(fn) ": test functions must be " PPSTRING(TEST_CALLCONV));      \
    static_assert(                                                              \
        ::test::details::is_valid_test_return<decltype(fn)>::value,             \
        PPSTRING(fn) ": test functions must have a void return type.")


#define TEST(fn)                                                                \
    STATIC_ASSERT_VALID_TEST_FN(fn);                                            \
    static_assert(                                                              \
        ::test::details::is_valid_test_args<decltype(fn)>::value,               \
        PPSTRING(fn) ": test functions must take no arguments.");               \
    extern "C" __declspec(dllexport)                                            \
    const ::test::details::TestFunc* GTI_FN_NAME(fn)()                          \
    {                                                                           \
        static const ::test::details::TestFunc info{                            \
            (::test::details::TestFn)fn,                                        \
            PPSTRING(fn),                                                       \
            __FILE__,                                                           \
            nullptr,                                                            \
            1, 0                                                                \
        };                                                                      \
        return &info;                                                           \
    }


#define ROW_TEST(fn, args)                                                      \
    STATIC_ASSERT_VALID_TEST_FN(fn);                                            \
    static_assert(                                                              \
        ::test::details::is_valid_rowtest_args<decltype(fn)>::value,            \
        PPSTRING(fn) ": row test functions must take 1 const& argument.");      \
    extern "C" __declspec(dllexport)                                            \
    const ::test::details::TestFunc* GTI_FN_NAME(fn)()                          \
    {                                                                           \
        static const ::test::details::TestFunc info{                            \
            (::test::details::TestFn)fn,                                        \
            PPSTRING(fn),                                                       \
            __FILE__,                                                           \
            (void*)&args,                                                       \
            COUNT(args), ARGSIZE(args)                                          \
        };                                                                      \
        return &info;                                                           \
    }
