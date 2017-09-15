// tiny_graphics.h 
// Description : graphics related gotcha. 
#pragma once

#include "tiny_base.h"


namespace tf
{
    class GpuDeviceImpl;

    // The GPU 
    class GpuDevice
    {
    private:
        GpuDeviceImpl*                  m_impl;
    public:
                 GpuDevice();
        virtual ~GpuDevice();


        GpuDeviceImpl*                  GetImpl() const;

    }; // class GpuDevice 


} // namespace tf 

