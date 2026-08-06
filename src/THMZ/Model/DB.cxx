#include "DB.hxx"

#include "THMX.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"
#include "THMZ/Model/migration/Pipeline.hxx"

namespace ThermFile
{
    const std::string topNodeName = "ThermModel";

    std::optional<ThermModel> loadThermModelFromFile(std::string_view fileName)
    {
        return Common::loadFromFile<ThermModel>(fileName, topNodeName);
    }

    std::optional<ThermModel> loadThermModelFromEntries(const std::map<std::string, std::string> & entries)
    {
        const auto modelEntry{ThermZip::findEntry(entries, ThermZip::ModelFileName)};
        if(modelEntry.empty())
        {
            return std::nullopt;
        }

        auto model{loadThermModelFromString(modelEntry)};
        if(model.has_value())
        {
            model = Migration::applyAllToModel(entries, std::move(*model));
        }
        return model;
    }

    std::optional<ThermModel> loadThermModelFromZipFile(std::string const & zipFileName)
    {
        if(!std::filesystem::exists(zipFileName))
        {
            return std::nullopt;
        }

        try
        {
            return loadThermModelFromEntries(ThermZip::unzipFiles(zipFileName));
        }
        catch(const std::runtime_error &)
        {
            return std::nullopt;
        }
    }

    int saveToFile(const ThermModel & model, std::string_view fileName, FileParse::FileFormat format)
    {
        switch(format)
        {
            case FileParse::FileFormat::JSON:
                return Common::saveToJSONFile(model, fileName, topNodeName);
            case FileParse::FileFormat::XML:
            default:
                return Common::saveToXMLFile(model, fileName, topNodeName);
        }
    }

    std::string saveToString(const ThermModel & model, FileParse::FileFormat format)
    {
        return Common::saveToString(model, topNodeName, format);
    }

    int saveToZipFile(const ThermModel & model, std::string_view zipFileName, FileParse::FileFormat format)
    {
        return Common::saveToZIPFile(model, ThermZip::ModelFileName, zipFileName, topNodeName, format);
    }

    std::optional<ThermModel> loadThermModelFromString(const std::string & str, FileParse::FileFormat format)
    {
        const std::string content{Common::stripUTF8BOM(str)};
        if(format == FileParse::FileFormat::Unknown)
        {
            return Common::loadFromString<ThermModel>(content, topNodeName);
        }
        return Common::loadFromString<ThermModel>(content, topNodeName, format);
    }
}   // namespace ThermFile
