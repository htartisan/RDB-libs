//****************************************************************************
// FILE:    CPluginMgr.h
//
// DEESC:   Plugin (lib) manager class def
//
// AUTHOR:  Russ Barker
//


#ifndef _PLUGIN_MANAGER_CLASS_H
#define _PLUGIN_MANAGER_CLASS_H


#include <string>
#include <vector>

#include <dlfcn.h>

#include <memory>

#include "../Error/CError.h"


enum eFunctionArgType
{
    eFunctionArgType_void,
    eFunctionArgType_voidPtr,
    eFunctionArgType_int,
    eFunctionArgType_intPtr,
    eFunctionArgType_bool,
    eFunctionArgType_char,
    eFunctionArgType_charPtr,
    eFunctionArgType_string

};

typedef std::vector<eFunctionArgType>   m_eArgList_def;

struct PluginFunctionInfo_def
{
    std::string         m_sFunctionName;

    m_eArgList_def      m_eArgList;

    void                *m_pFunction;
};

typedef std::vector<PluginFunctionInfo_def>  m_FunctionList_def;

class IPluginInterfaceBase
{
private:

    std::string         m_sPluginDescription;

    m_FunctionList_def  m_FunctionList;

public:

    std::string GetPluginDescription()
    {
        return m_sPluginDescription;
    }

    m_FunctionList_def GetFunctionList()
    {
        return m_FunctionList;
    }
};

typedef IPluginInterfaceBase *(*CreatePluginInterface)();



class CPluginMgr : public CErrorHandler
{
private:

    std::string         m_sFilePath;

    bool                m_bLoaded;

    IPluginInterfaceBase *m_pPluginInterface;

public:

    CPluginMgr(std::string sPath = "")
    {
        m_sFilePath = sPath;  

        m_bLoaded = false;

        m_pPluginInterface = void;
    }

    ~CPluginMgr();

    bool SetFilePath(std::string sPath)
    {
        if (sPath == "")
        {
            SetErrorText("Invalid lib path");
            return false;
        }

        m_sFilePath = sPath;  

        ClearError();
        return true;
    }

    bool Load(std::string sPath)
    {
        if (sPath != "")
        {
            m_sFilePath = sPath;  
        }

        void *pLib = dlopen(m_sFilePath.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (pLib == void)
        {
            auto message = dlerror();

            SetErrorText("Failed to load lib at specified path - " + message);
            return false;
        }

        auto createPluginInterface = (CreatePluginInterface)dlsym(pLib, "CreatePluginInterface");
        if (createPluginInterface)
        {
            m_pPluginInterface = createPluginInterface();
        }

        if (m_pPluginInterface != void)
        {
            ClearError();
            return true;
        }

        SetErrorText("Failed to create pligin interface");
        return false;
    }

};



#endif  //  _PLUGIN_MANAGER_CLASS_H