#include "Scene.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

NLOHMANN_JSON_NAMESPACE_BEGIN
template<>
struct adl_serializer<glm::vec3>
{
    static void to_json(json& j, const glm::vec3& vec)
    {
        j = { vec.x, vec.y, vec.z };
    }

    static void from_json(const json& j, glm::vec3& vec)
    {
        vec.x = j.at(0).get<float>();
        vec.y = j.at(1).get<float>();
        vec.z = j.at(2).get<float>();
    }
};
NLOHMANN_JSON_NAMESPACE_END

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Sphere, Position, Radius, MatIndex);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Material, Albedo, Roughness, EmissionColor, EmissionPower);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Scene, CameraPos, CameraLookAt, CameraVFOV, SkyColor, Spheres, Materials, EnableToneMapping);

Scene SceneFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Failed to open file: " + path + "!");

    json j = json::parse(file);
    return j.template get<Scene>();
}
