#include "particles.h"
#include <algorithm>
#include <GL/gl.h> // or glad/gl.h depending on your setup
#include <cstdlib>
#include <cmath>

ParticleSystem::ParticleSystem(int maxParticles)
    : maxParticles(maxParticles) {}

void ParticleSystem::emit(const glm::vec3& position, const glm::vec3& velocity) {
    if (particles.size() < maxParticles) {
        float angle = static_cast<float>(rand()) / RAND_MAX * 6.28f;
        float speed = 0.5f + static_cast<float>(rand()) / RAND_MAX * 1.5f;
        float zOffset = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.4f;

        glm::vec3 swirlVel = glm::vec3(cos(angle), sin(angle), zOffset) * speed;
        float size = 0.02f + static_cast<float>(rand()) / RAND_MAX * 0.05f;

        particles.push_back({ position, swirlVel, 1.0f, size });
    }
}

void ParticleSystem::update(float deltaTime) {
    for (auto& p : particles) {
        // Spiral motion
        glm::vec3 radial = glm::normalize(glm::vec3(-p.position.y, p.position.x, 0.0f));
        p.velocity += radial * 0.2f * deltaTime;

        p.position += p.velocity * deltaTime;
        p.life -= deltaTime * 0.5f;
    }

    particles.erase(std::remove_if(particles.begin(), particles.end(),
                                   [](const Particle& p) { return p.life <= 0.0f; }),
                    particles.end());
}

void ParticleSystem::render() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Glow layer
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        glColor4f(1.0f, 1.0f, 0.4f, p.life * 0.1f);
        glVertex3f(p.position.x, p.position.y, p.position.z);
    }
    glEnd();

    // Main particle layer
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        float hue = p.life;
        glColor4f(1.0f * hue, 0.8f * hue, 0.2f, p.life);
        glVertex3f(p.position.x, p.position.y, p.position.z);
    }
    glEnd();
}
