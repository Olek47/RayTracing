#include "Application.hpp"
#include "Ray.hpp"
#include "Timer.hpp"
#include "Random.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <iomanip>
#include <future>
#include <thread>

Application::Application(const AppSettings& settings)
    : m_Settings(settings)
{
    m_Image = std::make_unique<Image>(m_Settings.Width, m_Settings.Height);
}

void Application::CalculateProjection()
{
    m_Projection = glm::perspectiveFov(
        glm::radians(m_Scene->CameraVFOV),
        static_cast<float>(m_Image->GetWidth()),
        static_cast<float>(m_Image->GetHeight()),
        0.001f,
        1000.0f
    );

    m_CameraView = glm::lookAt(m_Scene->CameraPos, m_Scene->CameraLookAt, glm::vec3(0, 1, 0));

    m_InverseProjection = glm::inverse(m_Projection);
    m_InverseCameraView = glm::inverse(m_CameraView);
}

void Application::CalculateRayDirections(std::vector<glm::vec3>& rayDirections)
{
    float pxOffsetX = Random::Float();
    float pxOffsetY = Random::Float();

    for (uint32_t y = 0; y < m_Image->GetHeight(); y++)
    {
        for (uint32_t x = 0; x < m_Image->GetWidth(); x++)
        {
            glm::vec2 coord = {
                (static_cast<float>(x) + pxOffsetX) / m_Image->GetWidth(),
                (static_cast<float>(m_Image->GetHeight() - y) + pxOffsetY) / m_Image->GetHeight()
            };

            coord = coord * 2.0f - 1.0f;

            const glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
            const glm::vec3 direction = m_InverseCameraView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0);

            rayDirections[y * m_Image->GetWidth() + x] = direction;
        }
    }
}

void Application::SetScene(const Scene* scene)
{
    m_Scene = scene;
}

void Application::Render()
{
    if (!m_Scene)
        throw std::runtime_error("Cannot render without scene set!");

    CalculateProjection();

    Timer totalTimer;
    BuildSamples();
    PostProcess();
    std::cout << "Everything took " << totalTimer.Elapsed() << "ms!" << std::endl;

    m_Image->SaveToFile(m_Settings.OutputPath);
}

