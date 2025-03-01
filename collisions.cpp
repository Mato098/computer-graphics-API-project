
#include "collisions.h"

bool isCollidingSphereSphere(glm::vec3 pos1, glm::vec3 pos2, float radius1, float radius2) {
	float distance = glm::distance(pos1, pos2);
	if (distance < radius1 + radius2) {
		return true;
	}
	return false;

}
bool isCollidingSphereBox(glm::vec3 pos1, glm::vec3 pos2, float radius, glm::vec4 bbox) {
	glm::vec3 closestPoint = glm::vec3(0.0f);
	closestPoint.x = glm::clamp(pos1.x, bbox.x, bbox.y);
	closestPoint.y = glm::clamp(pos1.y, bbox.z, bbox.w);
	closestPoint.z = glm::clamp(pos1.z, bbox.x, bbox.y);

	float distance = glm::distance(pos1, closestPoint);
	if (distance < radius) {
		return true;
	}
	return false;
}

//calculate collision of two axis-aligned bounding boxes
bool isCollidingBoxBox(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 bbox1_1, glm::vec3 bbox1_2, glm::vec3 bbox2_1, glm::vec3 bbox2_2) {
	bbox1_1 += pos1;
	bbox1_2 += pos1;
	bbox2_1 += pos2;
	bbox2_2 += pos2;
	glm::vec3 bbox1_3 = glm::vec3(bbox1_1.x, bbox1_1.y, bbox1_2.z);
	glm::vec3 bbox1_4 = glm::vec3(bbox1_1.x, bbox1_2.y, bbox1_1.z);
	glm::vec3 bbox1_5 = glm::vec3(bbox1_1.x, bbox1_2.y, bbox1_2.z);
	glm::vec3 bbox1_6 = glm::vec3(bbox1_2.x, bbox1_1.y, bbox1_1.z);
	glm::vec3 bbox1_7 = glm::vec3(bbox1_2.x, bbox1_1.y, bbox1_2.z);
	glm::vec3 bbox1_8 = glm::vec3(bbox1_2.x, bbox1_2.y, bbox1_1.z);

	return (isPointInsideBBOX(bbox1_1, bbox2_1, bbox2_2) ||
		isPointInsideBBOX(bbox1_2, bbox2_1, bbox2_2) ||
		isPointInsideBBOX(bbox1_3, bbox2_1, bbox2_2) ||
		isPointInsideBBOX(bbox1_4, bbox2_1, bbox2_2) ||
		isPointInsideBBOX(bbox1_5, bbox2_1, bbox2_2) ||
		isPointInsideBBOX(bbox1_6, bbox2_1, bbox2_2) ||
		isPointInsideBBOX(bbox1_7, bbox2_1, bbox2_2) ||
		isPointInsideBBOX(bbox1_8, bbox2_1, bbox2_2));
}

glm::vec3 isCollidingLineBox(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 bbox1_1, glm::vec3 bbox1_2) {
	//spawn points along line and check with the bbox
	glm::vec3 line = pos2 - pos1;
	glm::vec3 step = line / 200.0f;
	glm::vec3 current = pos1;
	for (int i = 0; i < 200; i++) {
		if (current.y < -1.0f) {
			return current;
		}
		if (isPointInsideBBOX(current, bbox1_1, bbox1_2)) {
			return current;
		}
		current += step;
	}
	return glm::vec3(0.0f, 0.0f, 0.0f);
}

bool inline isPointInsideBBOX(glm::vec3 point, glm::vec3 bbox_min, glm::vec3 bbox_max) {
	return (point.x > bbox_min.x && point.x < bbox_max.x &&
		point.y > bbox_min.y && point.y < bbox_max.y &&
		point.z > bbox_min.z && point.z < bbox_max.z);
}

bool isCollidingOnMove(glm::vec3 target_pos, glm::vec3 collider_pos, glm::vec3 BBOX1_min, glm::vec3 BBOX1_max,
	glm::vec3 BBOX2_min, glm::vec3 BBOX2_max) {

	if (isCollidingBoxBox(target_pos, collider_pos, BBOX1_min, BBOX1_max, BBOX2_min, BBOX2_max)) {
		return true;
	}
	return false;
}