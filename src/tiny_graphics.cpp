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

#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

namespace tf
{
    ApplicationAdapter::ApplicationAdapter(const std::wstring& name)
        : m_name(name)
        , m_frameLeft(-1)
        , m_width(kApplicationDefaultWidth)
        , m_height(kApplicationDefaultHeight)
    {
    }

    void ApplicationAdapter::SetFrameCount(int frameCount)
    {
        m_frameLeft = frameCount;
    }

    void ApplicationAdapter::DeclementFrameCount()
    {
        if (m_frameLeft > 0)
        {
            m_frameLeft--;
        }
    }

    bool ApplicationAdapter::FinishByFrameLimit() const
    {
        return (m_frameLeft == 0) ? true : false;
    }

    void ApplicationAdapter::ParseCommandLineArgs(WCHAR* argv[], int argc)
    {
        (void)(argv);
        (void)(argc);
    }

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        ApplicationAdapter* adapter = reinterpret_cast<ApplicationAdapter*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        switch (message)
        {
        case WM_CREATE:
            {
                LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
            }
            return 0;

        case WM_PAINT:
            if (adapter)
            {
                adapter->Update();
                adapter->Render();
                adapter->DeclementFrameCount();
                if (adapter->FinishByFrameLimit())
                {
                    PostQuitMessage(0);
                }
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    HWND Application::s_hwnd = nullptr;

    Application::Application()
    {
    }

    Application::~Application()
    {
    }

    int Application::Run(ApplicationAdapter& adapter, HINSTANCE hinstance, int argumentCount)
    {
        // Parse the command line parameters
        int argc;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        adapter.ParseCommandLineArgs(argv, argc);
        LocalFree(argv);

        // Initialize the window class.
        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = hinstance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = L"DirectX Graphics Sample";
        RegisterClassEx(&windowClass);

        RECT windowRect = { 0, 0, static_cast<LONG>(adapter.GetWidth()), static_cast<LONG>(adapter.GetHeight()) };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        // Create the window and store a handle to it.
        Application::s_hwnd = CreateWindow(
            windowClass.lpszClassName,
            adapter.GetTitle(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right  - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,		// We have no parent window.
            nullptr,		// We aren't using menus.
            hinstance,
            &adapter);

        // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
        adapter.Initialize();
        ShowWindow(Application::s_hwnd, argumentCount);

        // Main sample loop.
        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            // Process any messages in the queue.
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage (&msg);
            }
        }

        adapter.Terminate();

        // Return this part of the WM_QUIT message to Windows.
        return static_cast<char>(msg.wParam);
    }


} // namespace tf 

namespace tf
{
namespace gpu
{
    class CommandContextImpl
    {
    private:
        ComPtr<ID3D12CommandQueue>          m_commandQueue;
        ComPtr<ID3D12CommandAllocator>      m_commandAllocators[BUFFERING_COUNT];
        ComPtr<ID3D12GraphicsCommandList>   m_commandList;
        ComPtr<ID3D12PipelineState>         m_pipelineState;

        ID3D12Resource*                     m_currentRtvResource;
        CD3DX12_CPU_DESCRIPTOR_HANDLE       m_rtvHandle;

        float                               m_clearColor[4];
        float                               m_clearDepth;
        uint8_t                             m_clearStencil;

    public:
        CommandContextImpl()
            : m_commandQueue        (nullptr)
            , m_commandAllocators   {}
            , m_commandList         (nullptr)
            , m_pipelineState       (nullptr)
            , m_currentRtvResource  (nullptr)
            , m_rtvHandle           ()
            , m_clearDepth          (1.0f)
            , m_clearStencil        (0)
        {
            for (int i = 0; i < BUFFERING_COUNT; ++i)
            {
                m_commandAllocators[i] = nullptr;
            }
            for (int i = 0; i < 4; ++i)
            {
                m_clearColor[i] = 0.0f;
            }
        }

        void                            Initialize(ID3D12Device* device);
        void                            Terminate ();

        void                            Begin(int frameIndex);
        void                            End();

        void                            SetDefaultSwapChain(SwapChainImpl& swapChain);
        void                            SetClearColor(const float clearColorRGBA[4]);
        void                            SetClearDepthStencil(float depth, uint8_t stencil);
        void                            ClearRenderTarget();

        void                            ExecuteList();

        ID3D12CommandQueue*             GetNativeCommandQueue() const
        {
            return m_commandQueue.Get();
        }

    }; // class CommandContextImpl 

    class SwapChainImpl
    {
    private:
        ComPtr<IDXGISwapChain3>         m_swapChain;
        ComPtr<ID3D12DescriptorHeap>    m_renderTargetViewHeap;
        ComPtr<ID3D12Resource>          m_renderTargets[BUFFERING_COUNT];

