// tiny_graphics.cpp 
#include <tiny_graphics.h>

namespace tf
{
    GpuDevice::GpuDevice()
        : m_impl(nullptr)
    {
    }

    GpuDevice::~GpuDevice()
    {
    }

    GpuDeviceImpl* GpuDevice::GetImpl() const
    {
        return m_impl;
    }


} // namespace tf 
