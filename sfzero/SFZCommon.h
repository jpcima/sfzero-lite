/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#ifndef SFZCOMMON_H_INCLUDED
#define SFZCOMMON_H_INCLUDED

#include "water/water.h"

#include <memory>

#if defined(_MSC_VER)
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

namespace sfzero
{

struct MemoryBlock
{
  std::unique_ptr<uint8_t[]> data;
  size_t size = 0;
};

bool loadFileAsData(const std::string &file, MemoryBlock &mb);

std::string getFileName(const std::string &file);
std::string getFileNameWithoutExtension(const std::string &file);
std::string getSiblingFile(const std::string &file, const std::string &sibling);
std::string getChildFile(const std::string &dir, const std::string &file);

}

#endif // SFZCOMMON_H_INCLUDED
