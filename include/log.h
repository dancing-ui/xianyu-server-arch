#ifndef LOGGER
#define LOGGER

#include <QObject>
#include <QSharedPointer>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QMap>
#include <QDateTime>
#include <functional>

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <QCryptographicHash>

#include "singleton.h"
#include "util.h"

#define XIANYU_LOG_LEVEL(logger, level) \
    if(logger->GetLevel() <= level) \
        xianyu::LogEventWrap(xianyu::LogEvent::ptr(new xianyu::LogEvent(logger, level, __FILE__, __LINE__, 0 \
                            , xianyu::GetThreadId(), xianyu::GetFiberId(), time(0)))).GetContentStream()

#define XIANYU_LOG_DEBUG(logger) XIANYU_LOG_LEVEL(logger, xianyu::LogLevel::DEBUG)
#define XIANYU_LOG_INFO(logger) XIANYU_LOG_LEVEL(logger, xianyu::LogLevel::INFO)
#define XIANYU_LOG_WARN(logger) XIANYU_LOG_LEVEL(logger, xianyu::LogLevel::WARN)
#define XIANYU_LOG_ERROR(logger) XIANYU_LOG_LEVEL(logger, xianyu::LogLevel::ERROR)
#define XIANYU_LOG_FATAL(logger) XIANYU_LOG_LEVEL(logger, xianyu::LogLevel::FATAL)

#define XIANYU_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->GetLevel() <= level) \
        xianyu::LogEventWrap(xianyu::LogEvent::ptr(new xianyu::LogEvent(logger, level, __FILE__, __LINE__, 0 \
                            , xianyu::GetThreadId(), xianyu::GetFiberId(), time(0)))).GetEvent()->format(fmt, __VA_ARGS__)

#define XIANYU_LOG_FMT_DEBUG(logger, fmt, ...) XIANYU_LOG_FMT_LEVEL(logger, xianyu::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define XIANYU_LOG_FMT_INFO(logger, fmt, ...) XIANYU_LOG_FMT_LEVEL(logger, xianyu::LogLevel::INFO, fmt, __VA_ARGS__)
#define XIANYU_LOG_FMT_WARN(logger, fmt, ...) XIANYU_LOG_FMT_LEVEL(logger, xianyu::LogLevel::WARN, fmt, __VA_ARGS__)
#define XIANYU_LOG_FMT_ERROR(logger, fmt, ...) XIANYU_LOG_FMT_LEVEL(logger, xianyu::LogLevel::ERROR, fmt, __VA_ARGS__)
#define XIANYU_LOG_FMT_FATAL(logger, fmt, ...) XIANYU_LOG_FMT_LEVEL(logger, xianyu::LogLevel::FATAL, fmt, __VA_ARGS__)

#define XIANYU_LOG_ROOT() xianyu::LoggerMgr::GetInstance()->GetRoot()
#define XIANYU_LOG_NAME(name) xianyu::LoggerMgr::GetInstance()->GetLogger(name)

namespace xianyu{
class Logger;
class LoggerManager;

/////////////////////////
/// \brief 日志级别
////////////////////////
class LogLevel
{
public:
    enum Level{
        UNKNOWN = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const QString ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const QString& str);
};
/////////////////////////
/// \brief 日志事件
////////////////////////
class LogEvent
{
public:
    using ptr = QSharedPointer<LogEvent>;
    explicit LogEvent(QSharedPointer<Logger> logger, LogLevel::Level level
                      , const QString& file_name, qint32 line, quint32 elapse
                      , quint32 thread_id, quint32 fiber_id, quint64 time);

