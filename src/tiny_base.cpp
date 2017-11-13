// tiny_base.cpp 
#define WIN32_LEAN_AND_MEAN
#include <tiny_base.h>
#include <malloc.h>

namespace tf
{

    Allocator::Allocator()
    {
    }

    Allocator::~Allocator()
    {
    }

    class DefaultMemoryAllocator : public Allocator
    {

    public:
        DefaultMemoryAllocator()
        {
        }

        virtual ~DefaultMemoryAllocator()
        {
        }

        virtual void* Allocate(size_t size, size_t alignment) override
        {
            return _aligned_malloc(size, alignment);
        }

        virtual void Free(void* block)
        {
            _aligned_free(block);
        }

    }; // class DefaultMemoryAllocator 

    static DefaultMemoryAllocator s_defaultMemoryAllocator;

    Allocator& DefaultAllocator()
    {
        return s_defaultMemoryAllocator;
    }

} // namespace tf 

