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
        if (j.is_array())
        {
            vec.x = j.at(0).get<float>();
            vec.y = j.at(1).get<float>();
            vec.z = j.at(2).get<float>();
        }
        else if (j.is_string())
        {
            std::string hexStr = j.get<std::string>();
            if (hexStr.starts_with("#"))
                hexStr = hexStr.substr(1);

            if (hexStr.length() == 3)
                hexStr += hexStr;
            else if (hexStr.length() != 6)
                throw std::invalid_argument("Unsupported color format!");

            uint32_t hex = std::stoul(hexStr, nullptr, 16);
            vec.r = (hex >> 16) & 0xFF;
            vec.g = (hex >> 8) & 0xFF;
            vec.b = hex & 0xFF;
            vec /= 256.0f;
        }
        else
        {
            vec = glm::vec3(j.get<float>());
        }
    }
};
NLOHMANN_JSON_NAMESPACE_END

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Sphere, Position, Radius, MatIndex);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Material, Albedo, Roughness, EmissionColor, EmissionPower);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Scene, CameraPos, CameraLookAt, CameraVFOV, SkyColor, SkyIntensity, Spheres, Materials, EnableToneMapping);

Scene SceneFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Failed to open file: " + path + "!");

    json j = json::parse(file);
    return j.template get<Scene>();
}
