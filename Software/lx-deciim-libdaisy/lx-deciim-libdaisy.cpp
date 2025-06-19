#include "daisy_seed.h"
#include "inc/DownSampler.h"

using namespace daisy;

#define N_BITS 12
#define MAX_DOWNSAMPLE_FREQ 1000

uint32_t bitNumberAnalog = 12;
float downSamplingIndex = 1.0f;

#define BIT_CRUSH_AN seed::A6
#define DOWN_SAMPLE_AC seed::A7

AdcChannelConfig bitCrushAn = AdcChannelConfig();
AdcChannelConfig downSampleAn = AdcChannelConfig();

DownSampler downSampler0 = DownSampler();
DownSampler downSampler1 = DownSampler();

float bitCrush(float in, uint8_t bits);

void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size)
{
    downSampler0.setDownSamplingFactor(downSamplingIndex);
    downSampler1.setDownSamplingFactor(downSamplingIndex);
    for (size_t i = 0; i < size; i += 2)
    {
        out[i] = downSampler0.Process(bitCrush(in[i], bitNumberAnalog));
        out[i + 1] = downSampler1.Process(bitCrush(in[+1], bitNumberAnalog));
    }
}

/** Global Hardware access */
DaisySeed hw;

float sample_rate;

int main(void)
{
    /** Initialize our hardware */
    hw.Init();
    hw.SetAudioBlockSize(48);

    // How many samples we'll output per second
    sample_rate = hw.AudioSampleRate();

    hw.StartLog();

    bitCrushAn.InitSingle(BIT_CRUSH_AN);
    downSampleAn.InitSingle(DOWN_SAMPLE_AC);
    AdcChannelConfig adcChannelConfigs[] = {bitCrushAn, downSampleAn};

    hw.adc.Init(adcChannelConfigs, 2);
    hw.adc.Start();

    hw.StartAudio(AudioCallback);

    /** Infinite Loop */
    while (1)
    {
        bitNumberAnalog = (hw.adc.Get(0) / 5940) + 1;
        float rawDownSamplingIndex = sample_rate / (hw.adc.Get(1) / 65536.0f) * (sample_rate - MAX_DOWNSAMPLE_FREQ) + MAX_DOWNSAMPLE_FREQ;
        // do those cast to set get only one decimal
        downSamplingIndex = static_cast<float>(static_cast<int>(rawDownSamplingIndex * 10.)) / 10.;

        hw.DelayMs(100);
    }
}

float bitCrush(float in, uint8_t bits)
{
    float dataIn = (in + 1.0f) / 2.0f;

    int16_t max = (1 << N_BITS) - 1; // 4095
    int16_t maxLevels = (1 << bits) - 1;

    int16_t intData = static_cast<int16_t>(roundf(dataIn * max));
    intData = (intData * maxLevels) / max; // Reduce resolution

    // Convert back to float in range [0,1]
    float crushed = static_cast<float>(intData) / maxLevels;

    // Scale back to [-1,1]
    return crushed * 2.0f - 1.0f;
}
