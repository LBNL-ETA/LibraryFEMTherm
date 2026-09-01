#include <filesystem>

#include <gtest/gtest.h>

#include "THMZ/ZipModule/ZipModule.hxx"

TEST(TestZippingStringToFile, AddAndVerifyFileContent)
{
    const std::string testString{"< test test >"};
    const std::string fileName{"Test.xml"};
    const std::string zipFileName{"Test.zip"};

    // Add the file to the zip
    auto result = ThermZip::addToZipFile(zipFileName, fileName, testString);

    // Check if the file was added successfully
    EXPECT_EQ(result, 1);

    // Extract the file from the zip to verify its content
    std::vector<std::string> fileNames{fileName};
    auto extractedFiles = ThermZip::unzipFiles(zipFileName, fileNames);

    // Ensure the file was found in the zip archive
    ASSERT_TRUE(extractedFiles.find(fileName) != extractedFiles.end());

    // Verify the content of the extracted file
    EXPECT_EQ(extractedFiles[fileName], testString);

    // Remove the zip file after the test
    std::filesystem::remove(zipFileName);
}

TEST(TestZippingStringToFile, AddedContentIsCompressed)
{
    const std::string fileName{"Repetitive.xml"};
    const std::string zipFileName{"CompressionRegression.zip"};
    std::filesystem::remove(zipFileName);

    // Highly repetitive ~1 MB payload; a stored (level 0) rewrite would leave the
    // archive at payload size, a deflated one shrinks it far below 10%.
    std::string payload;
    payload.reserve(1000000U);
    while(payload.size() < 1000000U)
    {
        payload += "<Value>21.500000</Value>";
    }

    EXPECT_EQ(ThermZip::addToZipFile(zipFileName, fileName, payload), 1);

    const auto archiveSize{std::filesystem::file_size(zipFileName)};
    EXPECT_LT(archiveSize, payload.size() / 10U);

    const auto extracted{ThermZip::unzipFile(zipFileName, fileName)};
    EXPECT_EQ(extracted, payload);

    std::filesystem::remove(zipFileName);
}

TEST(TestZippingStringToFile, OverwriteExistingFile)
{
    const std::string initialString{"< initial content >"};
    const std::string updatedString{"< updated content >"};
    const std::string fileName{"Test.xml"};
    const std::string zipFileName{"Test.zip"};

    // Add the initial content
    auto result = ThermZip::addToZipFile(zipFileName, fileName, initialString);
    EXPECT_EQ(result, 1);

    // Overwrite with updated content
    result = ThermZip::addToZipFile(zipFileName, fileName, updatedString);
    EXPECT_EQ(result, 1);

    // Extract the file to verify its updated content
    std::vector<std::string> fileNames{fileName};
    auto extractedFiles = ThermZip::unzipFiles(zipFileName, fileNames);

    // Ensure the file was found in the zip archive
    ASSERT_TRUE(extractedFiles.find(fileName) != extractedFiles.end());

    // Verify the content of the extracted file
    EXPECT_EQ(extractedFiles[fileName], updatedString);

    // Remove the zip file after the test
    std::filesystem::remove(zipFileName);
}

TEST(TestZippingStringToFile, HandleEmptyString)
{
    const std::string emptyString;
    const std::string fileName{"Empty.xml"};
    const std::string zipFileName{"Test.zip"};

    // Add an empty string to the zip
    auto result = ThermZip::addToZipFile(zipFileName, fileName, emptyString);
    EXPECT_EQ(result, 1);

    // Extract the file to verify its content
    std::vector<std::string> fileNames{fileName};
    auto extractedFiles = ThermZip::unzipFiles(zipFileName, fileNames);

    // Ensure the file was found in the zip archive
    ASSERT_TRUE(extractedFiles.find(fileName) != extractedFiles.end());

    // Verify the content of the extracted file is empty
    EXPECT_EQ(extractedFiles[fileName], emptyString);

    // Remove the zip file after the test
    std::filesystem::remove(zipFileName);
}