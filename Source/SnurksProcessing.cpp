/*
  ==============================================================================
    SnurksProcessing.h
  ==============================================================================
*/

#include "SnurksProcessing.h"

// === Bitcrusher ===
void Bitcrusher::prepare(double /*sampleRate*/) {
    sampleCounter = 0;
    lastOutput = 0.0f;
}

void Bitcrusher::setBitDepth(float newBitDepth) { bitDepth = newBitDepth; }
void Bitcrusher::setDownsampleFactor(int newFactor) { downsampleFactor = std::max(1, newFactor); }

void Bitcrusher::updateParameters(float newBitDepth, int newFactor) {
    setBitDepth(newBitDepth);
    setDownsampleFactor(newFactor);
}

float Bitcrusher::processSample(float inputSample) {
    if (sampleCounter++ % downsampleFactor == 0) {
        float maxAmplitude = std::pow(2.0f, bitDepth - 1) - 1.0f;
        lastOutput = std::round(inputSample * maxAmplitude) / maxAmplitude;
    }
    return lastOutput;
}

// === GlitchBuffer ===
void GlitchBuffer::prepare(double newSampleRate, int maxBufferSize) {
    sampleRate = newSampleRate;
    bufferSize = maxBufferSize;
    buffer.setSize(2, bufferSize);
    buffer.clear();
    hannWindow.setSize(1, bufferSize);
    for (int i = 0; i < bufferSize; ++i) {
        float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (bufferSize - 1)));
        hannWindow.setSample(0, i, window);
    }
}

void GlitchBuffer::updateParameters(int newLength, float newChance) {
    glitchLengthSamples = newLength;
    glitchChance = newChance;
}

void GlitchBuffer::writeToBuffer(const juce::AudioBuffer<float>& input) {
    const int numSamples = input.getNumSamples();
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int i = 0; i < numSamples; ++i) {
            buffer.setSample(channel, writeHead, input.getSample(channel, i));
        }
    }
    writeHead = (writeHead + numSamples) % bufferSize;
}

void GlitchBuffer::processBlock(juce::AudioBuffer<float>& output) {
    const int numSamples = output.getNumSamples();
    const int numChannels = output.getNumChannels();

    if (!isGlitching && juce::Random::getSystemRandom().nextFloat() < glitchChance) {
        glitchStart = (writeHead + bufferSize - glitchLengthSamples) % bufferSize;
        glitchHead = 0;
        currentGlitchLength = glitchLengthSamples;
        isGlitching = true;
    }

    for (int i = 0; i < numSamples; ++i) {
        for (int channel = 0; channel < numChannels; ++channel) {
            float dry = output.getSample(channel, i);
            float wet = dry;
            if (isGlitching) {
                int bufferIndex = (glitchStart + glitchHead) % bufferSize;
                float sample = buffer.getSample(channel, bufferIndex);
                float windowValue = hannWindow.getSample(0, glitchHead % bufferSize);
                wet = sample * windowValue;
            }
            output.setSample(channel, i, wet);
        }
        if (isGlitching) {
            glitchHead++;
            if (glitchHead >= currentGlitchLength) isGlitching = false;
        }
    }
}

// === GranularTimeStretch ===
GranularTimeStretch::GranularTimeStretch() : rng(std::random_device{}()) {}

void GranularTimeStretch::prepare(double newSampleRate, int maxBlockSize) {
    sampleRate = newSampleRate;
    bufferSize = static_cast<int>(sampleRate * 2.0);
    writeHead = 0;
    circularBuffer.resize(numChannels);
    for (auto& channel : circularBuffer) channel.assign(bufferSize, 0.0f);
    grains.resize(numChannels);
    for (auto& ch : grains) ch.resize(numGrainsPerChannel);
    windowBuffer.resize(2048);
    juce::dsp::WindowingFunction<float>::fillWindowingTables(windowBuffer.data(), windowBuffer.size(), juce::dsp::WindowingFunction<float>::hann, false);
    smoothedGrainSize.reset(sampleRate, 0.05);
    smoothedGrainSpacing.reset(sampleRate, 0.1);
    smoothedRate.reset(sampleRate, 0.05);
    smoothedJitter.reset(sampleRate, 0.05);
}

