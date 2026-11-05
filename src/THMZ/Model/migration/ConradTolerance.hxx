#pragma once

#include "THMZ/Model/THMX.hxx"

namespace ThermFile::Migration::ConradTolerance
{
    //! The first model version whose stored convergence tolerance reaches Conrad.
    inline constexpr int firstVersionWithConradTolerance{6};

    //! \brief Returns the model with the tolerance Conrad actually solved it at.
    //!
    //! Before version 6 Conrad took its tolerance from the application settings, 1e-6 by
    //! default, and never read the one stored in the file. Files therefore carry 1e-5 for
    //! Conrad without ever having run at it, and keeping that value would loosen the solve
    //! tenfold. A HygroThermFEM model keeps its value: that engine always read it.
    [[nodiscard]] ThermFile::ThermModel applyToModel(ThermFile::ThermModel model);
}   // namespace ThermFile::Migration::ConradTolerance
