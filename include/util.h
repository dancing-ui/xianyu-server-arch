#ifndef UTIL_H
#define UTIL_H

#include <QObject>
#include <QSet>

#ifdef Q_OS_LINUX
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace xianyu {
quint32 GetThreadId();
quint32 GetFiberId();
bool FindFirstNotOf(const QString& S, const QString& T);
}

#endif // UTIL_H
