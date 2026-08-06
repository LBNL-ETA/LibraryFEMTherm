#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include <fileParse/FileFormat.hxx>

namespace LibraryFiles
{
    //! Each library lives in exactly one file whose extension matches the active save
    //! format. When only the opposite-format file exists, its content is converted once;
    //! a leftover opposite-format file is then retired to ".bak" so the library is never
    //! present twice with diverging content. The conversion runs both ways, so switching
    //! the format back migrates the edits into the returning file.
    //!
    //! LibraryDB must be constructible from a file path and provide saveToString(format);
    //! every library DB in this repository (including the legacy Step1 ones) qualifies.
    template<typename LibraryDB>
    std::string resolvedPath(const std::string & xmlPath, FileParse::FileFormat format)
    {
        const std::filesystem::path canonical{xmlPath};
        std::filesystem::path jsonPath{canonical};
        jsonPath.replace_extension(".json");

        const std::filesystem::path & target{format == FileParse::FileFormat::JSON ? jsonPath : canonical};
        const std::filesystem::path & other{format == FileParse::FileFormat::JSON ? canonical : jsonPath};

        if(!std::filesystem::exists(target) && std::filesystem::exists(other))
        {
            LibraryDB source{other.string()};
            std::ofstream converted{target};
            converted << source.saveToString(format);
        }

        if(std::filesystem::exists(target) && std::filesystem::exists(other))
        {
            std::filesystem::path retired{other};
            retired += ".bak";
            std::filesystem::remove(retired);
            std::filesystem::rename(other, retired);
        }

        return target.string();
    }
}   // namespace LibraryFiles
