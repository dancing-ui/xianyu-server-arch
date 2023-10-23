#ifndef SINGLETON_H
#define SINGLETON_H

#include <QSharedPointer>
namespace xianyu {

template<class T, class X = void, int N = 0>
class Singleton
{
public:
    Singleton();
    static T* GetInstance()
    {
        static T v;
        return &v;
    }
};

template<class T, class X = void, int N = 0>
class SingletonPtr
{
public:
    static QSharedPointer<T> GetInstance()
    {
        static QSharedPointer<T> v(new T);
        return v;
    }
};


}


#endif // SINGLETON_H
