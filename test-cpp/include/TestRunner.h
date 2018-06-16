#pragma once
#include "TestReporter.h"
#include "TestFilter.h"

namespace test
{
    void runAllTests(TestReporter& reporter);
    void runAllTests(TestReporter& reporter, const TestFilter& filter);
}
