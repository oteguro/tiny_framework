// main_unittest.cpp
#include <gtest/gtest.h>

// Empty test. 
TEST(main_unittest, empty_test)
{
    EXPECT_EQ(0, 0);
}

// Starts from here. 
int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    int    retval = RUN_ALL_TESTS();
    return retval;

}