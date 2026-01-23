#include "FileManipulation.hxx"

#include <fstream>
#include <sstream>
#include <set>

namespace File
{
    void createFileFromString(std::string_view fileName, std::string_view fileContent)
    {
        std::ofstream out(fileName.data());
        out << fileContent;
        out.close();
    }

    std::string loadToString(const std::string & fileName)
    {
        std::ifstream inFile{fileName};
        std::string result;
        std::string line;

        inFile.seekg(0, std::ios::end);
        const auto fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);

        if(fileSize > 0)
        {
            result.reserve(static_cast<size_t>(fileSize));
        }

        while(std::getline(inFile, line))
        {
            result.append(line);
            result.push_back('\n');
        }
        inFile.close();
        return result;
    }

    std::set<std::string> loadFileToSet(const std::string & fileName)
    {
        std::set<std::string> result;
        std::ifstream in{fileName};
        std::string line;

        while (std::getline(in, line))
        {
            result.insert(line);
        }
        in.close();
        return result;
    }

    void saveSetToFile(const std::string & fileName, const std::set<std::string> & set)
    {
        std::ofstream out(fileName);
        for(const auto & value : set)
        {
            out << value << "\n";
        }
        out.close();
    }
}   // namespace File
