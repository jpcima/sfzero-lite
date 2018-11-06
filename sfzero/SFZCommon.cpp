/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/

#include "SFZCommon.h"

#include <math.h>
#include <stdio.h>

namespace sfzero
{

bool loadFileAsData(const std::string &file, MemoryBlock &mb)
{
    FILE *fh = fopen(file.c_str(), "rb");
    if (!fh)
        return false;

    if (fseek(fh, 0, SEEK_END) == -1)
      return false;

    unsigned long size = ftell(fh);
    if ((long)size == -1)
      return false;

    rewind(fh);
    std::unique_ptr<uint8_t[]> data(new uint8_t[size]);
    if (fread(data.get(), 1, size, fh) != size)
      return false;

    mb.data = std::move(data);
    mb.size = size;
    return true;
}

static bool isPathSeparator(char c)
{
    bool issep = c == '/';
#if defined(_WIN32)
    issep = issep || c == '\\';
#endif
    return issep;
}

static std::string simplifyPath(const std::string &path)
{
  std::string simple;
  simple.reserve(path.size());

  bool wassep = false;
  for (size_t i = 0, n = path.size(); i < n; ++i)
  {
    char cur = path[i];
    bool issep = isPathSeparator(cur);
    if (!issep)
      simple.push_back(cur);
    else if (!wassep)
      simple.push_back('/');
    wassep = issep;
  }

  if (!simple.empty() && isPathSeparator(simple.back()))
    simple.pop_back();

  return simple;
}

std::string getFileName(const std::string &file_)
{
  std::string file = simplifyPath(file_);

  size_t index = file.rfind('/');
  if (index == file.npos)
    return file;

  return file.substr(index + 1);
}

std::string getFileNameWithoutExtension(const std::string &file)
{
  std::string name = getFileName(file);

  size_t index = name.rfind('.');
  if (index == name.npos)
    return name;

  return name.substr(0, index);
}

std::string getSiblingFile(const std::string &file_, const std::string &sibling_)
{
  std::string file = simplifyPath(file_);
  std::string sibling = simplifyPath(sibling_);

  size_t index = file.rfind('/');
  if (index == file.npos)
    return sibling;

  return file.substr(0, index + 1) + sibling;
}

std::string getChildFile(const std::string &dir_, const std::string &file_)
{
  std::string dir = simplifyPath(dir_);
  std::string file = simplifyPath(file_);
  return simplifyPath(dir + '/' + file);
}

}
