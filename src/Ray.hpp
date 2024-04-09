#pragma once

#include "Scene.hpp"

#include <glm/glm.hpp>

struct HitPayload
{
    float HitDistance;
    glm::vec3 HitPosition;
    glm::vec3 WorldNormal;
    uint32_t ObjIndex;
};

class Ray
{
public:
    Ray(glm::vec3 origin, glm::vec3 direction)
        : m_Origin(origin), m_Direction(direction) {}

    glm::vec3 GetOrigin() const { return m_Origin; }
    glm::vec3 GetDirection() const { return m_Direction; }

    void SetOrigin(const glm::vec3& origin) { m_Origin = origin; }
    void SetDirection(const glm::vec3& direction) { m_Direction = direction; }

    void ChangeDirection(const glm::vec3& direction) { m_Direction += direction; }
    void ChangeOrigin(const glm::vec3& origin) { m_Origin += origin; }

    void FlipDirection() { m_Direction = -m_Direction; }

    void Reflect(const glm::vec3& normal)
    {
        m_Direction = glm::reflect(m_Direction, normal);
    }

    void ReflectWithOffset(const glm::vec3& normal, const glm::vec3& offset)
    {
        m_Direction = glm::normalize(glm::reflect(m_Direction, normal) + offset);
    }

    bool IsOnHemisphere(const glm::vec3& normal) const
    {
        return glm::dot(m_Direction, normal) > 0.0f;
    }

    HitPayload Trace(const Scene* scene) const
    {
        int closestSphere = -1;
        float hitDistance = std::numeric_limits<float>::max();
        for (size_t i = 0; i < scene->Spheres.size(); i++)
        {
            const Sphere& sphere = scene->Spheres[i];
            glm::vec3 origin = m_Origin - sphere.Position;

            float a = glm::dot(m_Direction, m_Direction);
            float b = 2.0f * glm::dot(origin, m_Direction);
            float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

            float disc = b * b - 4.0f * a * c;
            if (disc < 0.0f)
                continue;

            float closestT = (-b - glm::sqrt(disc)) / (2.0f * a);
            if (closestT > 0.0f && closestT < hitDistance)
            {
                hitDistance = closestT;
                closestSphere = static_cast<int>(i);
            }
        }
        
        if (closestSphere < 0)
            return Miss();

        return ClosestHit(scene, hitDistance, closestSphere);
    
    }

    HitPayload ClosestHit(const Scene* scene, float hitDistance, int objIndex) const
    {
        HitPayload payload;
        payload.HitDistance = hitDistance;
        payload.ObjIndex = objIndex;

        const Sphere& sphere = scene->Spheres[objIndex];
        glm::vec3 origin = m_Origin - sphere.Position;

        payload.HitPosition = origin + m_Direction * hitDistance;
        payload.WorldNormal = glm::normalize(payload.HitPosition);
        payload.HitPosition += sphere.Position;
        return payload;
    }

    HitPayload Miss() const
    {
        HitPayload payload;
        payload.HitDistance = -1;
        return payload;
    }
private:
    glm::vec3 m_Origin;
    glm::vec3 m_Direction;
};