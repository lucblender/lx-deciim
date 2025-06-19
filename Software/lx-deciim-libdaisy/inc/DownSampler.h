#ifndef DOWNSAMPLER_H
#define DOWNSAMPLER_H

#pragma once

class DownSampler
{
public:
    DownSampler();
    ~DownSampler();

    float Process(float in);
    void setDownSamplingFactor(float downSamplingFactor);

private:
    float storedSample = 0.0f;
    float downSamplingFactor = 1.0f;
    float sampleCounter = 0.0f;
    int sampleIndex = 0;
};

#endif