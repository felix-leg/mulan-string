#pragma once

#ifndef MULANSTR_BACKEND_HPP
#define MULANSTR_BACKEND_HPP

/**
 * Basic interface for backends used by MuLan String
 * to get translations
*/

#include <string>

namespace mls::backend {
	///lookup for a translation for string in the default catalogue 
	std::string getTranslation(const char* msgid);
	
	///lookup for a translation for string in the given catalogue 
	std::string getTranslation(const char* catalogue, const char* msgid);
	
	//Not writen here: an init(...) function, because
	//different backends may be initialized differently
}

#endif /* MULANSTR_BACKEND_HPP */
//file end
