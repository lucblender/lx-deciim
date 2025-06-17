#include "DaisyDuino.h"

#define N_BITS 12

DaisyHardware hw;
uint32_t bitNumberAnalog = 12;

void MyCallback(float **in, float **out, size_t size)
{

  for (size_t i = 0; i < size; i++)
  {
    out[0][i] = bitCrush(in[0][i], bitNumberAnalog);
    out[1][i] = bitCrush(in[1][i], bitNumberAnalog);
  }
}

void setup()
{
  Serial.begin(115200);
  // put your setup code here, to run once:
  float sample_rate;
  // Initialize for Daisy pod at 48kHz
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  sample_rate = DAISY.get_samplerate();

  DAISY.begin(MyCallback);
}

void loop()
{
  // put your main code here, to run repeatedly:
  bitNumberAnalog = analogRead(A6) / 93 + 1;
  delay(100);
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
