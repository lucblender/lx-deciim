#include "daisy_seed.h"

using namespace daisy;

void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t size)
{

    for (size_t i = 0; i < size; i += 2)
    {
        out[i] = in[i];
        out[i + 1] = in[i + 1];
    }
}

/** Global Hardware access */
DaisySeed hw;

float sample_rate;

int main(void)
{
    /** Initialize our hardware */
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);

    // How many samples we'll output per second
    sample_rate = hw.AudioSampleRate();

    hw.StartLog();

    hw.StartAudio(AudioCallback);

    /** Infinite Loop */
    while (1)
    {
    }
}