        UINT                            m_renderTargetViewDescriptorSize;
    public:
        SwapChainImpl()
            : m_swapChain           (nullptr)
            , m_renderTargetViewHeap(nullptr)
            , m_renderTargetViewDescriptorSize(0L)
        {
            for (int i = 0; i < BUFFERING_COUNT; ++i)
            {
                m_renderTargets[i] = nullptr;
            }
        }

        void                            Initialize(ID3D12Device*        pDevice,
                                                   IDXGIFactory4*       pFactory,
                                                   CommandContext&      command,
                                                   const SwapChainDesc& desc);
        void                            Terminate ();

        int                             GetCurrentFrameBufferIndex() const
        {
            assert(m_swapChain != nullptr);
            return m_swapChain->GetCurrentBackBufferIndex();
        }

        void                            Present();

        IDXGISwapChain3*                GetSwapChain() const
        {
            return m_swapChain.Get();
        }

        ID3D12Resource*                 GetCurrentFrameBufferResource() const
        {
            return m_renderTargets[GetCurrentFrameBufferIndex()].Get();
        }

        ID3D12DescriptorHeap*           GetRenderTargetViewHeap() const
        {
            return m_renderTargetViewHeap.Get();
        }

        UINT                            GetRenderTargetViewDescriptorSize() const
        {
            return m_renderTargetViewDescriptorSize;
        }

    }; // class SwapChainImpl 

    void SwapChainImpl::Initialize(ID3D12Device*    pDevice,
                                   IDXGIFactory4*   pFactory,
                                   CommandContext&  command,
                                   const SwapChainDesc& desc)
    {
        DXGI_SWAP_CHAIN_DESC1 scd = {};
        scd.BufferCount     = desc.m_bufferCount;
        scd.Width           = desc.m_width;
        scd.Height          = desc.m_height;
        scd.Format          = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage     = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.SwapEffect      = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scd.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        pFactory->CreateSwapChainForHwnd(
            command.GetImpl()->GetNativeCommandQueue(),
            Application::s_hwnd,
            &scd,
            nullptr,
            nullptr,
            &swapChain);

        pFactory->MakeWindowAssociation(Application::s_hwnd, DXGI_MWA_NO_ALT_ENTER);

        swapChain.As(&m_swapChain);

        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors  = desc.m_bufferCount;
            rtvHeapDesc.Type            = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags           = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_renderTargetViewHeap));

