#include <fstream>
#include <filesystem>

#include <gtest/gtest.h>

#include "Materials/Materials.hxx"
#include "Materials/Common.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

class TestImportExportFile : public testing::Test
{
protected:
    void SetUp() override
    {}
};

TEST_F(TestImportExportFile, ExportToCSV)
{
    const std::string fileName{"Test.csv"};
    const std::optional<std::vector<MaterialsLibrary::point>> vector({{0, 0}, {1, 1}, {2, 2}});
    MaterialsLibrary::saveToCSV(fileName, vector);

    // Now compare and test two files
    std::ifstream inFile;
    inFile.open(fileName);   // open the input file

    std::stringstream strStream;
    strStream << inFile.rdbuf();         // read the file
    std::string text{strStream.str()};   // str holds the content of the file

    std::string correctText{"0,0\n1,1\n2,2"};

    EXPECT_EQ(text, correctText);

    inFile.close();

    std::filesystem::remove(fileName);
}

TEST_F(TestImportExportFile, ImportFromCSV)
{
    const std::string fileContent{"0,0\n1,3\n2,5"};
    const std::string fileName{"Test.csv"};

    File::createFileFromString(fileName, fileContent);

    const auto values{MaterialsLibrary::loadFromCSV(fileName)};

    EXPECT_EQ(values.has_value(), true);

    if(values.has_value())
    {
        const auto & vec{values.value()};
        const std::vector<MaterialsLibrary::point> correctVector({{0, 0}, {1, 3}, {2, 5}});

        EXPECT_EQ(vec.size(), 3u);

        for(size_t i = 0u; i < vec.size(); ++i)
        {
            EXPECT_EQ(vec[i].x, correctVector[i].x);
            EXPECT_EQ(vec[i].y, correctVector[i].y);
        }
    }

    std::filesystem::remove(fileName);
}

TEST_F(TestImportExportFile, ImportFromFileToSet_1)
{
    const std::string fileContent{"Edge\n"
                                  "Frame\n"
                                  "None\n"
                                  "ShadeInETag\n"
                                  "ShadeOutETag\n"
                                  "SHGC Exterior\n"
                                  "Spacer"};

    const std::string fileName{"Test.csv"};

    File::createFileFromString(fileName, fileContent);

    const auto values{File::loadFileToSet(fileName)};

    const std::set<std::string> correctSet(
      {"Edge", "Frame", "None", "ShadeInETag", "ShadeOutETag", "SHGC Exterior", "Spacer"});

    EXPECT_EQ(values, correctSet);

    std::filesystem::remove(fileName);
}

TEST_F(TestImportExportFile, ImportFromFileToSet_2)
{
    const std::string fileContent{"Edge\n"
                                  "Frame\n"
                                  "Frame\n"
                                  "None\n"
                                  "ShadeInETag\n"
                                  "ShadeOutETag\n"
                                  "ShadeOutETag\n"
                                  "SHGC Exterior\n"
                                  "Spacer"};

    const std::string fileName{"Test.csv"};

    File::createFileFromString(fileName, fileContent);

    const auto values{File::loadFileToSet(fileName)};

    const std::set<std::string> correctSet(
      {"Edge", "Frame", "None", "ShadeInETag", "ShadeOutETag", "SHGC Exterior", "Spacer"});

    EXPECT_EQ(values, correctSet);

    std::filesystem::remove(fileName);
}

TEST_F(TestImportExportFile, ExportToFileSet)
{
    const std::set<std::string> correctSet(
      {"Edge", "Frame", "None", "ShadeInETag", "ShadeOutETag", "SHGC Exterior", "Spacer"});

    const std::string fileName{"Test.csv"};

    File::saveSetToFile(fileName, correctSet);

    const auto loadedSet{File::loadFileToSet(fileName)};

    EXPECT_EQ(correctSet, loadedSet);

    std::filesystem::remove(fileName);
}
