#pragma once

#ifndef MULANSTR_BACKEND_GETTEXT_HPP
#define MULANSTR_BACKEND_GETTEXT_HPP

/**
 * Backend for GNU GetText interface
*/

#include "../backend.hpp"

namespace mls::backend {
	
	/**
	 * initialization specific for GNU GetText backend.
	 * 
	 * @param packageName The program name (required)
	 * @param wantedLocale Which locale to set. If NULL, set the default system locale
	 * @param folderLookup Where to look for .mo files. If NULL, look in default Posix location
	*/
	void init(
		const char* packageName, 
		const char* wantedLocale = nullptr, 
		const char* folderLookup = nullptr
		);
	
}

#endif /* MULANSTR_BACKEND_GETTEXT_HPP */
//file end
