#include "Application.hpp"
#include "Random.hpp"

#include <iostream>

std::string_view GetOption(const std::vector<std::string_view>& args,
    const std::string_view& option,
    const std::string_view& shortOption)
{
    for (auto it = args.begin(), end = args.end(); it != end; it++)
        if ((*it == option || *it == shortOption) && it + 1 != end)
            return *(it + 1);
    return "";
}

#define CMDLINE_UINT32_ARG(name, shortName, destination) \
    option = GetOption(args, name, shortName); \
    if (!option.empty()) destination = std::stoul(std::string(option));

#define CMDLINE_STRING_ARG(name, shortName, destination) \
    option = GetOption(args, name, shortName); \
    if (!option.empty()) destination = std::string(option);

AppSettings ParseCommandLine(int argc, char* argv[])
{
    if (argc > 64)
        throw std::runtime_error("Too many input parameters!");

    AppSettings out;
    std::vector<std::string_view> args(argv + 1, argv + argc);
    std::string_view option;

    CMDLINE_UINT32_ARG("--width", "-w", out.Width);
    CMDLINE_UINT32_ARG("--height", "-h", out.Height);
    CMDLINE_UINT32_ARG("--samples", "-s", out.Samples);
    CMDLINE_UINT32_ARG("--bounces", "-b", out.Bounces);
    CMDLINE_UINT32_ARG("--threads", "-t", out.ThreadCount);

    CMDLINE_STRING_ARG("--input", "-i", out.ScenePath);
    CMDLINE_STRING_ARG("--out", "-o", out.OutputPath);

    if (out.ScenePath.empty())
        throw std::runtime_error("Input parameter is required!");

    return out;
}

int main(int argc, char* argv[])
{
    Random::Init();

    try 
    {
        AppSettings settings = ParseCommandLine(argc, argv);
        Application app(settings);
        Scene scene = SceneFromFile(settings.ScenePath);
        app.SetScene(&scene);
        app.Render();
    } 
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
