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
#include "AppCore.h"
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
    
    void audioReceived(float * input, int bufferSize, int nChannels);
    void audioRequested(float * output, int bufferSize, int nChannels);
    
    AppCore core;
    
#pragma mark - Myo Custom Methods
    
    void setupMyo();
    void updateMyo();
    
#pragma mark - Myo Custom Variables
    
    myo::Hub* hub;
    myo::Myo* myo;
    DataCollector collector;
    
    int roll_w, pitch_w, yaw_w;
    float roll, pitch, yaw;
    std::vector<float> emgVals;
};