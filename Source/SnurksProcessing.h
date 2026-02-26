/*
  ==============================================================================
    SnurksProcessing.h
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <vector>
#include <random>
#include <cmath>

class Bitcrusher
{
public:
    Bitcrusher() = default;
    void prepare(double sampleRate);
    void setBitDepth(float newBitDepth);
    void setDownsampleFactor(int newFactor);
    void updateParameters(float newBitDepth, int newFactor);
    float processSample(float inputSample);

private:
    float bitDepth = 8.0f;
    int downsampleFactor = 1;
    int sampleCounter = 0;
    float lastOutput = 0.0f;
};

class GlitchBuffer
{
public:
    void prepare(double newSampleRate, int maxBufferSize);
    void updateParameters(int newLength, float newChance);
    void writeToBuffer(const juce::AudioBuffer<float>& input);
    void processBlock(juce::AudioBuffer<float>& output);

private:
    juce::AudioBuffer<float> buffer;
    juce::AudioBuffer<float> hannWindow;

    int bufferSize = 0;
    int writeHead = 0;
    bool isGlitching = false;
    int glitchStart = 0;
    int glitchHead = 0;
    int currentGlitchLength = 0;
    int glitchLengthSamples = 2048;
    float glitchChance = 0.001f;
    double sampleRate = 44100.0;
};

class GranularTimeStretch
{
public:
    GranularTimeStretch();
    void prepare(double newSampleRate, int maxBlockSize);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void updateParameters(int grainSizeMs, int spacingMs, float rate, int jitterMs);

private:
    struct Grain
    {
        int startSample = 0;
        int length = 0;
        float readHead = 0.0f;
        bool active = false;
        std::vector<float> envelope;
    };

    void spawnGrain(int channel, int grainSizeSamples);
    float readGrainSample(const std::vector<float>& buffer, const Grain& grain, float playbackRate);

    double sampleRate = 44100.0;
    int bufferSize = 0;
    int writeHead = 0;
    int samplesSinceLastGrain = 0;

    juce::SmoothedValue<float> smoothedGrainSpacing;
    juce::SmoothedValue<float> smoothedGrainSize;
    juce::SmoothedValue<float> smoothedRate;
    juce::SmoothedValue<float> smoothedJitter;

    std::mt19937 rng;
    std::uniform_int_distribution<int> jitterDist;

    static constexpr int numChannels = 2;
    static constexpr int numGrainsPerChannel = 32;

    std::vector<std::vector<float>> circularBuffer;
    std::vector<std::vector<Grain>> grains;
    std::vector<float> windowBuffer;
};

class TapePitchModulator
{
public:
    void prepare(double newSampleRate, float maxDelayMs, int blockSize);
    void updateParameters(float depthInMs, float rateInHz);
    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    float getModulatedDelayTime();
    float cubicInterpolate(const float* buffer, int size, float index);

    double sampleRate = 44100.0;
    int bufferSize = 0;
    int writePosition = 0;

    std::vector<std::vector<float>> delayBuffer;
    juce::dsp::Oscillator<float> lfo;
    juce::SmoothedValue<float> crossfade{ 0.0f };
    juce::SmoothedValue<float> depthSamples;
    float lfoRateHz = 1.0f;
    bool useFirstTap = true;
    float lfoPhaseCounter = 0.0f;
};

class Distortion
{
public:
    void distort(float* channelData, int NumSamples);
    void updateDistort(const float newDist, const float newBlend);
private:
    float driveDist = 6.0;
    float blend = 0.0;
    float range = 1.0;
};

class StereoPanner
{
public:
    void prepare(double sampleRate);
    void setRate(float hz);
    void setDepth(float depth);
    void updateParameters(float newRate, float newDepth);
    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    double currentSampleRate = 44100.0;
    float lfoDepth = 1.0f;
    float modulationRate = 0.5f;
    juce::LinearSmoothedValue<float> panSmoothed;
    std::mt19937 rng;
    std::uniform_real_distribution<float> noiseDist;
    int samplesUntilNextPan = 0;
    juce::dsp::Oscillator<float> lfo;
};

class DropoutFreeze
{
public:
    void prepare(double newSampleRate);
    void setRate(float rateHz);
    void setHoldDuration(int durationSamples);
    void updateParameters(float newRate, int newDuration);
    float processSample(float input, int channel);

private:
    double sampleRate = 44100.0;
    int holdDuration = 200;
    float triggerRate = 1.0f;

    std::default_random_engine rng;
    std::uniform_real_distribution<float> chanceDist{ 0.0f, 1.0f };

    struct ChannelState {
        bool frozen = false;
        float heldSample = 0.0f;
        int holdCounter = 0;
    };

    ChannelState states[2];
    bool shouldTrigger();
};

class Snurks
{
public:
    Snurks() = default;
    void prepare(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples);
    void updateBitcrusher(float newBitDepth, int newFactor);
    void updateGlitch(int newLength, float newChance);
    void updateGranular(int newGrainSize, int newGrainSpacing, float newPlaybackRate, int newJitterAmount);
    void updatePitchmod(float newDepth, float newRate);
    void updateDistortion(float newDist, float newBlend);
    void updateStereoPanner(float newRate, float newDepth);
    void triggerEffect(bool snurking);
    void updateFadeAndBitcrusher(int numSamples);
    void setFadeInDuration(float seconds);
    void setBitDepthRampSpeed(float speed);

private:
    Bitcrusher bitcrusher;
    GlitchBuffer glitch;
    GranularTimeStretch granular;
    TapePitchModulator pitchmod;
    Distortion distortion;
    StereoPanner stereoPanner;
    DropoutFreeze dropout;

    bool effectActive = false;
    float fadeInDurationSeconds = 1.0f;
    float bitDepthRampSpeed = 1.0f;
    float effectIntensity = 0.0f;
    float targetbitDepth = 4.0f;
    double sampleRate = 44100.0;
    int fadeSampleCounter = 0;
};