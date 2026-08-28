/// 
/// \file   AppUtils.h
/// 
///         AppUtils functions defs
///


#include "AppUtils.h"


std::string getCWD()
{
    std::string sOut;

    std::filesystem::path currentPath = std::filesystem::current_path();

    sOut = currentPath.string();

    return sOut;
}

