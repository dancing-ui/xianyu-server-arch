#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QSharedPointer>
#include "thread.h"
#include "fiber.h"


namespace xianyu {

class Scheduler
{
public:
    using ptr = QSharedPointer<Scheduler>;
    using MutexType = Mutex;

    Scheduler(qint64 threads = 1,bool use_caller = true, const QString& name = "");
    virtual ~Scheduler();

    const QString& GetName() const {return name_;}

    static Scheduler* GetThis();
    static Fiber* GetMainFiber();

    void Start();
    void Stop();

    template<class FiberOrCb>
    void Schedule(FiberOrCb fc, qint32 thread_id = -1)
    {
        bool need_tickle = false;
        {
            MutexType::CommonLock lock(mutex_);
            need_tickle  ScheduleNoLock(fc, thread_id);
        }
        if(need_tickle)
        {
            Tickle();
        }
    }
    template<class InputIterator>
    void Schedule(InputIterator begin, InputIterator end)
    {
        bool need_tickle = false;
        {
            MutexType::CommonLock lock(mutex_);
            while(begin!=end)
            {
                need_tickle = ScheduleNoLock(&(*begin)) || need_tickle;
                ++begin;
            }
        }
        if(need_tickle)
        {
            Tickle();
        }
    }
protected:
    virtual void Tickle();
    void Run();
    virtual bool Stopping();
    void SetThis();
private:
    template<class FiberOrCb>
    bool ScheduleNoLock(FiberOrCb fc, qint32 thread_id = -1)
    {
        bool need_tickle = fibers_.empty();
        FiberAndThread ft(fc, thread_id);
        if(ft.fiber || ft.cb)
        {
            fibers_.push_back(ft);
        }
        return need_tickle;
    }

private:
    struct FiberAndThread
    {
        Fiber::ptr fiber;
        std::function<void()> cb;
        qint32 thread_id;

        FiberAndThread(Fiber::ptr f, qint32 thr)
            : fiber(f)
            , thread_id(thr)
        {

        }
        FiberAndThread(Fiber::ptr* f, qint32 thr)
            : thread_id(thr)
        {
            fiber.swap(*f);
        }
        FiberAndThread(std::function<void()> f, qint32 thr)
            : cb(f)
            , thread_id(thr)
        {

        }
        FiberAndThread(std::function<void()>* f, qint32 thr)
            : thread_id(thr)
        {
            cb.swap(*f);
        }
        FiberAndThread()
            : thread_id(-1)
        {

        }
        void Reset()
        {
            fiber = nullptr;
            cb = nullptr;
            thread_id = -1;
        }
    };

private:
    MutexType mutex_;
    QList<Thread::ptr> threads_;
    QList<FiberAndThread> fibers_;
    Fiber::ptr root_fiber_;
    QString name_;

protected:
    QList<qint32> thread_ids_;
    qint64 thread_count_{0};
    qint64 active_thread_count_{0};
    qint64 idle_thread_count_{0};
    bool stopping_{true};
    bool auto_stop_{false};
    qint32 root_thread_id_{0};
};


}



#endif // SCHEDULER_H
