#include <windows.h>

#define        VER_STR  "1.1.0.0"
VS_VERSION_INFO VERSIONINFO
FILEVERSION    1, 1, 0, 0
PRODUCTVERSION 1, 1, 0, 0
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904E4"
        BEGIN
            VALUE "CompanyName", "EMOJIGUN"
            VALUE "FileDescription", "emojigun - custom emoji sharing tool"
            VALUE "FileVersion", VER_STR
            VALUE "OriginalFilename", "emojigun.exe"
            VALUE "ProductName", "EMOJIGUN"
            VALUE "ProductVersion", VER_STR
            VALUE "LegalTrademarks1", "Licensed under GPL-3.0 license"
        END
    END

    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1252
    END
END
