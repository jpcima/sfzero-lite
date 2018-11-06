/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#ifndef SFZSAMPLE_H_INCLUDED
#define SFZSAMPLE_H_INCLUDED

#include "SFZCommon.h"

#include "CarlaJuceUtils.hpp"

#include <string>
#include <memory>

namespace sfzero
{

class SampleLoader;

struct SampleBuffer
{
    double sampleRate;
    uint64_t sampleLength, loopStart, loopEnd;
    unsigned int numChannels;
    std::shared_ptr<float[]> samples;

    SampleBuffer() : sampleRate(0), sampleLength(0), loopStart(0), loopEnd(0), numChannels(0) {}
    float *getReadPointer(unsigned int channel) { return &samples[channel * sampleLength]; }
};

class Sample
{
public:
  Sample(const std::string &fileIn, const std::string &defaultPath) : file_(fileIn), defaultPath_(defaultPath) {}
  virtual ~Sample();

  bool load(SampleLoader &loader);

  const std::string &getFile() { return file_; }
  SampleBuffer *getBuffer() { return &buffer_; }
  double getSampleRate() { return buffer_.sampleRate; }
  std::string getShortName();
  std::string dump();
  uint64_t getSampleLength() const { return buffer_.sampleLength; }
  uint64_t getLoopStart() const { return buffer_.loopStart; }
  uint64_t getLoopEnd() const { return buffer_.loopEnd; }

#ifdef DEBUG
  void checkIfZeroed(const char *where);
#endif

private:
  std::string file_;
  std::string defaultPath_;
  SampleBuffer buffer_;

  CARLA_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Sample)
};
}

#endif // SFZSAMPLE_H_INCLUDED
