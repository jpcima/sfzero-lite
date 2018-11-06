/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#ifndef SFZREGION_H_INCLUDED
#define SFZREGION_H_INCLUDED

#include "SFZCommon.h"

namespace sfzero
{

class Sample;

// Region is designed to be able to be bitwise-copied.

struct EGParameters
{
  float delay, start, attack, hold, decay, sustain, release;

  void clear();
  void clearMod();
};

struct Region
{
  enum Trigger
  {
    attack,
    release,
    first,
    legato
  };

  enum LoopMode
  {
    sample_loop,
    no_loop,
    one_shot,
    loop_continuous,
    loop_sustain
  };

  enum OffMode
  {
    fast,
    normal
  };

  Region();
  void clear();
  std::string dump();

  bool matches(int note, int velocity, Trigger trig)
  {
    return (note >= lokey && note <= hikey && velocity >= lovel && velocity <= hivel &&
            (trig == this->trigger || (this->trigger == attack && (trig == first || trig == legato))));
  }

  Sample *sample;
  int lokey, hikey;
  int lovel, hivel;
  Trigger trigger;
  int group;
  int64_t off_by;
  OffMode off_mode;

  int64_t offset;
  int64_t end;
  bool negative_end;
  LoopMode loop_mode;
  int64_t loop_start, loop_end;
  int transpose;
  int tune;
  int pitch_keycenter, pitch_keytrack;
  int bend_up, bend_down;

  float volume, pan;
  float amp_veltrack;

  EGParameters ampeg, ampeg_veltrack;

  static float timecents2Secs(int timecents);
};

}

#endif // SFZREGION_H_INCLUDED
