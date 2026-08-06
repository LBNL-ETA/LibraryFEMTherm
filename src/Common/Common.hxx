#pragma once

#include <string>
#include <chrono>

#include <fileParse/FileFormat.hxx>

namespace Common
{
    std::string generateXmlContent(const std::string & rootElement,
                                   const std::string & schemaLocation,
                                   const std::string & version);

    //! Minimal valid library file content for a freshly seeded (missing) library file.
    //! XML keeps the schemaLocation reference; JSON has no schema binding, so the
    //! parameter only participates in the XML branch.
    std::string generateLibraryContent(const std::string & rootElement,
                                       const std::string & schemaLocation,
                                       const std::string & version,
                                       FileParse::FileFormat format);

    void createDirectoryIfNotExists(std::string_view dirName);

    std::chrono::system_clock::time_point
      createTimePoint(int year, int month, int day, int hour, int minute, int second);
}   // namespace Common