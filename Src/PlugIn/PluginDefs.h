//****************************************************************************
// FILE:    Plugin.h
//
// DEESC:   Plugin (lib) header base file
//
// AUTHOR:  Russ Barker
//


#define _CRT_SECURE_NO_WARNINGS


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

          size_t dotPos = 0;

          std::string sTmp1 = "";
          std::string sTmp2 = "";

          dotPos = sVerNum.find('.', 1);

          if (dotPos == std::string::npos)
          {
              out.m_nMajorVersionNum = atoi(sVerNum.c_str());
              return out;
          }

          sTmp1 = sVerNum.substr(0, dotPos);

          out.m_nMajorVersionNum = atoi(sTmp1.c_str());

          sTmp2 = sVerNum.substr((dotPos + 1));

          dotPos = sTmp2.find('.', 1);

          if (dotPos == std::string::npos)
          {
              out.m_nMinorVersionNum = atoi(sTmp2.c_str());
              return out;
          }

          sTmp1 = sTmp2.substr(0, dotPos);

          out.m_nMinorVersionNum = atoi(sTmp1.c_str());

          sTmp1 = sTmp2.substr((dotPos + 1));

          out.m_nMajorVersionNum = atoi(sTmp1.c_str());

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
        sOut = std::to_string(m_nMinorVersionNum);
        sOut.append(".");
        sOut = std::to_string(m_nSubVersionNum);

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
