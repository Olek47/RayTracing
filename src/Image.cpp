#include "Image.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

constexpr int PNG_CHANNEL_COUNT = 3;

Image::Image(uint32_t w, uint32_t h)
    : m_Width(w), m_Height(h)
{
    m_Arr.resize(w * h, glm::vec3(0.0f));
}

void Image::SaveToFile(const std::string& path) const
{
    uint8_t* data = new uint8_t[m_Arr.size() * PNG_CHANNEL_COUNT];

    uint8_t* dataPtr = data;
    for (const auto& pixel : m_Arr)
    {
        dataPtr[0] = static_cast<uint8_t>(pixel.r * 255.0f);
        dataPtr[1] = static_cast<uint8_t>(pixel.g * 255.0f);
        dataPtr[2] = static_cast<uint8_t>(pixel.b * 255.0f);
        dataPtr += 3;
    }

    std::cout << "Saving to " << path << "..." << std::endl;
    stbi_write_png(
        path.c_str(),
        m_Width, m_Height, PNG_CHANNEL_COUNT,
        data,
        m_Width * PNG_CHANNEL_COUNT
    );

    delete[] data;
}