#pragma once
#include <Arduino.h>
#include <Encoder.h>
#include <Constants.h>

class PIDSpeed {
private:
    int16_t baseSpeed;
    int16_t timePerEncoder;
    Encoder enc1 = Encoder(Constants::LEFT_ENC_PIN), enc2 = Encoder(Constants::RIGHT_ENC_PIN);
    int8_t encoderCount = 0;
    int16_t error, previousError;
    int32_t initTime, counter, lastTime;
    bool movingState = false;
    int16_t enc1InitVal, enc2InitVal;
    int16_t ans;
    constexpr static int16_t maxSpeed = 255, minSpeed = 0;
public:
    void start(int16_t, int16_t);
    void start(int16_t, int16_t, Encoder);
    void start(int16_t, int16_t, Encoder, Encoder);
    void initValues();
    int32_t getEncoderVal() const;
    bool poll();
    int16_t getSpeed() const;
};
