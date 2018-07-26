#pragma once
#include <Arduino.h>
#include <OLED_I2C.h>
#include <UltrasoundBase.h>
#include <Encoder.h>
#include <hcsr04.h>
#include <ClawBase.h>
#include <Menu.h>
#include <LineFollower.h>
#include <FFT.h>
#include <InfraredBase.h>
#include <Movement.h>
#include "procedures.h"

void testUltrasound();
void testEncoder();
void testClawOnly();
void testMenu();
void testLineFollow();
void testPWM();
void testFFT();
void testEncoders();
void testLFandReverse();
void testPickingUpEwok();
void testLift();
void testMovement();
void crossBridge();
void findFFTSignal();
void testInfrared();
void encoderReadings();
void fftTuning();
