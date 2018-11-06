/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/

#include "SFZSample.h"
#include "SFZSampleLoader.h"
#include "SFZDebug.h"

namespace sfzero
{

bool Sample::load(SampleLoader &loader)
{
    SampleBuffer buffer;
    if (!loader.load(file_, defaultPath_, buffer))
      return false;

    buffer_ = buffer;
    return true;
}

Sample::~Sample() { }

std::string Sample::getShortName() { return getFileName(file_); }

std::string Sample::dump() { return file_ + "\n"; }

#ifdef DEBUG
void Sample::checkIfZeroed(const char *where)
{
  if (buffer_ == nullptr)
  {
    dbgprintf("SFZSample::checkIfZeroed(%s): no buffer!", where);
    return;
  }

  int samplesLeft = buffer_->getNumSamples();
  int64_t nonzero = 0, zero = 0;
  const float *p = buffer_->getReadPointer(0);
  for (; samplesLeft > 0; --samplesLeft)
  {
    if (*p++ == 0.0)
    {
      zero += 1;
    }
    else
    {
      nonzero += 1;
    }
  }
  if (nonzero > 0)
  {
    dbgprintf("Buffer not zeroed at %s (%lu vs. %lu).", where, nonzero, zero);
  }
  else
  {
    dbgprintf("Buffer zeroed at %s!  (%lu zeros)", where, zero);
  }
}
#endif // DEBUG

}
