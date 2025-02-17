//****************************************************************************
// FILE:    CFileStream.h
//
// DESC:    The class defined in this header
//          allows access to data if type "T"
//          to be read from or written to a
//          std file stream.
//
// AUTHOR:  Russ Barker
//


#ifndef _CFILESTREAM_H
#define _CFILESTREAM_H

#include <exception>
#include <stdexcept>
#include <mutex>
#include <fstream>
#include <iostream>
#include <string>


template <class T> class CFileStream
{
  private:

    std::string   _filepath = "";
    std::fstream  _readstream;
    std::ofstream _writestream;

    void          openFile()
    {
        _writestream.open(_filepath, std::fstream::out | std::ios::binary | std::ios::app);
        if (_writestream.fail())
            throw std::string("could not open file for writestream");

        _readstream.open(_filepath, std::fstream::in | std::ios::binary);
        if (_readstream.fail())
            throw std::string("could not open file for readstream");

        _readstream.seekg(0, std::ios::beg);
        _writestream.seekp(0, std::ios::end);
    }

    void closeFile()
    {
        _readstream.close();
        _writestream.close();
    }

  public:

    //Create a buffer on a specific file
    CFileStream(std::string filepath) :
        _filepath(filepath)
    {
        openFile();
    }

    ~CFileStream()
    {
        closeFile();
    }

    // return the maximum number elements in the file
    size_t capacity()
    {
        std::lock_guard<std::mutex> lock(this->_access);

        // save the current position...
        size_t pos = _writestream.tellp();

        // seek to end and get the position
        _writestream.seekp(0, std::ios::end);
        size_t cap = _writestream.tellp();

        // restore original position
        _writestream.seekp(pos, std::ios::beg);

        pos = _writestream.tellp(); // debug

        // return capacity in terms of element size
        return cap / sizeof(T);
    }

    //return the amount of unread elements in the file
    size_t remaining()
    {
        std::lock_guard<std::mutex> lock(this->_access);

        size_t                      writepos = _writestream.tellp();
        size_t                      readpos  = _readstream.tellp();
        size_t                      result   = (writepos / sizeof(T)) - (readpos / sizeof(T));

        return result;
    }

    // return true if the file is empty:
    bool empty()
    {
        return capacity() <= 0;
    }

    // Writes an object into the file at the current file location and increments internal indecies
    void write(T value)
    {
        std::lock_guard<std::mutex> lock(this->_access);

        _writestream.write((char *)&value, sizeof(T));
    }

    // Reads an object from the file at the current location and updates internal indecies
    bool read(T *val, int &err)
    {
        bool                        result = true;

        std::lock_guard<std::mutex> lock(this->_access);
        size_t                      readpos = _readstream.tellp(); // debug

        if (_readstream.peek() == std::char_traits<char>::eof())
        {
            err    = -1;
            result = false;
        }
        else
        {
            _readstream.read((char *)val, sizeof(T));
        }

        return result;
    }

    // Clears file buffer and resets internal indecies
    void reset()
    {
        closeFile();
        openFile();
    }

    // Flush the write stream...
    void flush()
    {
        std::lock_guard<std::mutex> lock(this->_access);
        _writestream.flush();
    }
};

#endif // _CFILESTREAM_H
