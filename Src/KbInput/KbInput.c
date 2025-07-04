
//******************************************************
//  Linux (POSIX) implementation of _kbhit() and _getch()
//


#pragma once

#include "KbInput.h"


// Save the current terminal i/o settings
void saveTermios(struct termios &save) 
{
    static const int STDIN = 0;
	
    // save old terminal i/o settings 
    tcgetattr(STDIN, &save);    
}


// Set terminal i/o settings (set KB Echo)
void setTermiosEcho(const bool bEcho, struct termios &save) 
{
    static const int STDIN = 0;
	
    // save old terminal i/o settings 
    tcgetattr(STDIN, &save);    

    // make new settings same as old settings 
    struct termios termIO;
    
    termIO = save;
    termIO.c_lflag &= ~ICANON;          // disable buffered i/o 

    if (bEcho == true) 
    {
        termIO.c_lflag |= ECHO;         // set echo mode 
    } 
    else 
    {
        termIO.c_lflag &= ~ECHO;        // set no echo mode
    }

    tcsetattr(STDIN, TCSANOW, &termIO); // use these new terminal i/o settings now 

	setbuf(stdin, NULL);
}


// Initialize new terminal i/o settings (no KB Buffering)
void setTermiosBuffering(const bool bBuffering, struct termios &save) 
{
    static const int STDIN = 0;
	
    // save old terminal i/o settings 
    tcgetattr(STDIN, &save);    

    // make new settings same as old settings 
    struct termios termIO;
    
    termIO = save;

    if (bBuffering == true) 
    {
        termIO.c_lflag &= ICANON;          // ensable buffered i/o 
    }
    else
    {
        termIO.c_lflag &= ~ICANON;         // disable buffered i/o 
    }

    tcsetattr(STDIN, TCSANOW, &termIO);    // use these new terminal i/o settings now 

	setbuf(stdin, NULL);
}


// Restore old terminal i/o settings (restore)
void resetTermios(struct termios &old) 
{
    tcsetattr(0, TCSANOW, &old);
}


// Check if a key has been pressed
int _kbhit() 
{
    static const int STDIN = 0;
	
    struct termios save;

    setTermiosBuffering(false, save);

    int bytesWaiting;
	
    ioctl(STDIN, FIONREAD, &bytesWaiting);

    resetTermios(save);
    
	return bytesWaiting;
}


// Read 1 character 
// echo defines echo mode
char getachar(const bool bEcho) 
{
    char ch;

    struct termios save;

    setTermiosEcho(bEcho, save);

    ch = getchar();

    resetTermios(save);

    return ch;
}


// Read 1 character 
char getachar() 
{
    char ch;

    ch = getchar();

    return ch;
}


// Read 1 character without echo 
char _getch(void) 
{
    //return getachar(false);
    return getachar();
}


// Read 1 character with echo 
char _getche(void) 
{
    return getachar();
}
