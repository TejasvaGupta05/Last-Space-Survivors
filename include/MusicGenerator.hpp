#ifndef MUSIC_GENERATOR_HPP
#define MUSIC_GENERATOR_HPP

#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class MusicGenerator {
public:
    static sf::SoundBuffer generateSpaceTrack() {
    const unsigned int sampleRate = 44100;
    const float bpm = 130.0f;
    const float beatDuration = 60.0f / bpm;
    const int totalBars = 4; // longer 4-bar loop
    const float totalDuration = beatDuration * 4.0f * totalBars;
    const size_t sampleCount = static_cast<size_t>(totalDuration * sampleRate);

    std::vector<std::int16_t> buffer(sampleCount, 0);

    auto addSample = [&](size_t index, float value) {
        if (index >= sampleCount) return;
        float current = buffer[index] / 32768.0f;
        float mix = current + value;
        mix = std::clamp(mix, -1.0f, 1.0f);
        buffer[index] = static_cast<std::int16_t>(mix * 32767.0f);
    };

    // -----------------------
    // 1. Kick Drum
    // -----------------------
    for (int beat = 0; beat < totalBars * 4; ++beat) {
        size_t start = static_cast<size_t>(beat * beatDuration * sampleRate);
        for (size_t i = 0; i < 12000; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            float freq = 55.0f + 250.0f * std::exp(-t * 25.0f);
            float amp = std::exp(-t * 12.0f);
            float click = std::exp(-t * 100.0f) * 0.4f;
            float sample = (std::sin(2.0f * M_PI * freq * t) * amp * 0.8f) + click;
            addSample(start + i, sample);
        }
    }

    // -----------------------
    // 2. Bass (Groovy saw)
    // -----------------------
    float bassNotes[] = {55.f, 55.f, 65.41f, 73.42f, 49.f, 55.f, 65.41f, 82.41f};
    int steps = totalBars * 8;
    float stepDuration = totalDuration / steps;

    for (int s = 0; s < steps; ++s) {
        float freq = bassNotes[s % 8];
        size_t start = static_cast<size_t>(s * stepDuration * sampleRate);
        size_t length = static_cast<size_t>(stepDuration * 0.7f * sampleRate);

        for (size_t i = 0; i < length; ++i) {
            float t = (float)i / sampleRate;

            float sample = 0;
            for (int k = 1; k <= 6; ++k) sample += std::sin(2*M_PI*freq*k*t) / k;
            sample *= 0.18f;

            float env = 1.0f - (float)i / length;
            sample *= env * 1.2f;

            float beatPos = std::fmod((s * stepDuration) + t, beatDuration);
            float sidechain = 1.0f - std::exp(-beatPos * 15.0f); // pump
            sample *= sidechain;

            addSample(start + i, sample);
        }
    }

    // -----------------------
    // 3. Hats (Filtered noise)
    // -----------------------
    std::mt19937 rng(999);
    std::uniform_real_distribution<float> noiseDist(-1.f, 1.f);

    for (int i = 0; i < totalBars * 16 * 4; ++i) {
        size_t start = static_cast<size_t>((i * beatDuration / 4.0f) * sampleRate);
        size_t length = static_cast<size_t>(0.03f * sampleRate);

        if (i % 4 == 2) continue; // skip for groove

        float last = 0;
        for (size_t j = 0; j < length; ++j) {
            float white = noiseDist(rng) * 0.2f;
            float hp = white - last * 0.7f; // simple HP filter
            last = white;

            float env = 1.0f - (float)j / length;
            addSample(start + j, hp * env);
        }
    }

    // -----------------------
    // 4. Pad (Chord movement)
    // -----------------------
    float chordFreqs[4][3] = {
        {220.0f, 260.63f, 320.63f}, // Am
        {174.61f, 220.0f, 260.63f}, // F
        {196.0f, 246.94f, 320.63f}, // G
        {220.0f, 260.63f, 320.63f}  // Am resolve
    };

    for (size_t i = 0; i < sampleCount; ++i) {
        float t = (float)i / sampleRate;
        int section = (int)((t / totalDuration) * 4);
        auto& ch = chordFreqs[section];

        float pad = 0;
        for (float f : ch)
            pad += std::sin(2 * M_PI * f * t + 0.3f * std::sin(t * 0.3f));

        pad *= 0.03f;
        pad *= 0.4f + 0.6f * std::sin(t * 0.5f); // slow swell

        float beatPos = std::fmod(t, beatDuration);
        float sidechain = 1.0f - std::exp(-beatPos * 10.0f);
        pad *= sidechain;

        addSample(i, pad);
    }

    sf::SoundBuffer sb;
    if (!sb.loadFromSamples(buffer.data(), buffer.size(), 1, sampleRate, {sf::SoundChannel::Mono})) {
        // Handle error if needed, for now just return empty or log? 
        // Since we generated it, it should be fine.
    }
    return sb;
}

};

#endif
