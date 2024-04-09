#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <random>

namespace Random
{
    extern thread_local std::mt19937 engine;
    extern std::uniform_int_distribution<std::mt19937::result_type> distribution;

    inline void Init()
    {
        engine.seed(std::random_device()());
    }

    inline uint32_t UInt32()
    {
        return distribution(engine);
    }

    inline uint32_t UInt32(uint32_t min, uint32_t max)
    {
        return min + (distribution(engine) % (max - min + 1));
    }

    inline float Float()
    {
        return static_cast<float>(distribution(engine)) / static_cast<float>(std::numeric_limits<std::mt19937::result_type>::max());
    }

    inline float Float(float min, float max)
    {
        return min + static_cast<float>(distribution(engine)) / static_cast<float>(std::numeric_limits<std::mt19937::result_type>::max() / (max - min));
    }

    inline glm::vec3 UnitSphere()
    {
        // Faster but not uniformly distributed
        // return glm::normalize(glm::vec3(Float(-1.0f, 1.0f), Float(-1.0f, 1.0f), Float(-1.0f, 1.0f)));

        float theta = Float(0.0f, glm::two_pi<float>());
		float phi = std::acos(Float(-1.0f, 1.0f));

		float x = std::sin(phi) * std::cos(theta);
		float y = std::sin(phi) * std::sin(theta);
		float z = std::cos(phi);

        return glm::vec3(x, y, z);
    }
}