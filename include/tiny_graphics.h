// tiny_graphics.h 
// Description : Graphics related definition. 
#pragma once
#define WIN32_LEAN_AND_MEAN
#define BUFFERING_COUNT (2)

#include "tiny_base.h"

#include <cstdint>
#include <string>

#include <windows.h>

namespace tf
{
    class ApplicationAdapter : private NonCopyable
    {
    private:
        const int kApplicationDefaultWidth = 1280;
        const int kApplicationDefaultHeight = 720;

        std::wstring                m_name;
        int                         m_frameLeft;
        uint16_t                    m_width;
        uint16_t                    m_height;

    public:
        ApplicationAdapter(const std::wstring& name);

        virtual void                Initialize() = 0;
        virtual void                Update()     = 0;
        virtual void                Render()     = 0;
        virtual void                Terminate()  = 0;

    public:
        uint16_t                    GetWidth()
        {
            return m_width;
        }

        uint16_t                    GetHeight()
        {
            return m_height;
        }

        const WCHAR*                GetTitle() const
        {
            return m_name.c_str();
        }

        void                        SetFrameCount(int frameCount);
        void                        DeclementFrameCount();
        bool                        FinishByFrameLimit() const;

        void                        ParseCommandLineArgs(WCHAR* argv[], int argc);

    }; // class ApplicationAdapter 

    class Application : private NonCopyable
    {
    public:
        static HWND             s_hwnd;

                 Application();
        virtual ~Application();

        void                        SetFrame(int frameLeft);

        virtual int                 Run(ApplicationAdapter& adapter, HINSTANCE hinstance, int argumentCount);

    }; // class Application 

} // namespace tf 

namespace tf
{
namespace gpu
{
    class DeviceImpl;
    class CommandContextImpl;
    class SwapChainImpl;
    class SynchronizationObjectImpl;

    class CommandContext;
    class SwapChain;
    class SynchronizationObject;

    struct CommandContextDesc
    {
        uint32_t                        m_dummy0;

        CommandContextDesc()
            : m_dummy0(0)
        {
        }

    }; // struct CommandContextDesc 

    struct SwapChainDesc
    {
        const uint16_t  kDefaultSwapChainWidth  = 1280;
        const uint16_t  kDefaultSwapChainHeight = 720;
        const uint8_t   kBufferCount = BUFFERING_COUNT;

        uint16_t                        m_width;
        uint16_t                        m_height;
        uint8_t                         m_bufferCount;

        SwapChainDesc()
            : m_width (kDefaultSwapChainWidth)
            , m_height(kDefaultSwapChainHeight)
            , m_bufferCount(kBufferCount)
        {
        }

    }; // struct SwapChainDesc 



    // The GPU device. 
    class Device
    {
    private:
        DeviceImpl*                     m_impl;
    public:
                 Device();
        virtual ~Device();

        CommandContext*                 CreateCommandContext(Allocator& alloc, const CommandContextDesc& desc=CommandContextDesc());
        SwapChain*                      CreateSwapChain(Allocator& alloc, CommandContext& command, const SwapChainDesc& desc=SwapChainDesc());
        SynchronizationObject*          CreateSynchronizationObject(Allocator& alloc);

        DeviceImpl*                     GetImpl() const;

    }; // class Device 


    class CommandContext //: private NonCopyable
    {
    private:
        friend class Device;
        friend class DeviceImpl;

        CommandContextImpl*             m_impl;

                 CommandContext();
        virtual ~CommandContext();

    public:

        void                            Begin(int frameIndex);
        void                            End  ();

        void                            SetDefaultSwapChain(SwapChain& swapChain);
        void                            SetClearColor(const float clearColorRGBA[4]);
        void                            SetClearDepthStencil(float depth, uint8_t stencilValue);
        void                            ClearRenderTarget();

        void                            ExecuteList();

        CommandContextImpl*             GetImpl() const;

    }; // class CommandContext 

    class SwapChain
    {
    private:
        friend class Device;
        friend class DeviceImpl;

        SwapChainImpl*                  m_impl;

                 SwapChain();
        virtual ~SwapChain();

    public:

        int                             GetCurrentFrameBufferIndex() const;
        void                            Present();

        SwapChainImpl*                  GetImpl() const;

    }; // class SwapChain 

    class SynchronizationObject
    {
        friend class Device;
        friend class DeviceImpl;

        SynchronizationObjectImpl*      m_impl;

                 SynchronizationObject();
        virtual ~SynchronizationObject();

    public:

        void                            WaitForPreviousFrame(CommandContext& command);
        void                            WaitForGpu(CommandContext& command, int frameIndex);
        void                            MoveToNextFrame(CommandContext& command, SwapChain& swapChain, int& frameIndex);

        SynchronizationObjectImpl*      GetImpl() const;

    }; // class SynchronizationObject 

} // namespace gpu 
} // namespace tf 
