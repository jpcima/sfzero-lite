/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/

#ifndef SFZVOICE_H_INCLUDED
#define SFZVOICE_H_INCLUDED

#include "SFZEG.h"

#include "water/synthesisers/Synthesiser.h"

namespace sfzero
{

struct Region;

class Voice : public water::SynthesiserVoice
{
public:
  Voice();
  virtual ~Voice();

  bool canPlaySound(water::SynthesiserSound *sound) override;
  void startNote(int midiNoteNumber, float velocity, water::SynthesiserSound *sound, int currentPitchWheelPosition) override;
  void stopNote(float velocity, bool allowTailOff) override;
  void stopNoteForGroup();
  void stopNoteQuick();
  void pitchWheelMoved(int newValue) override;
  void controllerMoved(int controllerNumber, int newValue) override;
  void renderNextBlock(water::AudioSampleBuffer &outputBuffer, int startSample, int numSamples) override;
  void renderNextBlock(float *outL, float *outR, int numSamples);
  bool isPlayingNoteDown();
  bool isPlayingOneShot();

  int getGroup();
  int64_t getOffBy();

  // Set the region to be used by the next startNote().
  void setRegion(Region *nextRegion);

  std::string infoString();

private:
  Region *region_;
  int trigger_;
  int curMidiNote_, curPitchWheel_;
  double pitchRatio_;
  float noteGainLeft_, noteGainRight_;
  double sourceSamplePosition_;
  EG ampeg_;
  int64_t sampleEnd_;
  int64_t loopStart_, loopEnd_;

  // Info only.
  int numLoops_;
  int curVelocity_;

  void calcPitchRatio();
  void killNote();
  double fractionalMidiNoteInHz(double note);

  CARLA_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Voice)
};
}

#endif // SFZVOICE_H_INCLUDED