void GranularTimeStretch::updateParameters(int grainSizeMs, int spacingMs, float rate, int jitterMs) {
    smoothedGrainSpacing.setTargetValue(static_cast<float>(spacingMs * sampleRate / 1000.0));
    smoothedGrainSize.setTargetValue(static_cast<float>(grainSizeMs * sampleRate / 1000.0));
    smoothedRate.setTargetValue(juce::jlimit(0.25f, 2.0f, rate));
    smoothedJitter.setTargetValue(static_cast<float>(jitterMs * sampleRate / 1000.0));
}

void GranularTimeStretch::spawnGrain(int channel, int grainSizeSamples) {
    for (auto& grain : grains[channel]) {
        if (!grain.active) {
            int jitter = jitterDist(rng);
            int start = (writeHead + bufferSize - grainSizeSamples + jitter + bufferSize) % bufferSize;
            std::vector<float> env(grainSizeSamples);
            juce::dsp::WindowingFunction<float>::fillWindowingTables(env.data(), grainSizeSamples, juce::dsp::WindowingFunction<float>::hann, false);
            grain = { start, grainSizeSamples, 0.0f, true, std::move(env) };
            break;
        }
    }
}

float GranularTimeStretch::readGrainSample(const std::vector<float>& buf, const Grain& grain, float playbackRate) {
    float readPos = grain.startSample + grain.readHead * playbackRate;
    int indexA = static_cast<int>(std::floor(readPos));
    int indexB = (indexA + 1) % bufferSize;
    float frac = readPos - indexA;
    float sample = buf[indexA % bufferSize] * (1.0f - frac) + buf[indexB] * frac;
    int winIndex = static_cast<int>((grain.readHead / grain.length) * grain.envelope.size());
    winIndex = juce::jlimit(0, static_cast<int>(grain.envelope.size()) - 1, winIndex);
    return sample * grain.envelope[winIndex];
}

void GranularTimeStretch::processBlock(juce::AudioBuffer<float>& buffer) {
    int numSamples = buffer.getNumSamples();
    for (int i = 0; i < numSamples; ++i) {
        int currentSpacing = static_cast<int>(smoothedGrainSpacing.getNextValue());
        int currentGrainSize = static_cast<int>(smoothedGrainSize.getNextValue());
        float currentRate = smoothedRate.getNextValue();
        int currentJitter = static_cast<int>(smoothedJitter.getNextValue());
        jitterDist = std::uniform_int_distribution<int>(-currentJitter, currentJitter);
        for (int ch = 0; ch < numChannels; ++ch) {
            auto& buf = circularBuffer[ch];
            buf[writeHead] = buffer.getReadPointer(ch)[i];
            float output = 0.0f;
            int activeGrains = 0;
            for (auto& grain : grains[ch]) {
                if (grain.active) {
                    output += readGrainSample(buf, grain, currentRate);
                    grain.readHead += 1.0f;
                    if (grain.readHead >= grain.length) grain.active = false;
                    ++activeGrains;
                }
            }
            if (activeGrains > 0) output /= static_cast<float>(activeGrains);
            buffer.getWritePointer(ch)[i] = output;
        }
        samplesSinceLastGrain++;
        if (samplesSinceLastGrain >= currentSpacing) {
            for (int ch = 0; ch < numChannels; ++ch) spawnGrain(ch, currentGrainSize);
            samplesSinceLastGrain = 0;
        }
        writeHead = (writeHead + 1) % bufferSize;
    }
}

// === TapePitchModulator ===
void TapePitchModulator::prepare(double newSampleRate, float maxDelayMs, int blockSize) {
    sampleRate = newSampleRate;
    int maxDelaySamples = static_cast<int>((maxDelayMs / 1000.0f) * sampleRate);
    bufferSize = maxDelaySamples * 2;
    delayBuffer.resize(2);
    for (auto& channel : delayBuffer) channel.assign(bufferSize, 0.0f);
    writePosition = 0;
    juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(blockSize), 1 };
    lfo.initialise([](float x) { return 0.5f * std::sin(x) + 0.5f; }, 128);
    lfo.prepare(spec);
    lfo.reset();
    crossfade.reset(sampleRate, 0.02f);
    depthSamples.reset(sampleRate, 0.05);
}

