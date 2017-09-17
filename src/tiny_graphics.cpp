// tiny_graphics.cpp 
#include <tiny_graphics.h>

#include <cassert>

namespace tf
{
    class GpuDeviceImpl
    {

    public:
        GpuDeviceImpl()
        {

        }

        ~GpuDeviceImpl()
        {

        }

        void                            Initialize();
        void                            Terminate ();

    }; // class GpuDeviceImpl 

    void GpuDeviceImpl::Initialize()
    {

    }

    void GpuDeviceImpl::Terminate()
    {

    }

    GpuDevice::GpuDevice()
        : m_impl(nullptr)
    {
        m_impl = new GpuDeviceImpl();
    }

    GpuDevice::~GpuDevice()
    {
        assert(m_impl != nullptr);
        delete m_impl;
        m_impl = nullptr;
    }

    GpuDeviceImpl* GpuDevice::GetImpl() const
    {
        return m_impl;
    }


} // namespace tf 
