#if ! defined POTEXT_PO_LANGUAGESPECS_HPP
#define POTEXT_PO_LANGUAGESPECS_HPP

/*
 *  This file is part of potext.
 *
 *  potext is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  potext is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with potext; if not, write to the Free Software Foundation, Inc., 59
 *  Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  See tinydoc/LICENSE.md for the original tinygettext licensing statement.
 *  If you do not like the changes or the GPL licensing, use the original
 *  tinygettext project, available at GitHub:
 *
 *      https://github.com/tinygettext/tinygettext
 */

/**
 * \file          languagespecs.hpp
 *
 *  Macros that depend upon the build platform.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-06
 * \updates       2024-03-15
 * \license       See above.
 *
 *  This module is best edited in wide-screen.
 */

#include <string>                       /* std::string                      */

namespace po
{

struct languagespec
{
    std::string language;        /**< Language code: "de", "en", ...         */
    std::string country;         /**< Country code: "BR", "DE", ..., 0       */
    std::string modifier;        /**< Mod/varint: "Latn", "ije", "latin", 0  */
    std::string name;            /**< Language name: "German", "English".    */
    std::string name_localized;  /**< Language name in specified language.   */
};

/**
 *  Definitions for the language class.
 */

#if defined POTEXT_BUILD_ALL_LANGUAGES

static const languagespec s_languages [] =
{
    { "aa",  "",   "",         "Afar",                          "ʿAfár af"                  },
    { "af",  "",   "",         "Afrikaans",                     "Afrikaans"                 },
    { "af",  "ZA", "",         "Afrikaans (South Africa)",      ""                          },
    { "am",  "",   "",         "Amharic",                       "ኣማርኛ"                      },
    { "ar",  "",   "",         "Arabic",                        "العربية"                   },
    { "ar",  "AR", "",         "Arabic (Argentina)",            ""                          },
    { "ar",  "OM", "",         "Arabic (Oman)",                 ""                          },
    { "ar",  "SA", "",         "Arabic (Saudi Arabia)",         ""                          },
    { "ar",  "SY", "",         "Arabic (Syrian Arab Republic)", ""                          },
    { "ar",  "TN", "",         "Arabic (Tunisia)",              ""                          },
    { "as",  "",   "",         "Assamese",                      "অসমীয়া"                      },
    { "ast", "",   "",         "Asturian",                      "Asturianu"                 },
    { "ay",  "",   "",         "Aymara",                        "aymar aru"                 },
    { "az",  "",   "",         "Azerbaijani",                   "Azərbaycanca"              },
    { "az",  "IR", "",         "Azerbaijani (Iran)",            ""                          },
    { "be",  "",   "",         "Belarusian",                    "Беларуская мова"           },
    { "be",  "",   "latin",    "Belarusian",                    "Беларуская мова"           },
    { "bg",  "",   "",         "Bulgarian",                     "български"                 },
    { "bg",  "BG", "",         "Bulgarian (Bulgaria)",          ""                          },
    { "bn",  "",   "",         "Bengali",                       "বাংলা"                        },
    { "bn",  "BD", "",         "Bengali (Bangladesh)",          ""                          },
    { "bn",  "IN", "",         "Bengali (India)",               ""                          },
    { "bo",  "",   "",         "Tibetan",                       "བོད་སྐད་"                    },
    { "br",  "",   "",         "Breton",                        "brezhoneg"                 },
    { "bs",  "",   "",         "Bosnian",                       "Bosanski"                  },
    { "bs",  "BA", "",         "Bosnian (Bosnia/Herzegovina)",  ""                          },
    { "bs",  "BS", "",         "Bosnian (Bahamas)",             ""                          },
    { "ca",  "ES", "valencia", "Catalan (valencia)",            ""                          },
    { "ca",  "ES", "",         "Catalan (Spain)",               ""                          },
    { "ca",  "",   "valencia", "Catalan (valencia)",            ""                          },
    { "ca",  "",   "",         "Catalan",                       ""                          },
    { "cmn", "",   "",         "Mandarin",                      ""                          },
    { "co",  "",   "",         "Corsican",                      "corsu"                     },
    { "cs",  "",   "",         "Czech",                         "Čeština"                   },
    { "cs",  "CZ", "",         "Czech (Czech Republic)",        "Čeština (Česká Republika)" },
    { "cy",  "",   "",         "Welsh",                         "Welsh"                     },
    { "cy",  "GB", "",         "Welsh (Great Britain)",         "Welsh (Great Britain)"     },
    { "cz",  "",   "",         "Unknown language",              "Unknown language"          },
    { "da",  "",   "",         "Danish",                        "Dansk"                     },
    { "da",  "DK", "",         "Danish (Denmark)",              "Dansk (Danmark)"           },
    { "de",  "",   "",         "German",                        "Deutsch"                   },
    { "de",  "AT", "",         "German (Austria)",              "Deutsch (Österreich)"      },
    { "de",  "CH", "",         "German (Switzerland)",          "Deutsch (Schweiz)"         },
    { "de",  "DE", "",         "German (Germany)",              "Deutsch (Deutschland)"     },
    { "dk",  "",   "",         "Unknown language",              "Unknown language"          },
    { "dz",  "",   "",         "Dzongkha",                      "རྫོང་ཁ"                      },
    { "el",  "",   "",         "Greek",                         "ελληνικά"                  },
    { "el",  "GR", "",         "Greek (Greece)",                ""                          },
    { "en",  "",   "",         "English",                       "English"                   },
    { "en",  "AU", "",         "English (Australia)",           "English (Australia)"       },
    { "en",  "CA", "",         "English (Canada)",              "English (Canada)"          },
    { "en",  "GB", "",         "English (Great Britain)",       "English (Great Britain)"   },
    { "en",  "US", "",         "English (United States)",       "English (United States)"   },
    { "en",  "ZA", "",         "English (South Africa)",        "English (South Africa)"    },
    { "en",  "",   "boldquot", "English",                       "English"                   },
    { "en",  "",   "quot",     "English",                       "English"                   },
    { "en",  "US", "piglatin", "English",                       "English"                   },
    { "eo",  "",   "",         "Esperanto",                     "Esperanto"                 },
    { "es",  "",   "",         "Spanish",                       "Español"                   },
    { "es",  "AR", "",         "Spanish (Argentina)",           ""                          },
    { "es",  "CL", "",         "Spanish (Chile)",               ""                          },
    { "es",  "CO", "",         "Spanish (Colombia)",            ""                          },
    { "es",  "CR", "",         "Spanish (Costa Rica)",          ""                          },
    { "es",  "DO", "",         "Spanish (Dominican Republic)",  ""                          },
    { "es",  "EC", "",         "Spanish (Ecuador)",             ""                          },
    { "es",  "ES", "",         "Spanish (Spain)",               ""                          },
    { "es",  "GT", "",         "Spanish (Guatemala)",           ""                          },
    { "es",  "HN", "",         "Spanish (Honduras)",            ""                          },
    { "es",  "LA", "",         "Spanish (Laos)",                ""                          },
    { "es",  "MX", "",         "Spanish (Mexico)",              ""                          },
    { "es",  "NI", "",         "Spanish (Nicaragua)",           ""                          },
    { "es",  "PA", "",         "Spanish (Panama)",              ""                          },
    { "es",  "PE", "",         "Spanish (Peru)",                ""                          },
    { "es",  "PR", "",         "Spanish (Puerto Rico)",         ""                          },
    { "es",  "SV", "",         "Spanish (El Salvador)",         ""                          },
    { "es",  "UY", "",         "Spanish (Uruguay)",             ""                          },
    { "es",  "VE", "",         "Spanish (Venezuela)",           ""                          },
    { "et",  "",   "",         "Estonian",                      "eesti keel"                },
    { "et",  "EE", "",         "Estonian (Estonia)",            ""                          },
    { "et",  "ET", "",         "Estonian (Ethiopia)",           ""                          },
    { "eu",  "",   "",         "Basque",                        "euskara"                   },
    { "eu",  "ES", "",         "Basque (Spain)",                ""                          },
    { "fa",  "",   "",         "Persian",                       "فارسى"                     },
    { "fa",  "AF", "",         "Persian (Afghanistan)",         ""                          },
    { "fa",  "IR", "",         "Persian (Iran)",                ""                          },
    { "fi",  "",   "",         "Finnish",                       "suomi"                     },
    { "fi",  "FI", "",         "Finnish (Finland)",             ""                          },
    { "fo",  "",   "",         "Faroese",                       "Føroyskt"                  },
    { "fo",  "FO", "",         "Faeroese (Faroe Islands)",      ""                          },
    { "fr",  "",   "",         "French",                        "Français"                  },
    { "fr",  "CA", "",         "French (Canada)",               "Français (Canada)"         },
    { "fr",  "CH", "",         "French (Switzerland)",          "Français (Suisse)"         },
    { "fr",  "FR", "",         "French (France)",               "Français (France)"         },
    { "fr",  "LU", "",         "French (Luxembourg)",           "Français (Luxembourg)"     },
    { "fy",  "",   "",         "Frisian",                       "Frysk"                     },
    { "ga",  "",   "",         "Irish",                         "Gaeilge"                   },
    { "gd",  "",   "",         "Gaelic Scots",                  "Gàidhlig"                  },
    { "gl",  "",   "",         "Galician",                      "Galego"                    },
    { "gl",  "ES", "",         "Galician (Spain)",              ""                          },
    { "gn",  "",   "",         "Guarani",                       "Avañe'ẽ"                   },
    { "gu",  "",   "",         "Gujarati",                      "ગુજરાતી"                    },
    { "gv",  "",   "",         "Manx",                          "Gaelg"                     },
    { "ha",  "",   "",         "Hausa",                         "حَوْسَ"                       },
    { "he",  "",   "",         "Hebrew",                        "עברית"                     },
    { "he",  "IL", "",         "Hebrew (Israel)",               ""                          },
    { "hi",  "",   "",         "Hindi",                         "हिन्दी"                     },
    { "hi",  "IN", "",         "Hindi (India)",                 ""                          },
    { "hr",  "",   "",         "Croatian",                      "Hrvatski"                  },
    { "hr",  "HR", "",         "Croatian (Croatia)",            ""                          },
    { "hu",  "",   "",         "Hungarian",                     "magyar"                    },
    { "hu",  "HU", "",         "Hungarian (Hungary)",           ""                          },
    { "hy",  "",   "",         "Armenian",                      "Հայերեն"                   },
    { "ia",  "",   "",         "Interlingua",                   "Interlingua"               },
    { "id",  "",   "",         "Indonesian",                    "Bahasa Indonesia"          },
    { "id",  "ID", "",         "Indonesian (Indonesia)",        ""                          },
    { "is",  "",   "",         "Icelandic",                     "Íslenska"                  },
    { "is",  "IS", "",         "Icelandic (Iceland)",           ""                          },
    { "it",  "",   "",         "Italian",                       "Italiano"                  },
    { "it",  "CH", "",         "Italian (Switzerland)",         ""                          },
    { "it",  "IT", "",         "Italian (Italy)",               ""                          },
    { "iu",  "",   "",         "Inuktitut",                     "ᐃᓄᒃᑎᑐᑦ/inuktitut"          },
    { "ja",  "",   "",         "Japanese",                      "日本語"                    },
    { "ja",  "JP", "",         "Japanese (Japan)",              ""                          },
    { "ka",  "",   "",         "Georgian",                      "ქართული"                   },
    { "kk",  "",   "",         "Kazakh",                        "Қазақша"                   },
    { "kl",  "",   "",         "Kalaallisut",                   "Kalaallisut"               },
    { "km",  "",   "",         "Khmer",                         "ភាសាខ្មែរ"                  },
    { "km",  "KH", "",         "Khmer (Cambodia)",              ""                          },
    { "kn",  "",   "",         "Kannada",                       "ಕನ್ನಡ"                      },
    { "ko",  "",   "",         "Korean",                        "한국어"                    },
    { "ko",  "KR", "",         "Korean (Korea)",                ""                          },
    { "ku",  "",   "",         "Kurdish",                       "Kurdî"                     },
    { "kw",  "",   "",         "Cornish",                       "Kernowek"                  },
    { "ky",  "",   "",         "Kirghiz",                       ""                          },
    { "la",  "",   "",         "Latin",                         "Latina"                    },
    { "lo",  "",   "",         "Lao",                           "ລາວ"                       },
    { "lt",  "",   "",         "Lithuanian",                    "Lietuvių"                  },
    { "lt",  "LT", "",         "Lithuanian (Lithuania)",        ""                          },
    { "lv",  "",   "",         "Latvian",                       "Latviešu"                  },
    { "lv",  "LV", "",         "Latvian (Latvia)",              ""                          },
    { "jbo", "",   "",         "Lojban",                        "La .lojban."               },
    { "mg",  "",   "",         "Malagasy",                      "Malagasy"                  },
    { "mi",  "",   "",         "Maori",                         "Māori"                     },
    { "mk",  "",   "",         "Macedonian",                    "Македонски"                },
    { "mk",  "MK", "",         "Macedonian (Macedonia)",        ""                          },
    { "ml",  "",   "",         "Malayalam",                     "മലയാളം"                    },
    { "mn",  "",   "",         "Mongolian",                     "Монгол"                    },
    { "mr",  "",   "",         "Marathi",                       "मराठी"                     },
    { "ms",  "",   "",         "Malay",                         "Bahasa Melayu"             },
    { "ms",  "MY", "",         "Malay (Malaysia)",              ""                          },
    { "mt",  "",   "",         "Maltese",                       "Malti"                     },
    { "my",  "",   "",         "Burmese",                       "မြန်မာဘာသာ"                 },
    { "my",  "MM", "",         "Burmese (Myanmar)",             ""                          },
    { "nb",  "",   "",         "Norwegian Bokmal",              ""                          },
    { "nb",  "NO", "",         "Norwegian Bokmål (Norway)",     ""                          },
    { "nds", "",   "",         "Low German",                    ""                          },
    { "ne",  "",   "",         "Nepali",                        ""                          },
    { "nl",  "",   "",         "Dutch",                         "Nederlands"                },
    { "nl",  "BE", "",         "Dutch (Belgium)",               ""                          },
    { "nl",  "NL", "",         "Dutch (Netherlands)",           ""                          },
    { "nn",  "",   "",         "Norwegian Nynorsk",             "Norsk nynorsk"             },
    { "nn",  "NO", "",         "Norwegian Nynorsk (Norway)",    ""                          },
    { "no",  "",   "",         "Norwegian",                     "Norsk bokmål"              },
    { "no",  "NO", "",         "Norwegian (Norway)",            ""                          },
    { "no",  "NY", "",         "Norwegian (NY)",                ""                          },
    { "nr",  "",   "",         "Ndebele, South",                ""                          },
    { "oc",  "",   "",         "Occitan post 1500",             "Occitan"                   },
    { "om",  "",   "",         "Oromo",                         "Oromoo"                    },
    { "or",  "",   "",         "Oriya",                         "ଓଡ଼ିଆ"                       },
    { "pa",  "",   "",         "Punjabi",                       "ਪੰਜਾਬੀ"                     },
    { "pl",  "",   "",         "Polish",                        "Polski"                    },
    { "pl",  "PL", "",         "Polish (Poland)",               ""                          },
    { "ps",  "",   "",         "Pashto",                        "پښتو"                      },
    { "pt",  "",   "",         "Portuguese",                    "Português"                 },
    { "pt",  "BR", "",         "Portuguese (Brazil)",           ""                          },
    { "pt",  "PT", "",         "Portuguese (Portugal)",         ""                          },
    { "qu",  "",   "",         "Quechua",                       "Runa Simi"                 },
    { "rm",  "",   "",         "Rhaeto-Romance",                "Rumantsch"                 },
    { "ro",  "",   "",         "Romanian",                      "Română"                    },
    { "ro",  "RO", "",         "Romanian (Romania)",            ""                          },
    { "ru",  "",   "",         "Russian",                       "Русский"                   },
    { "ru",  "RU", "",         "Russian (Russia",               ""                          },
    { "rw",  "",   "",         "Kinyarwanda",                   "Kinyarwanda"               },
    { "sa",  "",   "",         "Sanskrit",                      ""                          },
    { "sd",  "",   "",         "Sindhi",                        ""                          },
    { "se",  "",   "",         "Sami",                          "Sámegiella"                },
    { "se",  "NO", "",         "Sami (Norway)",                 ""                          },
    { "si",  "",   "",         "Sinhalese",                     ""                          },
    { "sk",  "",   "",         "Slovak",                        "Slovenčina"                },
    { "sk",  "SK", "",         "Slovak (Slovakia)",             ""                          },
    { "sl",  "",   "",         "Slovenian",                     "Slovenščina"               },
    { "sl",  "SI", "",         "Slovenian (Slovenia)",          ""                          },
    { "sl",  "SL", "",         "Slovenian (Sierra Leone)",      ""                          },
    { "sm",  "",   "",         "Samoan",                        ""                          },
    { "so",  "",   "",         "Somali",                        ""                          },
    { "sp",  "",   "",         "Unknown language",              ""                          },
    { "sq",  "",   "",         "Albanian",                      "Shqip"                     },
    { "sq",  "AL", "",         "Albanian (Albania)",            ""                          },
    { "sr",  "",   "",         "Serbian",                       "Српски / srpski"           },
    { "sr",  "YU", "",         "Serbian (Yugoslavia)",          ""                          },
    { "sr",  "",   "ije",      "Serbian",                       ""                          },
    { "sr",  "",   "latin",    "Serbian",                       ""                          },
    { "sr",  "",   "Latn",     "Serbian",                       ""                          },
    { "ss",  "",   "",         "Swati",                         ""                          },
    { "st",  "",   "",         "Sotho",                         ""                          },
    { "sv",  "",   "",         "Swedish",                       "Svenska"                   },
    { "sv",  "SE", "",         "Swedish (Sweden)",              ""                          },
    { "sv",  "SV", "",         "Swedish (El Salvador)",         ""                          },
    { "sw",  "",   "",         "Swahili",                       ""                          },
    { "ta",  "",   "",         "Tamil",                         ""                          },
    { "te",  "",   "",         "Telugu",                        ""                          },
    { "tg",  "",   "",         "Tajik",                         ""                          },
    { "th",  "",   "",         "Thai",                          "ไทย"                       },
    { "th",  "TH", "",         "Thai (Thailand)",               ""                          },
    { "ti",  "",   "",         "Tigrinya",                      ""                          },
    { "tk",  "",   "",         "Turkmen",                       ""                          },
    { "tl",  "",   "",         "Tagalog",                       ""                          },
    { "to",  "",   "",         "Tonga",                         ""                          },
    { "tr",  "",   "",         "Turkish",                       "Türkçe"                    },
    { "tr",  "TR", "",         "Turkish (Turkey)",              ""                          },
    { "ts",  "",   "",         "Tsonga",                        ""                          },
    { "tt",  "",   "",         "Tatar",                         ""                          },
    { "ug",  "",   "",         "Uighur",                        ""                          },
    { "uk",  "",   "",         "Ukrainian",                     "Українська"                },
    { "uk",  "UA", "",         "Ukrainian (Ukraine)",           ""                          },
    { "ur",  "",   "",         "Urdu",                          ""                          },
    { "ur",  "PK", "",         "Urdu (Pakistan)",               ""                          },
    { "uz",  "",   "",         "Uzbek",                         ""                          },
    { "uz",  "",   "cyrillic", "Uzbek",                         ""                          },
    { "vi",  "",   "",         "Vietnamese",                    "Tiếng Việt"                },
    { "vi",  "VN", "",         "Vietnamese (Vietnam)",          ""                          },
    { "wa",  "",   "",         "Walloon",                       ""                          },
    { "wo",  "",   "",         "Wolof",                         ""                          },
    { "xh",  "",   "",         "Xhosa",                         ""                          },
    { "yi",  "",   "",         "Yiddish",                       "ייִדיש"                     },
    { "yo",  "",   "",         "Yoruba",                        ""                          },
    { "zh",  "",   "",         "Chinese",                       "中文"                      },
    { "zh",  "CN", "",         "Chinese (simplified)",          ""                          },
    { "zh",  "HK", "",         "Chinese (Hong Kong)",           ""                          },
    { "zh",  "TW", "",         "Chinese (traditional)",         ""                          },
    { "zu",  "",   "",         "Zulu",                          ""                          },
    { "",    "",   "",         "",                              ""                          }
};

#else

