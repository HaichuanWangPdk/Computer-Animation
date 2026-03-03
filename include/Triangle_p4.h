#pragma once

#include "Particle.h"

#include "core.h"

class Triangle_p4 {
private:
	Particle* P1;
	Particle* P2;
	Particle* P3;

public:
	Triangle_p4(Particle* p1, Particle* p2, Particle* p3);
	void ComputeNApplyForce(const glm::vec3& vAir);
	glm::vec3 getNormal() const;
	void AddNormals();
};
