#include "procedures.h"
#include <Arduino.h>
#include <Menu.h>
#include <LineFollower.h>
#include <Encoder.h>
#include <ClawBase.h>
#include <Movement.h>
#include <InfraredBase.h>
#include <FFT.h>

Servo bridgeServo;

bool lookForEwok(int threshold, int movementRange, int forwardAmount, int tries) {
    bool foundEwok = false;
    Movement mvt;
    tries = 1;
    for(int i=0;i<tries;++i) {
        mvt.start(1, -1, movementRange, movementRange, 100);
        infrared.startMeasurement();
        while(mvt.poll()) {
            if(!infrared.poll()) {
                if(infrared.objectDetected(threshold)){
                    foundEwok = true;
                    break;
                }
                infrared.startMeasurement();
            }
        }
        if(foundEwok) {
            break;
        }
    }
    motor.speed(Constants::MOTOR_LEFT, -255);
    motor.speed(Constants::MOTOR_RIGHT, 255);
    delay(20);
    motor.speed(Constants::MOTOR_LEFT, 0);
    motor.speed(Constants::MOTOR_RIGHT, 0);
    return foundEwok;
}

bool moveForwardToEwok(int threshold, int maxDistance) {
    Movement mvt;
    bool foundEwok = false;
    mvt.start(1, 1, maxDistance, maxDistance, 90);
    infrared.startMeasurement();
    while(mvt.poll()) {
        if(!infrared.poll()) {
            if(infrared.lastMeasurement() > threshold) {
                foundEwok = true;
                break;
            }
            infrared.startMeasurement();
        }
    }
    motor.speed(Constants::MOTOR_LEFT, 0);
    motor.speed(Constants::MOTOR_RIGHT, 0);
    return foundEwok;
}

void initialLineFollow(Encoder& leftEnc, Encoder& rightEnc) {
    lineFollower.start();
    for(int32_t i=0;lineFollower.poll();++i) {
        Encoder::poll();
        if(rightEnc.getPosition()>=132) {
            oled.clrScr();
            oled.print("!! 135", 0, 0);
            oled.update();
            break;
        }
        if(i%10000 == 0)
        {
            oled.clrScr();
            oled.print(const_cast<char*>("L:"), 0, 0);
            oled.print(const_cast<char*>("R:"), 0, 10);
            oled.print(const_cast<char*>("E:"), 0, 20);
            oled.print(const_cast<char*>("G:"), 0, 30);
            oled.print(const_cast<char*>("ER"), 0, 40);
            oled.print(const_cast<char*>("EN"), 0, 50);
            oled.printNumI(lineFollower.sensorLeftReading, 20, 0);
            oled.printNumI(lineFollower.sensorRightReading, 20, 10);
            oled.printNumI(lineFollower.sensorEdgeReading, 20, 20);
            oled.printNumI(lineFollower.sensorLeftReadingAmb, 55, 0);
            oled.printNumI(lineFollower.sensorRightReadingAmb, 55, 10);
            oled.printNumI(lineFollower.sensorEdgeReadingAmb, 55, 20);
            oled.printNumI(lineFollower.sensorLeftReadingPow, 90, 0);
            oled.printNumI(lineFollower.sensorRightReadingPow, 90, 10);
            oled.printNumI(lineFollower.sensorEdgeReadingPow, 90, 20);
            oled.printNumI(lineFollower.g, 20, 30);
            oled.printNumI(lineFollower.error, 20, 40);
            oled.printNumI(leftEnc.getPosition(), 20, 50);
            oled.printNumI(rightEnc.getPosition(), 60, 50);
            oled.printNumI(lineFollower.lastCompTime, 90, 30);
            if(i%20000 == 0)
            {
                oled.print(const_cast<char*>("."), 100, 0);
            }
            oled.update();
        }
    }
    motor.speed(Constants::MOTOR_LEFT, 0);
    motor.speed(Constants::MOTOR_RIGHT, 0);
    oled.print("!!", 0, 10);
    oled.update();
}

