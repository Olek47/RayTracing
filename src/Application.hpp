#pragma once

#include "Image.hpp"
#include "Scene.hpp"

#include <memory>

struct AppSettings
{
    uint32_t Width = 256;
    uint32_t Height = 256;
    std::string OutputPath = "render.png";
    std::string ScenePath = "";

    uint32_t Samples = 16;
    uint32_t Bounces = 5;
    uint32_t ThreadCount = 4;
};

class Application
{
public:
    explicit Application(const AppSettings&);
    void SetScene(const Scene*);

    void Render();
private:
    std::unique_ptr<Image> m_Image;
    AppSettings m_Settings;
    const Scene* m_Scene;

    glm::mat4 m_Projection;
    glm::mat4 m_InverseProjection;
    glm::mat4 m_CameraView;
    glm::mat4 m_InverseCameraView;
    
    void CalculateProjection();
    void CalculateRayDirections(std::vector<glm::vec3>&);

    void BuildSamples();
    void PostProcess();

    glm::vec3 RayGen(uint32_t, uint32_t, const std::vector<glm::vec3>&) const;
};
