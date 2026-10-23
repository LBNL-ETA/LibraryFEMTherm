#pragma once

#include <optional>
#include <string>
#include <variant>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "BoundaryConditions/Definitions.hxx"

namespace SourceBinding
{
    //! A Source as a constructor takes it: a Constant, a FromTimeSeries, the SeriesRole a
    //! FromTimeSeries reads, or a plain number. Spelled out as a variant so the signature,
    //! and the stub generated from it, names the role and the number; an implicit conversion
    //! alone would leave a type checker refusing them. The number comes LAST: pybind11 tries
    //! the alternatives in order, and a SeriesRole, being an enum, would convert to a number
    //! before its own alternative got its turn.
    using SourceLike =
      std::variant<BCLibrary::Constant, BCLibrary::FromTimeSeries, TimeSeriesLibrary::SeriesRole, double>;

    [[nodiscard]] inline BCLibrary::Source toSource(const SourceLike & given)
    {
        if(const auto * number{std::get_if<double>(&given)})
        {
            return BCLibrary::Constant{*number};
        }
        if(const auto * role{std::get_if<TimeSeriesLibrary::SeriesRole>(&given)})
        {
            return BCLibrary::FromTimeSeries{*role};
        }
        if(const auto * fixed{std::get_if<BCLibrary::Constant>(&given)})
        {
            return *fixed;
        }
        return std::get<BCLibrary::FromTimeSeries>(given);
    }

    [[nodiscard]] inline std::optional<BCLibrary::Source> toSource(const std::optional<SourceLike> & given)
    {
        return given.has_value() ? std::optional{toSource(given.value())} : std::nullopt;
    }

    //! An input as every constructor takes it: the number, or the role it reads, so a
    //! record prints as it would be written.
    [[nodiscard]] inline std::string sourceRepr(const BCLibrary::Source & source)
    {
        if(const auto * constant{std::get_if<BCLibrary::Constant>(&source)})
        {
            return pybind11::repr(pybind11::cast(constant->value)).cast<std::string>();
        }
        return pybind11::str(pybind11::cast(std::get<BCLibrary::FromTimeSeries>(source).role)).cast<std::string>();
    }
}   // namespace SourceBinding