// /**
//  * Starting from the drop location, get the first ewok, return it to the start,
//  * and get back to the edge. If the ewok isn't found, then give up on it and just
//  * go back to the edge.
//  */
void handleFirstEwok(Encoder& leftEnc, Encoder& rightEnc) {

    Movement mvt;

    bool foundEwok = lookForEwok(Constants::distantInfraredThreshold1, 12, 3, 5);
    oled.clrScr();
    oled.printNumI(foundEwok,0,0);
    oled.update();
    delay(2000);

    if(foundEwok) {
        // TODO add the method to move foward to toward the ewok
        bool foundEwok2 = moveForwardToEwok(Constants::pickUpInfraredThreshold1, 10);

        oled.clrScr();
        oled.printNumI(foundEwok2,0,10);
        oled.update();

        claw.pickEwok();
        oled.clrScr();
        oled.print("Picked up ewok",0,20);
        oled.update();

        mvt.move(1,1,9,9,110);
    }

    //turn off leds
    digitalWrite(Constants::infraredLeds[0], LOW);
}

/**
 * Starting from the edge, maneuver to the location where the ewok is dropped.
 */
void maneuverToDropLocation(Encoder& leftEnc, Encoder& rightEnc) {
    Movement mvt;
    mvt.start(1, -1, 12, 12, 100);
    while(mvt.poll()){}
    // go back to the start

    oled.clrScr();
    oled.print("About to find line", 0, 0);
    oled.update();
    delay(1000);

    lineFollower.findLine(LineFollower::DIR_RIGHT, 80); // blocking
    oled.clrScr();
    oled.print("Done finding line", 0, 0);
    oled.update();

    delay(1000);

    leftEnc.reset();
    rightEnc.reset();

    int16_t leftEncPos = leftEnc.getPosition();
    int16_t rightEncPos = rightEnc.getPosition();

    lineFollower.start();
    while(lineFollower.poll()) {
        if(leftEnc.getPosition() > 100) {
            break;
        }
    }
    lineFollower.start(4096, 4096, Constants::EDGE_THRESHOLD.getVal(), 4096, 4096);
    while(lineFollower.poll()){}
    mvt.start(-1, -1, 10,10, 100);
    mvt.start(1,-1,12,12,100);
    while(mvt.poll()) {}
}

void maneuverToBridge() {
    Movement mvt;

    lineFollower.findLine(LineFollower::DIR_RIGHT, 80);

    lineFollower.start();
    while(lineFollower.poll()) {}

    //face gap
    mvt.start(-1,1,7,7,80);
    while(mvt.poll()){}
    motor.speed(Constants::MOTOR_LEFT, 255);
    motor.speed(Constants::MOTOR_RIGHT, -255);
    delay(30);
    motor.speed(Constants::MOTOR_LEFT, 0);
    motor.speed(Constants::MOTOR_RIGHT, 0);
}

void deployBridge(){
    Movement mvt;
    //turn around
    mvt.start(-1,1,20,20,80);
    while(mvt.poll()){}
    motor.speed(Constants::MOTOR_LEFT, 255);
    motor.speed(Constants::MOTOR_RIGHT, -255);
    delay(30);
    motor.speed(Constants::MOTOR_LEFT, 0);
    motor.speed(Constants::MOTOR_RIGHT, 0);

    bridgeServo.write(Constants::positionUnlock);
    //Go forward so that bridge drops
    mvt.start(1,1,4,4,110);
    while(mvt.poll()){}

    //go backwards across the bridge
    mvt.start(-1,-1,12,12,80);
    while(mvt.poll()){}
}

