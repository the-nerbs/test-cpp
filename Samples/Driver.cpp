#include "test-cpp\TestRunner.h"
#include "test-cpp\ConsoleReporter.h"
#include <cstdlib>

int main()
{
    test::ConsoleReporter reporter;
    test::runAllTests(reporter);
    std::system("pause");
}
