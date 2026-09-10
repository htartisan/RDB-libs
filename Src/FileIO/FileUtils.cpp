//**************************************************************************************************
//* FILE:		FileUtils.cpp
//* DESCRIP:	Cross-platform filesystem utility functions.
//**************************************************************************************************

#include <RDB-Libs/Src/FileIO/FileUtils.h>

#include <algorithm>
#include <set>

#include <Poco/Exception.h>
#include <Poco/File.h>
#include <Poco/Glob.h>
#include <Poco/Path.h>


std::string fileUtil::getCurrentDirectory()
{
	try
	{
		std::string result = Poco::Path::current();
		if (!result.empty() && result.back() != Poco::Path::separator())
		{
			result += Poco::Path::separator();
		}
		return result;
	}
	catch (const Poco::Exception &error)
	{
		throw std::runtime_error("Could not get current working directory: " + error.displayText());
	}
}

bool fileUtil::dirExists(const std::string &fullPath)
{
	try
	{
		const Poco::File file(fullPath);
		return file.exists() && file.isDirectory();
	}
	catch (const Poco::Exception &error)
	{
		throw std::runtime_error("Could not inspect directory: " + error.displayText());
	}
}

bool fileUtil::createDir(const std::string &fullPath)
{
	try
	{
		Poco::File file(fullPath);
		if (!file.exists())
		{
			file.createDirectories();
		}
		return file.isDirectory();
	}
	catch (const Poco::Exception &error)
	{
		throw std::runtime_error("Could not create directory: " + error.displayText());
	}
}

bool fileUtil::fileExists(const std::string &fullPath)
{
	try
	{
		const Poco::File file(fullPath);
		return file.exists() && file.isFile();
	}
	catch (const Poco::Exception &error)
	{
		throw std::runtime_error("Could not inspect file: " + error.displayText());
	}
}

bool fileUtil::pathExists(const std::string &fullPath)
{
	try
	{
		return Poco::File(fullPath).exists();
	}
	catch (const Poco::Exception &)
	{
		return false;
	}
}

fileUtil::filenameList_def fileUtil::directoryFileList(
	const std::string &pattern,
	bool fullPaths,
	bool forceLowerCase)
{
	try
	{
		std::set<std::string> matches;
		Poco::Glob::glob(pattern, matches, Poco::Glob::GLOB_DEFAULT);

		fileUtil::filenameList_def result;
		for (const std::string &match : matches)
		{
			std::string value = fullPaths ? match : Poco::Path(match).getFileName();
			if (forceLowerCase)
			{
				value = stringUtil::toLower(value);
			}
			result.push_back(value);
		}
		return result;
	}
	catch (const Poco::Exception &error)
	{
		throw std::runtime_error("Could not enumerate directory: " + error.displayText());
	}
}

bool fileUtil::isFileInList(
	const fileUtil::filenameList_def &fileList,
	const std::string &file)
{
	return std::find(fileList.begin(), fileList.end(), file) != fileList.end();
}

bool fileUtil::deleteFile(const std::string &fullPath)
{
	try
	{
		Poco::File file(fullPath);
		if (!file.exists())
		{
			return false;
		}
		file.remove();
		return true;
	}
	catch (const Poco::Exception &error)
	{
		throw std::runtime_error("Could not delete file: " + error.displayText());
	}
}

bool fileUtil::renameFile(
	const std::string &fullSrcPath,
	const std::string &fullDstPath)
{
	try
	{
		Poco::File(fullSrcPath).moveTo(fullDstPath);
		return true;
	}
	catch (const Poco::Exception &error)
	{
		throw std::runtime_error("Could not rename file: " + error.displayText());
	}
}

bool fileUtil::copyFile(
	const std::string &fullSrcPath,
	const std::string &fullDstPath)
{
	try
	{
		Poco::File(fullSrcPath).copyTo(fullDstPath);
		return true;
	}
	catch (const Poco::Exception &error)
	{
		throw std::runtime_error("Could not copy file: " + error.displayText());
	}
}
