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

//	// the number of libpd ticks per buffer,
//	// used to compute the audio buffer len: tpb * blocksize (always 64)
//	#ifdef TARGET_LINUX_ARM
//		// longer latency for Raspberry PI
//		int ticksPerBuffer = 32; // 32 * 64 = buffer len of 2048
//		int numInputs = 0; // no built in mic
//	#else
//		int ticksPerBuffer = 8; // 8 * 64 = buffer len of 512
//		int numInputs = 1;
//	#endif
//
//	// setup OF sound stream
//	ofSoundStreamSetup(2, numInputs, this, 44100, ofxPd::blockSize()*ticksPerBuffer, 3);
//
//	// setup the app core
//	core.setup(2, numInputs, 44100, ticksPerBuffer);
    
    this->setupMyo();
    
    this->setupAudio();
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
        myo = hub->waitForMyo(10000);
        
        // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
        if (!myo) {
            throw std::runtime_error("Unable to find a Myo!");
        }
        
        // We've found a Myo.
        std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        
        myo->setStreamEmg(myo::Myo::streamEmgEnabled);
        
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
void testApp::setupAudio(){
    ofSoundStreamSetup(2, 0, this, 44100, 256, 4);
    
    /*
     Generators and ControlGenerators both output a steady stream of data.
     Generators output at the sample rate (in this case, 44100 hertz.
     ControlGenerators output at the control rate, which is much lower than the sample rate.
     */
    
    // create a named parameter on the synth which we can set at runtime
    ControlGenerator midiNote = synth.addParameter("midiNumber");
    
    // convert a midi note to a frequency (plugging that parameter into another object)
    ControlGenerator noteFreq =  ControlMidiToFreq().input(midiNote);
    
    // Here's the actual noise-making object
    Generator tone = SawtoothWave().freq( noteFreq );
    
    // Let's put a filter on the tone
    tone = LPF12().input(tone).Q(10).cutoff((noteFreq * 2) + SineWave().freq(3) * 0.5 * noteFreq);
    
    // It's just a steady tone until we modulate the amplitude with an envelope
    ControlGenerator envelopeTrigger = synth.addParameter("trigger");
//    Generator toneWithEnvelope = tone * ADSR().attack(0.01).decay(1.5).sustain(0).release(0).trigger(envelopeTrigger).legato(true);
    Generator toneWithEnvelope = tone * ADSR().attack(0.0).decay(0.1).sustain(0).release(0).trigger(envelopeTrigger).legato(true);

    
//    // let's send the tone through some delay
//    Generator toneWithDelay = StereoDelay(0.5, 0.75).input(toneWithEnvelope).wetLevel(0.1).feedback(0.2);
    
    synth.setOutputGen( toneWithEnvelope ); //toneWithDelay );
}

//--------------------------------------------------------------
void testApp::update() {
//	core.update();
    
    this->updateMyo();
    
    scaleDegree = (roll + PI/2) / PI * NUMBER_OF_KEYS;
//    printf("scaleDegree: %i\n", scaleDegree);
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
        
        //collector.print();
        
        roll = collector.getRoll();
        pitch = collector.getPitch();
        yaw = collector.getYaw();
        
//        printf("roll: %f\n", roll);
//        printf("pitch: %f\n", pitch);
//        printf("yaw: %f\n", yaw);
        
        d_accel_x = collector.getAccelX() - accel_x;
        d_accel_y = collector.getAccelY() - accel_y;
        d_accel_z = collector.getAccelZ() - accel_z;
        
        float dlimit = 1.1f;
        if (abs(d_accel_x) > dlimit || abs(d_accel_y) > dlimit || abs(d_accel_z) > dlimit )
        {
            printf("\ndx: %f\n", d_accel_x);
            printf("dy: %f\n", d_accel_y);
            printf("dz: %f\n", d_accel_z);
            
            this->trigger();
        }
        
        accel_x = collector.getAccelX();
        accel_y = collector.getAccelY();
        accel_z = collector.getAccelZ();
        
        accelXValues.push_back(accel_x);
        accelYValues.push_back(accel_y);
        accelZValues.push_back(accel_z);
        
        float limit = 1.1f;
        if (abs(accel_x) > limit || abs(accel_y) > limit || abs(accel_z) > limit )
        {
            printf("\nx: %f\n", accel_x);
            printf("y: %f\n", accel_y);
            printf("z: %f\n", accel_z);
        }

        emgVals = collector.getEmgData();
        
        //if (emgVals.size() > 0) {
        //printf("len = %i", emgVals.size());
        
        //        printf("emg0: %f\n", emgVals[0]);
        //        printf("emg1: %f\n", emgVals[1]);
        //        printf("emg2: %f\n", emgVals[2]);
        //}
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}

//--------------------------------------------------------------
void testApp::trigger(){
    static int twoOctavePentatonicScale[10] = {0, 2, 4, 7, 9, 12, 14, 16, 19, 21};
    int degreeToTrigger = floor(ofClamp(scaleDegree, 0, 9));
    cout << "degreeToTrigger = " << degreeToTrigger << endl;
    
    // set a parameter that we created when we defined the synth
    synth.setParameter("midiNumber", 30); //44 + twoOctavePentatonicScale[degreeToTrigger]);
    
    // simply setting the value of a parameter causes that parameter to send a "trigger" message to any
    // using them as triggers
    synth.setParameter("trigger", 1);
}

//--------------------------------------------------------------
void testApp::setScaleDegreeBasedOnMouseX(){
    int newScaleDegree = ofGetMouseX() * NUMBER_OF_KEYS / ofGetWindowWidth();
    if(ofGetMousePressed() && ( newScaleDegree != scaleDegree )){
        scaleDegree = newScaleDegree;
        trigger();
    }else{
        scaleDegree = newScaleDegree;
    }
}

//--------------------------------------------------------------
void testApp::draw() {
//	core.draw();
    
    ofSetColor(ofColor::white);
    ofCircle(30, 30, accel_x * 10);
    ofCircle(60, 30, accel_y * 10);
    ofCircle(90, 30, accel_z * 10);
    
    ofCircle(130, 30, roll * 10);
    ofCircle(160, 30, pitch * 10);
    ofCircle(190, 30, yaw * 10);
    
    
    //ofLine(10 + (time-1), 10 + 10 * (accel_x-d_accel_x), 10 + time, 10 + 10 * accel_x);
    
    float amplitude = 10.0f;
    float margin = 100.0f;
    float axisMargin = 100.0f;
    for (int i = 0; i < time-1; i++) {
        // plot x accel values
        ofLine(margin + i, 0*axisMargin + margin + amplitude * accelXValues[i], margin + i+1, 0*axisMargin + margin + amplitude * accelXValues[i+1]);
        
        // plot y accel values
        ofLine(margin + i, 1*axisMargin + margin + amplitude * accelYValues[i], margin + i+1, 1*axisMargin + margin + amplitude * accelYValues[i+1]);

        // plot z accel values
        ofLine(margin + i, 2*axisMargin + margin + amplitude * accelZValues[i], margin + i+1, 2*axisMargin + margin + amplitude * accelZValues[i+1]);

    }
    
}

//--------------------------------------------------------------
void testApp::exit() {
//	core.exit();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key) {
//	core.keyPressed(key);
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

//--------------------------------------------------------------
void testApp::audioReceived(float * input, int bufferSize, int nChannels) {
//	core.audioReceived(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void testApp::audioRequested(float * output, int bufferSize, int nChannels) {
//	core.audioRequested(output, bufferSize, nChannels);
    synth.fillBufferOfFloats(output, bufferSize, nChannels);

}
