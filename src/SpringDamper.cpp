#include "SpringDamper.h"

#include "core.h"

SpringDamper::SpringDamper(float sprCst, float dmpCst, Particle* p1, Particle* p2) {
	SpringConstant = sprCst;
	DampingConstant = dmpCst;
	P1 = p1;
	P2 = p2;
	RestLength = glm::length(P2->getPosition() - P1->getPosition());
}
SpringDamper::SpringDamper(float sprCst, float dmpCst, float rstLth, Particle* p1, Particle* p2) {
	SpringConstant = sprCst;
	DampingConstant = dmpCst;
	RestLength = rstLth;
	P1 = p1;
	P2 = p2;
}

void SpringDamper::ComputeNApplyForce() {
	glm::vec3 e = P2->getPosition() - P1->getPosition();
	float l = glm::length(e);
	if (l < 0.0001f) return;
	e = glm::normalize(e);

	float vClose = glm::dot((P1->getVelocity() - P2->getVelocity()), e);
	float force = -SpringConstant * (RestLength - l) - DampingConstant * vClose;
	
	P1->ApplyForce(force * e);
	P2->ApplyForce(-force * e);
}

float SpringDamper::getSpringConstant() const{
	return SpringConstant;
}

float SpringDamper::getDampingConstant() const{
	return DampingConstant;
}

float SpringDamper::getRestLength() const{
	return RestLength;
}

void SpringDamper::setSpringConstant(float sprCst) {
	SpringConstant = sprCst;
}

void SpringDamper::setDampingConstant(float dmpCst) {
	DampingConstant = dmpCst;
}

void SpringDamper::setRestLength(float rstLth) {
	RestLength = rstLth;
}