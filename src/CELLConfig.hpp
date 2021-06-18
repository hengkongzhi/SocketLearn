#ifndef _CELL_CONFIG_HPP_
#define _CELL_CONFIG_HPP_
#include <string>
#include <string.h>
#include "CELLLog.hpp"
#include <map>
class CELLConfig
{
private:
    CELLConfig()
    {

    }
    ~CELLConfig()
    {
        
    }
public:
    static CELLConfig& Instance()
    {
        static CELLConfig obj;
        return obj;
    }
    void Init(int argc, char* args[])
    {
        _exePath = args[0];
        for (int i = 1; i < argc; i++)
        {
            madeCmd(args[i]);
        }
    }
    void madeCmd(char* cmd)
    {
        char* val = strchr(cmd, '=');
        if (val)
        {
            *val = '\0';
            val++;
            _kv[cmd] = val;
            CELLLOG_Debug("madeCmd k<%s> v<%s>", cmd, val);
        }
        else
        {
            _kv[cmd] = "";
            CELLLOG_Debug("madeCmd k<%s>", cmd);
        }

    }
    const char* getStr(const char* argName, const char* def)
    {
        auto itr = _kv.find(argName);
        if (itr == _kv.end())
        {
            CELLLOG_Error("CELLConfig::getStr no find argName=%s", argName);
        }
        else
        {
            def = itr->second.c_str();
        }
        CELLLOG_Info("CELLConfig::getStr %s=%s", argName, def);
        return def;
    }
    int getInt(const char* argName, int def)
    {
        auto itr = _kv.find(argName);
        if (itr == _kv.end())
        {
            CELLLOG_Error("CELLConfig::getInt no find argName=%s", argName);
        }
        else
        {
            def = atoi(itr->second.c_str());
        }
        CELLLOG_Info("CELLConfig::getInt %s=%d", argName, def);
        return def;
    }
    bool hasKey(const char* key)
    {
        auto itr = _kv.find(key);
        return itr != _kv.end();
    }
private:
    //当前程序的路径
    std::string _exePath;
    //配置传入的key-value数据
    std::map<std::string, std::string> _kv;
};
#endif