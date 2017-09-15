// graphics_unittest.cpp 
#include "graphics_unittest.h"

#include <gtest/gtest.h>

#include <tiny_graphics.h>

using namespace testing;

namespace tf_unittest
{
    TEST(tiny_graphics, create_device)
    {
        tf::GpuDevice* pDevice = new tf::GpuDevice();
        EXPECT_NE(pDevice, nullptr);
        EXPECT_NE(pDevice->GetImpl(), nullptr);
        delete pDevice;
    }



} // tf_unittest 


