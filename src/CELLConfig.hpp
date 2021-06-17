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
private:
    //当前程序的路径
    std::string _exePath;
    //配置传入的key-value数据
    std::map<std::string, std::string> _kv;
};
#endif