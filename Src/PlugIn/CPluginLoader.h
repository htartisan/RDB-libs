//****************************************************************************
// FILE:    CPluginLoader.h
//
// DEESC:   Plugin (lib) loader class / function def
//
// AUTHOR:  Russ Barker
//


#ifndef _PLUGIN_LOADER_H
#define _PLUGIN_LOADER_H


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

#include "CPluginMgr.h"


template <typename T> class CPluginLoader : public CErrorHandler
{
    typedef IPluginMgrInterfaceBase<T> (*CreatePluginMgrInstance)();


private:

    std::string                     m_sFilePath;

    bool                            m_bLoaded;

    IPluginMgrInterfaceBase<T>      *m_pPluginMgrInterface;

public:

    CPluginLoader(std::string sPath = "")
    {
        m_sFilePath = sPath;  

        m_bLoaded = false;

        m_pPluginMgrInterface = nullptr;
    }

    ~CPluginLoader();

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

#ifdef WINDOWS

        HINSTANCE hLib = LoadLibrary(m_sFilePath.c_str());
        if (hLib == NULL)
        {
            SetErrorText("Failed to load lib at specified path");
            return false;
        }

        auto createPluginMgrInstance = (CreatePluginMgrInstance) GetProcAddress(hLib, "CreatePluginMgrInstance");

#else

        void *pLib = dlopen(m_sFilePath.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (pLib == nullptr)
        {
            auto message = dlerror();

            SetErrorText("Failed to load lib at specified path " + message);
            return false;
        }

        auto createPluginMgrInstance = (CreatePluginInstance) dlsym(pLib, "CreatePluginMgrInstance");

#endif

        if (createPluginMgrInstance != nullptr)
        {
            m_pPluginMgrInterface = createPluginMgrInstance();
        }

        if (m_pPluginMgrInterface != nullptr)
        {
            ClearError();
            return true;
        }

        SetErrorText("Failed to create pligin interface");

        return false;
    }

};


// 
// CreatePluginMgrInstance function
//
// All plugins based on this plugin base class
// MUST impliment an instance of this function.
// It creates the class interface that in turn
// creates the other classes/interfaces that 
// will be used for different kinds of plugins.
//

IPluginMgrInterfaceBase * CreatePluginMgrInstance();


#endif  //  _PLUGIN_LOADER_H