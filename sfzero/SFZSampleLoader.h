/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#ifndef SFZSAMPLELOADER_H_INCLUDED
#define SFZSAMPLELOADER_H_INCLUDED

#include "SFZCommon.h"

#include "CarlaJuceUtils.hpp"

namespace sfzero
{

struct SampleBuffer;

class SampleLoader
{
public:
  SampleLoader() {}
  virtual ~SampleLoader() {}

  virtual bool load(const std::string &file, const std::string &defaultPath, SampleBuffer &buffer) = 0;

private:
  CARLA_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleLoader)
};

}

#endif
