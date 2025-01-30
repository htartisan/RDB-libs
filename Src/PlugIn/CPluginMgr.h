//****************************************************************************
// FILE:    CPluginMgr.h
//
// DEESC:   Plugin (lib) interface manager class def
//
// AUTHOR:  Russ Barker
//


#ifndef _PLUGIN_MANAGER_CLASS_H
#define _PLUGIN_MANAGER_CLASS_H


#include <string>
#include <vector>

#ifdef WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <memory>
#include <vector>

#include "../Error/CError.h"


// All plugins should include the definition of a class derrived from this class

template <typename T> class CPluginFileInstMgrBase
{
private:

    std::string         m_sPluginModuleName;

    std::string         m_sPluginDescription;

    int                 m_nPluginType;

public:

    CPluginMgrInterfaceBase()
    {
        clear();
    }

    ~CPluginMgrInterfaceBase()
    {
        clear();
    }

    void clear()
    {
        m_sPluginModuleName.clear();
        m_sPluginDescription.clear();
        m_nPluginType = 0;
    }

    std::string GetPluginModuleName()
    {
        return m_sPluginName;
    }

    std::string GetPluginDescription()
    {
        return m_sPluginDescription;
    }

    int GetPluginType()
    {
        return m_nPluginType;
    }

    std::shared_ptr<T> CreatePluginInstance()
    {
        auto pPluginInterface = std::make_shared<T>();

        return pPluginInterface;
    }

};




#endif  //  _PLUGIN_MANAGER_CLASS_H