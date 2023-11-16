#ifndef FIBER_H
#define FIBER_H

#include <ucontext.h>
#include <QEnableSharedFromThis>

namespace xianyu {


class Fiber : public QEnableSharedFromThis<Fiber>
{
public:
    using ptr = QSharedPointer<Fiber>;

    enum State{
        INIT,
        HOLD,
        EXEC,
        TERM, // terminate结束
        REDAY,
        EXCEPT
    };

private:
    Fiber();

public:
    Fiber(std::function<void()> cb, qint64 stack_size = 0);
    ~Fiber();
    ////
    /// \brief 重置协程函数，并重置状态
    /// \param cb
    ///
    void Reset(std::function<void()> cb);
    ///
    /// \brief 切换到当前协程执行
    ///
    void SwapIn();
    ///
    /// \brief 切换到后台执行
    ///
    void SwapOut();
    qint64 GetId()const {return id_;}
    State GetState() const {return state_;}

public:
    ///
    /// \brief 设置当前协程
    /// \param
    ///
    static void SetThis(Fiber* f);
    ///
    /// \brief 返回当前协程的智能指针
    /// \return
    ///
    static Fiber::ptr GetThis();
    ///
    /// \brief 协程切换到后台，并且设置为Ready状态
    ///
    static void YieldToReady();
    ///
    /// \brief 协程切换到后台，并且设置为Hold状态
    ///
    static void YieldToHold();
    ///
    /// \brief 总协程数
    ///
    static quint64 ToTalFibers();

    static void MainFunc();
    static quint64 GetFiberId();
private:
    qint64 id_;
    quint32 stack_size_;
    State state_{INIT};

    ucontext_t ctx_;
    void* stack_{nullptr};
    std::function<void()> cb_{nullptr};
};



}



#endif // FIBER_H
