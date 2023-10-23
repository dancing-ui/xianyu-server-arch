#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSharedPointer>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

#include <functional>
#include "log.h"

#define BOOST_VARIANT_USE_RELAXED_GET_BY_DEFAULT

namespace xianyu {

class ConfigVarBase
{
public:
    using ptr = QSharedPointer<ConfigVarBase>;
    ConfigVarBase(const QString& name, const QString& description = "")
        : name_(name.toLower()), description_(description) // yaml的键全为小写字符串，比如Format: ABc转为format: ABc
    {

    }
    virtual ~ConfigVarBase(){}

    const QString& GetName()const {return name_;}
    const QString& GetDescription()const {return description_;}

    virtual QString ToString() = 0;
    virtual bool FromString(const QString& val) = 0;

    virtual QString GetTypeName() const = 0;

protected:
    QString name_;
    QString description_;
};

//F from_type, T to_type
template<class F, class T>
class LexicalCast
{
public:
    T operator()(const F& v)
    {
        return boost::lexical_cast<T>(v);
    }
};

//QString ==> T, T为基础类型
////
/// \brief 该类是对std::string再进行一层包装
///
template<class T>
class LexicalCast<QString, T>
{
public:
    T operator()(const QString& v)
    {
        YAML::Node node = YAML::Load(v.toStdString());
        std::stringstream ss;
        ss.str("");
        ss << node;
        return LexicalCast<std::string, T>()(ss.str());
    }
};
//T ==> QString, T为基础类型
////
/// \brief 该类是对std::string再进行一层包装
///
template<class T>
class LexicalCast<T, QString>
{
public:
    QString operator()(const T& v)
    {
        YAML::Node node = YAML::Load(LexicalCast<T, std::string>()(v));
        std::stringstream ss;
        ss << node;
        return QString(ss.str().c_str());
    }
};

//QString ==> QList, QVector
template<class T>
class LexicalCast<QString, QList<T>>
{
public:
    QList<T> operator()(const QString& v)
    {
        YAML::Node node = YAML::Load(v.toStdString());
        QList<T> list;
        std::stringstream ss;
        for(std::size_t i = 0;i<node.size();i++)
        {
            ss.str("");
            ss << node[i];
            list.push_back(LexicalCast<QString, T>()(QString(ss.str().c_str())));
        }
        return list;
    }
};
//QList, QVector ==> QString
template<class T>
class LexicalCast<QList<T>, QString>
{
public:
    QString operator()(const QList<T>& v)
    {
        YAML::Node node;
        for(auto& i:v)
        {
            node.push_back(YAML::Load(LexicalCast<T, QString>()(i).toStdString()));
        }
        std::stringstream ss;
        ss << node;
        return QString(ss.str().c_str());
    }
};

//QString ==> QSet
template<class T>
class LexicalCast<QString, QSet<T>>
{
public:
    QSet<T> operator()(const QString& v)
    {
        YAML::Node node = YAML::Load(v.toStdString());
        QSet<T> set;
        std::stringstream ss;
        for(std::size_t i = 0;i<node.size();i++)
        {
            ss.str("");
            ss << node[i];
            set.insert(LexicalCast<QString, T>()(QString(ss.str().c_str())));
        }
        return set;
    }
};
//QSet ==> QString
template<class T>
class LexicalCast<QSet<T>, QString>
{
public:
    QString operator()(const QSet<T>& v)
    {
        YAML::Node node;
        for(auto& i:v)
        {
            node.push_back(YAML::Load(LexicalCast<T, QString>()(i).toStdString()));
        }
        std::stringstream ss;
        ss << node;
        return QString(ss.str().c_str());
    }
};

//QString ==> QMap
template<class T>
class LexicalCast<QString, QMap<QString, T>>
{
public:
    QMap<QString, T> operator()(const QString& v)
    {
        YAML::Node node = YAML::Load(v.toStdString());
        QMap<QString, T> map;
        std::stringstream ss;
        for(auto it = node.begin();it!=node.end();it++)
        {
            ss.str("");
            ss << it->second;
            map[QString::fromStdString(it->first.Scalar())] = LexicalCast<QString, T>()(QString(ss.str().c_str()));
        }
        return map;
    }
};
//QMap ==> QString
template<class T>
class LexicalCast<QMap<QString, T>, QString>
{
public:
    QString operator()(const QMap<QString, T>& v)
    {
        YAML::Node node;
        for(auto it = v.begin();it!=v.end();it++)
        {
            node[it.key().toStdString()] = YAML::Load(LexicalCast<T, QString>()(it.value()).toStdString());
        }
        std::stringstream ss;
        ss << node;
        return QString(ss.str().c_str());
    }
};

class Person
{
public:
    QString name_;
    int age_;
    bool sex_;

    QString ToString()const
    {
        std::stringstream ss;
        ss << "[Person name=" << name_.toStdString()
           << " age=" << age_
           << " sex=" <<sex_ <<']';
        return QString(ss.str().c_str());
    }

