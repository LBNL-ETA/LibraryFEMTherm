#include "Pipeline.hxx"

#include <algorithm>
#include <array>
#include <exception>
#include <variant>

#include "Common/DB.hxx"
#include "CurrentLibrary.hxx"
#include "LibraryUtilities/Common.hxx"
#include "SteadyStateLibrary.hxx"
#include "TransientLibrary.hxx"

namespace BCLibrary::Import
{
    namespace
    {
        struct Format
        {
            bool (*recognizes)(const std::string & text);
            std::vector<BoundaryCondition> (*records)(const std::string & text);
        };

        //! The only list of formats. A new format is one file in this folder and one line here.
        constexpr std::array formats{Format{SteadyStateLibrary::recognizes, SteadyStateLibrary::records},
                                     Format{TransientLibrary::recognizes, TransientLibrary::records},
                                     Format{CurrentLibrary::recognizes, CurrentLibrary::records}};

        ImportedRecords permanentRecords(const std::vector<BoundaryCondition> & records)
        {
            ImportedRecords result;
            for(const auto & record : records)
            {
                if(std::holds_alternative<RadiationSurface>(record.data))
                {
                    ++result.notApplicable;
                }
                else if(LibraryCommon::isRecordTemporary(record))
                {
                    ++result.projectRecords;
                }
                else
                {
                    result.records.push_back(record);
                }
            }
            return result;
        }
    }   // namespace

    std::optional<ImportedRecords> recordsFromLibrary(const std::string & text)
    {
        const std::string content{Common::stripUTF8BOM(text)};
        const auto format{std::ranges::find_if(
          formats, [&content](const Format & candidate) { return candidate.recognizes(content); })};
        if(format == formats.end())
        {
            return std::nullopt;
        }
        try
        {
            return permanentRecords(format->records(content));
        }
        catch(const std::exception &)
        {
            return std::nullopt;
        }
    }
}   // namespace BCLibrary::Import