void TapePitchModulator::updateParameters(float depthInMs, float rateInHz) {
    float newDepth = (depthInMs / 1000.0f) * static_cast<float>(sampleRate);
    depthSamples.setTargetValue(newDepth);
    lfoRateHz = rateInHz;
    lfo.setFrequency(lfoRateHz);
}

float TapePitchModulator::cubicInterpolate(const float* buf, int size, float index) {
    int i = static_cast<int>(index);
    if (i < 1 || i + 2 >= size) return buf[i % size];
    float frac = index - i;
    float y0 = buf[(i - 1) % size], y1 = buf[i % size], y2 = buf[(i + 1) % size], y3 = buf[(i + 2) % size];
    float a = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
    float b = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float c = -0.5f * y0 + 0.5f * y2;
    float d = y1;
    return a * frac * frac * frac + b * frac * frac + c * frac + d;
}

void TapePitchModulator::processBlock(juce::AudioBuffer<float>& buffer) {
    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin(2, buffer.getNumChannels());
    for (int i = 0; i < numSamples; ++i) {
        float cross = crossfade.getNextValue();
        depthSamples.skip(1);
        float currentDepth = depthSamples.getCurrentValue();
        float delayTime = lfo.processSample(0.0f) * currentDepth;
        for (int ch = 0; ch < numChannels; ++ch) {
            auto& buf = delayBuffer[ch];
            buf[writePosition] = buffer.getReadPointer(ch)[i];
            float delayB = currentDepth - delayTime;
            float readA = static_cast<float>(writePosition) - delayTime;
            if (readA < 0) readA += bufferSize;
            float readB = static_cast<float>(writePosition) - delayB;
            if (readB < 0) readB += bufferSize;
            float output = juce::jmap(cross, cubicInterpolate(buf.data(), bufferSize, readB), cubicInterpolate(buf.data(), bufferSize, readA));
            buffer.getWritePointer(ch)[i] = output;
        }
        writePosition = (writePosition + 1) % bufferSize;
        lfoPhaseCounter += 1.0f / sampleRate;
        if (lfoPhaseCounter >= (1.0f / lfoRateHz)) {
            useFirstTap = !useFirstTap;
            lfoPhaseCounter = 0.0f;
            crossfade.setTargetValue(useFirstTap ? 1.0f : 0.0f);
        }
    }
}

// === Distortion ===
void Distortion::distort(float* channelData, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        float clean = channelData[i];
        float x = clean * driveDist * range;
        x = (2.0f / juce::MathConstants<float>::pi) * std::atan(x);
        channelData[i] = blend * x + (1.0f - blend) * clean;
    }
}

void Distortion::updateDistort(const float newDist, const float newBlend) {
    driveDist = newDist;
    blend = newBlend;
}

// === Stereo Panner ===
void StereoPanner::prepare(double sampleRate) {
    currentSampleRate = sampleRate;
    rng.seed(std::random_device{}());
    noiseDist = std::uniform_real_distribution<float>(-1.0f, 1.0f);
    panSmoothed.reset(sampleRate, 0.1);
    panSmoothed.setCurrentAndTargetValue(0.0f);
    setRate(modulationRate);
}

void StereoPanner::setRate(float hz) {
    modulationRate = juce::jlimit(0.01f, 10.0f, hz);
    samplesUntilNextPan = static_cast<int>(currentSampleRate / modulationRate);
}

void StereoPanner::setDepth(float depth) { lfoDepth = juce::jlimit(0.0f, 1.0f, depth); }
void StereoPanner::updateParameters(float newRate, float newDepth) { setRate(newRate); setDepth(newDepth); }

void StereoPanner::processBlock(juce::AudioBuffer<float>& buffer) {
    const int numSamples = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2) return;
    float* left = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);
    for (int i = 0; i < numSamples; ++i) {
        if (--samplesUntilNextPan <= 0) {
            panSmoothed.setTargetValue(noiseDist(rng));
            samplesUntilNextPan = static_cast<int>(currentSampleRate / modulationRate);
        }
        float pan = panSmoothed.getNextValue() * lfoDepth;
        float leftGain = std::cos((1.0f + pan) * 0.25f * juce::MathConstants<float>::pi);
        float rightGain = std::cos((1.0f - pan) * 0.25f * juce::MathConstants<float>::pi);
        left[i] *= leftGain;
        right[i] *= rightGain;
    }
}

