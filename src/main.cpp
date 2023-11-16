#include <QCoreApplication>

#include "log.h"
//#include "util.h"
//#include "config.h"
//#include "thread.h"
#include "macro.h"
#include "fiber.h"

#include <QException>

#include <yaml-cpp/yaml.h>

void TestLogAndConfig()
{
    //    xianyu::Logger::ptr logger(new xianyu::Logger);
    //    logger->AddAppender(xianyu::LogAppender::ptr(new xianyu::StdoutLogAppender));
    //    XIANYU_LOG_DEBUG(logger) << "hello";
    //    XIANYU_LOG_FATAL(logger) << "world";

    //    xianyu::Logger::ptr logger(new xianyu::Logger);
    //    xianyu::FileLogAppender::ptr file_appender(new xianyu::FileLogAppender("./log.txt"));
    //    xianyu::LogFormatter::ptr fmt(new xianyu::LogFormatter("%d%T%m%n"));
    //    file_appender->SetFormatter(fmt);
    //    logger->AddAppender(file_appender);
    //    XIANYU_LOG_FMT_DEBUG(logger, "fmt debug %s", "fengxu");

    //    auto l = xianyu::LoggerMgr::GetInstance()->GetLogger("xx");
    //    XIANYU_LOG_INFO(l) << "bbb";

    //    auto c = xianyu::Config::Lookup("system.port", (int)8080, "system port");//默认值是8080
    //    auto d = xianyu::Config::Lookup("system.port", (float)8080, "system port");//前面已经被初始化为int，现在又变为float，会产生类型错误
    //    XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "before: " << c->GetValue();
    //    YAML::Node root = YAML::LoadFile("./log.yml");//从yaml文件中重新加载
    //    xianyu::Config::LoadFromYaml(root);
    //    XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "after: " << c->GetValue();//读取yaml中的值

    //    auto c = xianyu::Config::Lookup("system.int_vec", QSet<int>{}, "system int_vec");//默认值是8080
    //    for(auto& i:c->GetValue())
    //    {
    //        XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "before: " << i;
    //    }
    //    YAML::Node root = YAML::LoadFile("./log.yml");//从yaml文件中重新加载
    //    xianyu::Config::LoadFromYaml(root);
    //    for(auto& i:c->GetValue())
    //    {
    //        XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "after: " << i;
    //    }

    //    auto c = xianyu::Config::Lookup("system.int_map", QMap<QString, int>{}, "system int_map");//默认值是8080
    //    auto v = c->GetValue();
    //    for(auto it = v.begin();it!=v.end();it++)
    //    {
    //        XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "before: " << it.key() <<"==>" << it.value();
    //    }
    //    YAML::Node root = YAML::LoadFile("./log.yml");//从yaml文件中重新加载
    //    xianyu::Config::LoadFromYaml(root);
    //    v = c->GetValue();
    //    for(auto it = v.begin();it!=v.end();it++)
    //    {
    //        XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "after: " << it.key() <<"==>" << it.value();
    //    }

    //    auto c = xianyu::Config::Lookup("class.person", xianyu::Person{}, "class person");//默认值是8080
    //    auto v = c->GetValue();
    //    XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "before: " << v.name_ << " " <<v.age_ << " "<<v.sex_;
    //    YAML::Node root = YAML::LoadFile("./log.yml");//从yaml文件中重新加载
    //    xianyu::Config::LoadFromYaml(root);
    //    v = c->GetValue();
    //    XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "after: " << v.name_ << " " <<v.age_ << " "<<v.sex_;

    //    auto c = xianyu::Config::Lookup("class.person", xianyu::Person(), "class person");
    //    c->AddListener(10, [](const xianyu::Person& old_value, const xianyu::Person& new_value) {
    //        XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "old value="<<old_value.ToString() << ",new value=" << new_value.ToString();
    //    });
    //    auto v = c->GetValue();
    //    XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "before: " << v.name_;
    //    YAML::Node root = YAML::LoadFile("./log.yml");//从yaml文件中重新加载
    //    xianyu::Config::LoadFromYaml(root);
    //    v = c->GetValue();
    //    XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "after: " << v.name_;


    //    XIANYU_LOG_INFO(XIANYU_LOG_NAME("system")) << "hello";
}
quint64 count = 0;

xianyu::Mutex mutex_;
void fun1()
{
//    qDebug() << "thread_name=" << xianyu::Thread::GetStaticName()
//             << "this.name=" << xianyu::Thread::GetThis()->GetName()
//             << " thread_id=" << xianyu::Thread::GetThis()->GteThreadId();
    for(quint64 i=0;i<10000000;i++)
    {
        XIANYU_LOG_INFO(XIANYU_LOG_NAME("system")) << i << " " << xianyu::Thread::GetStaticName();
        xianyu::Mutex::CommonLock lock(mutex_);
        count++;
    }
}

void TestThread()
{
    qDebug() << "thread test begin";
    QVector<xianyu::Thread::ptr> thrs;
    auto start = clock();
    for(int i=0;i<1;i++)
    {
        xianyu::Thread::ptr thr(new xianyu::Thread(&fun1, QString("name_%1").arg(i)));
        thrs.push_back(thr);
    }
    for(int i=0;i<1;i++)
    {
        thrs[i]->Join();
    }
    auto end = clock();
    qDebug() << "thread test end:time="<<(double)(end - start)/ CLOCKS_PER_SEC<<"s";
    qDebug() << count;
}

void TestAssert()
{
    XIANYU_ASSERT(0==1);
}

static xianyu::Logger::ptr g_logger = XIANYU_LOG_ROOT();

void RunInFiber()
{
    XIANYU_LOG_INFO(g_logger) << "run in fiber begin";
    xianyu::Fiber::YieldToHold();
    XIANYU_LOG_INFO(g_logger) << "run in fiber end";
    //xianyu::Fiber::YieldToHold();
}

void TestFiber()
{

    xianyu::Fiber::GetThis();//通过GetThis函数去创建主协程,主协程负责子协程的创建、调度、销毁
    XIANYU_LOG_INFO(g_logger) << "main begin";
    xianyu::Fiber::ptr fiber(new xianyu::Fiber(RunInFiber));
    fiber->SwapIn();
    XIANYU_LOG_INFO(g_logger) << "main after swapIn";
    fiber->SwapIn();
    XIANYU_LOG_INFO(g_logger) << "main after end";
}

void TestMultiFiber()
{
    xianyu::Thread::SetName("main");
    QList<xianyu::Thread::ptr> thrs;
    for(int i=0;i<3;i++)
    {
        thrs.push_back(xianyu::Thread::ptr(new xianyu::Thread(&TestFiber, "name_" + QString::number(i))));
    }
    for(int i=0;i<3;i++)
    {
        thrs[i]->Join();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TestMultiFiber();
    return 0;
}
