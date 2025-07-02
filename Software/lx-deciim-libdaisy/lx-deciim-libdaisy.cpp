#include "daisy_seed.h"
#include "inc/DownSampler.h"

using namespace daisy;

#define N_BITS 12
#define MAX_DOWNSAMPLE_FREQ 1000.0f

// uncomment for debug gpios
// #define DEBUG_ANALOG
// #define DEBUG_DIGITAL

float downSamplingIndex = 1.0f;

#define DOWN_SAMPLE_AC seed::A11

daisy::Pin triggerPins[12] = {
    seed::D15, // LSB
    seed::D16,
    seed::D17,
    seed::D18,
    seed::D19,
    seed::D20,
    seed::D21,
    seed::D22,
    seed::D23,
    seed::D24,
    seed::D25,
    seed::D26 // MSB
};
GPIO triggerGPIOs[12] = {GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO()};
daisy::Pin switchPins[12] = {
    seed::D12, // LSB
    seed::D11,
    seed::D10,
    seed::D9,
    seed::D8,
    seed::D7,
    seed::D6,
    seed::D5,
    seed::D4,
    seed::D3,
    seed::D2,
    seed::D1 // MSB
};
GPIO switchGPIOs[12] = {GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO(), GPIO()};

uint16_t concatTrigger = 0;
uint16_t concatSwitch = 0;

AdcChannelConfig downSampleAn = AdcChannelConfig();

DownSampler downSampler0 = DownSampler();
DownSampler downSampler1 = DownSampler();

/** Global Hardware access */
DaisySeed hw;

float sample_rate;

float bitCrush(float in);
void readAnalogs();
void readDigitals();

void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size)
{
    downSampler0.setDownSamplingFactor(downSamplingIndex);
    downSampler1.setDownSamplingFactor(downSamplingIndex);

    for (size_t i = 0; i < size; i += 2)
    {
#ifndef DEBUG_DIGITAL
        readDigitals();
#endif
        out[i] = downSampler1.Process(bitCrush(in[i + 1]));
        out[i + 1] = downSampler0.Process(bitCrush(in[i]));
    }
}

int main(void)
{
    /** Initialize our hardware */
    hw.Init();
    hw.SetAudioBlockSize(48);

    // How many samples we'll output per second
    sample_rate = hw.AudioSampleRate();

    hw.StartLog();

    downSampleAn.InitSingle(DOWN_SAMPLE_AC);
    AdcChannelConfig adcChannelConfigs[] = {downSampleAn};

    hw.adc.Init(adcChannelConfigs, 1);
    hw.adc.Start();

    // triggers pin, input, no pull up/down
    for (int i = 0; i < 12; i++)
    {
        triggerGPIOs[i].Init(triggerPins[i], GPIO::Mode::INPUT);
    }

    // switch pin, input, pull up
    for (int i = 0; i < 12; i++)
    {
        switchGPIOs[i].Init(switchPins[i], GPIO::Mode::INPUT, GPIO::Pull::PULLUP, GPIO::Speed::LOW);
    }

    hw.StartAudio(AudioCallback);

    /** Infinite Loop */
    while (1)
    {
        readAnalogs();
#ifdef DEBUG_DIGITAL
        readDigitals();
        hw.PrintLine("result for 1.0f %f", bitCrush(1.0f));
        hw.PrintLine("result for -1.0f %f", bitCrush(-1.0f));
        hw.PrintLine("result for 0.5f %f", bitCrush(0.5f));
        hw.PrintLine("result for -0.5f %f", bitCrush(-0.5f));
        hw.PrintLine("result for 0.0f %f", bitCrush(0.0f));
#endif

    }
}

void readAnalogs()
{
    uint16_t anDownSampling = hw.adc.Get(0);

    float rawDownSamplingIndex = sample_rate / ((anDownSampling / 65536.0f) * (sample_rate - MAX_DOWNSAMPLE_FREQ) + MAX_DOWNSAMPLE_FREQ);
    // do those cast to set get only one decimal
    downSamplingIndex = static_cast<float>(static_cast<int>(rawDownSamplingIndex * 10.)) / 10.;

#ifdef DEBUG_ANALOG
    hw.PrintLine("anDownSampling: %d", anDownSampling);
    hw.PrintLine("downSamplingIndex: %f", downSamplingIndex);
#endif
}

void readDigitals()
{
    bool tmpRead = 0;
    uint16_t tmpConcatSwitch = 0;
    uint16_t tmpConcatTrigger = 0;
#ifdef DEBUG_DIGITAL
    hw.Print("switchGPIOs:\t");
#endif
    for (int i = 11; i >= 0; i--) // do from 11 to 0 so it's MSB to LSB (just for printing and debug)
    {
        tmpRead = switchGPIOs[i].Read();
        /* This turn off the leds when switches are set to off, this sadly cause problems with the triggers
        if (tmpRead == 0)
        {
            triggerGPIOs[i].Init(triggerPins[i], GPIO::Mode::OUTPUT);
            triggerGPIOs[i].Write(0);
        }
        else
            triggerGPIOs[i].Init(triggerPins[i], GPIO::Mode::INPUT);
        */
        tmpConcatSwitch = tmpConcatSwitch + (tmpRead << i);
#ifdef DEBUG_DIGITAL
        hw.Print("%d ", tmpRead);
#endif
    }
    concatSwitch = tmpConcatSwitch;

#ifdef DEBUG_DIGITAL
    hw.PrintLine("-\t concatSwitch:%d", concatSwitch);
    hw.Print("triggerGPIOs:\t");
#endif

    for (int i = 11; i >= 0; i--) // do from 11 to 0 so it's MSB to LSB (just for printing and debug)
    {
        tmpRead = triggerGPIOs[i].Read();
        tmpConcatTrigger = tmpConcatTrigger + (tmpRead << i);
#ifdef DEBUG_DIGITAL
        hw.Print("%d ", tmpRead);
#endif
    }
    concatTrigger = tmpConcatTrigger;
#ifdef DEBUG_DIGITAL
    hw.PrintLine("-\t concatTrigger:%d", concatTrigger);
#endif
}

float bitCrush(float in)
{
    // add a little offset 0f 0.01
    //  if it's 0.5 and there is nothing plugged
    // the float value oscillate a little above and below
    // and so with decimation, it will oscillate between big -1 and +1
    in = in + 0.01; //[-1,1] (plus little offset)

    float dataIn = (in + 1.0f) / 2.0f; //[0,1]

    uint16_t max = (1 << N_BITS) - 1; // 4095
    uint16_t intData = static_cast<uint16_t>(roundf(dataIn * max));

    intData = ((intData & (~concatSwitch)) & max) + ((concatTrigger & concatSwitch) & max);

    // Convert back to float in range [0,1]
    float crushed = static_cast<float>(intData) / max;

    return (crushed * 2.0f) - 1.0f; // return between [-1,1]
}