    const QString& GetFileName()const {return file_name_;}
    qint32 GetLine()const {return line_;}
    quint32 GetElapse()const {return elapse_;}
    quint32 GetThreadId()const {return thread_id_;}
    quint32 GetFiberId()const {return fiber_id_;}
    quint64 GetTime()const {return time_;}
    QString& GetContent() {return content_;}
    QTextStream& GetContentStream(){return content_stream_;}
    QSharedPointer<Logger> GetLogger()const {return logger_;}
    LogLevel::Level GetLevel()const {return level_;}

    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

private:
    QString file_name_;  //文件名
    qint32 line_ = 0;               //行号
    quint32 elapse_ = 0;            //程序启动开始到现在的毫秒数
    quint32 thread_id_ = 0 ;        //线程id
    quint32 fiber_id_ = 0;           //协程id
    quint64 time_ = 0;              //时间戳
    QString content_;               //日志内容
    QTextStream content_stream_;
    QSharedPointer<Logger> logger_;
    LogLevel::Level level_;
};

class LogEventWrap
{
public:
    LogEventWrap(LogEvent::ptr event);
    ~LogEventWrap();

    QTextStream& GetContentStream();
    LogEvent::ptr GetEvent() const {return event_;}
private:
    LogEvent::ptr event_;
};

/////////////////////////
/// \brief 日志格式器
////////////////////////
class LogFormatter
{
public:
    using ptr = QSharedPointer<LogFormatter>;
    LogFormatter(const QString& pattern);

