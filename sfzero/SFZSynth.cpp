/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/

#include "SFZSynth.h"
#include "SFZSound.h"
#include "SFZVoice.h"

#include <sstream>

namespace sfzero
{

Synth::Synth() : Synthesiser()
{
    carla_zeroStructs(noteVelocities_, 128);
}

void Synth::noteOn(int midiChannel, int midiNoteNumber, float velocity)
{
  int i;

  int midiVelocity = static_cast<int>(velocity * 127);

  // First, stop any currently-playing sounds in the group.
  //*** Currently, this only pays attention to the first matching region.
  int group = 0;
  Sound *sound = dynamic_cast<Sound *>(getSound(0));

  if (sound)
  {
    Region *region = sound->getRegionFor(midiNoteNumber, midiVelocity);
    if (region)
    {
      group = region->group;
    }
  }
  if (group != 0)
  {
    for (i = voices.size(); --i >= 0;)
    {
      Voice *voice = dynamic_cast<Voice *>(voices.getUnchecked(i));
      if (voice == nullptr)
      {
        continue;
      }
      if (voice->getGroup() == group)
      {
        voice->stopNoteForGroup();
      }
    }
  }

  // Are any notes playing?  (Needed for first/legato trigger handling.)
  // Also stop any voices still playing this note.
  bool anyNotesPlaying = false;
  for (i = voices.size(); --i >= 0;)
  {
    Voice *voice = dynamic_cast<Voice *>(voices.getUnchecked(i));
    if (voice == nullptr)
    {
      continue;
    }
    if (voice->isPlayingChannel(midiChannel))
    {
      if (voice->isPlayingNoteDown())
      {
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
        {
          if (!voice->isPlayingOneShot())
          {
            voice->stopNoteQuick();
          }
        }
        else
        {
          anyNotesPlaying = true;
        }
      }
    }
  }

  // Play *all* matching regions.
  Region::Trigger trigger = (anyNotesPlaying ? Region::legato : Region::first);
  if (sound)
  {
    int numRegions = sound->getNumRegions();
    for (i = 0; i < numRegions; ++i)
    {
      Region *region = sound->regionAt(i);
      if (region->matches(midiNoteNumber, midiVelocity, trigger))
      {
        Voice *voice =
            dynamic_cast<Voice *>(findFreeVoice(sound, midiNoteNumber, midiChannel, isNoteStealingEnabled()));
        if (voice)
        {
          voice->setRegion(region);
          startVoice(voice, sound, midiChannel, midiNoteNumber, velocity);
        }
      }
    }
  }

  noteVelocities_[midiNoteNumber] = midiVelocity;
}

void Synth::noteOff(int midiChannel, int midiNoteNumber, float velocity, bool allowTailOff)
{
  Synthesiser::noteOff(midiChannel, midiNoteNumber, velocity, allowTailOff);

  // Start release region.
  Sound *sound = dynamic_cast<Sound *>(getSound(0));
  if (sound)
  {
    Region *region = sound->getRegionFor(midiNoteNumber, noteVelocities_[midiNoteNumber], Region::release);
    if (region)
    {
      Voice *voice = dynamic_cast<Voice *>(findFreeVoice(sound, midiNoteNumber, midiChannel, false));
      if (voice)
      {
        // Synthesiser is too locked-down (ivars are private rt protected), so
        // we have to use a "setRegion()" mechanism.
        voice->setRegion(region);
        startVoice(voice, sound, midiChannel, midiNoteNumber, noteVelocities_[midiNoteNumber] / 127.0f);
      }
    }
  }
}

int Synth::numVoicesUsed()
{
  int numUsed = 0;

  for (int i = voices.size(); --i >= 0;)
  {
    if (voices.getUnchecked(i)->getCurrentlyPlayingNote() >= 0)
    {
      numUsed += 1;
    }
  }

  return numUsed;
}

std::string Synth::voiceInfoString()
{
  enum
  {
    maxShownVoices = 20,
  };

  std::ostringstream info;
  int numUsed = 0, numShown = 0;

  for (int i = voices.size(); --i >= 0;)
  {
    Voice *voice = dynamic_cast<Voice *>(voices.getUnchecked(i));
    if (voice->getCurrentlyPlayingNote() < 0)
    {
      continue;
    }
    numUsed += 1;
    if (numShown >= maxShownVoices)
    {
      continue;
    }
    info << voice->infoString();
  }

  return "voices used: " + std::to_string(numUsed) + '\n' + info.str();
}

}
