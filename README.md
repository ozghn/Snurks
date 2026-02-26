# Snurks - Glitch/Degradation Audio Plugin

A multi-effect glitch processor that combines bitcrushing, granular time-stretching, tape modulation, and randomized glitch effects for creative audio degradation.

## Description
Snurks is a experimental audio plugin that transforms clean audio through various degradation and glitch effects. It features a main "Snurk" toggle that progressively degrades the signal with a configurable fade-in, creating everything from subtle lo-fi textures to extreme digital destruction.

## Features
- **Bitcrusher**: Variable bit depth reduction (4-16 bit) with downsampling
- **Glitch Buffer**: Randomly triggered buffer repeats with Hann window smoothing
- **Granular Time-Stretch**: Multi-voice granular processing with independent grain size, spacing, and playback rate
- **Tape Pitch Modulation**: LFO-driven pitch wobble with crossfaded delay taps
- **Distortion**: Arctangent waveshaping with blend control
- **Stereo Panner**: Random LFO-based panning modulation
- **Dropout Freeze**: Random sample-and-hold style freezes
- **Master Toggle**: "Snurk" button with configurable fade-in and bit depth ramping

## Parameters

### Core Effects
| Parameter | Range | Description |
|-----------|-------|-------------|
| Bit Depth | 4-16 bits | Bit reduction amount |
| Downsample Factor | 1-20 | Sample rate reduction |
| Glitch Length | 64-2048 samples | Length of glitch buffer repeats |
| Glitch Chance | 0.01-100% | Probability of glitch triggering |

### Granular
| Parameter | Range | Description |
|-----------|-------|-------------|
| Grain Size | 10-200 ms | Length of each grain |
| Grain Spacing | 1-100 ms | Time between grains |
| Playback Rate | 0.25-2.0x | Grain playback speed |
| Jitter Amount | 0-20 ms | Random grain position variation |

### Modulation
| Parameter | Range | Description |
|-----------|-------|-------------|
| Pitch Depth | 0.1-50 ms | Tape modulation depth |
| Pitch Rate | 0.01-10 Hz | Tape modulation speed |
| Drive | 0-1 | Distortion amount |
| Blend | 0-1 | Distortion dry/wet mix |
| Stereo Depth | 0-1 | Pan modulation amount |
| Stereo Rate | 0.01-10 Hz | Pan modulation speed |

### Macro Controls
| Parameter | Range | Description |
|-----------|-------|-------------|
| Fade In Duration | 0.01-10 s | Time for full effect to engage |
| Bit Depth Ramp Speed | 0.01-10 | How quickly bit depth reduces |
| Enable Effect | On/Off | Master "Snurk" toggle |

## Technical Implementation

### Key Algorithms
- **Bitcrusher**: Sample-and-hold with configurable bit depth
- **Glitch Buffer**: Circular buffer with random reads and Hann window crossfades
- **Granular**: Multi-voice grain scheduler with Hermite interpolation
- **Tape Modulation**: Dual-tap delay with LFO-controlled crossfading
- **Distortion**: Arctangent waveshaper (`(2/π) * atan(x * drive * range)`)
- **Stereo Panner**: Random-walk LFO with sin/cos panning law

### Smoothing
All parameters include ramp smoothing to prevent zipper noise during automation.

## Requirements
- JUCE Framework
- Projucer

## Building
1. Open the `.jucer` file in Projucer
2. Export to your preferred IDE (Visual Studio, Xcode, etc.)
3. Build the project

## Author
Omar Zaghouani
313 Studios

## Notes
This is an experimental glitch processor designed for creative sound design. For best results, experiment with automation of the master "Snurk" toggle and parameter combinations.