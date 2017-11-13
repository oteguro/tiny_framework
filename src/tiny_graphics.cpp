// tiny_graphics.cpp 
#include <tiny_graphics.h>

#include <windows.h>

#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <cassert>
#include <cstring>
#include <wrl.h>
#include <shellapi.h>

using Microsoft::WRL::ComPtr;

namespace tf
{
namespace gpu
{
    class CommandContextImpl
    {
    private:
        ComPtr<ID3D12CommandQueue>      m_commandQueue;
    public:
        CommandContextImpl()
            : m_commandQueue(nullptr)
        {
        }

        void                            Initialize(ID3D12Device* device);
        void                            Terminate ();

    }; // class CommandContextImpl 

    void CommandContextImpl::Initialize(ID3D12Device* device)
    {
        assert(device != nullptr); // please create device before create commandcontext. 

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
        assert(m_commandQueue != nullptr);
    }

    void CommandContextImpl::Terminate()
    {
    }


    void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
    {
        ComPtr<IDXGIAdapter1> adapter;
        *ppAdapter = nullptr;

        for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }

        *ppAdapter = adapter.Detach();
    }

    class DeviceImpl
    {
    private:
        ComPtr<ID3D12Device>            m_device;
        bool                            m_useWarpDevice;

    public:
        DeviceImpl()
            : m_device          (nullptr)
            , m_useWarpDevice   (false)
        {

        }

        ~DeviceImpl()
        {

        }

        void                            Initialize();
        void                            Terminate ();

        CommandContext*                 CreateCommandContext(Allocator& alloc, CommandContextDesc& desc);


    }; // class GpuDeviceImpl 

    void DeviceImpl::Initialize()
    {
        UINT dxgiFactoryFlags = 0;

#if defined(TF_DEBUG)
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif // TF_DEBUG 

        ComPtr<IDXGIFactory4> dxgiFactory;
        CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory));

        if(m_useWarpDevice)
        {
            ComPtr<IDXGIAdapter> warpAdapter;
            dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

            D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
        }
        else
        {
            ComPtr<IDXGIAdapter1> hardwareAdapter;
            GetHardwareAdapter(dxgiFactory.Get(), &hardwareAdapter);

            D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
        }

    }

    void DeviceImpl::Terminate()
    {

    }

    CommandContext* DeviceImpl::CreateCommandContext(Allocator& alloc, CommandContextDesc& desc)
    {
        CommandContext* createdContext = new CommandContext();
        assert(createdContext->m_impl != nullptr);
        createdContext->m_impl->Initialize(m_device.Get());

        return createdContext;
    }

    Device::Device()
        : m_impl(nullptr)
    {
        m_impl = new DeviceImpl();
        m_impl->Initialize();
    }

    Device::~Device()
    {
        assert(m_impl != nullptr);
        m_impl->Terminate();
        delete m_impl;
        m_impl = nullptr;
    }

    CommandContext* Device::CreateCommandContext(Allocator& alloc, CommandContextDesc& desc)
    {
        assert(m_impl != nullptr);
        if (m_impl == nullptr)
        {
            return nullptr;
        }
        return m_impl->CreateCommandContext(alloc, desc);
    }


    DeviceImpl* Device::GetImpl() const
    {
        return m_impl;
    }

    CommandContext::CommandContext()
        : m_impl(nullptr)
    {
        m_impl = new CommandContextImpl();
    }

    CommandContext::~CommandContext()
    {
        assert(m_impl != nullptr);
        delete m_impl;
        m_impl = nullptr;
    }

    CommandContextImpl * CommandContext::GetImpl() const
    {
        return m_impl;
    }

} // namespace gpu 
} // namespace tf 
