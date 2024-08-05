//! \brief Creates concrete examples that will be used for testing. All examples will have in memory and equivalent
//! nodal structure pair that will have equivalent data.
//! \file FilePropertiesTestExamples.hxx
#pragma once

#include "THMZ/Properties/Properties.hxx"
#include "THMZ/Model/THMX.hxx"
#include "MockNodeAdapter.hxx"

namespace Helper
{
    namespace FileProperties1
    {
        ThermFile::Properties testObject();
        Helper::MockNode equivalentNodeStructure();
    }

    namespace THMXFile1
    {
        ThermFile::ThermModel testObject();
        Helper::MockNode equivalentNodeStructure();
    }
}   // namespace Helper