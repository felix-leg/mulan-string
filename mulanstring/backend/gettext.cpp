/**
 * Implementation of GNU GetText backend
*/

#include "gettext.hpp"
#include "../interface.hpp"
#include <libintl.h>
#include <locale.h>

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
	
	char* getTranslation(const char* msgid) {
		return gettext(msgid);
	}
	
	char* getTranslation(const char* catalogue, const char* msgid) {
		return dgettext(catalogue, msgid);
	}
}
