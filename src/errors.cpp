/**
 * @file errors.cpp
 * @brief All errors used by MuLan String
 * 
 */

#include "errors.h"

//CUT-START

namespace mls {
	
	InvalidTemplateState::InvalidTemplateState(std::string errString): err{errString} {};
	const char* InvalidTemplateState::what() const noexcept {
		return err.c_str();
	}
	
};

//CUT-END
