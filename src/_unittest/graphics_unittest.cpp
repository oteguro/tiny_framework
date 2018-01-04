// graphics_unittest.cpp 
#include "graphics_unittest.h"

#include <gtest/gtest.h>

#include <tiny_graphics.h>

using namespace testing;

namespace tf_unittest
{
    class UnitTestDirectXApplicationAdapter : public tf::ApplicationAdapter
    {
        tf::Allocator&              m_allocator;
        tf::gpu::Device*            m_device;
        tf::gpu::CommandContext*    m_commandContext;
        tf::gpu::SwapChain*         m_swapChain;
        tf::gpu::SynchronizationObject* m_fence;

        int                         m_frameIndex;
    public:
        UnitTestDirectXApplicationAdapter(const std::wstring& name);

        virtual void                Initialize() override;
        virtual void                Update() override;
        virtual void                Render() override;
        virtual void                Terminate() override;

    }; // class UnitTestDirectXApplicationAdapter 

    UnitTestDirectXApplicationAdapter::UnitTestDirectXApplicationAdapter(const std::wstring& name)
        : ApplicationAdapter(name)
        , m_allocator       (tf::DefaultAllocator())
        , m_device          (nullptr)
        , m_commandContext  (nullptr)
        , m_swapChain       (nullptr)
        , m_fence           (nullptr)
        , m_frameIndex      (-1)
    {
//         static const int kUnitTestFrameCount = 60;
//         SetFrameCount(kUnitTestFrameCount);
    }

    void UnitTestDirectXApplicationAdapter::Initialize()
    {
        m_device = new tf::gpu::Device();
        EXPECT_NE(m_device, nullptr);
        EXPECT_NE(m_device->GetImpl(), nullptr);

        tf::gpu::CommandContextDesc ccDesc;
        m_commandContext = m_device->CreateCommandContext(m_allocator, ccDesc);
        EXPECT_NE(m_commandContext, nullptr);

        tf::gpu::SwapChainDesc scDesc;
        m_swapChain = m_device->CreateSwapChain(m_allocator, *m_commandContext, scDesc);
        EXPECT_NE(m_swapChain, nullptr);

        m_frameIndex = m_swapChain->GetCurrentFrameBufferIndex();
        EXPECT_GE(m_frameIndex, 0);

        m_fence = m_device->CreateSynchronizationObject(m_allocator);
        EXPECT_NE(m_fence, nullptr);
    }

    void UnitTestDirectXApplicationAdapter::Update()
    {
    }

    void UnitTestDirectXApplicationAdapter::Render()
    {
        {
            const float clearColor[] = { 0.0f, 0.25f, 0.25f, 1.0f };

            m_commandContext->Begin(m_frameIndex);

            m_commandContext->SetClearColor(clearColor);
            m_commandContext->SetDefaultSwapChain(*m_swapChain);
            m_commandContext->ClearRenderTarget();



            m_commandContext->End();
        }
        {
            m_commandContext->ExecuteList();
            m_swapChain->Present();
        }
        {
            m_fence->MoveToNextFrame(*m_commandContext, *m_swapChain, m_frameIndex);
        }
    }

    void UnitTestDirectXApplicationAdapter::Terminate()
    {
        m_fence->WaitForGpu(*m_commandContext, m_frameIndex);
    }

    TEST(tiny_graphics, create_device)
    {
        UnitTestDirectXApplicationAdapter adapter(L"tiny_graphics::create_device");
        tf::Application app;
        app.Run(adapter, GetModuleHandle(NULL), 1);


    }



} // tf_unittest 


