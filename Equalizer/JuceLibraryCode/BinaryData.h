/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace DNVAudioData
{
    extern const char*   FBlogo_png;
    const int            FBlogo_pngSize = 4248;

    extern const char*   DNVtext_png;
    const int            DNVtext_pngSize = 9038;

    extern const char*   GitLablogo_png;
    const int            GitLablogo_pngSize = 60239;

    extern const char*   Inlogo_png;
    const int            Inlogo_pngSize = 2295;

    extern const char*   LogoDNV_png;
    const int            LogoDNV_pngSize = 22839;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 5;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
