#include "Random.hpp"

namespace Random
{
    thread_local std::mt19937 engine;
    std::uniform_int_distribution<std::mt19937::result_type> distribution;
}