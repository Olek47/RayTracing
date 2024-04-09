#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

class Image
{
public:
    Image(uint32_t, uint32_t);

    void SaveToFile(const std::string&) const;

    glm::vec3 Get(uint32_t x, uint32_t y) const { return m_Arr[y * m_Width + x]; };
    void Set(uint32_t x, uint32_t y, glm::vec3 c) { m_Arr[y * m_Width + x] = c; }
    void Fill(glm::vec3 c) { std::fill(m_Arr.begin(), m_Arr.end(), c); }

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    size_t GetSize() const { return m_Arr.size(); }
    std::vector<glm::vec3>& GetRawArr() { return m_Arr; }
private:
    uint32_t m_Width;
    uint32_t m_Height;
    std::vector<glm::vec3> m_Arr;
};