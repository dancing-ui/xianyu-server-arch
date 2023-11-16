#include "scheduler.h"
#include "log.h"
#include "macro.h"

namespace xianyu {

static xianyu::Logger::ptr g_logger = XIANYU_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_fiber = nullptr;

Scheduler::Scheduler(qint64 threads, bool use_caller, const QString &name)
    : name_(name)
{
    XIANYU_ASSERT(threads > 0);
    if(use_caller)
    {
        xianyu::Fiber::GetThis();
        --threads;//首次执行GetThis会创建一个主协程，主协程也算一个协程数
        XIANYU_ASSERT(GetThis() == nullptr);//必须保证只能有1个协程调度器
        t_scheduler = this;
        root_fiber_.reset(new Fiber(std::bind(&Schedule::Run, this)));//新建root协程去执行具体的调度

        xianyu::Thread::SetName(name_);
        t_fiber = root_fiber_.get();
        root_thread_id_ = xianyu::GetThreadId();
        thread_ids_.push_back(root_thread_id_);
    }
    else
    {
        root_thread_id_ = -1;
    }
    thread_count_ = threads;
}

Scheduler::~Scheduler()
{
    XIANYU_ASSERT(stopping_ == true);
    if(GetThis() == this)
    {
        t_scheduler = nullptr;
    }
}

Scheduler *Scheduler::GetThis()
{
    return t_scheduler;
}

Fiber *Scheduler::GetMainFiber()
{
    return t_fiber;
}

void Scheduler::Start()
{
    MutexType::CommonLock lock(mutex_);
    if(!stopping_)
    {
        return;
    }
    stopping_ = false;
    XIANYU_ASSERT(threads_.isEmpty());

    threads_.resize(thread_count_);
    for(qint64 i = 0;i<thread_count_;i++)
    {
        threads_[i].reset(new Thread(std::bind(&Scheduler::Run, this), name_ + "_" + QString::number(i)));
        thread_ids_.push_back(threads_[i]->GteThreadId());
    }
}

void Scheduler::Stop()
{
    auto_stop_ = true;
    if(root_fiber_ && thread_count_ == 0 && (root_fiber_->GetState() == Fiber::TERM || root_fiber_->GetState() == Fiber::INIT))
    {
        stopping_ = true;
        if(Stopping())
        {
            return;
        }
    }

    if(root_thread_id_ != -1)
    {
        XIANYU_ASSERT(GetThis() == this);
    }
    else
    {
        XIANYU_ASSERT(GetThis() != this);
    }
    stopping_ = true;
    for(qint64 i=0;i<thread_count_;i++)
    {
        Tickle();
    }
    if(root_fiber_)
    {
        Tickle();
    }
    if(Stopping())
    {
        return;
    }
}

void Scheduler::Run()
{
    SetThis();
    //if(xianyu::)
}



}


