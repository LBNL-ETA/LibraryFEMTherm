#pragma once

#include "THMZ/Model/THMX.hxx"

namespace ThermFile::Migration::UnderlayState
{
    //! The first model version that records whether the underlay is showing.
    inline constexpr int firstVersionWithUnderlayState{7};

    //! \brief Returns the model with its underlay showing if it names one.
    //!
    //! Before version 7 THERM never wrote this flag, so every file says the underlay is off
    //! whatever the user had chosen, and THERM showed the underlay of any file that named
    //! one. Keeping the stored value would hide those underlays.
    [[nodiscard]] ThermFile::ThermModel applyToModel(ThermFile::ThermModel model);
}   // namespace ThermFile::Migration::UnderlayState
