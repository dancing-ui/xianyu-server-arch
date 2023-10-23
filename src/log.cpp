#include "log.h"
#include "config.h"

namespace xianyu {

const QString LogLevel::ToString(LogLevel::Level level)
{

    switch(level)
    {
#define XX(name) \
    case LogLevel::name: \
        { \
            return #name; \
            break; \
        }
        XX(DEBUG)
        XX(INFO)
        XX(WARN)
        XX(ERROR)
        XX(FATAL)
#undef XX
    }
    return "UNKNOWN";
}

LogLevel::Level LogLevel::FromString(const QString &str)
{
    QString val = str.toUpper();
#define XX(name) \
    if(val == #name) \
        return LogLevel::name;
    XX(DEBUG)
    XX(INFO)
    XX(WARN)
    XX(ERROR)
    XX(FATAL)
    return LogLevel::UNKNOWN;
#undef XX
}

LogEvent::LogEvent(QSharedPointer<Logger> logger, LogLevel::Level level
                   , const QString& file_name, qint32 line, quint32 elapse
                   , quint32 thread_id, quint32 fiber_id, quint64 time)
    : file_name_(file_name)
    , line_(line)
    , elapse_(elapse)
    , thread_id_(thread_id)
    , fiber_id_(fiber_id)
    , time_(time)
    , logger_(logger)
    , level_(level)
{
    content_stream_.setString(&content_);
}

void LogEvent::format(const char* fmt, ...)
{
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al)
{
    char* buf = nullptr;
    auto len  = vasprintf(&buf,fmt, al);
    if(len != -1)
    {
        content_stream_ << std::string(buf, len).c_str();
        free(buf);
    }
}

LogEventWrap::LogEventWrap(LogEvent::ptr event)
    : event_(event)
{

}

LogEventWrap::~LogEventWrap()
{
    event_->GetLogger()->log(event_->GetLevel(), event_);//析构的时候，输出日志
}

QTextStream& LogEventWrap::GetContentStream()
{
    return event_->GetContentStream();
}

LogFormatter::LogFormatter(const QString &pattern)
    : pattern_(pattern)
{
    Init();
}

QString LogFormatter::format(QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    QTextStream ts;
    QString res;
    ts.setString(&res);
    for(auto& i:items_)
    {
        i->format(ts, logger, level, event);
    }
    return res;
}

void LogFormatter::Init()
{
    struct Node
    {
        QString s;
        QString format;
        qint32 type;
    };

    QVector<Node> vec;
    QString nstr;
    for(qsizetype i = 0;i < pattern_.size();i++)
    {
        if(pattern_[i] != '%')
        {
            nstr.append(pattern_[i]);
            continue;
        }
        if(i+1<pattern_.size())
        {
            if(pattern_[i+1]=='%')
            {
                nstr.append('%');
                continue;
            }
        }
        qint32 fmt_status = 0;
        qint32 fmt_begin = 0;

        qsizetype n = i+1;
        QString str;
        QString fmt;
        while(n < pattern_.size())
        {
            if(!fmt_status && !isalpha(pattern_[n].toLatin1()) && pattern_[n]!='{' && pattern_[n]!='}')
            {
                str = pattern_.sliced(i+1,n-i-1);
                break;
            }
            if(fmt_status == 0)
            {
                if(pattern_[n]=='{')
                {
                    str = pattern_.sliced(i+1, n - i - 1);
                    fmt_status = 1;
                    fmt_begin = n;
                    n++;
                    continue;
                }
            }
            if(fmt_status == 1)
            {
                if(pattern_[n] == '}')
                {
                    fmt = pattern_.sliced(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 0;
                    n++;
                    break;
                }
            }
            n++;
            if(n==pattern_.size())
            {
                if(str.isEmpty())
                {
                    str = pattern_.sliced(i+1);
                }
            }
        }
        if(fmt_status == 0)
        {
            if(nstr.size())
            {
                vec.push_back({nstr, "", 0});
                nstr.clear();
            }
            vec.push_back({str, fmt, 1});
            i = n - 1;
        }
        else if(fmt_status == 1)
        {
            //qDebug()<< "pattern parse error: "<<pattern_<<"-"<<pattern_.sliced(i) ;
            vec.push_back({"<<pattern_error>>", fmt,0});
            error_format_ = true;
        }
    }
    if(nstr.size())
    {
        vec.push_back({nstr, "", 0});
    }
    //%m -- 消息体
    //%p -- level
    //%r -- 启动后的时间
    //%c -- 日志名称
    //%t -- 线程id
    //%n -- 回车换行
    //%d -- 时间
    //%f -- 文件名
    //%l -- 行号
    //%T -- tab
    //通过占位符，构造类对象
    static QMap<QString,  std::function<FormatItem::ptr(const QString&)>> format_items = {
#define XX(str, C) \
        {#str, [](const QString& fmt) { return FormatItem::ptr(new C(fmt));}}
        XX(m, MessageFormatItem),
        XX(p, LevelFormatItem),
        XX(r, ElapseFormatItem),
        XX(c, NameFormatItem),
        XX(t, ThreadIdFormatItem),
        XX(n, NewLineFormatItem),
        XX(d, DateTimeFormatItem),
        XX(f, FileNameFormatItem),
        XX(l, LineFormatItem),
        XX(T, TabFormatItem),
        XX(F, FiberIdFormatItem)
#undef XX
    };

    for(auto& i:vec)
    {
        if(i.type==0)
        {
            items_.push_back(FormatItem::ptr(new StringFormatItem(i.s)));
        }
        else
        {
            auto it = format_items.find(i.s);
            if(it==format_items.end())
            {
                items_.push_back(FormatItem::ptr(new StringFormatItem(QString("<<error_format %") + i.s + ">>")));
                error_format_ = true;
            }
            else
            {
                auto func = it.value();
                items_.push_back(func(i.format));
            }
        }
        //qDebug()<<i.s<<' '<<i.format<<' '<<i.type;
    }
    //qDebug()<<items_.size();
}

Logger::Logger(const QString &kName)
    : name_(kName)
    , level_(LogLevel::DEBUG)
{
    formatter_.reset(new LogFormatter(QString("%d{yyyy-MM-dd hh:mm:ss}%T%t%T%F%T[%p]%T[%c]%T<%f:%l>%T%m%n")));
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event)
{
    if(level >= level_)
    {
        auto self = sharedFromThis();
        if(!appenders_.isEmpty())
        {
            for(auto& i:appenders_)
            {
                i->log(self, level, event);
            }
        }
        else if(root_.get() != nullptr)
        {
            root_->log(level, event);
        }
    }
}
void Logger::Debug(LogEvent::ptr event)
{
    log(LogLevel::DEBUG, event);
}
void Logger::Info(LogEvent::ptr event)
{
    log(LogLevel::INFO, event);
}
void Logger::Warn(LogEvent::ptr event)
{
    log(LogLevel::WARN, event);
}
void Logger::Error(LogEvent::ptr event)
{
    log(LogLevel::ERROR, event);
}
void Logger::Fatal(LogEvent::ptr event)
{
    log(LogLevel::FATAL, event);
}

void Logger::AddAppender(LogAppender::ptr appender)
{
    if(appender->GetFomatter().get()==nullptr)
    {
        appender->SetFormatter(formatter_);
    }
    appenders_.push_back(appender);
}
void Logger::DelAppender(LogAppender::ptr appender)
{
    for(auto it = appenders_.begin();it!=appenders_.end();it++)
    {
        if(*it==appender)
        {
            appenders_.erase(it);
            break;
        }
    }
}

void Logger::ClearAppender()
{
    appenders_.clear();
}

void Logger::SetFormatter(LogFormatter::ptr val)
{
    formatter_ == val;
}

void Logger::SetFormatter(const QString &val)
{
    xianyu::LogFormatter::ptr new_val(new xianyu::LogFormatter(val));
    if(new_val->GetErrorFormat())
    {
        std::cout<< "Logger setFormatter name=" << name_.toStdString() << " value=" << val.toStdString() << "invalid formatter" << std::endl;
        return;
    }
    formatter_ = new_val;
}

LogFormatter::ptr Logger::GetFormatter() const
{
    return formatter_;
}

QString Logger::ToYamlString()
{
    YAML::Node node;
    node["name"] = name_.toStdString();
    if(level_ != LogLevel::UNKNOWN)
        node["level"] = LogLevel::ToString(level_).toStdString();
    if(formatter_->GetPattern().isEmpty())
        node["formatter"] = formatter_->GetPattern().toStdString();
    for(auto& i:appenders_)
    {
        node["appenders"].push_back(YAML::Load(i->ToYamlString().toStdString()));
    }
    std::stringstream ss;
    ss << node;
    return QString(ss.str().c_str());
}

void StdoutLogAppender::log(QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if(level >= level_)
    {
        std::cout << formatter_->format(logger, level, event).toStdString();
    }
}

QString StdoutLogAppender::ToYamlString()
{
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if(level_ != LogLevel::UNKNOWN)
        node["level"] = LogLevel::ToString(level_).toStdString();
    if(!formatter_->GetPattern().isEmpty())
        node["formatter"] = formatter_->GetPattern().toStdString();
    std::stringstream ss;
    ss << node;
    return QString(ss.str().c_str());
}

FileLogAppender::FileLogAppender(const QString &kFileName)
    :file_name_(kFileName)
{

}

void FileLogAppender::log(QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if(level >= level_)
    {
        if(!file_.isOpen()||(file_.isOpen()&&file_.fileName()!=file_name_))
        {
            reopen();
        }
        if(!file_.exists()||!file_.isOpen())
            return;
        QTextStream file_stream(&file_);
        file_stream << formatter_->format(logger, level, event);
    }
}

QString FileLogAppender::ToYamlString()
{
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["filename"] = file_name_.toStdString();
    if(level_ != LogLevel::UNKNOWN)
        node["level"] = LogLevel::ToString(level_).toStdString();
    if(!formatter_->GetPattern().isEmpty())
        node["formatter"] = formatter_->GetPattern().toStdString();
    std::stringstream ss;
    ss << node;
    return QString(ss.str().c_str());
}

void FileLogAppender::reopen()
{
    if(file_.isOpen())
    {
        file_.close();
    }
    file_.setFileName(file_name_);
    file_.open(QIODevice::Append);
    if(file_.isOpen())
        return;
    file_.close();
}

LoggerManager::LoggerManager()
{
    root_.reset(new Logger);
    root_->AddAppender(LogAppender::ptr(new StdoutLogAppender));
    loggers_[root_->name_] = root_;
    Init();
}

Logger::ptr LoggerManager::GetLogger(const QString &name)
{
    auto it = loggers_.find(name);
    if(it != loggers_.end())
        return it.value();
    Logger::ptr logger(new Logger(name));
    logger->root_ = root_;
    loggers_[name] = logger;
    return logger;
}

void LoggerManager::Init()
{

}

QString LoggerManager::ToYamlString()
{
    YAML::Node node;
    for(auto& i:loggers_)
    {
        node.push_back(YAML::Load(i->ToYamlString().toStdString()));
    }
    std::stringstream ss;
    ss << node;
    return QString(ss.str().c_str());
}


struct LogAppenderDefine
{
    int type_ = 0; // 1 File, 2 Stdout
    LogLevel::Level level_  = LogLevel::UNKNOWN;
    QString formatter_;
    QString  file_name_;

    bool operator==(const LogAppenderDefine& oth) const
    {
        return type_ == oth.type_
               && level_ == oth.level_
               && formatter_ == oth.formatter_
               && file_name_ == oth.file_name_;
    }
};
struct LogDefine
{
    QString name_;
    LogLevel::Level level_ = LogLevel::UNKNOWN;
    QString formatter_;
    QList<LogAppenderDefine> appenders_;

    bool operator==(const LogDefine& oth) const
    {
        return name_ == oth.name_
               && level_ == oth.level_
               && formatter_ == oth.formatter_
               && appenders_ == oth.appenders_;
    }
};
uint qHash(const LogDefine& key)
{
    return QCryptographicHash::hash(key.name_.toLatin1(), QCryptographicHash::Sha1).toUInt();
}

//QString ==> LogDefine
template<>
class LexicalCast<QString, LogDefine>
{
public:
    LogDefine operator()(const QString& v)
    {
        YAML::Node node = YAML::Load(v.toStdString());
        LogDefine ld;
        if(!node["name"].IsDefined())
        {
            std::cout<< "log config error: name is null, " << node << std::endl;
            throw std::logic_error("log config name is null");
        }
        ld.name_ =  QString(node["name"].as<std::string>().c_str());
        ld.level_ = LogLevel::FromString(QString(node["level"].IsDefined() ? node["level"].as<std::string>().c_str() : ""));
        if(node["formatter"].IsDefined())
        {
            ld.formatter_ = QString(node["formatter"].as<std::string>().c_str());
        }
        if(node["appenders"].IsDefined())
        {
            for(quint64 x = 0; x < node["appenders"].size(); x++)
            {
                auto a = node["appenders"][x];
                if(!a["type"].IsDefined())
                {
                    std::cout<< "log config error: appender type is null, " << a << std::endl;
                    continue;
                }
                QString type = QString(a["type"].as<std::string>().c_str());
                LogAppenderDefine lad;
                if(type == "FileLogAppender")
                {
                    lad.type_ = 1;
                    if(!a["filename"].IsDefined())
                    {
                        std::cout<< "log config error: fileappender filename is null, " << a << std::endl;
                        continue;
                    }
                    lad.file_name_ = QString(a["filename"].as<std::string>().c_str());
                    if(a["formatter"].IsDefined())
                    {
                        lad.formatter_ = QString(a["formatter"].as<std::string>().c_str());
                    }
                }
                else if(type == "StdoutLogAppender")
                {
                    lad.type_ = 2;
                    if(a["formatter"].IsDefined())
                    {
                        lad.formatter_ = QString(a["formatter"].as<std::string>().c_str());
                    }
                }
                else
                {
                    std::cout<< "log config error: appender type is invalid, " << a << std::endl;
                    continue;
                }
                ld.appenders_.push_back(lad);
            }
        }
        return ld;
    }
};
//LogDefine ==> QString
template<>
class LexicalCast<LogDefine, QString>
{
public:
    QString operator()(const LogDefine& ld)
    {
        YAML::Node node;
        node["name"] = ld.name_.toStdString();
        if(ld.level_ != LogLevel::UNKNOWN)
        {
            node["level"] = LogLevel::ToString(ld.level_).toStdString();
        }
        if(!ld.formatter_.isEmpty())
        {
            node["formatter"] = ld.formatter_.toStdString();
        }
        for(auto& a : ld.appenders_)
        {
            YAML::Node na;
            if(a.type_ == 1)
            {
                na["type"] = "FileLogAppender";
                na["filename"] = a.file_name_.toStdString();
            }
            else if(a.type_ == 2)
            {
                na["type"] = "StdoutLogAppender";
            }
            if(a.level_ != LogLevel::UNKNOWN)
            {
                na["level"] = LogLevel::ToString(a.level_).toStdString();
            }
            if(!a.formatter_.isEmpty())
            {
                na["formatter"] = a.formatter_.toStdString();
            }
            node["appenders"].push_back(na);
        }
        std::stringstream ss;
        ss << node;
        return QString(ss.str().c_str());
    }
};

xianyu::ConfigVar<QSet<xianyu::LogDefine>>::ptr g_log_defines = xianyu::Config::Lookup("logs", QSet<xianyu::LogDefine>(), "log config");
void TestForDefine()
{
    auto t = xianyu::LoggerMgr::GetInstance();
    std::cout<< xianyu::LoggerMgr::GetInstance()->ToYamlString().toStdString() << '\n';
    YAML::Node root = YAML::LoadFile("./log.yml");//从yaml文件中重新加载
    xianyu::Config::LoadFromYaml(root);
    std::cout<< xianyu::LoggerMgr::GetInstance()->ToYamlString().toStdString() << '\n';
}
struct LogIniter
{
    LogIniter()
    {
        g_log_defines->AddListener(0xF1E2D3, [](const QSet<xianyu::LogDefine>& old_value, const QSet<xianyu::LogDefine>& new_value) {
            XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "on_logger_conf_changed";
            for(auto& i:new_value)
            {
                auto it = old_value.find(i);
                xianyu::Logger::ptr logger;
                if(it==old_value.end())
                {
                    //add logger
                    logger = XIANYU_LOG_NAME(i.name_);
                }
                else
                {
                    if(!(i==*it))
                    {
                        //modify logger
                        logger = XIANYU_LOG_NAME(i.name_);
                    }
                    else
                    {
                        continue;
                    }
                }
                logger->SetLevel(i.level_);
                if(!i.formatter_.isEmpty())
                {
                    logger->SetFormatter(i.formatter_);
                }
                logger->ClearAppender();
                for(auto& a:i.appenders_)
                {
                    xianyu::LogAppender::ptr ap;
                    if(a.type_ == 1)
                    {
                        ap.reset(new FileLogAppender(a.file_name_));
                    }
                    else if(a.type_ == 2)
                    {
                        ap.reset(new StdoutLogAppender);
                    }
                    ap->SetLevel(a.level_);
                    logger->AddAppender(ap);
                }
            }

            for(auto& i:old_value)
            {
                auto it = new_value.find(i);
                if(it==new_value.end())
                {
                    //del
                    auto logger = XIANYU_LOG_NAME(i.name_);
                    logger->SetLevel((LogLevel::Level)100);//级别越高，越难打印
                    logger->ClearAppender();
                }
            }
        });
    }
};

static LogIniter __log_init;

}