void Application::BuildSamples()
{
    uint32_t samplesPerThread = m_Settings.Samples / m_Settings.ThreadCount;

    // Do not waste threads when sample count is lower than thread count
    if (samplesPerThread <= 0)
    {
        samplesPerThread = 1;
        m_Settings.ThreadCount = m_Settings.Samples;
    }

    // Predicted memory usage in MiB
    // 2 image sized vectors per thread + m_Image
    const uint32_t memUsage = (2 * m_Settings.ThreadCount + 1) * m_Image->GetSize() * sizeof(glm::vec3) / 1024 / 1024;

    // Array storing every thread task
    std::vector<std::future<std::vector<glm::vec3>>> threads;
    threads.reserve(m_Settings.ThreadCount);

    struct ThreadStatus
    {
        uint32_t completedSamples = 0;
        uint32_t sampleCount;
    };

    std::vector<ThreadStatus> threadStatus;
    threadStatus.resize(m_Settings.ThreadCount);

    // Build and launch every thread
    for (uint32_t th = 0; th < m_Settings.ThreadCount; th++)
    {
        ThreadStatus* statusPtr = &threadStatus[th];
        statusPtr->sampleCount = samplesPerThread + (th < m_Settings.Samples % m_Settings.ThreadCount);

        // Thread body
        auto threadFn = [this, statusPtr]()
        {
            Random::Init();

            std::vector<glm::vec3> arr;
            arr.resize(m_Image->GetSize(), glm::vec3(0.0f));

            std::vector<glm::vec3> rayDirections;
            rayDirections.resize(m_Image->GetSize());

            for (uint32_t s = 0; s < statusPtr->sampleCount; s++)
            {
                CalculateRayDirections(rayDirections);
                for (uint32_t y = 0; y < m_Image->GetHeight(); y++)
                {
                    for (uint32_t x = 0; x < m_Image->GetWidth(); x++)
                    {
                        arr[y * m_Image->GetWidth() + x] += RayGen(x, y, rayDirections);
                    }
                }
                statusPtr->completedSamples++;
            }

            return arr;
        };

        threads.emplace_back(std::async(std::launch::async, threadFn));
    }

    // Show render progress
    std::cout << "\x1b[H\x1b[J\x1b[0m";
    bool done = false;
    while (!done)
    {
        std::cout << "\x1b[H";
        std::cout << m_Settings.Width << "x" << m_Settings.Height << " "<< m_Settings.Samples << " samples " << memUsage << "MiB required\n";
        done = true;
        for (uint32_t i = 0; i < m_Settings.ThreadCount; i++)
        {
            uint32_t sampleCount = threadStatus[i].sampleCount;
            uint32_t completedSamples = threadStatus[i].completedSamples;
            if (completedSamples < sampleCount)
                done = false;
            
            float progress = static_cast<float>(completedSamples) / sampleCount;
            std::string buff;
            const uint32_t STATUS_WIDTH = 32;
            for (uint32_t x = 1; x <= STATUS_WIDTH; x++)
                buff.push_back(static_cast<float>(x) / STATUS_WIDTH <= progress ? '#' : ' ');

            std::cout << "Thread " << std::setw(2) << i + 1 << ": [\x1b[32m" << buff << "\x1b[0m] "
                << completedSamples << "/" << sampleCount << "\n";
            
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Merge all thread results into one single image
    std::vector<glm::vec3>& rawImage = m_Image->GetRawArr();
    for (auto& th : threads)
    {
        std::vector<glm::vec3> arr = th.get();
        std::vector<glm::vec3>::iterator imagePtr = rawImage.begin();
        for (const auto& px : arr)
        {
            *imagePtr += px / glm::vec3(m_Settings.Samples);
            imagePtr++;
        }
    }
}

void Application::PostProcess()
{
    std::cout << "Performing post process pass... " << std::flush;
    Timer postProcessTimer;
    for (uint32_t y = 0; y < m_Image->GetHeight(); y++)
    {
        for (uint32_t x = 0; x < m_Image->GetWidth(); x++)
        {
            glm::vec3 color = m_Image->Get(x, y);

            // ACES Tone mapping
            // https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
            auto tonemapACES = [](glm::vec3 x) -> glm::vec3
            {
                float a = 2.51f;
                float b = 0.03f;
                float c = 2.43f;
                float d = 0.59f;
                float e = 0.14f;
                return (x * (a * x + b)) / (x * (c * x + d) + e);
            };

            if (m_Scene->EnableToneMapping)
                color = tonemapACES(color);
                
            color = glm::clamp(color, 0.0f, 1.0f);

            // Output modified color
            m_Image->Set(x, y, color);
        }
    }
    std::cout << postProcessTimer.Elapsed() << "ms" << std::endl;
}

glm::vec3 Application::RayGen(uint32_t x, uint32_t y, const std::vector<glm::vec3>& rayDirections) const
{
    Ray ray(m_Scene->CameraPos, rayDirections[y * m_Image->GetWidth() + x]);

    glm::vec3 light(0.0f);
    glm::vec3 throughput(1.0f);

    for (uint32_t i = 0; i <= m_Settings.Bounces; i++)
    {
        HitPayload payload = ray.Trace(m_Scene);
        if (payload.HitDistance < 0) {
            light += m_Scene->GetSkyLight() * throughput;
            break;
        }

        const Sphere& sphere = m_Scene->Spheres[payload.ObjIndex];
        const Material& material = m_Scene->Materials[sphere.MatIndex];
        
        light += material.GetEmission() * throughput;
        throughput *= material.Albedo;

        ray.SetOrigin(payload.HitPosition + payload.WorldNormal * 0.0001f);
        ray.ReflectWithOffset(payload.WorldNormal, material.Roughness * Random::UnitSphere());
        if (!ray.IsOnHemisphere(payload.WorldNormal))
            ray.FlipDirection();
    }

    return light;
}
