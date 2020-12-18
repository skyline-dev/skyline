#pragma once

namespace nn {
namespace settings {
    namespace system {
        struct FirmwareVersion {
            u8 major;
            u8 minor;
            u8 micro;
            u8 padding1;
            u8 revision_major;
            u8 revision_minor;
            u8 padding2;
            u8 padding3;
            char platform[0x20];
            char version_hash[0x40];
            char display_version[0x18];
            char display_title[0x80];

            constexpr inline u32 getVersion() const {
                return (static_cast<u32>(major) << 16) | (static_cast<u32>(minor) << 8) |
                       (static_cast<u32>(micro) << 0);
            }
        };

        Result GetFirmwareVersion(FirmwareVersion*);
    }  // namespace system

    enum Language {
        Language_Japanese,
        Language_English,
        Language_French,
        Language_German,
        Language_Italian,
        Language_Spanish,
        Language_Chinese,
        Language_Korean,
        Language_Dutch,
        Language_Portuguese,
        Language_Russian,
        Language_Taiwanese,
        Language_BritishEnglish,
        Language_CanadianFrench,
        Language_LatinAmericanSpanish
    };

    struct LanguageCode {
        char code[0x8];

        static LanguageCode Make(nn::settings::Language);
    };

    bool operator==(nn::settings::LanguageCode const&, nn::settings::LanguageCode const&);
};  // namespace settings
};  // namespace nn