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