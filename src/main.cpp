#include <QCoreApplication>

#include "log.h"
#include "util.h"
#include "config.h"

#include <QException>

#include <yaml-cpp/yaml.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

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
    XIANYU_LOG_INFO(XIANYU_LOG_NAME("system")) << "hello";
    return 0;
}
