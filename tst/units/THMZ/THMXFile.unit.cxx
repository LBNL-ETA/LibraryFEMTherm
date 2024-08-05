#include <filesystem>
#include <gtest/gtest.h>

#include "THMZ/Model/DB.hxx"
#include "THMZ/Model/Serializers.hxx"

#include "THMXHelper.hxx"
#include "TestExamples.hxx"
#include "MockNodeAdapter.hxx"

class TestTHMXFile : public testing::Test
{};

TEST_F(TestTHMXFile, File1Deserializer)
{
    auto mockData{Helper::THMXFile1::equivalentNodeStructure()};

    auto elementNode(mockData);
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ThermModel model;
    adapter >> model;

    ThermFile::ThermModel correctTHMZModel{Helper::THMXFile1::testObject()};

    constexpr auto tolerance{1e-6};
    Helper::expect_near(correctTHMZModel, model, tolerance);
}

TEST_F(TestTHMXFile, File1Serializer)
{
    ThermFile::ThermModel model{Helper::THMXFile1::testObject()};

    Helper::MockNode node{"ThermModel"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << model;

    auto correctNodes{Helper::THMXFile1::equivalentNodeStructure()};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestTHMXFile, XMLOperations)
{
        ThermFile::ThermModel model{Helper::THMXFile1::testObject()};

        const std::string fileName{"TestFile.xml"};
        std::filesystem::remove(fileName);
        ThermFile::saveToFile(model, fileName);

        ThermFile::ThermModel loadedModel{ThermFile::loadThermModelFromFile(fileName)};

        constexpr auto tolerance{1e-6};
        Helper::expect_near(model, loadedModel, tolerance);

        std::filesystem::remove(fileName);
}