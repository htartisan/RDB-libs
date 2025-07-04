
//******************************************************
//  Linux (POSIX) implementation of _kbhit() and _getch()
//


#ifndef LINUX_KB_UTILS_H
#define LINUX_KB_UTILS_H


#include <stdio.h>

#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>


// Save the current terminal i/o settings
void saveTermios(struct termios &save) ;

// Set terminal i/o settings (Echo)
void setTermiosEcho(const bool bEcho, struct termios &save);

// Set terminal i/o settings (Buffering)
void setTermiosBuffering(const bool bBuffering, struct termios &save);

// Restore old terminal i/o settings
void resetTermios(struct termios &old);

// Check if a key has been pressed
int _kbhit();

// Read 1 character 
char getachar();

// Read 1 character 
// echo defines echo mode
char getachar(const bool bEcho);

// Read 1 character without echo 
char _getch(void);

// Read 1 character with echo 
char _getche(void);


#endif  //  #ifndef LINUX_KB_UTILS_H