    bool operator==(const Person& p) const
    {
        return name_==p.name_
               &&age_==p.age_
               &&sex_==p.sex_;
    }
};
//QString ==> Person
template<>
class LexicalCast<QString, Person>
{
public:
    Person operator()(const QString& v)
    {
        YAML::Node node = YAML::Load(v.toStdString());
        Person p;
        p.name_ = QString(node["name"].as<std::string>().c_str());
        p.age_ = node["age"].as<int>();
        p.sex_ = node["sex"].as<bool>();
        return p;
    }
};
//Person ==> QString
template<>
class LexicalCast<Person, QString>
{
public:
    QString operator()(const Person& p)
    {
        YAML::Node node;
        node["name"] = p.name_.toStdString();
        node["age"] = p.age_;
        node["sex"] = p.sex_;
        std::stringstream ss;
        ss << node;
        return QString(ss.str().c_str());
    }
};

template<class T, class FromStr = LexicalCast<QString, T>, class ToStr = LexicalCast<T, QString>>
class ConfigVar : public ConfigVarBase
{
public:
    using ptr = QSharedPointer<ConfigVar>;
    using on_change_cb = std::function<void(const T& old_value, const T& new_value)>;//cb ==> CallBack回调
    ConfigVar(const QString& name, const T& default_value, const QString& description)
        : ConfigVarBase(name, description)
        , val_(default_value)
    {

    }

    QString ToString() override
    {
        try {
            return ToStr()(val_);
        } catch(std::exception& e) {
            XIANYU_LOG_ERROR(XIANYU_LOG_ROOT()) << "ConfigVar::ToString exception" << e.what()
                                                << "convert: " << typeid(val_).name() << " to QString";
        }
        return "";
    }

    bool FromString(const QString& val) override
    {
        try {
            SetValue(FromStr()(val));
            return true;;
        } catch(std::exception& e) {
            XIANYU_LOG_ERROR(XIANYU_LOG_ROOT()) << " ConfigVar::FromString exception " << e.what()
                                                << " convert: QString to " << typeid(val_).name();
        }
        return false;
    }

    const T GetValue()const {return val_;}
    void SetValue(const T& v)
    {
        if(v == val_)
        {
            return;
        }
        for(auto& func : cbs_)
        {
            func(val_, v);
        }
        val_ = v;
    }

    QString GetTypeName() const override { return typeid(T).name(); }

    void AddListener(quint64 key, on_change_cb cb)
    {
        cbs_[key] = cb;
    }
    void DelListener(quint64 key)
    {
        cbs_.erase(key);
    }
    on_change_cb GetListener(quint64 key)
    {
        auto it = cbs_.find(key);
        return it == cbs_.end() ? nullptr : it.value();
    }
private:
    T val_;
    //变更回调函数组,用hash值作为key,每个hash值对应一个回调
    QMap<quint64, on_change_cb> cbs_;
};

class Config
{
public:
    using ConfigVarMap = QMap<QString, ConfigVarBase::ptr>;

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const QString& name)
    {
        auto it = GetDatas().find(name);
        if(it == GetDatas().end())
        {
            return nullptr;
        }
        return qSharedPointerDynamicCast<ConfigVar<T>>(it.value());
    }

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const QString& name, const T& default_value, const QString& description = "")
    {
        auto it = GetDatas().find(name);
        if(it != GetDatas().end())
        {
            auto tmp = qSharedPointerDynamicCast<ConfigVar<T>>(it.value());
            if(tmp.get() != nullptr)
            {
                XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << "Looup name=" << name << " exists";
                return tmp;
            }
            else
            {
                XIANYU_LOG_ERROR(XIANYU_LOG_ROOT()) << "Lookup name=" << name << "exists but type not " << typeid(T).name() << " real_type=" << it.value()->GetTypeName()
                                                    << " " << it.value()->ToString();
                return nullptr;
            }
        }

        if(FindFirstNotOf(name, "abcdefghijklmnopqrstuvwxyz._0123456789") == true)
        {
            XIANYU_LOG_ERROR(XIANYU_LOG_ROOT()) << "Lookup name invalid " << name;
            throw std::invalid_argument(name.toStdString());
        }
        //在C++中，typename 关键字通常用于告诉编译器，后面的标识符是一个类型名字（type name）。这是因为在模板的上下文中，有时编译器可能无法确定标识符是一个类型名还是一个变量名，因此需要使用 typename 明确指示它是一个类型。
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        GetDatas()[name] = v;
        return v;
    }

    static void LoadFromYaml(const YAML::Node& root);

    static ConfigVarBase::ptr LookupBase(const QString& name);

private:
    static ConfigVarMap& GetDatas()
    {
        static ConfigVarMap datas_;
        return datas_;
    }

};


static void PrintYaml(const YAML::Node& node, int level)
{
    if(node.IsScalar())
    {
        XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << QString::fromStdString(std::string(level * 4,' ')) << QString::fromStdString(node.Scalar()) << " - "<< node.Type() << " - "<< level;
    }
    else if(node.IsNull())
    {
        XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << QString::fromStdString(std::string(level * 4, ' ')) << "NULL - "<<node.Type() <<" - "<<level;
    }
    else if(node.IsMap())
    {
        for(auto it = node.begin();it!=node.end();it++)
        {
            XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << QString::fromStdString(std::string(level * 4, ' ')) << QString::fromStdString(it->first.as<std::string>()) << " - "<<it->second.Type() << " - "<<level;
            PrintYaml(it->second, level + 1);
        }
    }
    else if(node.IsSequence())
    {
        for(qsizetype i = 0;i<node.size();i++)
        {
            XIANYU_LOG_INFO(XIANYU_LOG_ROOT()) << QString::fromStdString(std::string(level * 4, ' ')) << i << " - "<<node[i].Type() << " - "<<level;
            PrintYaml(node[i], level + 1);
        }
    }
}


}



#endif // CONFIG_H
