#include "util.h"


namespace xianyu {

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
    return 0;
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

}


