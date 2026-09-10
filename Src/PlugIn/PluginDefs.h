//****************************************************************************
// FILE:    Plugin.h
//
// DEESC:   Plugin (lib) header base file
//
// AUTHOR:  Russ Barker
//


#ifdef WINDOWS
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT 
#endif


#ifndef PLUGIN_MODULE_DEFS
#define PLUGIN_MODULE_DEFS


#include <string>


#ifdef EXPORT_LIB

// if this is included from a "plugin lib" module,
// then export the functions and classes.

#ifndef PLUGIN_LIB_API
#ifdef WINDOWS
#define PLUGIN_LIB_API      __declspec(dllexport)
#else
#define PLUGIN_LIB_API      __attribute__((visibility("default"))) 
#endif
#endif

#else

// if this is included from an application,
// then import the functions and classes.

#ifndef PLUGIN_LIB_API
#ifdef WINDOWS
#define PLUGIN_LIB_API       __declspec(dllimport)
#else
#define PLUGIN_LIB_API       
#endif
#endif

#endif


#ifndef COMPARE_RESULT_DEF
#define COMPARE_RESULT_DEF


enum eCompareResult
{
    CompareResult_lessThan = -1,
    CompareResult_equalTo = 0,
    CompareResult_greaterThan = 1,

    CompareResult_error = 10
};

#endif


// Utility defs


#ifndef VERSION_NUMBER_DEF
#define VERSION_NUMBER_DEF

class CVersionNumber
{
  protected:

      CVersionNumber strToVersionNum(const std::string& sVerNum)
      {
        
          CVersionNumber out;

          auto status = 
              sscanf
              (
                  sVerNum.c_str(), 
                  "%d.%d.%d", 
                  &(out.m_nMajorVersionNum), 
                  &(out.m_nMinorVersionNum), 
                  &(out.m_nSubVersionNum)
              );
          if (status != 3) 
          {
              // String parse error
              out.m_nMajorVersionNum = 0;
              out.m_nMinorVersionNum = 0;
              out.m_nSubVersionNum = 0;
          }

          return out;
      };

  public:

    unsigned int    m_nMajorVersionNum;
    unsigned int    m_nMinorVersionNum;
    unsigned int    m_nSubVersionNum;

  public:

    CVersionNumber() :
        m_nMajorVersionNum(0),
        m_nMinorVersionNum(0),
        m_nSubVersionNum(0)
    {
    }

    CVersionNumber(unsigned int nMajorVer, unsigned int nMinorVer, unsigned int nSubVer) :
        m_nMajorVersionNum(nMajorVer),
        m_nMinorVersionNum(nMinorVer),
        m_nSubVersionNum(nSubVer)
    {
    }

    CVersionNumber(const std::string& sVer)
    {
        (*this) = strToVersionNum(sVer);
    }

    ~CVersionNumber()
    {
    }

    void set(unsigned int nMajorVer, unsigned int nMinorVer, unsigned int nSubVer)
    {
        m_nMajorVersionNum = nMajorVer;
        m_nMinorVersionNum = nMinorVer;
        m_nSubVersionNum = nSubVer;
    }

    void set(const std::string& sVer)
    {
        *(this) = strToVersionNum(sVer);
    }

    CVersionNumber operator=(const std::string& sVer)
    {
        *(this) = strToVersionNum(sVer);

        return *(this);
    }

    eCompareResult compare(unsigned int nMajorVer, unsigned int nMinorVer, unsigned int nSubVer)
    {
        if (nMajorVer > m_nMajorVersionNum)
        {
            return eCompareResult::CompareResult_greaterThan;
        }

        if (nMajorVer < m_nMajorVersionNum)
        {
            return eCompareResult::CompareResult_lessThan;
        }

        if (nMinorVer > m_nMinorVersionNum)
        {
            return eCompareResult::CompareResult_greaterThan;
        }

        if (nMinorVer < m_nMinorVersionNum)
        {
            return eCompareResult::CompareResult_lessThan;
        }

        if (nSubVer > m_nSubVersionNum)
        {
            return eCompareResult::CompareResult_greaterThan;
        }

        if (nSubVer < m_nSubVersionNum)
        {
            return eCompareResult::CompareResult_lessThan;
        }

        return eCompareResult::CompareResult_equalTo;
    }

    eCompareResult compare(const CVersionNumber& ref)
    {
        if (ref.m_nMajorVersionNum > m_nMajorVersionNum)
        {
            return eCompareResult::CompareResult_greaterThan;
        }

        if (ref.m_nMajorVersionNum < m_nMajorVersionNum)
        {
            return eCompareResult::CompareResult_lessThan;
        }

        if (ref.m_nMinorVersionNum > m_nMinorVersionNum)
        {
            return eCompareResult::CompareResult_greaterThan;
        }

        if (ref.m_nMinorVersionNum < m_nMinorVersionNum)
        {
            return eCompareResult::CompareResult_lessThan;
        }

        if (ref.m_nSubVersionNum > m_nSubVersionNum)
        {
            return eCompareResult::CompareResult_greaterThan;
        }

        if (ref.m_nSubVersionNum < m_nSubVersionNum)
        {
            return eCompareResult::CompareResult_lessThan;
        }

        return eCompareResult::CompareResult_equalTo;
    }

    eCompareResult compare(const std::string& sVer)
    {
        CVersionNumber tmp = strToVersionNum(sVer);

        return compare(tmp);
    }

    std::string getString()
    {
        std::string sOut;

        sOut = std::to_string(m_nMajorVersionNum);
        sOut.append(".");
        sOut.append(std::to_string(m_nMinorVersionNum));
        sOut.append(".");
        sOut.append(std::to_string(m_nSubVersionNum));

        return sOut;
    }
};

#endif  //  VERSION_NUMBER_DEF


// define thhe 2 primary functions that will be 
// included / exported from within the plugin libs.

extern "C"
{
    PLUGIN_LIB_API void* CreatePluginFileInfoInstance();

    PLUGIN_LIB_API void* CreatePluginClassInstance(void *pInstData);

};


#endif  //  PLUGIN_MODULE_DEFS