    /**
     *  This provides a small sample for testing. We might add more once we
     *  check out the existing test "po" files.
     */

static const languagespec s_languages [] =
{
    { "ar",  "",   "",         "Arabic",                        "العربية"                   },
    { "be",  "",   "latin",    "Belarusian",                    "Беларуская мова"           },
    { "ca",  "",   "",         "Catalan",                       ""                          },
    { "cs",  "CZ", "",         "Czech (Czech Republic)",        "Čeština (Česká Republika)" },
    { "de",  "",   "",         "German",                        "Deutsch"                   },
    { "el",  "",   "",         "Greek",                         "ελληνικά"                  },
    { "en",  "",   "",         "English",                       "English"                   },
    { "en",  "GB", "",         "English (Great Britain)",       "English (Great Britain)"   },
    { "en",  "US", "",         "English (United States)",       "English (United States)"   },
    { "en",  "US", "piglatin", "English",                       "English"                   },
    { "es",  "",   "",         "Spanish",                       "Español"                   },
    { "fr",  "",   "",         "French",                        "Français"                  },
    { "nl",  "",   "",         "Dutch",                         "Nederlands"                },
    { "pl",  "",   "",         "Polish",                        "Polski"                    },
    { "pt",  "",   "",         "Portuguese",                    "Português"                 },
    { "ru",  "",   "",         "Russian",                       "Русский"                   },
    { "sv",  "",   "",         "Swedish",                       "Svenska"                   },
    { "tr",  "",   "",         "Turkish",                       "Türkçe"                    },
    { "yi",  "",   "",         "Yiddish",                       "ייִדיש"                     },
    { "",    "",   "",         "",                              ""                          }
};

#endif          // defined POTEXT_BUILD_ALL_LANGUAGES

}               // namespace po

#endif          // POTEXT_PO_LANGUAGESPECS_HPP

/*
 * languagespecs.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */


