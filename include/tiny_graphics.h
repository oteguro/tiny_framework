// tiny_graphics.h 
// Description : graphics related gotcha. 
#pragma once

#include "tiny_base.h"

#include <cstdint>

namespace tf
{
namespace gpu
{
    class DeviceImpl;
    class CommandContextImpl;

    class CommandContext;


    struct CommandContextDesc
    {
        uint32_t                        m_dummy0;

        CommandContextDesc()
            : m_dummy0(0)
        {
        }

    }; // struct CommandContextDesc 



    // The GPU device. 
    class Device
    {
    private:
        DeviceImpl*                  m_impl;
    public:
                 Device();
        virtual ~Device();

        CommandContext*              CreateCommandContext(Allocator& alloc, CommandContextDesc& desc);



        DeviceImpl*                  GetImpl() const;

    }; // class Device 


    class CommandContext //: private NonCopyable
    {
    private:
        friend class Device;
        friend class DeviceImpl;

        CommandContextImpl*          m_impl;

                 CommandContext();
        virtual ~CommandContext();

    public:





        CommandContextImpl*          GetImpl() const;

    }; // class CommandContext 

} // namespace gpu 
} // namespace tf 

