#include "Triangle_p4.h"

#include "Particle.h"

const float drag = 1.28f;
const float fDensity = 1.225f;

Triangle_p4::Triangle_p4(Particle* p1, Particle* p2, Particle* p3) {
	P1 = p1;
	P2 = p2;
	P3 = p3;
}


void Triangle_p4::ComputeNApplyForce(const glm::vec3& vAir) {
	glm::vec3 vSurface = (P1->getVelocity() + P2->getVelocity() + P3->getVelocity()) / 3.0f;
	glm::vec3 v = vSurface - vAir;
	float vLen = glm::length(v);
	if (vLen < 0.0001f) return;

	glm::vec3 edge1 = P2->getPosition() - P1->getPosition();
	glm::vec3 edge2 = P3->getPosition() - P1->getPosition();
	glm::vec3 crossProd = glm::cross(edge1, edge2);
	float area = glm::length(crossProd);
	if (area < 0.0001f) return;
	glm::vec3 n = glm::normalize(crossProd);

	float cSecArea = 0.5 * area * glm::dot(glm::normalize(v), n);
	float K = -0.5 * fDensity * vLen * vLen * drag * cSecArea;
	glm::vec3 f = K * n;
	P1->ApplyForce(f / 3.0f);
	P2->ApplyForce(f / 3.0f);
	P3->ApplyForce(f / 3.0f);
}

glm::vec3 Triangle_p4::getNormal() const{
	glm::vec3 edge1 = P2->getPosition() - P1->getPosition();
	glm::vec3 edge2 = P3->getPosition() - P1->getPosition();
	glm::vec3 crossProd = glm::cross(edge1, edge2);
	float len = glm::length(crossProd);

	if (len < 0.0001f) {
		return glm::vec3(0.0f, 1.0f, 0.0f);
	}

	return crossProd / len;
}

void Triangle_p4::AddNormals(){
	glm::vec3 n = getNormal();
	P1->AddNormal(n);
	P2->AddNormal(n);
	P3->AddNormal(n);
}