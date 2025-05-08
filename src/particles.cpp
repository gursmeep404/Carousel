#include "particles.h"
#include<algorithm>
#include <GL/gl.h> // or glad/gl.h depending on your setup

#include <cstdlib>  // for rand()
#include <cmath>    // for sin, cos

void ParticleSystem::emit(const glm::vec3& position, const glm::vec3& velocity) {
    if (particles.size() < maxParticles) {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float speed = 0.5f + static_cast<float>(rand()) / RAND_MAX * 1.0f; // Vary speed
        glm::vec3 swirlVel = glm::vec3(cos(angle), sin(angle), 0.0f) * speed;
        glm::vec3 finalVel = velocity + swirlVel;

        particles.push_back({position, finalVel, 1.0f, 0.05f + static_cast<float>(rand()) / RAND_MAX * 0.1f});
    }
}

void ParticleSystem::update(float deltaTime) {
    for (auto& p : particles) {
        p.position += p.velocity * deltaTime;

        // swirl effect: slight perpendicular adjustment
        float swirlStrength = 0.2f;
        glm::vec3 radial = glm::normalize(glm::vec3(-p.position.y, p.position.x, 0.0f));
        p.velocity += radial * swirlStrength * deltaTime;

        p.life -= deltaTime;
        p.size *= 0.98f; // shrink slightly
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
                                   [](Particle& p) { return p.life <= 0.0f; }),
                    particles.end());
}

ParticleSystem::ParticleSystem(int maxParticles)
    : maxParticles(maxParticles) {}


void ParticleSystem::render() {
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        glColor4f(1.0f, 1.0f - p.life, 0.0f, p.life); // subtle color shift with life
        glPointSize(p.size * 100.0f); // scale up for visibility
        glVertex3f(p.position.x, p.position.y, p.position.z);
    }
    glEnd();
}


