// base_unittest.cpp 
// Description : Baseモジュールのユニットテスト. 
#include <_unittest/unittest.h>

#include <gtest/gtest.h>

#include <tiny_base.h>

using namespace testing;

namespace tf_unittest
{
    TEST(tiny_base, alignment)
    {
        EXPECT_EQ(TF_ALIGNMENT(15, 16), 16);

    }

    TEST(tiny_base, scope_exit)
    {
        void* ptr = nullptr;
        {
            ptr = malloc(sizeof(char));
            TF_SCOPE_EXIT(free(ptr); ptr=nullptr;);
        }
        EXPECT_EQ(ptr, nullptr);
    }

    TEST(tiny_base, allocator_basic)
    {
        tf::Allocator& alloc = tf::DefaultAllocator();
        {
            void* ptr = nullptr;
            ptr = alloc.Allocate(1024);

            EXPECT_NE(ptr, nullptr);

            alloc.Free(ptr);
        }



    }



} // namespace unittest 



