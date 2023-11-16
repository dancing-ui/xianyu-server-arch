#include "fiber.h"
#include "config.h"
#include "macro.h"

namespace xianyu {


static std::atomic<qint64> s_fiber_id{0};
static std::atomic<qint64> s_fiber_count{0};

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_threadFiber = nullptr;

static xianyu::ConfigVar<quint32>::ptr g_fiber_stack_size = xianyu::Config::Lookup<quint32>("fiber.stack_size", 1024 * 1024, "fiber stack size");
static Logger::ptr g_logger = XIANYU_LOG_NAME("system");

class MallocStackAllocator
{
public:
    static void* Alloc(qint64 size)
    {
        return malloc(size);
    }
    static void Dealloc(void* vp, qint64 size)
    {
        free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

Fiber::Fiber()
{
    state_ = EXEC;
    SetThis(this);

    if(getcontext(&ctx_))
    {
        XIANYU_ASSERT2(false, "getcontext");
    }
    ++s_fiber_count;
}

Fiber::Fiber(std::function<void()> cb, qint64 stack_size)
    : id_(++s_fiber_id)
    , cb_(cb)
{
    ++s_fiber_count;
    stack_size_ = stack_size ? stack_size : g_fiber_stack_size->GetValue();

    stack_ = StackAllocator::Alloc(stack_size_);
    if(getcontext(&ctx_))
    {
        XIANYU_ASSERT2(false, "getcontext");
    }
    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stack_size_;

    makecontext(&ctx_, &Fiber::MainFunc, 0);
    XIANYU_LOG_INFO(g_logger) << "Fiber: fiber id=" << id_ ;
}

Fiber::~Fiber()
{
    --s_fiber_count;
    if(stack_ != nullptr)
    {
        XIANYU_ASSERT(state_ == TERM || state_ == INIT || state_ == EXCEPT);
        StackAllocator::Dealloc(stack_, stack_size_);
    }
    else
    {
        XIANYU_ASSERT(cb_ == nullptr);
        XIANYU_ASSERT(state_ == EXEC);
        Fiber* cur = t_fiber;
        if(cur == this)
        {
            SetThis(nullptr);
        }
    }
    XIANYU_LOG_INFO(g_logger) << "~Fiber: fiber id=" << id_ ;
}

void Fiber::Reset(std::function<void ()> cb)
{
    XIANYU_ASSERT(stack_);
    XIANYU_ASSERT(state_ == TERM || state_ == INIT || state_ == EXCEPT);
    cb_ = cb;
    if(getcontext(&ctx_))
    {
        XIANYU_ASSERT2(false, "getcontext");
    }
    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stack_size_;
    makecontext(&ctx_, &Fiber::MainFunc, 0);
    state_ = INIT;
}

void Fiber::SwapIn()
{
    SetThis(this);
    XIANYU_ASSERT(state_ != EXEC);
    if(swapcontext(&t_threadFiber->ctx_, &ctx_))
    {
        XIANYU_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SwapOut()
{
    SetThis(t_threadFiber.get());
    if(swapcontext(&ctx_, &t_threadFiber->ctx_))
    {
        XIANYU_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SetThis(Fiber *f)
{
    t_fiber = f;
}
///
/// \brief t_fiber为空时，说明当前没有协程，那么创建主协程，同时需要保证只能有一个主协程通过GetThis函数去创建主协程
/// \return
///
Fiber::ptr Fiber::GetThis()
{
    if(t_fiber)
    {
        return t_fiber->sharedFromThis();
    }
    Fiber::ptr main_fiber(new Fiber);
    XIANYU_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->sharedFromThis();
}

void Fiber::YieldToReady()
{
    Fiber::ptr cur = GetThis();
    cur->state_ = REDAY;
    cur->SwapOut();
}

void Fiber::YieldToHold()
{
    Fiber::ptr cur = GetThis();
    cur->state_ = HOLD;
    cur->SwapOut();
}

quint64 Fiber::ToTalFibers()
{
    return s_fiber_count;
}

void Fiber::MainFunc()
{
    Fiber::ptr cur = GetThis();
    XIANYU_ASSERT(cur);
    try {
        cur->cb_();
        cur->cb_ = nullptr;
        cur->state_ = TERM;
    } catch(std::exception& ex) {
        cur->state_ = EXCEPT;
        XIANYU_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what();
    } catch(...) {
        cur->state_ = EXCEPT;
        XIANYU_LOG_ERROR(g_logger) << "Fiber Except";
    }
    /// 当子协程的MainFunc执行完，一个子协程就完全结束了，
    /// 但是在执行Swapout之后，cur还在，这就会导致cur一直不会被释放，
    /// 所以我们可以把裸指针取出来，然后把cur重置为空，最后使用裸指针去Swapout
    //cur->SwapOut();
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->SwapOut();

    XIANYU_ASSERT2(false, "never reach");
}

quint64 Fiber::GetFiberId()
{
    if(t_fiber)
    {
        return t_fiber->GetId();
    }
    return 0;
}



}
