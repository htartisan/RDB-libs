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
#include "../String/StrUtils.h"
#else
#include <dlfcn.h>
#endif

#include "../Error/CError.h"

#include "CPluginFileInfoMgr.h"

#ifdef WINDOWS
#include <codecvt>
#include <libloaderapi.h> 
#endif


#ifdef UNICODE
ERROR_MESSAGE("Unicode NOT supported.  Multi byte compile type must be set.")
#endif

#include "../FileIO/FileUtils.h"


//class CPluginLoader;



template <typename T>
class CPluginLibLoader : public CErrorHandler
{
    typedef void * (*VoidFunctionPtr)();

    typedef void* (*VoidPtrFunctionPtr)(void *);

protected:

    std::string                             m_sSharedLibSearchPath;

    std::string                             m_sFilePath;

#ifdef WINDOWS
    HINSTANCE                               m_hLib;
#else
    void                                    *m_hLib;
#endif

    bool                                    m_bLoaded;

    VoidFunctionPtr                         m_createFileInfoFunc;

    VoidPtrFunctionPtr                      m_createClassInstFunc;

    std::shared_ptr<CPluginFileInfoMgrBase> m_pPluginFileInfoMgr;

public:

    CPluginLibLoader(std::string sFilePath = "", std::string sSearchPath = "")
    {
        clear();

        m_sSharedLibSearchPath = sSearchPath;

        m_sFilePath = sFilePath;

        m_hLib = nullptr;

        m_bLoaded = false;

        m_createFileInfoFunc= nullptr;
        m_createClassInstFunc = nullptr;

        m_pPluginFileInfoMgr = nullptr;
    }

    ~CPluginLibLoader()
    {
        //clear();

        if (m_hLib != nullptr)
        {
            try
            { 
#ifdef WINDOWS
                FreeLibrary(m_hLib);
#else
                free(m_hLib);
#endif
            }
            catch(...)
            {
            }

            m_hLib = 0;
        }
    }

    void clear()
    {
        m_sFilePath.clear();
        
        m_bLoaded = false;

        m_pPluginFileInfoMgr = nullptr;
    }

    bool SetSharedLibSearchPath(std::string sPath)
    {
        if (sPath == "")
        {
            SetErrorText("Invalid lib search path");
            return false;
        }

        m_sSharedLibSearchPath = sPath;

        ClearError();

        return true;
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
            std::string sAbsPath = "";

            if (m_sSharedLibSearchPath != "")
            { 
                try
                { 
                    sAbsPath = getAbsolutePath(m_sSharedLibSearchPath);
                    std::wstring wsPath = StrUtils::tows(sAbsPath.c_str());

                    auto status = AddDllDirectory(wsPath.c_str());

                    if (status == 0)
                    {
                        std::string sErrText = ("Problem while adding DLL serach folder: ");
                        sErrText.append(sAbsPath);
                        SetErrorText(sErrText);
                    }
                }
                catch (...)
                {
                    std::string sErrText = ("Problem while adding DLL serach folder: ");
                    sErrText.append(sAbsPath);
                    SetErrorText(sErrText);
                }
            }

            try
            { 
                sAbsPath = getAbsolutePath(m_sFilePath);

                m_hLib = LoadLibrary(sAbsPath.c_str());
            }
            catch (...)
            {
                std::string sErrText = ("Problem while loading shared library: ");
                sErrText.append(sAbsPath);
                SetErrorText(sErrText);
            }
        }

        if (m_hLib == nullptr)
        {
            DWORD errorNo = GetLastError();

            LPSTR errorMsg = nullptr; // Pointer for the formatted error message

            FormatMessage
            (
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorNo,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPSTR) &errorMsg,
                0,
                NULL
            );

            std::string sErrText = ("Failed to load lib at specified path - ErrNo:");
            sErrText.append(StrUtils::tos(errorNo));
            sErrText.append(", ErrMsg:");
            sErrText.append(errorMsg);

            SetErrorText(sErrText);
            
            return false;
        }

        try
        { 
            m_createFileInfoFunc = (VoidFunctionPtr) GetProcAddress(m_hLib, "CreatePluginFileInfoInstance");

            m_createClassInstFunc =  (VoidPtrFunctionPtr) GetProcAddress(m_hLib, "CreatePluginClassInstance");
        }
        catch (...)
        {
            std::string sErrText = ("Problem getting entry pointer for shared library: ");
            sErrText.append(m_sFilePath);
            SetErrorText(sErrText);
        }

#else

        if (m_hLib == nullptr)
        {
            try
            { 
                m_hLib = dlopen(m_sFilePath.c_str(), RTLD_LAZY | /*RTLD_NOW |*/ RTLD_GLOBAL);
            }
            catch (...)
            {
                std::string sErrText = ("Problem while loading shared library: ");
                sErrText.append(m_sFilePath);
                SetErrorText(sErrText);
            }
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

            m_createClassInstFunc = (VoidPtrFunctionPtr) dlsym(m_hLib, "CreatePluginClassInstance");
        }
        catch(...)
        {
            std::string sErrText = ("Problem getting entry pointer for shared library: ");
            sErrText.append(m_sFilePath);
            SetErrorText(sErrText);
        }

#endif

        if (m_createFileInfoFunc == nullptr)
        {
            SetErrorText("Failed to load CreatePluginFileInfoInstance - lib type not supported");
            return false;
        }

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

        if (m_pPluginFileInfoMgr == nullptr)
        {
            SetErrorText("Failed to create pligin functions / interfaces");
            return false;
        }

        ClearError();

        m_bLoaded = true;

        return true;
    }

    std::shared_ptr<CPluginFileInfoMgrBase> GetPluginFileInfoMgr()
    {
        return m_pPluginFileInfoMgr;
    }

    std::shared_ptr<T>  createPluginClassInst(void *pInstData)
    {   
        void *pClassInst = nullptr;

        try
        { 
            pClassInst = m_createClassInstFunc(pInstData);
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