// === Dropout Freeze ===
void DropoutFreeze::prepare(double newSampleRate) { sampleRate = newSampleRate; rng.seed(std::random_device{}()); }
void DropoutFreeze::setRate(float rateHz) { triggerRate = rateHz; }
void DropoutFreeze::setHoldDuration(int durationSamples) { holdDuration = durationSamples; }
void DropoutFreeze::updateParameters(float newRate, int newDuration) { setRate(newRate); setHoldDuration(newDuration); }
bool DropoutFreeze::shouldTrigger() { return chanceDist(rng) < (triggerRate / sampleRate); }

float DropoutFreeze::processSample(float input, int channel) {
    auto& state = states[channel];
    if (state.frozen) {
        if (++state.holdCounter >= holdDuration) state.frozen = false;
        return state.heldSample;
    }
    else if (shouldTrigger()) {
        state.frozen = true;
        state.heldSample = input;
        state.holdCounter = 0;
        return state.heldSample;
    }
    return input;
}

// === Snurks Wrapper Methods ===
void Snurks::prepare(double sampleRate, int samplesPerBlock) {
    this->sampleRate = sampleRate;
    bitcrusher.prepare(sampleRate);
    glitch.prepare(sampleRate, static_cast<int>(sampleRate));
    granular.prepare(sampleRate, samplesPerBlock);
    pitchmod.prepare(sampleRate, 100.0f, samplesPerBlock);
    stereoPanner.prepare(sampleRate);
    dropout.prepare(sampleRate);
    fadeSampleCounter = 0;
    effectIntensity = 0.0f;
}

void Snurks::updateBitcrusher(float newBitDepth, int newFactor) { bitcrusher.updateParameters(newBitDepth, newFactor); }
void Snurks::updateGlitch(int newLength, float newChance) { glitch.updateParameters(newLength, newChance); }
void Snurks::updateGranular(int nSize, int nSpace, float nRate, int nJitter) { granular.updateParameters(nSize, nSpace, nRate, nJitter); }
void Snurks::updatePitchmod(float newDepth, float newRate) { pitchmod.updateParameters(newDepth, newRate); }
void Snurks::updateDistortion(float newDist, float newBlend) { distortion.updateDistort(newDist, newBlend); }
void Snurks::updateStereoPanner(float newRate, float newDepth) { stereoPanner.updateParameters(newRate, newDepth); }

void Snurks::triggerEffect(bool snurking) {
    effectActive = snurking;
    if (!effectActive) {
        fadeSampleCounter = 0;
        effectIntensity = 0.0f;
        bitcrusher.setBitDepth(16.0f);
    }
}

void Snurks::setFadeInDuration(float seconds) { fadeInDurationSeconds = std::max(0.01f, seconds); }
void Snurks::setBitDepthRampSpeed(float speed) { bitDepthRampSpeed = std::max(0.01f, speed); }

void Snurks::updateFadeAndBitcrusher(int numSamples) {
    if (!effectActive) {
        effectIntensity = 0.0f;
        bitcrusher.setBitDepth(16.0f);
        return;
    }
    int fadeInSamples = static_cast<int>(fadeInDurationSeconds * sampleRate);
    for (int i = 0; i < numSamples; ++i) {
        effectIntensity = (fadeSampleCounter < fadeInSamples) ? static_cast<float>(fadeSampleCounter++) / fadeInSamples : 1.0f;
        float targetDepth = 16.0f - (12.0f * std::min(1.0f, effectIntensity * bitDepthRampSpeed));
        bitcrusher.setBitDepth(targetDepth);
    }
}

void Snurks::processBlock(juce::AudioBuffer<float>& buffer, int numSamples) {
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        distortion.distort(channelData, numSamples);
        for (int sample = 0; sample < numSamples; ++sample) {
            channelData[sample] = bitcrusher.processSample(channelData[sample]);
        }
    }
    glitch.writeToBuffer(buffer);
    glitch.processBlock(buffer);
    pitchmod.processBlock(buffer);
    granular.processBlock(buffer);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        auto* wet = buffer.getWritePointer(channel);
        auto* dry = dryBuffer.getReadPointer(channel);
        for (int i = 0; i < numSamples; ++i) {
            wet[i] = dry[i] * (1.0f - effectIntensity) + wet[i] * effectIntensity;
        }
    }
}