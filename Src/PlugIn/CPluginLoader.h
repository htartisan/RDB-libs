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
#include <mutex>


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


#ifdef WINDOWS
typedef HINSTANCE        LibHandle_def;
#else
typedef void *           LibHandle_def;
#endif


template <typename T> 
class CPluginLoader : 
    public CErrorHandler
{
    typedef T * (*createPluginFileMgrFunc)();

protected:

    std::string            m_sFilePath;

    bool                   m_bLoaded;

    T                      *m_pPluginFileInstMgr;

    LibHandle_def           m_hLib;

    std::mutex              m_mutex;

public:

    CPluginLoader(std::string sPath = "", LibHandle_def hLib = nullptr)
    {
        m_sFilePath = sPath;  

        m_hLib = hLib;

        if (m_hLib != nullptr)
        {
            m_bLoaded = true;
        }
        else
        {
            m_bLoaded = false;
        }

        m_pPluginFileInstMgr = nullptr;
    }

    ~CPluginLoader()
    {
        //clear();
    }

    void clear()
    {
        if (m_bLoaded == true)
        {
#ifdef WINDOWS
            FreeLibrary(m_hLib);
#else
            dlclose(m_hLib);
#endif
        }

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

        try
        {
            std::scoped_lock lock(m_mutex);

#ifdef WINDOWS

            m_hLib = LoadLibrary(m_sFilePath.c_str());
            if (m_hLib == NULL)
            {
                SetErrorText("Failed to load lib at specified path");
                return false;
            }

            auto createPluginFileMgr = (createPluginFileMgrFunc) GetProcAddress(m_hLib, "CreatePluginMgrInstance");

#else

            m_Lib = dlopen(m_sFilePath.c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (m_Lib == nullptr)
            {
                auto message = dlerror();
                SetErrorText("Failed to load lib at specified path " + message);
                return false;
            }

            auto createPluginFileMgr = (createPluginFileMgrFunc) dlsym(m_hLib, "CreatePluginMgrInstance");

#endif
        }
        catch(const std::exception& e)
        {
            SetErrorText(e.what());
        }
        catch(...)
        {
            SetErrorText("Unknown exception during CPluginLoader::Load");
        }

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

    LibHandle_def getLibHandle()
    {
        return m_hLib;
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