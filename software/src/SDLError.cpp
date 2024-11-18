/**
 * @file
 * @author Chen Chu
 * @date 2024/11/13
 */
#include <SDLError.hpp>

#include <fmt/format.h>

using namespace std;

const SDLErrorCategory& SDLErrorCategory::GetInstance() noexcept
{
    static const SDLErrorCategory kInstance;
    return kInstance;
}

const char* SDLErrorCategory::name() const noexcept
{
    return "SDLError";
}

std::string SDLErrorCategory::message(int ev) const
{
    return fmt::format("SDL error: {}", ev);
}
