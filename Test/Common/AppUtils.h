/// 
/// \file   AppUtils.h
/// 
///         AppUtils header file
///


#ifndef APP_UTILS_H
#define APP_UTILS_H


#include <string>
#include <filesystem>

#include "../../Libs/argspp-lib/src/args.h"


#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)


std::string getCWD();


#endif
