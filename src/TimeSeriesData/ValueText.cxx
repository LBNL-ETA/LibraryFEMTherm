#include <string>

#include <fileParse/Base.hxx>
#include <fileParse/Formatter.hxx>
#include <lbnl/algorithm.hxx>

#include "ValueText.hxx"

namespace TimeSeriesLibrary
{
    std::string formatSeriesValues(const std::vector<double> & values)
    {
        const auto & config{FileParse::SerializationConfig::getInstance()};

        std::string text;
        for(const double value : values)
        {
            if(!text.empty())
            {
                text += ',';
            }
            text += FileParse::formatDouble(
              value, config.precision, config.scientificLowerBound, config.scientificUpperBound);
        }

        return text;
    }

    std::vector<double> parseSeriesValues(std::string_view text)
    {
        const auto tokens{lbnl::split(text, ',')};
        return lbnl::transform_to_vector(tokens,
                                         [](const std::string & token) { return std::stod(token); });
    }
}   // namespace TimeSeriesLibrary
