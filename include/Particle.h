#pragma once

#include "core.h"

class Particle {
private:
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec3 Force;
	float Mass;
	bool Fixed;
	glm::vec3 Normal;

public:
	Particle();
	Particle(const glm::vec3& pos, float mass);
	Particle(const glm::vec3& pos, const glm::vec3& vel, float mass, bool fixed);

	void ApplyForce(const glm::vec3& f);
	void Integrate(float deltaTime);
	void ApplyImpulse(const glm::vec3& imp);

	glm::vec3 getPosition() const;
	glm::vec3 getVelocity() const;
	float getMass() const;
	bool isFixed() const;
	glm::vec3 getNormal() const;

	void setPosition(const glm::vec3& pos);
	void setVelocity(const glm::vec3& vel);
	void setMass(float mass);
	void setFixed(bool fixed);
	void setNormal(const glm::vec3& normal);

	void ZeroNormal();
	void AddNormal(const glm::vec3& normal);
	void NormalizeNormal();

};