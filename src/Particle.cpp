#include "Particle.h"

Particle::Particle() {
	Position = glm::vec3(0.0f);
	Velocity = glm::vec3(0.0f);
	Mass = 1.0f;
	Force = glm::vec3(0.0f);
	Fixed = false;
	Normal = glm::vec3(0.0f, 1.0f, 0.0f);
}

Particle::Particle(const glm::vec3& pos, float mass) {
	Position = pos;
	Velocity = glm::vec3(0.0f);
	Mass = mass;
	Force = glm::vec3(0.0f);
	Fixed = false;
	Normal = glm::vec3(0.0f, 1.0f, 0.0f);
}

Particle::Particle(const glm::vec3& pos, const glm::vec3& vel, float mass, bool fixed) {
	Position = pos;
	Velocity = vel;
	Mass = mass;
	Force = glm::vec3(0.0f);
	Fixed = fixed;
	Normal = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Particle::ApplyForce(const glm::vec3& f) {
	if (!isFixed()) {
		Force += f;
	}
}

void Particle::Integrate(float deltaTime) {
	if (!isFixed()) {
		glm::vec3 accel = Force / Mass;
		Velocity += accel * deltaTime;
		Position += Velocity * deltaTime;
		Force = glm::vec3(0.0f);
	}
}

void Particle::ApplyImpulse(const glm::vec3& imp) {
	if (!isFixed()) {  
		Velocity += imp / Mass;
	}
}

glm::vec3 Particle::getPosition() const{
	return Position;
}

glm::vec3 Particle::getVelocity() const{
	return Velocity;
}

float Particle::getMass() const{
	return Mass;
}

bool Particle::isFixed() const {
	return Fixed;
}

glm::vec3 Particle::getNormal() const {
	return Normal;
}

void Particle::setPosition(const glm::vec3& pos) {
	Position = pos;
}

void Particle::setVelocity(const glm::vec3& vel) {
	Velocity = vel;
}

void Particle::setMass(float mass) {
	if (mass > 0.0f) {
		Mass = mass;
	}else {
		Mass = 1.0f;
	}
}

void Particle::setFixed(bool fixed) {
	Fixed = fixed;
}

void Particle::setNormal(const glm::vec3& normal) {
	Normal = normal;
}

void Particle::ZeroNormal() {
	Normal = glm::vec3(0.0f);
}

void Particle::AddNormal(const glm::vec3& n) {
	Normal += n;
}

void Particle::NormalizeNormal() {
	float len = glm::length(Normal);
	if (len > 0.0001f) {
		Normal = glm::normalize(Normal);
	}
	else {
		Normal = glm::vec3(0.0f, 1.0f, 0.0f);
	}
}