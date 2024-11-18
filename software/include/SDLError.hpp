/**
 * @file
 * @author chu
 * @date 2024/11/13
 */
#pragma once
#include <system_error>

/**
 * SDL 错误分类
 */
class SDLErrorCategory : public std::error_category
{
public:
    static const SDLErrorCategory& GetInstance() noexcept;

public:
    const char* name() const noexcept override;
    std::string message(int ev) const override;
};

inline std::error_code MakeSDLError(int ev) noexcept
{
    return { ev, SDLErrorCategory::GetInstance() };
}
