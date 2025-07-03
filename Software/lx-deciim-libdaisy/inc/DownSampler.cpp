#include "DownSampler.h"

DownSampler::DownSampler()
{
}

DownSampler::~DownSampler()
{
}

void DownSampler::setDownSamplingFactor(float downSamplingFactor)
{
    this->downSamplingFactor = downSamplingFactor;
}

float DownSampler::Process(float in)
{
    float out;

    if ((uint32_t)(this->sampleIndex - this->sampleCounter) >= this->downSamplingFactor)
    {
        this->sampleCounter = this->sampleIndex;
        out = in;
        this->storedSample = in;
    }
    else
    {
        out = this->storedSample;
    }

    this->sampleIndex++;
    return out;
}
