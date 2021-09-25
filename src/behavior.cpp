/*
 * Copyright (C) 2018 Ola Benderius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "behavior.hpp"
#include <math.h>
#include <iostream>

Behavior::Behavior() noexcept:
  m_frontUltrasonicReading{},
  m_rearUltrasonicReading{},
  m_leftIrReading{},
  m_rightIrReading{},
  m_groundSteeringAngleRequest{},
  m_pedalPositionRequest{},
  m_aimAngleReading{},
  m_kiwiDetect{},
  m_intersectionDetect{},
  m_frontUltrasonicReadingMutex{},
  m_rearUltrasonicReadingMutex{},
  m_leftIrReadingMutex{},
  m_rightIrReadingMutex{},
  m_groundSteeringAngleRequestMutex{},
  m_pedalPositionRequestMutex{},
  m_aimAngleReadingMutex{},
  m_kiwiDetectMutex{},
  m_intersectionDetectMutex{}
{
}

opendlv::proxy::GroundSteeringRequest Behavior::getGroundSteeringAngle() noexcept
{
  std::lock_guard<std::mutex> lock(m_groundSteeringAngleRequestMutex);
  return m_groundSteeringAngleRequest;
}

opendlv::proxy::PedalPositionRequest Behavior::getPedalPositionRequest() noexcept
{
  std::lock_guard<std::mutex> lock(m_pedalPositionRequestMutex);
  return m_pedalPositionRequest;
}

void Behavior::setFrontUltrasonic(opendlv::proxy::DistanceReading const &frontUltrasonicReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_frontUltrasonicReadingMutex);
  m_frontUltrasonicReading = frontUltrasonicReading;
}

void Behavior::setRearUltrasonic(opendlv::proxy::DistanceReading const &rearUltrasonicReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_rearUltrasonicReadingMutex);
  m_rearUltrasonicReading = rearUltrasonicReading;
}

void Behavior::setLeftIr(opendlv::proxy::VoltageReading const &leftIrReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_leftIrReadingMutex);
  m_leftIrReading = leftIrReading;
}

void Behavior::setRightIr(opendlv::proxy::VoltageReading const &rightIrReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_rightIrReadingMutex);
  m_rightIrReading = rightIrReading;
}


// Aim angle is recieved from the other microservice here
void Behavior::setAimAngle(opendlv::proxy::AngleReading const &aimAngleReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_aimAngleReadingMutex);
  m_aimAngleReading = aimAngleReading;
}

// Kiwi detection recieved from Multicast
void Behavior::setKiwiDetect(opendlv::logic::perception::KiwiDetection const &kiwiDetect) noexcept
{
  std::lock_guard<std::mutex> lock(m_kiwiDetectMutex);
  m_kiwiDetect = kiwiDetect;
}

void Behavior::setIntersectionDetect(opendlv::proxy::SwitchStateRequest const &intersectionDetect) noexcept
{
  std::lock_guard<std::mutex> lock(m_intersectionDetectMutex);
  m_intersectionDetect = intersectionDetect;
}

void Behavior::step() noexcept
{
  opendlv::proxy::DistanceReading frontUltrasonicReading;
  opendlv::proxy::DistanceReading rearUltrasonicReading;
  opendlv::proxy::VoltageReading leftIrReading;
  opendlv::proxy::VoltageReading rightIrReading;
  opendlv::proxy::AngleReading aimAngleReading;
  opendlv::logic::perception::KiwiDetection kiwiDetect;
  opendlv::proxy::SwitchStateRequest intersectionDetect;
  {
    std::lock_guard<std::mutex> lock1(m_frontUltrasonicReadingMutex);
    std::lock_guard<std::mutex> lock2(m_rearUltrasonicReadingMutex);
    std::lock_guard<std::mutex> lock3(m_leftIrReadingMutex);
    std::lock_guard<std::mutex> lock4(m_rightIrReadingMutex);
    std::lock_guard<std::mutex> lock5(m_aimAngleReadingMutex);
    std::lock_guard<std::mutex> lock6(m_kiwiDetectMutex);
    std::lock_guard<std::mutex> lock7(m_intersectionDetectMutex);

    frontUltrasonicReading = m_frontUltrasonicReading;
    rearUltrasonicReading = m_rearUltrasonicReading;
    leftIrReading = m_leftIrReading;
    rightIrReading = m_rightIrReading;
    aimAngleReading = m_aimAngleReading;
    kiwiDetect = m_kiwiDetect;
    intersectionDetect = m_intersectionDetect;
  }

  float frontDistance = frontUltrasonicReading.distance();
  float rearDistance = rearUltrasonicReading.distance();
  double leftDistance = convertIrVoltageToDistance(leftIrReading.voltage());
  double rightDistance = convertIrVoltageToDistance(rightIrReading.voltage());
  
  
  // Aim angle
  float aimAngle = aimAngleReading.angle();


//////////////////////////////// Task 1 ///////////////////////////////////

  // The ground steering angle is based on the aim angle
  float groundSteeringAngle = 0.1f*(aimAngle);
  
  // Set pedal position
  float pedalPosition = 0.15f;
  //std::cout<<groundSteeringAngle<<std::endl;
  
  // If in a curve then reduce the speed to avoid the car going out of track
  if (fabs(groundSteeringAngle)>0.06){
     pedalPosition = 0.09f;
  }
  
  
//////////////////////////////// Task 2 ///////////////////////////////////

  float kiwiCenterX = kiwiDetect.xCenter();
  float kiwiCenterY = kiwiDetect.yCenter();
  float kiwiWidth = kiwiDetect.width();
  float kiwiHeight = kiwiDetect.height();
  
  
  // The distance to kiwi is inversly proportional to the pixels it covers on the image
  
  // Overall focal length is calulated using the following but to enable easy detection an amplification factor of 2 has been considered.
  
  // https://www.scantips.com/lights/subjectdistance.html
  float KiwiRealHeight = 0.101f;
  int overallfocalLength = 1632;
  float Distance2Kiwi = KiwiRealHeight*overallfocalLength/kiwiHeight;
  
  // If there is a kiwi present we check the distance to the kiwi to determine the pedal position
  if (kiwiHeight>0){
     // If the distance to the kiwi in front is very low then stop
     if(Distance2Kiwi < 0.6f || frontDistance < 0.25f){
        pedalPosition = 0.0f;
        // std::cout<<"STOP!!! Danger ahead"<<std::endl;
     }
     // Else brake proportional to the distance infront
     else if(Distance2Kiwi < 1.1f){
        pedalPosition = pedalPosition*Distance2Kiwi/1.5f;
        // std::cout<<"Slow Down!!! Speed =  "<<pedalPosition<<std::endl;
     }
  }
  
//////////////////////////////// Task 3 ///////////////////////////////////
  
  // Intersection
  int intersection_located = intersectionDetect.state();
  
  if(kiwiWidth>0 && kiwiCenterX > 425 && intersection_located !=0)
  {
     pedalPosition = 0.0f;
     std::cout<<"STOP INTERSECTION"<<std::endl;
  }
  
  (void) rearDistance;
  (void) leftDistance;
  (void) rightDistance;
  (void) kiwiCenterY;


  {
    std::lock_guard<std::mutex> lock1(m_groundSteeringAngleRequestMutex);
    std::lock_guard<std::mutex> lock2(m_pedalPositionRequestMutex);

    opendlv::proxy::GroundSteeringRequest groundSteeringAngleRequest;
    groundSteeringAngleRequest.groundSteering(groundSteeringAngle);
    m_groundSteeringAngleRequest = groundSteeringAngleRequest;

    opendlv::proxy::PedalPositionRequest pedalPositionRequest;
    pedalPositionRequest.position(pedalPosition);
    m_pedalPositionRequest = pedalPositionRequest;
  }
}

// TODO: This is a rough estimate, improve by looking into the sensor specifications.
double Behavior::convertIrVoltageToDistance(float voltage) const noexcept
{
  double voltageDividerR1 = 1000.0;
  double voltageDividerR2 = 1000.0;

  double sensorVoltage = (voltageDividerR1 + voltageDividerR2) / voltageDividerR2 * voltage;
  double distance = (2.5 - sensorVoltage) / 0.07;
  return distance;
}
