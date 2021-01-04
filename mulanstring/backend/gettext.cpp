/**
 * Implementation of GNU GetText backend
*/

#include "gettext.hpp"
#include "../interface.hpp"
#include <libintl.h>
#include <locale.h>
#include <string>

namespace mls::backend {
	
	void init(const char* packageName, const char* wantedLocale, const char* folderLookup) {
		const char* theLocale;
		
		if( wantedLocale == nullptr) theLocale = setlocale(LC_ALL, "");
		else theLocale = setlocale(LC_ALL, wantedLocale);
		
		if( folderLookup == nullptr) bindtextdomain(packageName, "/usr/local/share/locale");
		else bindtextdomain(packageName, folderLookup);
		
		bind_textdomain_codeset(packageName, "utf-8");
		
		textdomain(packageName);
		
		mls::initLocale(theLocale);
	}
	
	std::string getTranslation(const char* msgid) {
		return std::string( gettext(msgid) );
	}
	
	std::string getTranslation(const char* catalogue, const char* msgid) {
		return std::string( dgettext(catalogue, msgid) );
	}
}
