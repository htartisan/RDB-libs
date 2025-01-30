///****************************************************************************
/// FILE:    CPluginLoader.h
///
/// DEESC:   Plugin (lib) loader class / function def
///
/// AUTHOR:  Russ Barker
///


#ifndef _PLUGIN_LOADER_H
#define _PLUGIN_LOADER_H




#include <string>
#include <vector>
#include <memory>
#include <vector>

#ifdef WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "../Error/CError.h"

#include "CPluginClassInstMgr.h"


#ifdef UNICODE
ERROR_MESSAGE("Unicode NOT supported.  Multi byte compile type must be set.")
#endif


//class CPluginLoader;



template <typename T> 
class CPluginLoader : public CErrorHandler
{
    typedef T * (*createPluginFileMgrFunc)();

protected:

    std::string            m_sFilePath;

    bool                   m_bLoaded;

    T                      *m_pPluginFileInstMgr;

public:

    CPluginLoader(std::string sPath = "")
    {
        clear();

        m_sFilePath = sPath;  
    }

    ~CPluginLoader()
    {
        clear();
    }

    void clear()
    {
        m_sFilePath.clear();
        
        m_bLoaded = false;

        m_pPluginFileInstMgr = nullptr;
    }

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

        auto createPluginFileMgr = (createPluginFileMgrFunc) GetProcAddress(hLib, "CreatePluginMgrInstance");

#else

        void *pLib = dlopen(m_sFilePath.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (pLib == nullptr)
        {
            auto message = dlerror();

            SetErrorText("Failed to load lib at specified path " + message);
            return false;
        }

        auto createPluginFileMgr = (createPluginFileMgrFunc) dlsym(pLib, "CreatePluginMgrInstance");

#endif

        if (createPluginFileMgr != nullptr)
        {
            m_pPluginFileInstMgr = (T *) createPluginFileMgr();
        }

        if (m_pPluginFileInstMgr != nullptr)
        {
            ClearError();
            return true;
        }

        SetErrorText("Failed to create pligin interface");

        return false;
    }

    T * GetPluginFileInstMgr()
    {
        return m_pPluginFileInstMgr;
    }

};




/// 
/// CreatePluginMgrInstance function
///
/// All plugins based on this plugin base class
/// MUST impliment an instance of this function.
/// It creates the class interface pointer to a 
/// class derrived from the CPluginClassInstMgrBase 
/// that in turn creates the other classes/interfaces 
/// that will be used for different kinds of plugins.
///


template <typename T>
T * CreatePluginMgrInstance();


#endif  //  _PLUGIN_LOADER_H