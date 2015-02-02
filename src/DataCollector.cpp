//
//  DataCollector.cpp
//  emptyExample
//
//  Created by Benjamin Reynolds on 1/11/15.
//
//

#include "DataCollector.h"
#include <myo/myo.hpp>


void DataCollector::setupDataCollector()
{
    // SETUP emgVals WITH 8 DEFAULT VALUES
    for (int i = 0; i < 8; i++) {
        emgVals.push_back(0.0);
    }
}

// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.

#pragma mark - Event Listeners

void DataCollector::onPair(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion)
{
    // add this myo to the list of known myos
    //knownMyos.push_back(myo);
}

// This is a utility function implemented for this sample that maps a myo::Myo* to a unique ID starting at 1.
// It does so by looking for the Myo pointer in knownMyos, which onPair() adds each Myo into as it is paired.
size_t DataCollector::identifyMyo(myo::Myo* myo) {
    int num = 0;
    
    // Walk through the list of Myo devices that we've seen pairing events for.
    for (size_t i = 0; i < knownMyos.size(); ++i) {
        // If two Myo pointers compare equal, they refer to the same Myo device.
        if (knownMyos[i] == myo) {
            num = i + 1;
        }
    }
    
//    std::cout << "num = " << num << std::endl;
    
    return num;
}

    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
void DataCollector::onUnpair(myo::Myo* myo, uint64_t timestamp)
{
    // remove this myo from the list of known myos
    knownMyos.erase(std::remove(knownMyos.begin(), knownMyos.end(), myo), knownMyos.end());
    
    // We've lost a Myo.
    // Let's clean up some leftover state.
//    roll_w = 0;
//    pitch_w = 0;
//    yaw_w = 0;
    onArm = false;
    isUnlocked = false;
}
    
// onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
// as a unit quaternion.
void DataCollector::onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
{
    using std::atan2;
    using std::asin;
    using std::sqrt;
    using std::max;
    using std::min;
    
    if (identifyMyo(myo) == 1) {

        // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
        roll1 = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                           1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
        pitch1 = asin(max(-1.0f, min(1.0f, 2.0f * (quat.w() * quat.y() - quat.z() * quat.x()))));
        yaw1 = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
                          1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));
    
    } else {
        
        // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
        roll2 = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                      1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
        pitch2 = asin(max(-1.0f, min(1.0f, 2.0f * (quat.w() * quat.y() - quat.z() * quat.x()))));
        yaw2 = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
                     1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));
        
    }
    
}
    
// onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
// making a fist, or not making a fist anymore.
void DataCollector::onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
{
    if (this->getArmForInt(identifyMyo(myo)) == myo::armRight) {

        currentPose = pose;
        
        if (pose != myo::Pose::unknown && pose != myo::Pose::rest) {
            // Tell the Myo to stay unlocked until told otherwise. We do that here so you can hold the poses without the
            // Myo becoming locked.
            myo->unlock(myo::Myo::unlockHold);
            
            // Notify the Myo that the pose has resulted in an action, in this case changing
            // the text on the screen. The Myo will vibrate.
            myo->notifyUserAction();
        } else {
            // Tell the Myo to stay unlocked only for a short period. This allows the Myo to stay unlocked while poses
            // are being performed, but lock after inactivity.
            myo->unlock(myo::Myo::unlockTimed);
        }
    
    }
}

void DataCollector::onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
{
    if (this->getArmForInt(identifyMyo(myo)) == myo::armRight) {

        for (int i = 0; i < 8; i++) {
            emgVals[i] = emg[i];
        }
        
    }
}

// onArmSync() is called whenever Myo has recognized a Sync Gesture after someone has put it on their
// arm. This lets Myo know which arm it's on and which way it's facing.
void DataCollector::onArmSync(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection)
{
    onArm = true;
    //whichArm = arm;
    
    if (identifyMyo(myo) == 2) {
        arm2 = arm;
    } else {
        arm1 = arm;
    }
}

myo::Arm DataCollector::getArmForInt(int myoNumber) {
    if (myoNumber == 1) {
        return arm1;
    } else {
        return arm2;
    }
}
    
// onArmUnsync() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
// it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
// when Myo is moved around on the arm.
void DataCollector::onArmUnsync(myo::Myo* myo, uint64_t timestamp)
{
    onArm = false;
}
    
// onUnlock() is called whenever Myo has become unlocked, and will start delivering pose events.
void DataCollector::onUnlock(myo::Myo* myo, uint64_t timestamp)
{
    isUnlocked = true;
}

// onLock() is called whenever Myo has become locked. No pose events will be sent until the Myo is unlocked again.
void DataCollector::onLock(myo::Myo* myo, uint64_t timestamp)
{
    isUnlocked = false;
}

void DataCollector::onAccelerometerData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3<float> &accel)
{
    if (this->getArmForInt(identifyMyo(myo)) == myo::armRight) {

        accel_x = accel.x();
        accel_y = accel.y();
        accel_z = accel.z();
    
    }
}

#pragma mark - Getters

// Float Roll/Pitch/Yaw on scale of -PI to PI
float DataCollector::getRoll(int playerNumber)
{
    if (playerNumber == 1) {
        return roll1;
    } else {
        return roll2;
    }
}

float DataCollector::getPitch(int playerNumber)
{
    if (playerNumber == 1) {
        return pitch1;
    } else {
        return pitch2;
    }
}

float DataCollector::getYaw(int playerNumber)
{
    if (playerNumber == 1) {
        return yaw1;
    } else {
        return yaw2;
    }
}

std::vector<float> DataCollector::getEmgData()
{
    return emgVals;
}






