#include "test-cpp\include\TestRunner.h"
#include "test-cpp\include\ConsoleReporter.h"
#include <cstdlib>

int main()
{
    test::ConsoleReporter reporter;
    test::runAllTests(reporter);
    std::system("pause");
}
