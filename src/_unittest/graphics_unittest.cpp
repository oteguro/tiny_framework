// graphics_unittest.cpp 
#include "graphics_unittest.h"

#include <gtest/gtest.h>

#include <tiny_graphics.h>

using namespace testing;

namespace tf_unittest
{
    TEST(tiny_graphics, create_device)
    {
        tf::Allocator& alloc = tf::DefaultAllocator();

        tf::gpu::Device* pDevice = new tf::gpu::Device();
        EXPECT_NE(pDevice, nullptr);
        EXPECT_NE(pDevice->GetImpl(), nullptr);
        TF_SCOPE_EXIT(delete pDevice);


        tf::gpu::CommandContextDesc ccDesc;
        tf::gpu::CommandContext* pCommandContext = pDevice->CreateCommandContext(alloc, ccDesc);
        EXPECT_NE(pCommandContext, nullptr);




    }



} // tf_unittest 


