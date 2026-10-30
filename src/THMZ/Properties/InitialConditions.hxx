#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ThermFile
{
    //! \brief A uniform state: temperature in °C, relative humidity as a fraction.
    struct UniformStart
    {
        double temperature{21.0};
        double humidity{0.5};

        [[nodiscard]] friend bool operator==(const UniformStart & lhs, const UniformStart & rhs) = default;
    };

    //! \brief A boundary segment held under another record while the starting state is
    //! solved: the segment's index in the model's boundary list and the record it reads.
    struct StartCondition
    {
        std::size_t segment{0U};
        std::string bcUUID{};

        [[nodiscard]] friend bool operator==(const StartCondition & lhs, const StartCondition & rhs) = default;
    };

    //! \brief The starting state is the steady state of the model with `conditions` swapped
    //! in. `start` is where that solve begins; none means the engine's own default. A field
    //! the model does not solve keeps its start value.
    struct SteadyStart
    {
        std::vector<StartCondition> conditions{};
        std::optional<UniformStart> start{};

        [[nodiscard]] friend bool operator==(const SteadyStart & lhs, const SteadyStart & rhs) = default;
    };

    //! \brief The starting state is the end of a transient run over `stepLengths` (seconds)
    //! with `conditions` swapped in, itself started from `start`. Segments reading a dataset
    //! read it from its beginning, as the main run then does again.
    struct TransientStart
    {
        std::vector<StartCondition> conditions{};
        std::vector<double> stepLengths{};
        UniformStart start{};

        [[nodiscard]] friend bool operator==(const TransientStart & lhs, const TransientStart & rhs) = default;
    };

    //! \brief How a run's initial state is set: a constant everywhere, or solved first by a
    //! steady or a transient run of the same model. A steady-state run reads only the
    //! uniform form, as the start of its iteration.
    struct InitialConditions
    {
        std::variant<UniformStart, SteadyStart, TransientStart> state{UniformStart{}};

        [[nodiscard]] friend bool operator==(const InitialConditions & lhs, const InitialConditions & rhs) = default;
    };

    //! \brief The uniform values of initial conditions: the constant itself, or where a
    //! solved start begins (the default start when a steady start names none).
    [[nodiscard]] inline UniformStart uniformStartOf(const InitialConditions & initialConditions)
    {
        return std::visit(
          [](const auto & start) -> UniformStart {
              using Start = std::decay_t<decltype(start)>;
              if constexpr(std::is_same_v<Start, UniformStart>)
              {
                  return start;
              }
              else if constexpr(std::is_same_v<Start, SteadyStart>)
              {
                  return start.start.value_or(UniformStart{});
              }
              else
              {
                  return start.start;
              }
          },
          initialConditions.state);
    }
}   // namespace ThermFile
