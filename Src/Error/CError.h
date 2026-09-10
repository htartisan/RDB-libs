///****************************************************************************
/// FILE:    CError.h
///
/// DEESC:   Error handler class def
///
/// AUTHOR:  Russ Barker
///


#ifdef WINDOWS
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT 
#endif


#ifndef _ERROR_HANDLER_CLASS_H
#define _ERROR_HANDLER_CLASS_H

#include <string>


#ifndef COMPILE_ERROR
#define COMPILE_ERROR(msg)  static_assert(1, msg);
#endif


class CErrorHandler
{
private:

    std::string  m_sErrorText;

public:

    CErrorHandler()
    {
        m_sErrorText = "";
    }

    ~CErrorHandler()
    {
        ClearError();
    }

    void ClearError()
    {
        m_sErrorText = "";
    }

    void SetErrorText(const std::string &sText)
    {
        m_sErrorText = sText;
    }

    std::string GetErrorText()
    {
        return m_sErrorText;
    }
};




#endif  //  _ERROR_HANDLER_CLASS_H
