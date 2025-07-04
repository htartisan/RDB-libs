//******************************************************
//  Console mgr class definition
//


#ifndef C_CONSOLE_MGR_H
#define C_CONSOLE_MGR_H

#include <cstdio>

#ifdef WINDOWS
#include <stdio.h>
#include <conio.h>
#else
//#include "KbInput.h"
#include "KbInput.c"
#endif


class CConsoleMgr
{
#ifndef WINDOWS
    struct termios      m_tioSave;

    static const int    STDIN = 0;	
#endif

    bool                m_bKbBufferingDisabled;
    bool                m_bKbEchoDisabled;

public:

    CConsoleMgr()
    {
        m_bKbBufferingDisabled = false;
        m_bKbEchoDisabled = false;

#ifndef WINDOWS
        saveTermios(m_tioSave);
#endif
    }

    ~CConsoleMgr()
    {
        if (m_bKbBufferingDisabled == true || m_bKbEchoDisabled == true)
        {
            restore();
        }
    }

    void restore()
    {
#ifndef WINDOWS
        resetTermios(m_tioSave);
#endif
    }

    bool dsableKbBuffering(const bool val)
    {
#ifndef WINDOWS
        struct termios  save;
#endif

        if (val == true)
        {
            if (m_bKbBufferingDisabled == true)
            {
                return false;
            }

#ifndef WINDOWS
            setTermiosBuffering(false, save);
#endif

            m_bKbBufferingDisabled = true;
        }
        else
        {
            if (m_bKbBufferingDisabled == false)
            {
                return false;
            }

#ifndef WINDOWS
            setTermiosBuffering(true, save);
#endif

            m_bKbBufferingDisabled = false;
        }

        return true;
    }

    bool dsableKbEcho(const bool val)
    {
#ifndef WINDOWS
        struct termios  save;
#endif

        if (val == true)
        {
            if (m_bKbEchoDisabled == true)
            {
                return false;
            }

#ifndef WINDOWS
            setTermiosEcho(false, save);
#endif

            m_bKbEchoDisabled = true;
        }
        else
        {
            if (m_bKbEchoDisabled == false)
            {
                return false;
            }

#ifndef WINDOWS
            setTermiosEcho(true, save);
#endif

            m_bKbEchoDisabled = false;
        }

        return true;
    }

    int getNumKeyPress()
    {
        int bytesWaiting = 0;
	
#ifdef WINDOWS
        bytesWaiting = (int) _kbhit();
#else
        ioctl(STDIN, FIONREAD, &bytesWaiting);
#endif

        return bytesWaiting;
    }

    char getachar()
    {
        char ch = 0;

#ifdef WINDOWS
        if (m_bKbEchoDisabled == true)
        {
            ch = (char) _getch();
        }
        else
        {
            ch = (char) _getche();
        }
#else
        ch = (char) getchar();
#endif

        return ch;
    }

};

#endif  //  #ifndef C_CONSOLE_MGR_H
