#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl/filesystem.h>

#include <filesystem>
#include <string>
#include <utility>

//! A file path that Python hands over, and receives, as a `pathlib.Path`. A string is
//! refused, so a path stays a path on the Python side and is never mistaken for other text.
struct PathOnly
{
    std::filesystem::path value;
};

namespace pybind11::detail
{
    template<>
    struct type_caster<PathOnly>
    {
        PYBIND11_TYPE_CASTER(PathOnly, const_name("pathlib.Path"));

        bool load(const handle source, bool)
        {
            if(!isinstance(source, module_::import("pathlib").attr("PurePath")))
            {
                return false;
            }
            value.value = source.cast<std::filesystem::path>();
            return true;
        }

        static handle cast(const PathOnly & source, const return_value_policy policy, const handle parent)
        {
            return type_caster<std::filesystem::path>::cast(source.value, policy, parent);
        }
    };
}   // namespace pybind11::detail

namespace PathBinding
{
    //! The path as the UTF-8 text the library's file functions take.
    inline std::string utf8Of(const std::filesystem::path & path)
    {
        const auto text{path.u8string()};
        return {text.begin(), text.end()};
    }

    //! The path named by UTF-8 text.
    inline std::filesystem::path pathOf(const std::string & utf8)
    {
        return std::filesystem::path{std::u8string{utf8.begin(), utf8.end()}};
    }

    //! Binds `function(path, rest...)`, whose path is text in C++, so that Python gives the
    //! path as a `pathlib.Path`.
    template<typename Result, typename PathText, typename... Rest>
    auto pathFirst(Result (*function)(PathText, Rest...))
    {
        return [function](const PathOnly & path, Rest... rest) -> Result {
            return function(utf8Of(path.value), std::forward<Rest>(rest)...);
        };
    }

    //! The same for `function(subject, path, rest...)`: what is written, then where.
    template<typename Result, typename Subject, typename PathText, typename... Rest>
    auto pathSecond(Result (*function)(Subject, PathText, Rest...))
    {
        return [function](Subject subject, const PathOnly & path, Rest... rest) -> Result {
            return function(std::forward<Subject>(subject), utf8Of(path.value), std::forward<Rest>(rest)...);
        };
    }

    //! The same for a member function `object.function(path, rest...)`.
    template<typename Result, typename Class, typename PathText, typename... Rest>
    auto pathFirst(Result (Class::*function)(PathText, Rest...))
    {
        return [function](Class & self, const PathOnly & path, Rest... rest) -> Result {
            return (self.*function)(utf8Of(path.value), std::forward<Rest>(rest)...);
        };
    }

    //! The same for a const member function.
    template<typename Result, typename Class, typename PathText, typename... Rest>
    auto pathFirst(Result (Class::*function)(PathText, Rest...) const)
    {
        return [function](const Class & self, const PathOnly & path, Rest... rest) -> Result {
            return (self.*function)(utf8Of(path.value), std::forward<Rest>(rest)...);
        };
    }

    //! A constructor of `Type` from a file, the file given as a `pathlib.Path`.
    template<typename Type>
    auto fromFile()
    {
        return pybind11::init([](const PathOnly & path) { return Type(utf8Of(path.value)); });
    }
}   // namespace PathBinding
