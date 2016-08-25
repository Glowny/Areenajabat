#pragma once
#include <glm/glm.hpp>

namespace arena
{
#define PI 3.141592
#define HALFPI 1.570796
	glm::vec2 getShoulderPointWithTorsoRotation(float torsoRotation)
	{
		glm::vec2 shoulderPoint;

		shoulderPoint.x = cos(torsoRotation - HALFPI) * 31.0f;
		shoulderPoint.y = sin(torsoRotation - HALFPI) * 31.0f;

		return shoulderPoint;
	}

	glm::vec2 getShoulderPointWithAimAngle(float aimAngle)
	{
		float angleLimit1 = 0.4f; //used for limiting head and torso rotation
		float angleLimit2 = 2.74f; //used for limiting head and torso rotation, = pi - angleLimit1
		float torsoRotation = 0.5f; // how much the character is leaning forward when aiming forward
		bool direction;

		if (aimAngle < PI / 2 && aimAngle > -PI / 2)
			direction = 1;
		else direction = 0;

		if (direction) {
			if (aimAngle >= -angleLimit1 - torsoRotation && aimAngle < angleLimit1 - torsoRotation) {
				torsoRotation = aimAngle + torsoRotation;
			}
			else if (aimAngle >= angleLimit1 - torsoRotation) {
				torsoRotation = angleLimit1;
			}
			else torsoRotation = -angleLimit1;
		}
		else // Upper body facing left : we must subtract or add pi to all return values for the rotation to work correctly.
		{
			if (aimAngle <= -angleLimit2 + torsoRotation || aimAngle <= PI && aimAngle > angleLimit2 + torsoRotation) {
				torsoRotation = aimAngle - PI - torsoRotation;
			}
			else if (aimAngle < angleLimit2 + torsoRotation && aimAngle >= 1.57f) {
				torsoRotation = angleLimit2 - PI;
			}
			else {
				torsoRotation = -angleLimit2 - PI;
			}
		}

		return getShoulderPointWithTorsoRotation(torsoRotation);
	}
}