/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#ifndef SFZSOUND_H_INCLUDED
#define SFZSOUND_H_INCLUDED

#include "SFZRegion.h"

#include "water/synthesisers/Synthesiser.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace sfzero
{

class Sample;
class SampleLoader;

class Sound : public water::SynthesiserSound
{
public:
  struct LoadingIdleCallback {
      void (*callback)(void*);
      void* callbackPtr;
  };

  explicit Sound(const std::string &file);
  virtual ~Sound();

  typedef water::ReferenceCountedObjectPtr<Sound> Ptr;

  bool appliesToNote(int midiNoteNumber) override;
  bool appliesToChannel(int midiChannel) override;

  void addRegion(Region *region); // Takes ownership of the region.
  Sample *addSample(const std::string &path, const std::string &defaultPath);
  void addError(const std::string &message);
  void addUnsupportedOpcode(const std::string &opcode);

  virtual void loadRegions();
  virtual void loadSamples(SampleLoader &loader, const LoadingIdleCallback& cb);

  Region *getRegionFor(int note, int velocity, Region::Trigger trigger = Region::attack);
  int getNumRegions();
  Region *regionAt(int index);

  const std::vector<std::string> &getErrors() { return errors_; }
  const std::vector<std::string> &getWarnings() { return warnings_; }

  std::string dump();
  void dumpToConsole();

  std::vector<Region *> &getRegions() { return regions_; }
  const std::string &getFile() { return file_; }

private:
  std::string file_;
  std::vector<Region *> regions_;
  std::vector<std::unique_ptr<Sample>> samples_;
  std::vector<std::string> errors_;
  std::vector<std::string> warnings_;
  std::unordered_set<std::string> unsupportedOpcodes_;

  CARLA_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Sound)
};
}

#endif // SFZSOUND_H_INCLUDED
