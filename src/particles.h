#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;
    float size;
};

class ParticleSystem {
public:
    ParticleSystem(int maxParticles);

    void emit(const glm::vec3& position, const glm::vec3& velocity);
    void update(float deltaTime);
    void render();
    
private:
    std::vector<Particle> particles;
    int maxParticles;
};

