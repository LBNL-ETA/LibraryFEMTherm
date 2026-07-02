```mermaid
graph TD
    %% Package definitions
    xmlParser[xmlParser<br/><font color='red'>v1.0.5</font>]
    nlohmann_json[nlohmann_json<br/><font color='red'>v3.12.0</font>]
    FileParse[FileParse<br/><font color='red'>Version_1.1.5</font>]
    LBNLCPPCommon[LBNLCPPCommon<br/><font color='red'>v0.18</font>]
    miniz[miniz<br/><font color='red'>89d7a5f6c3ce8893ea042b0a9d2a2d9975589ac9</font>]
    LibraryFEMTHERM[LibraryFEMTHERM<br/><font color='red'>Version_0.23.2</font>]

    %% Package relations
    LibraryFEMTHERM --> FileParse
    LibraryFEMTHERM --> LBNLCPPCommon
    LibraryFEMTHERM --> miniz
    FileParse --> xmlParser
    FileParse --> nlohmann_json
```
