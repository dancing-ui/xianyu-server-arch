#ifndef THREAD_H
#define THREAD_H

#include <thread>
#include <functional>
#include <QSharedPointer>
#include <semaphore.h>
#include <pthread.h>


namespace xianyu {

class Semaphore
{
public:
    Semaphore(quint32 count = 0);
    ~Semaphore();

    void Wait();
    void Notify();

private:
    Semaphore(const Semaphore&) = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

private:
    sem_t semaphore_;
};

template<class T>
struct ScopeLockImpl
{
public:
    ScopeLockImpl(T& mutex)//锁的构造参数传互斥量
        : mutex_(mutex)
    {
        mutex_.Lock();
        locked_ = true;
    }
    ~ScopeLockImpl()
    {
        Unlock();
    }

    void Lock()
    {
        if(locked_ == false)
        {
            mutex_.Lock();
            locked_ = true;
        }
    }

    void Unlock()
    {
        if(locked_ == true)
        {
            mutex_.Unlock();
            locked_ = false;
        }
    }

private:
    T& mutex_;
    bool locked_;
};

template<class T>
struct ReadScopeLockImpl
{
public:
    ReadScopeLockImpl(T& mutex)
        : mutex_(mutex)
    {
        mutex_.RdLock();
        locked_ = true;
    }
    ~ReadScopeLockImpl()
    {
        Unlock();
    }

    void Lock()
    {
        if(locked_ == false)
        {
            mutex_.RdLock();
            locked_ = true;
        }
    }

    void Unlock()
    {
        if(locked_ == true)
        {
            mutex_.Unlock();
            locked_ = false;
        }
    }

private:
    T& mutex_;
    bool locked_;
};

template<class T>
struct WriteScopeLockImpl
{
public:
    WriteScopeLockImpl(T& mutex)
        : mutex_(mutex)
    {
        mutex_.WtLock();
        locked_ = true;
    }
    ~WriteScopeLockImpl()
    {
        Unlock();
    }

    void Lock()
    {
        if(locked_ == false)
        {
            mutex_.WtLock();
            locked_ = true;
        }
    }

    void Unlock()
    {
        if(locked_ == true)
        {
            mutex_.Unlock();
            locked_ = false;
        }
    }

private:
    T& mutex_;
    bool locked_;
};

class Mutex
{
public:
    using CommonLock = ScopeLockImpl<Mutex>;
    Mutex()
    {
        pthread_mutex_init(&mutex_, nullptr);
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex_);
    }

    void Lock()
    {
        pthread_mutex_lock(&mutex_);
    }

    void Unlock()
    {
        pthread_mutex_unlock(&mutex_);
    }

private:
    pthread_mutex_t mutex_;
};

class NullMutex
{
public:
    using CommonLock = ScopeLockImpl<NullMutex>;
    NullMutex()
    {

    }
    ~NullMutex()
    {

    }

    void Lock()
    {

    }

    void Unlock()
    {

    }
};
//////////
/// \brief 读写锁：读多写少的时候使用
/////////
class RWMutex
{
public:
    using ReadLock = ReadScopeLockImpl<RWMutex>;
    using WriteLock = WriteScopeLockImpl<RWMutex>;
    RWMutex()
    {
        pthread_rwlock_init(&lock_, nullptr);
    }
    ~RWMutex()
    {
        pthread_rwlock_destroy(&lock_);
    }

    void RdLock()
    {
        pthread_rwlock_rdlock(&lock_);
    }

    void WtLock()
    {
        pthread_rwlock_wrlock(&lock_);
    }

    void Unlock()
    {
        pthread_rwlock_unlock(&lock_);
    }

private:
    pthread_rwlock_t lock_;

};

class NullRWMutex
{
public:
    using CommonLock = ScopeLockImpl<NullRWMutex>;
    NullRWMutex()
    {

    }
    ~NullRWMutex()
    {

    }

    void RdLock()
    {

    }

    void WtLock()
    {

    }

    void Unlock()
    {

    }
};

class SpinLock
{
public:
    using CommonLock = ScopeLockImpl<SpinLock>;
    SpinLock()
    {
        pthread_spin_init(&mutex_, 0);
    }
    ~SpinLock()
    {
        pthread_spin_destroy(&mutex_);
    }
    void Lock()
    {
        pthread_spin_lock(&mutex_);
    }
    void Unlock()
    {
        pthread_spin_unlock(&mutex_);
    }

private:
    pthread_spinlock_t mutex_;
};

class CASLock
{
public:
    using CommonLock = ScopeLockImpl<CASLock>;
    CASLock()
    {
        mutex_.clear();
    }
    ~CASLock()
    {

    }
    void Lock()
    {
        while (std::atomic_flag_test_and_set_explicit(&mutex_, std::memory_order_acquire));
    }
    void Unlock()
    {
        std::atomic_flag_clear_explicit(&mutex_, std::memory_order_release);
    }

private:
    volatile std::atomic_flag mutex_;
};

class Thread
{
public:
    using ptr = QSharedPointer<Thread>;
    Thread(std::function<void()> cb, const QString& name);
    ~Thread();


    qint32 GteThreadId() const;
    const QString& GetName() const;

    void Join();

    static Thread* GetThis();
    static const QString& GetStaticName();
    static void SetName(const QString& name);
    //静态方法属于类方法,不需要实例对象就可以访问

private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;

    static void* Run(void* arg);
private:
    qint32 thread_id_{-1};
    pthread_t thread_{0};
    std::function<void()> cb_;
    QString name_;
    Semaphore semaphore_;

};



}



#endif // THREAD_H
