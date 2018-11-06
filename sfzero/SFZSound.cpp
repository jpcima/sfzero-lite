/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/

#include "SFZSound.h"
#include "SFZReader.h"
#include "SFZRegion.h"
#include "SFZSample.h"

#include <algorithm>
#include <sstream>

namespace sfzero
{

Sound::Sound(const std::string &fileIn) : file_(fileIn) {}
Sound::~Sound()
{
  int numRegions = regions_.size();

  for (int i = 0; i < numRegions; ++i)
  {
    delete regions_[i];
    regions_[i] = nullptr;
  }
}

bool Sound::appliesToNote(int /*midiNoteNumber*/)
{
  // Just say yes; we can't truly know unless we're told the velocity as well.
  return true;
}

bool Sound::appliesToChannel(int /*midiChannel*/) { return true; }
void Sound::addRegion(Region *region) { regions_.push_back(region); }
Sample *Sound::addSample(const std::string &path, const std::string &defaultPath)
{
  Sample *sample = new Sample(path, defaultPath);
  samples_.emplace_back(sample);
  return sample;
}

void Sound::addError(const std::string &message)
{
  errors_.push_back(message);
}

void Sound::addUnsupportedOpcode(const std::string &opcode)
{
  if (unsupportedOpcodes_.insert(opcode).second)
    warnings_.push_back("unsupported opcode: " + opcode);
}

void Sound::loadRegions()
{
  Reader reader(this);

  reader.read(file_);
}

void Sound::loadSamples(SampleLoader &loader, const LoadingIdleCallback& cb)
{
    int numSamples = samples_.size();

    for (int i = 0; i < numSamples; ++i)
    {
        Sample* const sample = samples_[i].get();

        if (sample->load(loader))
        {
            carla_debug("Loaded sample '%s'", sample->getShortName().toRawUTF8());
            if (cb.callback)
                cb.callback(cb.callbackPtr);
        }
        else
        {
            addError("Couldn't load sample \"" + sample->getShortName() + "\"");
        }
    }
}

Region *Sound::getRegionFor(int note, int velocity, Region::Trigger trigger)
{
  int numRegions = regions_.size();

  for (int i = 0; i < numRegions; ++i)
  {
    Region *region = regions_[i];
    if (region->matches(note, velocity, trigger))
    {
      return region;
    }
  }

  return nullptr;
}

int Sound::getNumRegions() { return regions_.size(); }

Region *Sound::regionAt(int index) { return regions_[index]; }

std::string Sound::dump()
{
  std::ostringstream info;
  const std::vector<std::string>& errors = getErrors();
  const int numErrors   = errors.size();
  if (numErrors > 0)
  {
    info << errors.size() << " errors: \n";
    for (size_t i = 0; i < numErrors; ++i)
        info << errors[i] << "\n";
  }
  else
  {
    info << "no errors.\n\n";
  }

  const std::vector<std::string>& warnings = getWarnings();
  const int numWarnings = warnings.size();
  if (numWarnings > 0)
  {
    info << warnings.size() << " warnings: \n";
    for (size_t i = 0; i < numWarnings; ++i)
        info << warnings[i] << "\n";
  }
  else
  {
    info << "no warnings.\n";
  }

#ifdef DEBUG
  if (regions_.size() > 0)
  {
    info << regions_.size() << " regions: \n";
    for (int i = 0; i < regions_.size(); ++i)
    {
      info << regions_[i]->dump();
    }
  }
  else
  {
    info << "no regions.\n";
  }

  if (samples_.size() > 0)
  {
    int numSamples = samples_.size();
    info << numSamples << " samples: \n";
    for (int i = 0; i < numSamples; ++i)
    {
      info << samples_[i]->dump();
    }
  }
  else
  {
    info << "no samples.\n";
  }
#endif

  return info.str();
}

void Sound::dumpToConsole()
{
  const std::string filename(getFileNameWithoutExtension(file_));

  const std::vector<std::string>& errors(getErrors());
  const std::vector<std::string>& warnings(getWarnings());

  const int numErrors   = errors.size();
  const int numWarnings = warnings.size();

  if (numErrors == 0 && numWarnings == 0)
  {
      fprintf(stderr, "SFZ '%s' loaded without errors or warnings, nice! :)", filename.c_str());
      return;
  }

  if (numErrors != 0)
  {
      fprintf(stderr, "SFZ '%s' loaded with %i errors and %i warnings:", filename.c_str(), numErrors, numWarnings);

      if (numWarnings != 0)
          fprintf(stderr, "Errors:");

      for (size_t i = 0; i < numErrors; ++i)
          fprintf(stderr, "%s\n", errors[i].c_str());

      if (numWarnings != 0)
      {
          fprintf(stderr, "Warnings:");
          for (size_t i = 0; i < numWarnings; ++i)
              fprintf(stderr, "%s\n", warnings[i].c_str());
      }
  }

  fprintf(stderr, "SFZ '%s' loaded without errors, but has %i warnings:", filename.c_str(), numWarnings);
  for (size_t i = 0; i < numWarnings; ++i)
      fprintf(stderr, "%s\n", warnings[i].c_str());
}

}