    QString format(QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

public:
    class FormatItem
    {
    public:
        using ptr = QSharedPointer<FormatItem>;
        FormatItem(const QString& fmt){}
        virtual ~FormatItem(){}
        virtual void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    const QString& GetPattern()const { return pattern_;}

    void Init();

    bool GetErrorFormat() const { return error_format_; }
private:
    QString pattern_;
    QVector<FormatItem::ptr> items_;
    bool error_format_ = false;

signals:

};
/////////////////////////
/// \brief 日志输出地
////////////////////////
class LogAppender
{
public:
    using ptr = QSharedPointer<LogAppender>;
    virtual ~LogAppender(){}

    virtual void log(QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    virtual QString ToYamlString() = 0;

    void SetFormatter(LogFormatter::ptr val) {formatter_= val;}
    LogFormatter::ptr GetFomatter()const {return formatter_;}
    LogLevel::Level GetLevel()const {return level_;}
    void SetLevel(LogLevel::Level level){level_ = level;}//设置不同级别的目的：存储不同级别的对象，比如只存ERROR级别以上的对象，只存DEBUG级别以上的对象

protected:
    LogLevel::Level level_ = LogLevel::DEBUG;
    LogFormatter::ptr formatter_;

signals:

};
/////////////////////////
/// \brief 日志器
////////////////////////
class Logger : public QEnableSharedFromThis<Logger>
{
    friend class LoggerManager;
public:
    using ptr = QSharedPointer<Logger>;
    Logger(const QString& kName = "root");

    void log(LogLevel::Level level, LogEvent::ptr event);

    void Debug(LogEvent::ptr event);
    void Info(LogEvent::ptr event);
    void Warn(LogEvent::ptr event);
    void Error(LogEvent::ptr event);
    void Fatal(LogEvent::ptr event);

    void AddAppender(LogAppender::ptr appender);
    void DelAppender(LogAppender::ptr appender);
    void ClearAppender();
    LogLevel::Level GetLevel()const {return level_;}
    void SetLevel(LogLevel::Level val) {level_ = val;}
    void SetFormatter(LogFormatter::ptr val);
    void SetFormatter(const QString& val);
    LogFormatter::ptr GetFormatter()const;

    const QString& GetName()const {return name_;}

    QString ToYamlString();

private:
    QString name_;                    //日志名称
    LogLevel::Level level_;           //日志级别
    QList<LogAppender::ptr> appenders_;     //appender集合
    LogFormatter::ptr formatter_;
    Logger::ptr root_; //默认logger

signals:

};
/////////////////////////
/// \brief 输出到控制台的Appender
////////////////////////
class StdoutLogAppender : public LogAppender
{
public:
    using ptr = QSharedPointer<StdoutLogAppender>;
    void log(QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
    QString ToYamlString() override;
};
/////////////////////////
/// \brief 输出到文件的Appender
////////////////////////
class FileLogAppender : public LogAppender
{
public :
    using ptr = QSharedPointer<FileLogAppender>;
    explicit FileLogAppender(const QString& kFileName);
    void log(QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
    QString ToYamlString() override;

    void reopen();
    void SetFileName(const QString& val) {file_name_ = val;}
    const QString& GetFileName()const {return file_name_;}
private:
    QString file_name_;
    QFile file_;
};


//////////////////
/// \brief %m -- 消息体
/////////////////
class MessageFormatItem : public LogFormatter::FormatItem
{
public:
    MessageFormatItem(const QString& str)
        :FormatItem(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << event->GetContent();
    }
};
//////////////////
/// \brief %p -- level
/////////////////
class LevelFormatItem : public LogFormatter::FormatItem
{
public:
    LevelFormatItem(const QString& str)
        :FormatItem(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << LogLevel::ToString(level);
    }
};
//////////////////
/// \brief %r -- 启动后的时间
/////////////////
class ElapseFormatItem : public LogFormatter::FormatItem
{
public:
    ElapseFormatItem(const QString& str)
        :FormatItem(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << event->GetElapse();
    }
};
//////////////////
/// \brief %c -- 日志名称
/////////////////
class NameFormatItem : public LogFormatter::FormatItem
{
public:
    NameFormatItem(const QString& str)
        :FormatItem(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << logger->GetName();
    }
};
//////////////////
/// \brief %t -- 线程id
/////////////////
class ThreadIdFormatItem : public LogFormatter::FormatItem
{
public:
    ThreadIdFormatItem(const QString& str)
        :FormatItem(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << event->GetThreadId();
    }
};
//////////////////
/// \brief %n -- 回车换行
/////////////////
class NewLineFormatItem : public LogFormatter::FormatItem
{
public:
    NewLineFormatItem(const QString& str)
        :FormatItem(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << '\n';
    }
};
//////////////////
/// \brief %d -- 时间
/////////////////
class DateTimeFormatItem : public LogFormatter::FormatItem
{
public:
    DateTimeFormatItem(const QString& str = "yyyy-MM-dd hh:mm:ss")
        :FormatItem(str), format_(str)
    {
        if(str.isEmpty())
            format_ = "yyyy-MM-dd hh:mm:ss";
    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        QDateTime dt = QDateTime::fromSecsSinceEpoch(event->GetTime());
        ts << dt.toString(format_);
    }
private:
    QString format_;
};
//////////////////
/// \brief %f -- 文件名
/////////////////
class FileNameFormatItem : public LogFormatter::FormatItem
{
public:
    FileNameFormatItem(const QString& str)
        :FormatItem(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << event->GetFileName();
    }
};
//////////////////
/// \brief %l -- 行号
/////////////////
class LineFormatItem : public LogFormatter::FormatItem
{
public:
    LineFormatItem(const QString& str)
        :FormatItem(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << event->GetLine();
    }
};
//////////////////
/// \brief %T -- tab
/////////////////
class TabFormatItem : public LogFormatter::FormatItem
{
public:
    TabFormatItem(const QString& str)
        :FormatItem(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << "\t";
    }
};
//////////////////
/// \brief %F -- 协程ID
/////////////////
class FiberIdFormatItem : public LogFormatter::FormatItem
{
public:
    FiberIdFormatItem(const QString& str)
        :FormatItem(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << event->GetFiberId();
    }
};
//////////////////
/// \brief 输出字符串到日志流中
/////////////////
class StringFormatItem : public LogFormatter::FormatItem
{
public:
    StringFormatItem(const QString& str)
        :FormatItem(str), string_(str)
    {

    }
    void format(QTextStream& ts, QSharedPointer<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        ts << string_;
    }
private:
    QString string_;
};
//////////////////
/// \brief 日志管理器：管理不同的日志器
/////////////////
class LoggerManager
{
public:
    LoggerManager();
    Logger::ptr GetLogger(const QString& name);

    void Init();
    Logger::ptr GetRoot()const {return root_;}
    QString ToYamlString();

private:
    QMap<QString, Logger::ptr> loggers_;
    Logger::ptr root_;
};

using LoggerMgr = xianyu::Singleton<LoggerManager>;
void TestForDefine();



}

#endif // LOGGER
