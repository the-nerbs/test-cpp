#include "test-cpp\include\TestRunner.h"
#include "test-cpp\include\ConsoleReporter.h"
#include "test-cpp\include\TestFilter.h"
#include <cstdlib>


class TagFilter :
    public test::TestFilter
{
public:
    TagFilter(std::string tag)
        : _tag{ std::move(tag) }
    { }

    bool shouldRunTest(const test::TestInfo& test) const override
    {
        return (test.tags.find(_tag) != test.tags.cend());
    }

private:
    std::string _tag;
};


int main(int argc, const char* argv[])
{
    bool filter = true;

    for (int i = 1; i < argc; i++)
    {
        constexpr char FilterArg[] = "filter";
        filter |= (strncmp(argv[i], FilterArg, sizeof(FilterArg)/sizeof(FilterArg[0])) == 0);
    }

    TagFilter tagFilter{ "row test" };

    const test::TestFilter& testFilter = filter
        ? (const test::TestFilter&)tagFilter
        : (const test::TestFilter&)test::NoFilter::Instance;

    test::ConsoleReporter reporter;
    test::runAllTests(reporter, testFilter);
    std::system("pause");
}
