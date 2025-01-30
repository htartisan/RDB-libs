///****************************************************************************
/// FILE:    CPluginClassInstMgr.h
///
/// DEESC:   Plugin (lib) interface manager class def
///
/// AUTHOR:  Russ Barker
///


#ifndef _PLUGIN_CLASS_INST_MANAGER_H
#define _PLUGIN_CLASS_INST_MANAGER_H


#include <string>
#include <vector>
#include <memory>
#include <vector>

#include "../Error/CError.h"



template <typename T>
class CPluginClassMgrBase
{
    friend class        CAudioPluginClassMgrBase;
    friend class        CCmdPluginClassMgrBase;

protected:

    std::string         m_sPluginModuleName;

    std::string         m_sPluginDescription;

    int                 m_nPluginType;

public:

    CPluginClassMgrBase()
    {
        clear();
    }

    ~CPluginClassMgrBase()
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
        return m_sPluginModuleName;
    }

    std::string GetPluginDescription()
    {
        return m_sPluginDescription;
    }

    int GetPluginType()
    {
        return m_nPluginType;
    }

    virtual std::shared_ptr<T> CreatePluginClassInstance() = 0;

};



#endif  //  _PLUGIN_CLASS_INST_MANAGER_H