void handleSecondEwok(){
    Movement mvt;

    bool foundEwok = lookForEwok(Constants::distantInfraredThreshold2, 6, 3, 5);
    oled.clrScr();
    oled.printNumI(foundEwok,0,0);
    oled.update();
    delay(2000);

    if(foundEwok) {
        // TODO add the method to move foward to toward the ewok
        bool foundEwok2 = moveForwardToEwok(Constants::pickUpInfraredThreshold2, 10);

        oled.clrScr();
        oled.printNumI(foundEwok2,0,10);
        oled.update();

        claw.pickEwok();
        oled.clrScr();
        oled.print("Picked up ewok",0,20);
        oled.update();
    }

    //turn off leds
    digitalWrite(Constants::infraredLeds[0], LOW);
}

void IRBeacon() {
    fft.init();
    float totalAmount;
    //Movement mvt;
    //mvt.start(1, -1, 3, 3, 100);
    //while(mvt.poll()) {}
    bool foundIRBeacon = false;
    while(true) {
        //mvt.start(-1, 1, 6, 6, 90);
        //while(mvt.poll()) {
        while(true) {  
            float f10=0, f1=0;
            for(int i=0;i<10;++i) {
                FFTPair fftPair = fft.sample();
                f10 += fftPair.highAmount;
                f1 += fftPair.lowAmount;
            }
            totalAmount = sqrt(f10*f10 + f1*f1);
            if(totalAmount > Constants::FFT_THRESHOLD_TOTAL)
            {
                foundIRBeacon = true;
                break;
            }
        }
        if(foundIRBeacon) {
            break;
        }
        //mvt.start(1, -1, 6, 6, 90);
        //while(mvt.poll()) {}
    }
    //motor.speed(Constants::MOTOR_LEFT, 0);
    //motor.speed(Constants::MOTOR_RIGHT, 0);
    oled.clrScr();
    oled.print("ir beacon found", 0, 0);
    oled.printNumI(totalAmount,0,10);
    oled.update();
    int32_t lastOne = 0;
    while(true) {
        //mvt.start(-1, 1, 6, 6, 90);
        //while(mvt.poll()) {
        while(true) {  
            float f10=0, f1=0;
            for(int i=0;i<10;++i) {
                FFTPair fftPair = fft.sample();
                f10 += fftPair.highAmount;
                f1 += fftPair.lowAmount;
            }
            if(totalAmount > Constants::FFT_THRESHOLD_TOTAL)
            {
                if(f10/f1 > Constants::FFT_THRESHOLD_10K_FACTOR && millis()-lastOne < 200) {
                    break;
                } else if(f1/f10 > Constants::FFT_THRESHOLD_1K_FACTOR) {
                    lastOne = millis();
                }
            }
        }
        if(foundIRBeacon) {
            break;
        }

        //mvt.start(1, -1, 6, 6, 90);
        //while(mvt.poll()) {}
    }

    oled.clrScr();
    oled.print("GO!", 0,0);
    oled.update();
    delay(1000);
}

void mainRun() {
    Menu m;
    m.run();
    Encoder leftEnc(Constants::LEFT_ENC_PIN);
    Encoder rightEnc(Constants::RIGHT_ENC_PIN);
    bridgeServo.attach(Constants::BRIDGE_SERVO_PIN);
    bridgeServo.write(Constants::positionLock);
    oled.invertText(false);

    initialLineFollow(leftEnc, rightEnc);

    delay(2000);

    handleFirstEwok(leftEnc, rightEnc);

    delay(2000);

    maneuverToDropLocation(leftEnc, rightEnc);

    claw.dropEwok();

    maneuverToBridge();

    /*deployBridge();

    handleSecondEwok(); */

    // // we want to return to drop location so we go back onto the bridge
    // maneuverToBridge();

    // getToSecondEwok();

    // boolean gotSecondEwok = handleSecondEwok();

    // IRBeacon();

    // crossArch();

    // dropSecondEwok(gotSecondEwok);

    // handleThirdEwok();
}

void oneEwokRun() {
    // TODO
}
