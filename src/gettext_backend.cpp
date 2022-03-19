/**
 * @file gettext_backend.cpp
 * @brief The GetText backend for MuLan-String library
 * 
 */

#include <libintl.h>
#include <locale.h>
#include <string>
#include <map>
#include <vector>

#include "gettext_backend.h"

//CUT-START

namespace mls::backend {
	
	locale::Locale *defaultLocale = nullptr;
	
	locale::Locale *getLocaleFromName(std::string_view searchLocaleName) {
		static std::map<std::string, std::vector<std::string>> LOCALES{
			//English - United Kingdom
			{"en_GB", {"en_GB","en-GB","English_United Kingdom"}},
			//English - American
			{"en_US", {"en_US","en-US","English_United States"}},
			//Polish
			{"pl_PL", {"pl_PL","pl-PL","Polish_Poland"}}
		};
		
		//Search for supported locales
		for(auto&[locale_name, locale_list] : LOCALES) {
			for(auto& some_locale_name : locale_list) {
				if( some_locale_name == searchLocaleName ) {
					return &(locale::getLocale(locale_name));
				}
			}
		}
		//unable to find the locale
		return nullptr;
	}
	
	void init(const char* packageName, const char* wantedLocale, const char* folderLookup) {
		const char* theLocale;
		
		if( wantedLocale == nullptr ) {
			theLocale = setlocale(LC_ALL, "");
		} else {
			setlocale(LC_ALL, wantedLocale);
			theLocale = wantedLocale;
		}
		
		std::string_view correctLocale{theLocale};
		//check for presence of unwanted suffixes or wrong locale names
		if( correctLocale == "C" || correctLocale == "POSIX" ) {
			correctLocale = "en_US";
		}
		auto dot = correctLocale.find_last_of(".");
		if( dot != std::string_view::npos ) {
			correctLocale.remove_suffix( correctLocale.size() - dot );
		}
		auto at = correctLocale.find_last_of("@");
		if( at != std::string_view::npos ) {
			correctLocale.remove_suffix( correctLocale.size() - at );
		}
		
		defaultLocale = getLocaleFromName(correctLocale);
		if( defaultLocale == nullptr ) {//unable to find supported locale?
			defaultLocale = &(locale::getLocale("en_US"));//fallback to English
			setlocale(LC_ALL, "C");//Force non-translated strings
		}
		
		if( folderLookup == nullptr ) {
			bindtextdomain(packageName, "/usr/local/share/locale");
		} else {
			bindtextdomain(packageName, folderLookup);
		}
		
		bind_textdomain_codeset(packageName, "utf-8");
		textdomain(packageName);
	}
	
};

namespace mls {
	
	Template translate(const char* msgid) {
		if( backend::defaultLocale == nullptr ) {
			throw backend::IntlNotInitialized();
		}
		return Template{std::string{ gettext(msgid) }, *backend::defaultLocale};
	}
	
	Template translate(const char* catalog, const char* msgid) {
		if( backend::defaultLocale == nullptr ) {
			throw backend::IntlNotInitialized();
		}
		return Template{std::string{ dgettext(catalog, msgid) }, *backend::defaultLocale};
	}
	
};

//CUT-END
