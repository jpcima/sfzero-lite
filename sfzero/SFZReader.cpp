/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#include "SFZReader.h"
#include "SFZRegion.h"
#include "SFZSound.h"

#include <sstream>

namespace sfzero
{

Reader::Reader(Sound *soundIn) : sound_(soundIn), line_(1) {}

Reader::~Reader() {}

void Reader::read(const std::string &file)
{
  MemoryBlock contents;
  bool ok = loadFileAsData(file, contents);

  if (!ok)
  {
    sound_->addError("Couldn't read \"" + file + "\"");
    return;
  }

  read(reinterpret_cast<const char *>(contents.data.get()), static_cast<unsigned int>(contents.size));
}

void Reader::read(const char *text, unsigned int length)
{
  const char *p = text;
  const char *end = text + length;
  char c = 0;

  Region curGlobal;
  Region curGroup;
  Region curRegion;
  Region *buildingRegion = nullptr;
  bool inControl = false;
  bool inGroup = false;
  std::string defaultPath;

  while (p < end)
  {
    // We're at the start of a line; skip any whitespace.
    while (p < end)
    {
      c = *p;
      if ((c != ' ') && (c != '\t'))
      {
        break;
      }
      p += 1;
    }
    if (p >= end)
    {
      break;
    }

    // Check if it's a comment line.
    if (c == '/')
    {
      // Skip to end of line.
      while (p < end)
      {
        c = *++p;
        if ((c == '\n') || (c == '\r'))
        {
          break;
        }
      }
      p = handleLineEnd(p);
      continue;
    }

    // Check if it's a blank line.
    if ((c == '\r') || (c == '\n'))
    {
      p = handleLineEnd(p);
      continue;
    }

    // Handle elements on the line.
    while (p < end)
    {
      c = *p;

      // Tag.
      if (c == '<')
      {
        p += 1;
        const char *tagStart = p;
        while (p < end)
        {
          c = *p++;
          if ((c == '\n') || (c == '\r'))
          {
            error("Unterminated tag");
            goto fatalError;
          }
          else if (c == '>')
          {
            break;
          }
        }
        if (p >= end)
        {
          error("Unterminated tag");
          goto fatalError;
        }
        std::string/*_view*/ tag(tagStart, p - 1);
        if (tag == "global")
        {
            curGlobal.clear();
            buildingRegion = &curGlobal;
            inControl = false;
            inGroup = false;
        }
        else if (tag == "region")
        {
          if (buildingRegion && (buildingRegion == &curRegion))
          {
            finishRegion(&curRegion);
          }
          curRegion = curGroup;
          buildingRegion = &curRegion;
          inControl = false;
          inGroup = false;
        }
        else if (tag == "group")
        {
          if (buildingRegion && (buildingRegion == &curRegion))
          {
            finishRegion(&curRegion);
          }
          if (! inGroup)
          {
            curGroup = curGlobal;
            buildingRegion = &curGroup;
            inControl = false;
            inGroup = true;
          }
        }
        else if (tag == "control")
        {
          if (buildingRegion && (buildingRegion == &curRegion))
          {
            finishRegion(&curRegion);
          }
          curGroup.clear();
          buildingRegion = nullptr;
          inControl = true;
        }
        else
        {
          error("Illegal tag");
        }
      }
      // Comment.
      else if (c == '/')
      {
        // Skip to end of line.
        while (p < end)
        {
          c = *p;
          if ((c == '\r') || (c == '\n'))
          {
            break;
          }
          p += 1;
        }
      }
      // Parameter.
      else
      {
        // Get the parameter name.
        const char *parameterStart = p;
        while (p < end)
        {
          c = *p++;
          if ((c == '=') || (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))
          {
            break;
          }
        }
        if ((p >= end) || (c != '='))
        {
          error("Malformed parameter");
          goto nextElement;
        }
        std::string/*_view*/ opcode(parameterStart, p - 1);
        if (inControl)
        {
          if (opcode == "default_path")
          {
            p = readPathInto(&defaultPath, p, end);
          }
          else
          {
            const char *valueStart = p;
            while (p < end)
            {
              c = *p;
              if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
              {
                break;
              }
              p++;
            }
            std::string value(valueStart, p);
            std::string fauxOpcode = opcode + " (in <control>)";
            sound_->addUnsupportedOpcode(fauxOpcode);
          }
        }
        else if (opcode == "sample")
        {
          std::string path;
          p = readPathInto(&path, p, end);
          if (!path.empty())
          {
            if (buildingRegion)
            {
              buildingRegion->sample = sound_->addSample(path, defaultPath);
            }
            else
            {
              error("Adding sample outside a group or region");
            }
          }
          else
          {
            error("Empty sample path");
          }
        }
        else
        {
          const char *valueStart = p;
          while (p < end)
          {
            c = *p;
            if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
            {
              break;
            }
            p++;
          }
          std::string value(valueStart, p);
          std::istringstream ivalue(value);
          ivalue.imbue(std::locale::classic());
          if (buildingRegion == nullptr)
          {
            error("Setting a parameter outside a region or group");
          }
          else if (opcode == "lokey")
          {
            buildingRegion->lokey = keyValue(value);
          }
          else if (opcode == "hikey")
          {
            buildingRegion->hikey = keyValue(value);
          }
          else if (opcode == "key")
          {
            buildingRegion->hikey = buildingRegion->lokey = buildingRegion->pitch_keycenter = keyValue(value);
          }
          else if (opcode == "lovel")
          {
            ivalue >> buildingRegion->lovel;
          }
          else if (opcode == "hivel")
          {
            ivalue >> buildingRegion->hivel;
          }
          else if (opcode == "trigger")
          {
            buildingRegion->trigger = static_cast<Region::Trigger>(triggerValue(value));
          }
          else if (opcode == "group")
          {
            ivalue >> buildingRegion->group;
          }
          else if (opcode == "off_by" || opcode == "offby")
          {
            ivalue >> buildingRegion->off_by;
          }
          else if (opcode == "offset")
          {
            ivalue >> buildingRegion->offset;
          }
          else if (opcode == "end")
          {
            int64_t end2;
            ivalue >> end2;
            if (end2 < 0)
            {
              buildingRegion->negative_end = true;
            }
            else
            {
              buildingRegion->end = end2;
            }
          }
          else if (opcode == "loop_mode" || opcode == "loopmode")
          {
            bool modeIsSupported = value == "no_loop" || value == "one_shot" || value == "loop_continuous";
            if (modeIsSupported)
            {
              buildingRegion->loop_mode = static_cast<Region::LoopMode>(loopModeValue(value));
            }
            else
            {
              std::string fauxOpcode = opcode + "=" + value;
              sound_->addUnsupportedOpcode(fauxOpcode);
            }
          }
          else if (opcode == "loop_start" || opcode == "loopstart")
          {
            ivalue >> buildingRegion->loop_start;
          }
          else if (opcode == "loop_end" || opcode == "loopend")
          {
            ivalue >> buildingRegion->loop_end;
          }
          else if (opcode == "transpose")
          {
            ivalue >> buildingRegion->transpose;
          }
          else if (opcode == "tune")
          {
            ivalue >> buildingRegion->tune;
          }
          else if (opcode == "pitch_keycenter")
          {
            buildingRegion->pitch_keycenter = keyValue(value);
          }
          else if (opcode == "pitch_keytrack")
          {
            ivalue >> buildingRegion->pitch_keytrack;
          }
          else if (opcode == "bend_up" || opcode == "bendup")
          {
            ivalue >> buildingRegion->bend_up;
          }
          else if (opcode == "bend_down" || opcode == "benddown")
          {
            ivalue >> buildingRegion->bend_down;
          }
          else if (opcode == "volume")
          {
            ivalue >> buildingRegion->volume;
          }
          else if (opcode == "pan")
          {
            ivalue >> buildingRegion->pan;
          }
          else if (opcode == "amp_veltrack")
          {
            ivalue >> buildingRegion->amp_veltrack;
          }
          else if (opcode == "ampeg_delay")
          {
            ivalue >> buildingRegion->ampeg.delay;
          }
          else if (opcode == "ampeg_start")
          {
            ivalue >> buildingRegion->ampeg.start;
          }
          else if (opcode == "ampeg_attack")
          {
            ivalue >> buildingRegion->ampeg.attack;
          }
          else if (opcode == "ampeg_hold")
          {
            ivalue >> buildingRegion->ampeg.hold;
          }
          else if (opcode == "ampeg_decay")
          {
            ivalue >> buildingRegion->ampeg.decay;
          }
          else if (opcode == "ampeg_sustain")
          {
            ivalue >> buildingRegion->ampeg.sustain;
          }
          else if (opcode == "ampeg_release")
          {
            ivalue >> buildingRegion->ampeg.release;
          }
          else if (opcode == "ampeg_vel2delay")
          {
            ivalue >> buildingRegion->ampeg_veltrack.delay;
          }
          else if (opcode == "ampeg_vel2attack")
          {
            ivalue >> buildingRegion->ampeg_veltrack.attack;
          }
          else if (opcode == "ampeg_vel2hold")
          {
            ivalue >> buildingRegion->ampeg_veltrack.hold;
          }
          else if (opcode == "ampeg_vel2decay")
          {
            ivalue >> buildingRegion->ampeg_veltrack.decay;
          }
          else if (opcode == "ampeg_vel2sustain")
          {
            ivalue >> buildingRegion->ampeg_veltrack.sustain;
          }
          else if (opcode == "ampeg_vel2release")
          {
            ivalue >> buildingRegion->ampeg_veltrack.release;
          }
          else if (opcode == "default_path")
          {
            error("\"default_path\" outside of <control> tag");
          }
          else
          {
            sound_->addUnsupportedOpcode(opcode);
          }
        }
      }

    // Skip to next element.
    nextElement:
      c = 0;
      while (p < end)
      {
        c = *p;
        if ((c != ' ') && (c != '\t'))
        {
          break;
        }
        p += 1;
      }
      if ((c == '\r') || (c == '\n'))
      {
        p = handleLineEnd(p);
        break;
      }
    }
  }

fatalError:
  if (buildingRegion && (buildingRegion == &curRegion))
  {
    finishRegion(buildingRegion);
  }
}

const char *Reader::handleLineEnd(const char *p)
{
  // Check for DOS-style line ending.
  char lineEndChar = *p++;

  if ((lineEndChar == '\r') && (*p == '\n'))
  {
    p += 1;
  }
  line_ += 1;
  return p;
}

const char *Reader::readPathInto(std::string *pathOut, const char *pIn, const char *endIn)
{
  // Paths are kind of funny to parse because they can contain whitespace.
  const char *p = pIn;
  const char *end = endIn;
  const char *pathStart = p;
  const char *potentialEnd = nullptr;

  while (p < end)
  {
    char c = *p;
    if (c == ' ')
    {
      // Is this space part of the path?  Or the start of the next opcode?  We
      // don't know yet.
      potentialEnd = p;
      p += 1;
      // Skip any more spaces.
      while (p < end && *p == ' ')
      {
        p += 1;
      }
    }
    else if ((c == '\n') || (c == '\r') || (c == '\t'))
    {
      break;
    }
    else if (c == '=')
    {
      // We've been looking at an opcode; we need to rewind to
      // potentialEnd.
      p = potentialEnd;
      break;
    }
    p += 1;
  }
  if (p > pathStart)
  {
    *pathOut = std::string(pathStart, p);
  }
  else
  {
    *pathOut = std::string();
  }
  return p;
}

int Reader::keyValue(const std::string &str)
{
  const char* const chars = str.c_str();

  char c = chars[0];

  if ((c >= '0') && (c <= '9'))
  {
    int x;
    std::istringstream ivalue(str);
    ivalue.imbue(std::locale::classic());
    ivalue >> x;
    return x;
  }

  int note = 0;
  static const int notes[] = {
      12 + 0, 12 + 2, 3, 5, 7, 8, 10,
  };
  if ((c >= 'A') && (c <= 'G'))
  {
    note = notes[c - 'A'];
  }
  else if ((c >= 'a') && (c <= 'g'))
  {
    note = notes[c - 'a'];
  }
  int octaveStart = 1;

  c = chars[1];
  if ((c == 'b') || (c == '#'))
  {
    octaveStart += 1;
    if (c == 'b')
    {
      note -= 1;
    }
    else
    {
      note += 1;
    }
  }

  std::string octaveStr = str.substr(octaveStart);
  std::istringstream ivalue(octaveStr);
  ivalue.imbue(std::locale::classic());

  int octave;
  ivalue >> octave;
  // A3 == 57.
  int result = octave * 12 + note + (57 - 4 * 12);
  return result;
}

int Reader::triggerValue(const std::string &str)
{
  if (str == "release")
  {
    return Region::release;
  }
  if (str == "first")
  {
    return Region::first;
  }
  if (str == "legato")
  {
    return Region::legato;
  }
  return Region::attack;
}

int Reader::loopModeValue(const std::string &str)
{
  if (str == "no_loop")
  {
    return Region::no_loop;
  }
  if (str == "one_shot")
  {
    return Region::one_shot;
  }
  if (str == "loop_continuous")
  {
    return Region::loop_continuous;
  }
  if (str == "loop_sustain")
  {
    return Region::loop_sustain;
  }
  return Region::sample_loop;
}

void Reader::finishRegion(Region *region)
{
  Region *newRegion = new Region();

  *newRegion = *region;
  sound_->addRegion(newRegion);
}

void Reader::error(const std::string &message)
{
  std::string fullMessage = message;

  fullMessage += " (line " + std::to_string(line_) + ").";
  sound_->addError(fullMessage);
}

}
