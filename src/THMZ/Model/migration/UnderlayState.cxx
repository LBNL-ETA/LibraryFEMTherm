#include "UnderlayState.hxx"

namespace ThermFile::Migration::UnderlayState
{
    ThermFile::ThermModel applyToModel(ThermFile::ThermModel model)
    {
        auto & underlay{model.preferences.underlay};
        underlay.isUnderlayOn = !underlay.name.empty();

        return model;
    }
}   // namespace ThermFile::Migration::UnderlayState
