#include "test-cpp\include\Test.h"
#include "test-cpp\include\Assert.h"

struct TestRow
{
    int x, y, z;
};

const TestRow sampleRowTestRows[]{
    { 0, 1, 2 },
    { 0, 2, 1 },
};

void sampleRowTest(const TestRow& row)
{
    test::Assert::areEqual(0, row.x, "expected X to be 0");
    test::Assert::areNotEqual(row.y, row.z, "expected y to not be equal to z");
}
ROW_TEST(sampleRowTest, sampleRowTestRows);
TAG_TEST(sampleRowTest, "samples");
TAG_TEST(sampleRowTest, "row test");
