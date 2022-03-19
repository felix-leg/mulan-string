#pragma once
#ifndef MULAN_STRING_GETTEXT_BACKEND
#define MULAN_STRING_GETTEXT_BACKEND

#include "mls_locale.h"
#include "template.h"

//CUT-START

namespace mls::backend {
	
	class IntlNotInitialized : public std::exception {};
	
	/**
	 * @brief initialize the GetText utility
	 * 
	 * @param packageName Name of the GetText unit to pass to `bindtextdomain(...)`
	 * @param wantedLocale Force select a specific locale. Pass `nullptr` to use system locale
	 * @param folderLookup Setup a specific folder to look for `*.mo` files. Pass `nullptr` to use the default `/usr/local/share/locale`
	 */
	void init(
		const char* packageName, 
		const char* wantedLocale = nullptr, 
		const char* folderLookup = nullptr
		);
};

namespace mls {
	
	///Get template for `msgid` string
	Template translate(const char* msgid);
	
	///Get template for `msgid` string in a .mo `catalog`
	Template translate(const char* catalog, const char* msgid);
	
};

#ifndef MULANSTR_DONT_USE_UNDERSCORE

#	define _(msgid) mls::translate(msgid)
#	define _c(catalog, msgid) mls::translate(catalog, msgid)

#endif

//CUT-END

#endif //!MULAN_STRING_GETTEXT_BACKEND
