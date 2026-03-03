#pragma once

#include "Particle.h"

class SpringDamper {
private:
	float SpringConstant;
	float DampingConstant;
	float RestLength;
	Particle* P1;
	Particle* P2;

public:
	SpringDamper(float sprCst, float dmpCst, Particle* p1, Particle* p2);
	SpringDamper(float sprCst, float dmpCst, float rstLth, Particle* p1, Particle* p2);
	void ComputeNApplyForce();
	float getSpringConstant() const;
	float getDampingConstant()const;
	float getRestLength() const;
	void setSpringConstant(float sprCst);
	void setDampingConstant(float dmpCst);
	void setRestLength(float rstLth);


};
