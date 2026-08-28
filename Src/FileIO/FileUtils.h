/// 
/// \file       FileUtils.h
/// 
///             File utility function definitions
///


#ifndef _UTILITY_FUNCTION_DEFS_
#define _UTILITY_FUNCTION_DEFS_

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "../String/StrUtils.h"


namespace fileUtil
{

template<class S>
S stripPath(const S &s, const S &delim) throw()
{
	typename S::size_type pos = s.find_last_of(delim);
	if (pos == S::npos)
	{
		return s;
	}
	if (pos == s.size() - 1)
	{
		return S();
	}
	return s.substr(pos + 1);
}

inline std::string stripPath(const std::string &s) throw()
{
	return fileUtil::stripPath(s, std::string("\\/"));
}

#if defined(_WIN32) || defined(WIN32)
inline std::wstring stripPath(const std::wstring &ws) throw()
{
	return fileUtil::stripPath(ws, std::wstring(L"\\/"));
}
#endif


template<class S>
S stripName(const S &s, const S &delim, const S &delim2) throw()
{
	typename S::size_type pos = s.find_last_of(delim);
	if (pos == S::npos)
	{
		pos = s.find_last_of(delim2);
		if (pos == S::npos)
		{
			return S();
		}
		return s.substr(0, pos);
	}
	if (pos == s.size() - 1)
	{
		return S();
	}
	return s.substr(0, pos);
}

inline std::string stripName(const std::string &s) throw()
{
	return fileUtil::stripName(s, std::string("\\/"), std::string(":"));
}

#if defined(_WIN32) || defined(WIN32)
inline std::wstring stripName(const std::wstring &ws) throw()
{
	return fileUtil::stripName(ws, std::wstring(L"\\/"), std::wstring(L":"));
}
#endif


template<class S>
S stripSuffix(const S &s, const S &delim) throw()
{
	return s.substr(0, s.rfind(delim));
}

inline std::string stripSuffix(const std::string &s) throw()
{
	return fileUtil::stripSuffix(s, std::string("."));
}

#if defined(_WIN32) || defined(WIN32)
inline std::wstring stripSuffix(const std::wstring &ws) throw()
{
	return fileUtil::stripSuffix(ws, std::wstring(L"."));
}
#endif


template<class S>
S getSuffix(const S &s, const S &delim) throw()
{
	S empty;
	typename S::size_type pos = s.rfind(delim);
	if (pos == S::npos)
	{
		return empty;
	}
	return s.substr(pos + 1);
}

inline std::string getSuffix(const std::string &s) throw()
{
	return fileUtil::getSuffix(s, std::string("."));
}

#if defined(_WIN32) || defined(WIN32)
inline std::wstring getSuffix(const std::wstring &ws) throw()
{
	return fileUtil::getSuffix(ws, std::wstring(L"."));
}
#endif


inline std::string removeLeadingSpaces(std::string &sStr)
{
	std::string sOut = "";

	bool bFirstCharFound = false;

	for (auto x = sStr.begin(); x != sStr.end(); x++)
	{
		if (bFirstCharFound == false && (*x) == ' ')
		{
			continue;
		}

		bFirstCharFound = true;
		sOut.push_back((*x));
	}

	return sOut;
}


inline std::string removeTrailingSpaces(std::string &sStr)
{
	std::string sOut = "";

	int nLen = (int) sStr.size();

	for (auto x = (sStr.end() - 1); x != sStr.begin(); x--)
	{
		if ((*x) != ' ')
		{
			break;
		}

		nLen--;
	}

	sOut.append(sStr.substr(0, nLen));

	return sOut;
}


inline std::string getFileDir(std::string &sPath)
{
	std::filesystem::path filePath(sPath);

	return filePath.parent_path().string();
}


inline std::string getFileExt(std::string &sPath)
{
	std::filesystem::path filePath(sPath);

	return filePath.extension().string();
}


inline std::string getFileName(std::string &sPath)
{
	std::filesystem::path filePath(sPath);

	return filePath.stem().string();
}


inline int getNumericStringAt(const std::string &sText, const unsigned int pos)
{
	auto len = sText.length();

	if (pos >= len)
	{
		return -1;
	}

	std::string sTemp = sText.substr(pos);

	sTemp = removeLeadingSpaces(sTemp);
	sTemp = removeTrailingSpaces(sTemp);

	if (!StrUtils::isNumeric(sTemp))
	{
		return -1;
	}

	return std::stoi(sTemp);
}


inline std::string getAbsolutePath(const std::string &sPath)
{
	std::filesystem::path sAbsPath = std::filesystem::canonical(sPath);

	return sAbsPath.string();
}


typedef std::vector<std::string> filenameList_def;

std::string getCurrentDirectory();

bool dirExists(const std::string &fullPath);

bool createDir(const std::string &fullPath);

bool fileExists(const std::string &fullPath);

bool pathExists(const std::string &fullPath);

filenameList_def directoryFileList(
	const std::string &pattern,
	bool fullPaths,
	bool forceLowerCase = false);

bool isFileInList(
	const filenameList_def &fileList,
	const std::string &file);

bool deleteFile(const std::string &fullPath);

bool renameFile(
	const std::string &fullSrcPath,
	const std::string &fullDstPath);

bool copyFile(
	const std::string &fullSrcPath,
	const std::string &fullDstPath);

};

#endif
