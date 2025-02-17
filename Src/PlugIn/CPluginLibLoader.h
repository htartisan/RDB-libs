///****************************************************************************
/// FILE:    CPluginLibLoader.h
///
/// DEESC:   Plugin (lib) loader class / function def
///
/// AUTHOR:  Russ Barker
///


#ifndef _PLUGIN_LIB_LOADER_H
#define _PLUGIN_LIB_LOADER_H




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

#include "CPluginFileInfoMgr.h"


#ifdef UNICODE
ERROR_MESSAGE("Unicode NOT supported.  Multi byte compile type must be set.")
#endif


//class CPluginLoader;


template <typename T>
class CPluginLibLoader : public CErrorHandler
{
    typedef void * (*VoidFunctionPtr)();

protected:

    std::string                             m_sFilePath;

#ifdef WINDOWS
    HINSTANCE                               m_hLib;
#else
    void                                    *m_hLib;
#endif

    bool                                    m_bLoaded;

    VoidFunctionPtr                         m_createFileInfoFunc;

    VoidFunctionPtr                         m_createClassInstFunc;

    std::shared_ptr<CPluginFileInfoMgrBase> m_pPluginFileInfoMgr;

public:

    CPluginLibLoader(std::string sPath = "")
    {
        clear();

        m_sFilePath = sPath;  

        m_hLib = nullptr;


        m_createFileInfoFunc= nullptr;
        m_createClassInstFunc = nullptr;

        m_pPluginFileInfoMgr = nullptr;
    }

    ~CPluginLibLoader()
    {
        //clear();

        if (m_hLib != nullptr)
        {
#ifdef WINDOWS
            FreeLibrary(m_hLib);
#else
            free(m_hLib);
#endif
        }
    }

    void clear()
    {
        m_sFilePath.clear();
        
        m_bLoaded = false;

        m_pPluginFileInfoMgr = nullptr;
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

        if (m_hLib == nullptr)
        { 
            m_hLib = LoadLibrary(m_sFilePath.c_str());
        }

        if (m_hLib == nullptr)
        {
            SetErrorText("Failed to load lib at specified path");
            return false;
        }

        try
        { 
            m_createFileInfoFunc = (VoidFunctionPtr) GetProcAddress(m_hLib, "CreatePluginFileInfoInstance");

            m_createClassInstFunc =  (VoidFunctionPtr) GetProcAddress(m_hLib, "CreatePluginClassInstance");
        }
        catch (...)
        {

        }

#else

        if (m_hLib == nullptr)
        {
            m_hLib = dlopen(m_sFilePath.c_str(), RTLD_LAZY | /*RTLD_NOW |*/ RTLD_GLOBAL);
        }

        if (m_hLib == nullptr)
        {
            auto message = dlerror();

            std::string msgText = 
                "Failed to load lib at specified path ";
            msgText.append(message);
            
            SetErrorText(msgText);
            
            return false;
        }

        try
        {
            m_createFileInfoFunc = (VoidFunctionPtr) dlsym(m_hLib, "CreatePluginFileInfoInstance");

            m_createClassInstFunc = (VoidFunctionPtr) dlsym(m_hLib, "CreatePluginClassInstance");
        }
        catch(...)
        {
        }

#endif

        if (m_createFileInfoFunc == nullptr)
        {
            SetErrorText("Failed to load CreatePluginFileInfoInstance - lib type not supported");
            return false;
        }
        else
        {
            void *pFileInfoMgr = nullptr;

            try
            {
                pFileInfoMgr = m_createFileInfoFunc();
            }
            catch (...)
            {

            }

            std::shared_ptr<CPluginFileInfoMgrBase> sharePtr((CPluginFileInfoMgrBase *) pFileInfoMgr);

            m_pPluginFileInfoMgr = sharePtr;
        }

        if (m_createClassInstFunc == nullptr)
        {
            SetErrorText("Failed to load CreatePluginFileInfoInstance");
            return false;
        }

        if (m_pPluginFileInfoMgr != nullptr)
        {
            ClearError();
            return true;
        }

        SetErrorText("Failed to create pligin functions / interfaces");

        return false;
    }

    std::shared_ptr<CPluginFileInfoMgrBase> GetPluginFileInfoMgr()
    {
        return m_pPluginFileInfoMgr;
    }

    std::shared_ptr<T>  createPluginClassInst()
    {   
        void *pClassInst = nullptr;

        try
        { 
            pClassInst = m_createClassInstFunc();
        }
        catch (...)
        {

        }

        std::shared_ptr<T> sharePtr((T*) pClassInst);

        return sharePtr;
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


// template <typename T>
// T * CreatePluginMgrInstance();


#endif  //  _PLUGIN_LOADER_H