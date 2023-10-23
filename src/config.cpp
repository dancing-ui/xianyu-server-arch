#include "config.h"

namespace xianyu {

static void ListAllMember(const QString& prefix, const YAML::Node& node, QList<QPair<QString, YAML::Node>>& output)
{
    if(FindFirstNotOf(prefix, "abcdefghijklmnopqrstuvwxyz._0123456789") == true)
    {
        XIANYU_LOG_ERROR(XIANYU_LOG_ROOT()) << "Config invalid name: " << prefix << " : " <<QString::fromStdString(node.as<std::string>());
        return;
    }
    output.push_back({prefix, node});
    if(node.IsMap())
    {
        for(auto it = node.begin();it!=node.end();it++)
        {
            ListAllMember(prefix.isEmpty() ? QString::fromStdString(it->first.as<std::string>()) : prefix + "." + QString::fromStdString(it->first.Scalar()), it->second, output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node &root)
{
    QList<QPair<QString, YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);
//    for(auto& [x,y]:all_nodes)
//    {
//        qDebug()<<x;
//    }

    for(auto& [key, val] : all_nodes)
    {
        if(key.isEmpty())
        {
            continue;
        }
        key = key.toLower();
        auto var = LookupBase(key);

        if(var.get() != nullptr)
        {
            if(val.IsScalar())
            {
                var->FromString(QString::fromStdString(val.Scalar()));
            }
            else
            {
                std::stringstream ss;
                ss << val;
                var->FromString(QString::fromStdString(ss.str()));
            }
        }
    }
}

ConfigVarBase::ptr Config::LookupBase(const QString &name)
{
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it.value();
}

}

