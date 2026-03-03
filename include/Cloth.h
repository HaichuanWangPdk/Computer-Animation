#pragma once

#include "core.h"

#include "Triangle_p4.h"

#include "Particle.h"

#include "SpringDamper.h"

#include <vector>

class Cloth {
private:
	std::vector<Particle> particles;
	std::vector<SpringDamper> springDampers;
	std::vector<Triangle_p4> triangles;

	int Width;
	int Height;
	float Spacing;

	glm::vec3 Wind;
	glm::vec3 Gravity;

	float Ground;
	float Elasticity;
	float Friction;

	int GetParticleIndex(int x, int y) const;

	void InitializeParticles();
	void InitializeSprings(float springK, float dampingK);
	void InitializeTriangles();

	void ApplyGravity();
	void ComputeSpringForces();
	void ComputeAerodynamicForces();
	void IntegrateParticles(float deltaTime);
	void HandleGroundCollisions();
	void ComputeSmoothNormals(); 


	

public:
	Cloth();
	Cloth(int width, int height, float spacing, float springK, float dampingK);

	int getWidth() const;
	int getHeight() const;
	float getSpacing() const;
	glm::vec3 getWind() const;
	glm::vec3 getGravity() const;
	float getGround() const;
	float getElasticity() const;
	float getFriction() const;

	void setWind(const glm::vec3& wind);
	void setGravity(const glm::vec3& gravity);
	void setGround(float ground);
	void setElasticity(float elasticity);
	void setFriction(float friction);

	void Update(float deltaTime);
	void Draw(const glm::mat4& viewProjMtx, GLuint shader);

	void ReleaseAllFixed();
	void TranslateFixedRow(const glm::vec3& delta);
	void RotateFixedRow(float angleDegrees, const glm::vec3& axis);



	

};