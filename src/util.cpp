#include "util.h"
#include <execinfo.h>
#include "log.h"
#include "fiber.h"

namespace xianyu {

xianyu::Logger::ptr g_logger = XIANYU_LOG_NAME("system");

quint32 GetThreadId()
{
#ifdef Q_OS_LINUX
    return syscall(SYS_gettid);
#endif

#ifdef Q_OS_WIN
    return  GetCurrentThreadId();
#endif

}

quint32 GetFiberId()
{
    return Fiber::GetFiberId();
}

bool FindFirstNotOf(const QString &S, const QString &T)
{
    QSet<QChar> st(T.begin(), T.end());
    for(auto& i:S)
    {
        if(!st.contains(i))
            return true;
    }
    return false;
}

void Backtrace(QList<QString> &bt, int size, int skip)
{
    void** array = (void**)malloc(sizeof(void*) * size);
    int s = ::backtrace(array, size);

    char** strings = backtrace_symbols(array, s);
    if(strings == NULL)
    {
        XIANYU_LOG_ERROR(g_logger) << "backtrace_symbols error";
        free(array);
        return;
    }
    for(int i=skip;i<s;i++)
    {
        bt.push_back(strings[i]);
    }
    free(strings);
    free(array);
}

QString BacktraceToString(int size, int skip, const QString& prefix)
{
    QList<QString> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for(int i=0;i<bt.size();i++)
    {
        ss << prefix.toStdString() << bt[i].toStdString() <<std::endl;
    }
    return QString(ss.str().c_str());
}

}


