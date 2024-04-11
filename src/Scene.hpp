#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Material
{
    glm::vec3 Albedo{1.0f};
    float Roughness = 1.0f;
    // float Metallic = 0.0f;

    glm::vec3 EmissionColor{0.0f};
    float EmissionPower = 0.0f;
    glm::vec3 GetEmission() const { return EmissionColor * EmissionPower; }
};

struct Sphere
{
    glm::vec3 Position{0.0f};
    float Radius = 0.5f;
    int MatIndex = 0;
};

struct Scene
{
    glm::vec3 CameraPos;
    glm::vec3 CameraLookAt;
    float CameraVFOV;

    glm::vec3 SkyColor;
    float SkyIntensity = 1.0f;
    glm::vec3 GetSkyLight() const { return SkyColor * SkyIntensity; }

    std::vector<Sphere> Spheres;
    std::vector<Material> Materials;
    
    bool EnableToneMapping = true;
};

Scene SceneFromFile(const std::string& path);
