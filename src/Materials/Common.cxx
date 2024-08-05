#include <fstream>
#include <sstream>

#include "Common.hxx"

#include "Materials.hxx"

namespace MaterialsLibrary
{
    void saveToCSV(const std::string & fileName, const std::optional<std::vector<point>> & vector)
    {
        std::ofstream out(fileName);
        if(vector.has_value())
        {
            auto counter{0u};
            for(const auto & value : vector.value())
            {
                if(counter != 0u)
                {
                    out << "\n";
                }
                ++counter;
                out << value.x << "," << value.y;
            }
        }
        out.close();
    }

    std::optional<std::vector<point>> loadFromCSV(const std::string & fileName)
    {
        std::optional<std::vector<point>> result;
        std::ifstream in{fileName};
        std::string line;

        // Extract each column name
        while(std::getline(in, line))
        {
            std::stringstream ss(line);

            double val;

            std::vector<double> res;
            while(ss >> val)
            {
                res.push_back(val);
                if(ss.peek() == ',')
                {
                    ss.ignore();
                }
            }

            if(!result.has_value())
            {
                result = std::vector<point>();
            }

            result->push_back({res[0], res[1]});
        }

        return result;
    }
}   // namespace MaterialsLibrary
