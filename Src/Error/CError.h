///****************************************************************************
/// FILE:    CError.h
///
/// DEESC:   Error handler class def
///
/// AUTHOR:  Russ Barker
///


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
