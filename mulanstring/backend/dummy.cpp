/*
 * Implementation of a Dummy backend
*/

#include "dummy.hpp"
#include "../interface.hpp"
#include <string>

namespace mls::backend {
	
	void init() {
		mls::initLocale("en_US");
	}
	
	std::string getTranslation(const char* msgid) {
		return std::string( msgid );
	}
	
	std::string getTranslation(const char* catalogue, const char* msgid) {
		return std::string( msgid );
	}
}

