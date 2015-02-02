/*
 * Copyright (c) 2011 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/danomatika/ofxPd for documentation
 *
 */
#pragma once

#include "ofMain.h"
//#include "AppCore.h"
#include <myo/myo.hpp>
#include "DataCollector.h"

/// a desktop os app wrapper
class testApp : public ofBaseApp{

public:
    
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed  (int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    
#pragma mark - Myo Custom Methods
    
    void setupMyo();
    void setupAudio();
    void updateMyo();
    
#pragma mark - Myo Custom Variables
    
    myo::Hub* hub;
    std::vector<myo::Myo*> knownMyos;
    DataCollector collector;
    
    int roll_w, pitch_w, yaw_w;
    float accel_x, accel_y, accel_z;
    float d_accel_x, d_accel_y, d_accel_z;
    int time;
    float roll, pitch, yaw;
    std::vector<float> emgVals;
    std::vector<float> accelXValues;
    std::vector<float> accelYValues;
    std::vector<float> accelZValues;
    
    void drawAccelTimeline();
    
#pragma mark - Game Logic
    
    void chooseNewPosition();
    int determineWinner();
    float goalRoll, goalPitch, goalYaw;
    
    int maxTime;
    int timeLeft;
    
    int wins1, wins2;
    int prevWinner;
    
};