            m_renderTargetViewDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (UINT n = 0; n < desc.m_bufferCount; n++)
            {
                m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
                pDevice->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_renderTargetViewDescriptorSize);
            }
        }
    }

    void SwapChainImpl::Present()
    {
        m_swapChain->Present(1, 0);
    }

    void SwapChainImpl::Terminate()
    {
    }


    class SynchronizationObjectImpl
    {
    private:
        HANDLE                          m_fenceEvent;
        ComPtr<ID3D12Fence>             m_fence;
        UINT64                          m_fenceValues[BUFFERING_COUNT];

    public:
        SynchronizationObjectImpl()
            : m_fenceEvent  (nullptr)
            , m_fence       (nullptr)
            , m_fenceValues ()
        {
            for (int i = 0; i < BUFFERING_COUNT; ++i)
            {
                m_fenceValues[i] = 0ull;
            }
        }

        ~SynchronizationObjectImpl();

        void                            Initialize(ID3D12Device* pDevice);

        void                            WaitForPreviousFrame(CommandContextImpl& commandContextImpl);

        void                            WaitForGpu(ID3D12CommandQueue* commandQueue, int frameIndex);

        void                            MoveToNextFrame(ID3D12CommandQueue* commandQueue, IDXGISwapChain3* swapChain, int& frameIndex);

    }; // class SynchronizationObjectImpl 

    SynchronizationObjectImpl::~SynchronizationObjectImpl()
    {
        CloseHandle(m_fenceEvent);
    }

    void SynchronizationObjectImpl::Initialize(ID3D12Device* pDevice)
    {
        pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
        m_fenceValues[0] = 1ull;

        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        assert(m_fenceEvent != nullptr);
    }

    void SynchronizationObjectImpl::WaitForPreviousFrame(CommandContextImpl& commandContextImpl)
    {
        const UINT64 fence = m_fenceValues[0];
        commandContextImpl.GetNativeCommandQueue()->Signal(m_fence.Get(), fence);
        m_fenceValues[0]++;

        // Wait until the previous frame is finished.
        if (m_fence->GetCompletedValue() < fence)
        {
            m_fence->SetEventOnCompletion(fence, m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    void SynchronizationObjectImpl::WaitForGpu(ID3D12CommandQueue* commandQueue, int frameIndex)
    {
        commandQueue->Signal(m_fence.Get(), m_fenceValues[frameIndex]);

        m_fence->SetEventOnCompletion(m_fenceValues[frameIndex], m_fenceEvent);
        WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

        m_fenceValues[frameIndex]++;
    }

    void SynchronizationObjectImpl::MoveToNextFrame(ID3D12CommandQueue* commandQueue, IDXGISwapChain3* swapChain, int& frameIndex)
    {
        const UINT64 currentFenceValue = m_fenceValues[frameIndex];
        commandQueue->Signal(m_fence.Get(), currentFenceValue);

        frameIndex = swapChain->GetCurrentBackBufferIndex();

        if (m_fence->GetCompletedValue() < m_fenceValues[frameIndex])
        {
            m_fence->SetEventOnCompletion(m_fenceValues[frameIndex], m_fenceEvent);
            WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
        }

        m_fenceValues[frameIndex] = currentFenceValue + 1;
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

    void CommandContextImpl::Initialize(ID3D12Device* device)
    {
        assert(device != nullptr); // please create device before create command context. 

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
        assert(m_commandQueue != nullptr);

        for (int i = 0; i < BUFFERING_COUNT; ++i)
        {
            device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
        }

        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));
        m_commandList->Close();
    }

    void CommandContextImpl::Terminate()
    {
    }

    void CommandContextImpl::Begin(int frameIndex)
    {
        m_commandAllocators[frameIndex]->Reset();
        m_commandList->Reset(m_commandAllocators[frameIndex].Get(), m_pipelineState.Get());
    }

    void CommandContextImpl::End()
    {
        if (m_currentRtvResource)
        {
            m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_currentRtvResource,
                                                                                    D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                                    D3D12_RESOURCE_STATE_PRESENT));
            m_currentRtvResource = nullptr;
        }
        m_commandList->Close();
    }

    void CommandContextImpl::SetDefaultSwapChain(SwapChainImpl& swapChain)
    {
        m_currentRtvResource = swapChain.GetCurrentFrameBufferResource();
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_currentRtvResource,
                                                                                D3D12_RESOURCE_STATE_PRESENT,
                                                                                D3D12_RESOURCE_STATE_RENDER_TARGET));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(swapChain.GetRenderTargetViewHeap()->GetCPUDescriptorHandleForHeapStart(),
                                                swapChain.GetCurrentFrameBufferIndex(),
                                                swapChain.GetRenderTargetViewDescriptorSize());
        m_rtvHandle = rtvHandle;
    }

    void CommandContextImpl::SetClearColor(const float clearColorRGBA[4])
    {
        for (int i = 0; i < 4; ++i)
        {
            m_clearColor[i] = clearColorRGBA[i];
        }
    }

    void CommandContextImpl::SetClearDepthStencil(float depth, uint8_t stencil)
    {
        m_clearDepth = depth;
        m_clearStencil = stencil;
    }

    void CommandContextImpl::ClearRenderTarget()
    {
        m_commandList->ClearRenderTargetView(m_rtvHandle, m_clearColor, 0, nullptr);
    }

    void CommandContextImpl::ExecuteList()
    {
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    }

    class DeviceImpl
    {
    private:
        ComPtr<IDXGIFactory4>           m_dxgiFactory;
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

        CommandContext*                 CreateCommandContext(Allocator& alloc, const CommandContextDesc& desc);
        SwapChain*                      CreateSwapChain(Allocator& alloc, CommandContext& command, const SwapChainDesc& desc);
        SynchronizationObject*          CreateSynchronizationObject(Allocator& alloc);

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

        CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory));

        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;
        if(m_useWarpDevice)
        {
            ComPtr<IDXGIAdapter> warpAdapter;
            m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

            D3D12CreateDevice(warpAdapter.Get(), featureLevel, IID_PPV_ARGS(&m_device));
        }
        else
        {
            ComPtr<IDXGIAdapter1> hardwareAdapter;
            GetHardwareAdapter(m_dxgiFactory.Get(), &hardwareAdapter);

            D3D12CreateDevice(hardwareAdapter.Get(), featureLevel, IID_PPV_ARGS(&m_device));
        }

    }

    void DeviceImpl::Terminate()
    {

    }

    CommandContext* DeviceImpl::CreateCommandContext(Allocator& alloc, const CommandContextDesc& desc)
    {
        CommandContext* createdContext = new CommandContext();
        assert(createdContext->m_impl != nullptr);
        createdContext->m_impl->Initialize(m_device.Get());

        return createdContext;
    }

    SwapChain* DeviceImpl::CreateSwapChain(Allocator& alloc, CommandContext& command, const SwapChainDesc& desc)
    {
        SwapChain* createdSwapChain = new SwapChain();
        assert(createdSwapChain->m_impl != nullptr);
        createdSwapChain->m_impl->Initialize(m_device.Get(), m_dxgiFactory.Get(), command, desc);

        return createdSwapChain;
    }

    SynchronizationObject* DeviceImpl::CreateSynchronizationObject(Allocator& alloc)
    {
        SynchronizationObject* createdSynchronizationObject = new SynchronizationObject();
        assert(createdSynchronizationObject != nullptr);
        createdSynchronizationObject->m_impl->Initialize(m_device.Get());

        return createdSynchronizationObject;
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

    CommandContext* Device::CreateCommandContext(Allocator& alloc, const CommandContextDesc& desc)
    {
        assert(m_impl != nullptr);
        if (m_impl == nullptr)
        {
            return nullptr;
        }
        return m_impl->CreateCommandContext(alloc, desc);
    }

    SwapChain* Device::CreateSwapChain(Allocator& alloc, CommandContext& command, const SwapChainDesc& desc)
    {
        assert(m_impl != nullptr);
        if (m_impl == nullptr)
        {
            return nullptr;
        }
        return m_impl->CreateSwapChain(alloc, command, desc);
    }

    SynchronizationObject* Device::CreateSynchronizationObject(Allocator& alloc)
    {
        assert(m_impl != nullptr);
        if (m_impl == nullptr)
        {
            return nullptr;
        }
        return m_impl->CreateSynchronizationObject(alloc);
    }

    DeviceImpl* Device::GetImpl() const
    {
        return m_impl;
    }

    CommandContext::CommandContext()
        : m_impl(nullptr)
    {
        m_impl = new CommandContextImpl;
    }

    CommandContext::~CommandContext()
    {
        assert(m_impl != nullptr);
        delete m_impl;
        m_impl = nullptr;
    }

    void CommandContext::Begin(int frameIndex)
    {
        assert(m_impl != nullptr);
        m_impl->Begin(frameIndex);
    }

    void CommandContext::End()
    {
        assert(m_impl != nullptr);
        m_impl->End();
    }

    void CommandContext::SetDefaultSwapChain(SwapChain& swapChain)
    {
        assert(m_impl != nullptr);
        m_impl->SetDefaultSwapChain(*(swapChain.GetImpl()));
    }

    void CommandContext::SetClearColor(const float clearColorRGBA[4])
    {
        assert(m_impl != nullptr);
        m_impl->SetClearColor(clearColorRGBA);
    }

    void CommandContext::SetClearDepthStencil(float depth, uint8_t stencilValue)
    {
        assert(m_impl != nullptr);

    }

    void CommandContext::ClearRenderTarget()
    {
        assert(m_impl != nullptr);
        m_impl->ClearRenderTarget();
    }

    void CommandContext::ExecuteList()
    {
        assert(m_impl != nullptr);
        m_impl->ExecuteList();
    }

    CommandContextImpl * CommandContext::GetImpl() const
    {
        return m_impl;
    }

    SwapChain::SwapChain()
        : m_impl(nullptr)
    {
        m_impl = new SwapChainImpl;
    }

    SwapChain::~SwapChain()
    {
        assert(m_impl != nullptr);
        delete m_impl;
        m_impl = nullptr;
    }

    int SwapChain::GetCurrentFrameBufferIndex() const
    {
        assert(m_impl != nullptr);
        return m_impl->GetCurrentFrameBufferIndex();
    }

    void SwapChain::Present()
    {
        assert(m_impl != nullptr);
        m_impl->Present();
    }

    SwapChainImpl* SwapChain::GetImpl() const
    {
        return m_impl;
    }

    SynchronizationObject::SynchronizationObject()
        : m_impl(nullptr)
    {
        m_impl = new SynchronizationObjectImpl;
    }

    SynchronizationObject::~SynchronizationObject()
    {
        assert(m_impl != nullptr);
        delete m_impl;
        m_impl = nullptr;
    }

    void SynchronizationObject::WaitForPreviousFrame(CommandContext& command)
    {
        assert(m_impl != nullptr);
        m_impl->WaitForPreviousFrame(*(command.GetImpl()));
    }

    void SynchronizationObject::WaitForGpu(CommandContext& command, int frameIndex)
    {
        assert(m_impl != nullptr);
        m_impl->WaitForGpu(command.GetImpl()->GetNativeCommandQueue(), frameIndex);
    }

    void SynchronizationObject::MoveToNextFrame(CommandContext& command, SwapChain& swapChain, int& frameIndex)
    {
        assert(m_impl != nullptr);
        m_impl->MoveToNextFrame(command.GetImpl()->GetNativeCommandQueue(), swapChain.GetImpl()->GetSwapChain(), frameIndex);
    }

    SynchronizationObjectImpl* SynchronizationObject::GetImpl() const
    {
        return m_impl;
    }


} // namespace gpu 
} // namespace tf 
