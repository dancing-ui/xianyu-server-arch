#include "thread.h"
#include "log.h"

namespace xianyu {

static thread_local Thread* t_thread = nullptr;
static thread_local QString t_thread_name = "UNKNOWN";

static xianyu::Logger::ptr g_logger = XIANYU_LOG_NAME("system");

void *Thread::Run(void *arg)
{
    Thread* thread = (Thread* )arg;
    t_thread = thread;
    t_thread_name = thread->name_;
    thread->thread_id_ = xianyu::GetThreadId();
    pthread_setname_np(pthread_self(), thread->name_.toStdString().c_str());

    std::function<void()> cb;
    cb.swap(thread->cb_);

    thread->semaphore_.Notify();//子线程得到回调函数之后，视为已就绪

    if(cb != nullptr)
        cb();
    return 0;
}

Thread::Thread(std::function<void ()> cb, const QString &name)
    : cb_(cb), name_(name)
{
    if(name.isEmpty())
    {
        name_ = "UNKNOWN";
    }
    int rt = pthread_create(&thread_, nullptr, &Thread::Run, this);
    if(rt)
    {
        XIANYU_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
                                   << " name=" << name;
        throw std::logic_error("pthread_create error");
    }
    semaphore_.Wait();//等待所有子线程就绪
}

Thread::~Thread()
{
    if(thread_)
    {
        pthread_detach(thread_);
    }
}

qint32 Thread::GteThreadId() const {return thread_id_;}

const QString &Thread::GetName() const {return name_;}

void Thread::Join()
{
    if(thread_)
    {
        int rt = pthread_join(thread_, nullptr);
        if(rt)
        {
            XIANYU_LOG_ERROR(g_logger) << "pthread_hoin thread fail, rt=" << rt
                                       << "name=" <<name_;
        }
        thread_ = 0;
    }
}

Thread *Thread::GetThis() {return t_thread;}

const QString &Thread::GetStaticName() {return t_thread_name;}

void Thread::SetName(const QString &name)
{
    if(t_thread != nullptr)
    {
        t_thread->name_ = name;
    }
    t_thread_name = name;
}

Semaphore::Semaphore(quint32 count)
{
    if(sem_init(&semaphore_, 0, count))
    {
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore()
{
    sem_destroy(&semaphore_);
}

void Semaphore::Wait()
{
    if(sem_wait(&semaphore_))
    {
        throw std::logic_error("sem_wait error");
    }
}

void Semaphore::Notify()
{
    if(sem_post(&semaphore_))
    {
        throw std::logic_error("sem_post error");
    }
}




}
