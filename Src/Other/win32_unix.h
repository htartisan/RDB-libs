//**************************************************************************************************
//* FILE:		win32_unix.h
//*
//* DESCRIP:	
//*
//*


#ifndef win32_unix_H_
#define win32_unix_H_


#ifdef WINDOWS

//#include <windows.h>

#else

#include <cassert>
#include <time.h>
//#include "Thread/thread.h"
#include <vector>
#include <chrono>
#include <stdexcept>


void Sleep(int s);  //throw();
int timeGetTime();  //throw();

#define DWORD int

typedef const char * LPCTSTR;
typedef char * LPTSTR;
typedef int INT;
typedef unsigned int UINT;
typedef char TCHAR;

#ifndef _ASSERTE
#define _ASSERTE assert
#endif
#ifndef ASSERT
#define ASSERT assert
#endif
#define SYSTEMTIME struct tm
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif


inline void GetSystemTime(const time_t *pTime)   //throw()
{
    std::localtime(pTime);
}


#define HRESULT bool
#define FAILED(x) (!x)
#define SUCCEEDED(x) (x)


inline bool CoInitialize(void *)
{
    return true;
}

inline void CoUninitialize() {}

#ifdef WINDOWS
#define Win32SyncObject*    HANDLE
#else
#define Win32SyncObject    void
#endif

DWORD GetPrivateProfileString(
    LPCTSTR lpAppName,
    LPCTSTR lpKeyName,
    LPCTSTR lpDefault,
    LPTSTR lpReturnedString,
    DWORD nSize,
    LPCTSTR lpFileName
);  //throw();

UINT GetPrivateProfileInt(
    LPCTSTR lpAppName,
    LPCTSTR lpKeyName,
    INT nDefault,
    LPCTSTR lpFileName
);  //throw();


////// reference count base stuff
class refCountBase
{
    template <class T> friend class refPtr;
public:
    /// return the number of references to this object (useful for debugging)
    int RefCount() const
    {
        return m_refCount;
    }

protected:
    refCountBase() : m_refCount(0) {}
    virtual ~refCountBase()
    {
        // make sure nobody has deleted this directly with delete
        assert(m_refCount == 0);
    }

private:
    mutable int m_refCount;

    void RefIncrement() const
    {
        ++m_refCount;
    }
    int RefDecrement() const
    {
        return --m_refCount;
    }
};


template <class T>
class refPtr
{
public:
    typedef T *pointer;

    // construction
    refPtr() : m_object(NULL) {}
    refPtr(const refPtr<T> &rhs) : m_object(rhs.m_object)
    {
        if (m_object)
            m_object->refCountBase::RefIncrement();
    }
    refPtr(T *object) : m_object((refCountBase *)object)
    {
        if (m_object)
            m_object->refCountBase::RefIncrement();
    }
    ~refPtr()
    {
        if (m_object && m_object->refCountBase::RefDecrement() == 0)
            delete m_object;
    }

    // asignment
    refPtr<T> &operator=(const refPtr<T> &rhs)
    {
        if (m_object != rhs.m_object)
        {
            if (m_object && m_object->refCountBase::RefDecrement() == 0)
                delete m_object;
            m_object = rhs.m_object;
            if (m_object)
                m_object->refCountBase::RefIncrement();
        }
        return *this;
    }


    /// test if pointers are the same
    inline bool operator==(const refPtr<T> &rhs) const
    {
        return m_object == rhs.m_object;
    }

    inline bool operator==(void *nl) const
    {
        return m_object == nl;
    }

    /// test if pointers are not the same
    inline bool operator!=(const refPtr<T> &rhs) const
    {
        return m_object != rhs.m_object;
    }

    inline bool operator!=(void *nl) const
    {
        return m_object != nl;
    }

    // dereferencing
    T *operator->() const
    {
        assert(m_object);
        return (T *)m_object;
    }
    T &operator*() const
    {
        assert(m_object);
        return *((T *)m_object);
    }
    operator T *() const
    {
        return (T *)m_object;
    }

    /// explicit get - do NOT delete this pointer!
    T *get() const
    {
        return (T *)m_object;
    }

    /** these member templates allows conversion of this smart pointer to
    	other smart pointers in the parent hierarchy. This simulates up-casting
    	the pointer to a base */
    template <class newType>
    operator refPtr<newType>()
    {
        return refPtr<newType>((T *)m_object);
    }
    template <class newType>
    operator const refPtr<newType>() const
    {
        return refPtr<newType>((T *)m_object);
    }

private:
    refCountBase *m_object;
};


#endif

#endif

