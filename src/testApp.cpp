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
#include "testApp.h"

#define NUMBER_OF_KEYS 10

#include <Poco/Path.h>

//--------------------------------------------------------------
void testApp::setup() {
    
    this->setupMyo();
    this->chooseNewPosition();
    
    maxTime = 300;
    timeLeft = 300;
    
}

void testApp::setupMyo(){
    // We catch any exceptions that might occur below -- see the catch statement for more details.
    try {
        
        // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
        // publishing your application. The Hub provides access to one or more Myos.
        //        myo::Hub hub("com.example.hello-myo");
        hub = new myo::Hub("com.example.hello-myo");
        //        hub = myo::Hub("com.example.hello-myo");
        
        std::cout << "Attempting to find a Myo..." << std::endl;
        
        // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
        // immediately.
        // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
        // if that fails, the function will return a null pointer.
        myo::Myo* myo1 = hub->waitForMyo(10000);
        
        // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
        if (!myo1) {
            throw std::runtime_error("Unable to find a Myo!");
        }
        
        // We've found a Myo.
        std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        
        myo1->setStreamEmg(myo::Myo::streamEmgEnabled);
        knownMyos.push_back(myo1);
 
        ///////////////
        
        std::cout << "Attempting to find a Myo..." << std::endl;
        
        // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
        // immediately.
        // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
        // if that fails, the function will return a null pointer.
        myo::Myo* myo2 = hub->waitForMyo(10000);
        
        // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
        if (!myo2) {
            throw std::runtime_error("Unable to find a Myo!");
        }
        
        // We've found a Myo.
        std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        
        myo2->setStreamEmg(myo::Myo::streamEmgEnabled);
        knownMyos.push_back(myo2);
        
        collector.knownMyos = knownMyos;
        
        ///////////////
        
        
        // Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
        
        collector.setupDataCollector();
        
        // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
        // Hub::run() to send events to all registered device listeners.
        hub->addListener(&collector);
        
        // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}

//--------------------------------------------------------------
void testApp::update() {
    this->updateMyo();
    
    timeLeft--;
    if (timeLeft == 0) {
        timeLeft = maxTime;
        prevWinner = this->determineWinner();
        this->chooseNewPosition();
    }
}

void testApp::updateMyo() {
    
    time++;
    if (time > ofGetWindowWidth() - 200) {
        time = 0;
        accelXValues.clear();
        accelYValues.clear();
        accelZValues.clear();
    }
    
    try {
        // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
        // In this case, we wish to update our display 20 times a second, so we run for 1000/20 milliseconds.
        hub->run(1000/20);
        // After processing events, we call the print() member function we defined above to print out the values we've
        // obtained from any events that have occurred.
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}

//--------------------------------------------------------------
void testApp::draw() {
    
    
    ofSetColor(ofColor::darkGrey);
    ofRect(ofGetWindowWidth()/3, 0, ofGetWindowWidth()/3, ofGetWindowHeight());
    
    ofSetColor(ofColor::black, 100);
    ofRect(0, 0, ofGetWindowWidth(), 100);

    // GOAL POSITIONS
    
    ofSetColor(ofColor::white);
    
    ofCircle(ofGetWindowWidth() * 2/4, ofGetWindowHeight() * 1/4, goalRoll * 20);
    ofCircle(ofGetWindowWidth() * 2/4, ofGetWindowHeight() * 2/4, goalPitch * 20);
    ofCircle(ofGetWindowWidth() * 2/4, ofGetWindowHeight() * 3/4, goalYaw * 20);
    
    // PLAYER 1 POSITIONS
    
    if (prevWinner == 1) {
        ofSetColor(ofColor::gold);
    } else {
        ofSetColor(ofColor::white);
    }
    
    ofCircle(ofGetWindowWidth() * 1/6, ofGetWindowHeight() * 1/4, collector.getRoll(1) * 20);
    ofCircle(ofGetWindowWidth() * 1/6, ofGetWindowHeight() * 2/4, collector.getPitch(1) * 20);
    ofCircle(ofGetWindowWidth() * 1/6, ofGetWindowHeight() * 3/4, collector.getYaw(1) * 20);
    
    // PLAYER 2 POSITIONS
    
    if (prevWinner == 2) {
        ofSetColor(ofColor::gold);
    } else {
        ofSetColor(ofColor::white);
    }
    
    ofCircle(ofGetWindowWidth() * 5/6, ofGetWindowHeight() * 1/4, collector.getRoll(2) * 20);
    ofCircle(ofGetWindowWidth() * 5/6, ofGetWindowHeight() * 2/4, collector.getPitch(2) * 20);
    ofCircle(ofGetWindowWidth() * 5/6, ofGetWindowHeight() * 3/4, collector.getYaw(2) * 20);
    
    // OTHER UI
    
    ofSetColor(ofColor::white);
    
    ofDrawBitmapString("GOAL: MATCH THIS POSITION", ofPoint(ofGetWindowWidth()/2-100, 30));
    string timeLeftString = "Time Left = " + ofToString(timeLeft);
    ofDrawBitmapString(timeLeftString, ofPoint(ofGetWindowWidth()/2-60, 70));
    
    if (wins1 > wins2) {
        ofSetColor(ofColor::gold);
    } else {
        ofSetColor(ofColor::white);
    }
    
    ofDrawBitmapString("PLAYER 1", ofPoint(ofGetWindowWidth()*1/6-30, 30));
    string score1String = "SCORE: " + ofToString(wins1);
    ofDrawBitmapString(score1String, ofPoint(ofGetWindowWidth()*1/6-30, 70));
    
    if (wins2 > wins1) {
        ofSetColor(ofColor::gold);
    } else {
        ofSetColor(ofColor::white);
    }
    
    ofDrawBitmapString("PLAYER 2", ofPoint(ofGetWindowWidth()*5/6-30, 30));
    string score2String = "SCORE: " + ofToString(wins2);
    ofDrawBitmapString(score2String, ofPoint(ofGetWindowWidth()*5/6-30, 70));
    
}

void testApp::drawAccelTimeline()
{
    float amplitude = 10.0f;
    float margin = 100.0f;
    float axisMargin = 100.0f;
    
    ofSetColor(ofColor::darkGrey);
    ofLine(0, 0*axisMargin + margin, ofGetWindowWidth(), 0*axisMargin + margin);
    ofLine(0, 1*axisMargin + margin, ofGetWindowWidth(), 1*axisMargin + margin);
    ofLine(0, 2*axisMargin + margin, ofGetWindowWidth(), 2*axisMargin + margin);
    ofSetColor(ofColor::white);
    
    for (int i = 0; i < time-1; i++) {
        
        //        cout << accelXValues[i] << endl;
        
        if (abs(accelXValues[i]) > 0.5) {
            ofSetColor(ofColor::red);
            if (abs(accelXValues[i+1]) - abs(accelXValues[i]) < -0.25f) {
                ofSetColor(ofColor::yellowGreen);
            }
            
        } else {
            ofSetColor(ofColor::white);
        }
        
        // plot x accel values
        ofLine(margin + i, 0*axisMargin + margin + amplitude * accelXValues[i], margin + i+1, 0*axisMargin + margin + amplitude * accelXValues[i+1]);
        
        ofSetColor(ofColor::white);
        
        // plot y accel values
        ofLine(margin + i, 1*axisMargin + margin + amplitude * accelYValues[i], margin + i+1, 1*axisMargin + margin + amplitude * accelYValues[i+1]);
        
        // plot z accel values
        ofLine(margin + i, 2*axisMargin + margin + amplitude * accelZValues[i], margin + i+1, 2*axisMargin + margin + amplitude * accelZValues[i+1]);
    }
}

#pragma mark - Game Logic

void testApp::chooseNewPosition()
{
    goalRoll = ofRandom(-PI, PI);
    goalPitch = ofRandom(-PI, PI);
    goalYaw = ofRandom(-PI, PI);
}

int testApp::determineWinner()
{
    float score1 = abs(goalRoll - collector.getRoll(1)) + abs(goalPitch - collector.getPitch(1)) + abs(goalYaw - collector.getYaw(1));
    float score2 = abs(goalRoll - collector.getRoll(2)) + abs(goalPitch - collector.getPitch(2)) + abs(goalYaw - collector.getYaw(2));
    
    cout << "score1 is " << score1 << " and score2 is " << score2 << endl;
    
    if (score1 < score2) {
        wins1++;
        return 1;
    } else {
        wins2++;
        return 2;
    }
}

//--------------------------------------------------------------
void testApp::exit() {
//	core.exit();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key) {
//	core.keyPressed(key);
    
    if (key == OF_KEY_SHIFT) {
        wins1 = 0;
        wins2 = 0;
    }
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y) {
    //setScaleDegreeBasedOnMouseX();
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button) {}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button) {}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button) {}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h) {}
