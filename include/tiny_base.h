// tiny_base.h 
#pragma once

#if defined(_DEBUG)
#define TF_DEBUG                (1)
#endif

#define TF_PLATFORM_WINDOWS     (1)
#define TF_COMPILER_MSVC        (1)

#if defined(TF_COMPILER_MSVC)
    #define TF_FORCE_INLINE                 __forceinline
    #define TF_NO_INLINE                    __declspec(noinline)
    #define TF_FASTCALL                     __fastcall
    #define TF_PLATFORM_DEBUG_BREAK()       __debugbreak()
    #define TF_THREAD_LS                    __declspec(thread)
    #define TF_LIKELY(cond)                 (cond)
    #define TF_UNLIKELY(cond)               (cond)
    #define TF_CACHELINE_SIZE               16
    #define TF_CACHELINE_ALIGNED            __declspec(align(TF_CACHELINE_SIZE))
    #define TF_FUNCTION                     __FUNCSIG__
#elif defined(TF_COMPILER_GCC) || defined(TF_COMPILER_CLANG)
    #define TF_FORCE_INLINE                 inline __attribute__((always_inline))
    #define TF_NO_INLINE                    __attribute__((noinline))
    #define TF_FASTCALL                     __attribute__((fastcall))
    #define TF_PLATFORM_DEBUG_BREAK()       __builtin_trap()
    #define TF_THREAD_LS                    __thread
    #define TF_LIKELY(cond)                 __builtin_expect(!!(cond), 1)
    #define TF_UNLIKELY(cond)               __builtin_expect((cond), 0)
    #define TF_CACHELINE_SIZE               32
    #define TF_CACHELINE_ALIGNED            __attribute__((aligned(TF_CACHELINE_SIZE)))
    #define TF_FUNCTION                     __PRETTY_FUNCTION__
#endif

#define TF_UNUSED(var)                      (void)(var)
#define TF_CONCAT_IMPL(x,y)                 x ## y
#define TF_CONCAT(x,y)                      TF_CONCAT_IMPL(x,y)
#define TF_CONCAT_STRING_IMPL(x,y)          (x y)
#define TF_CONCAT_STRING(x,y)               TF_CONCAT_STRING_IMPL(x,y)
#define TF_STR_IMPL(x)                      #x
#define TF_STR(x)                           TF_STR_IMPL(x)
#define TF_ARRAY_SIZE(a)                    (sizeof(a)/sizeof(a[0]))
#define TF_ALIGNMENT(value, align)          ((value)+(-(value)&((align)-1)))

#define TF_DEFAULT_ALIGNMENT_SIZE           (16)

// ”CˆÓ‚ÌŒ^‚Ì‹«ŠE‚ð’²‚×‚é. 
#if defined(__cplusplus)
    template <typename T> class TfAlignof
    {
        struct Helper { char a_; T b_; };
    public:
        static const std::size_t value = offsetof(Helper, b_);
    };
    #define TF_ALIGNOF(_type)	TfAlignof<_type>::value
#else
    #define TF_ALIGNOF(_type)	offsetof(struct{char a; _type b;}, b)
#endif // __cplusplus 

namespace tf
{
    //! Non-copyable attribute. 
    class NonCopyable
    {
    protected:
        NonCopyable()
        {
        }

        ~NonCopyable()
        {
        }

    private:
        NonCopyable             (const NonCopyable&);
        void operator =         (const NonCopyable&);

    }; // class NonCopyable 

    //! ScopeExit tempalte class. 
    template<typename T> class ScopeExit
    {
    public:
        T                       m_func;
        ScopeExit(T func)
            : m_func(func)
        {
        }
        ~ScopeExit()
        {
            m_func();
        }
    }; // class ScopeExit 

    template<typename T> ScopeExit<T> MakeScopeExit(T func)
    {
        return ScopeExit<T>(func);
    }

    //! Memory allocator base class. 
    class Allocator
    {
    public:
                 Allocator();
        virtual ~Allocator();

        virtual void*                   Allocate(size_t size, size_t alignment=TF_DEFAULT_ALIGNMENT_SIZE)=0;
        virtual void                    Free(void* block)=0;

    }; // class Allocator 



    //! Retrieve default allocator. 
    Allocator& DefaultAllocator();

} // namespace tf 

// Scope exit macro. 
#define TF_SCOPE_EXIT(code) auto TF_CONCAT(scopeExit, __LINE__) = tf::MakeScopeExit([&](){code;})




