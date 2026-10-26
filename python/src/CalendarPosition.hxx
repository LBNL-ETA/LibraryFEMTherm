#pragma once

#include <cstddef>
#include <variant>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "TimeSeriesData/TimeSeriesData.hxx"

namespace CalendarPositionBinding
{
    //! A month as a caller states it: by name, `Month.March`, or by its number, 3. The Month
    //! comes FIRST: pybind11 tries the alternatives in order, and an enum would convert to a
    //! number before its own alternative got its turn.
    using MonthLike = std::variant<TimeSeriesLibrary::Month, std::size_t>;

    [[nodiscard]] inline std::size_t positionNumber(const std::size_t value)
    {
        return value;
    }

    [[nodiscard]] inline std::size_t positionNumber(const MonthLike & month)
    {
        if(const auto * named{std::get_if<TimeSeriesLibrary::Month>(&month)})
        {
            return TimeSeriesLibrary::monthNumber(*named);
        }
        return std::get<std::size_t>(month);
    }

    //! A calendar position as Python states it, a dataset's TimeAxis or a case's Start: month,
    //! day and hour from 1, minute from 0. A position LibraryFEMTherm refuses is a
    //! ValueError carrying the reason; nothing is clamped.
    template<typename Position>
    [[nodiscard]] Position checkedPosition(const Position & position)
    {
        if(const auto error{
             TimeSeriesLibrary::calendarPositionError(position.month, position.day, position.hour, position.minute)})
        {
            throw pybind11::value_error(*error);
        }
        return position;
    }

    //! One field as a property whose setter checks the whole position it would make, so a
    //! refused value leaves the position as it was.
    //! `Given` is what the setter takes: a number, or for the month a MonthLike. The getter
    //! gives the number either way.
    template<auto Field, typename Given = std::size_t, typename Position>
    void addField(pybind11::class_<Position> & cls, const char * name, const char * doc)
    {
        cls.def_property(
          name,
          [](const Position & position) { return position.*Field; },
          [](Position & position, const Given & value) {
              Position changed{position};
              changed.*Field = positionNumber(value);
              position = checkedPosition(changed);
          },
          doc);
    }

    template<typename Position>
    void addCalendarPosition(pybind11::class_<Position> & cls)
    {
        addField<&Position::month, MonthLike>(cls,
                                              "month",
                                              "1..12, counted from 1 as a calendar and an EnergyPlus weather file "
                                              "count it; set by number or by name, `Month.March`, and read back as "
                                              "the number.");
        addField<&Position::day>(
          cls, "day", "1 to the days of the month, counted from 1; the nominal year has no leap day.");
        addField<&Position::hour>(cls,
                                  "hour",
                                  "1..24, counted from 1 as an EnergyPlus weather file counts it: hour 1 is "
                                  "00:00 to 01:00.");
        addField<&Position::minute>(cls, "minute", "0..59, into the hour: hour 1, minute 30 is 00:30.");
    }
}   // namespace CalendarPositionBinding
