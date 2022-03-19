#pragma once
#ifndef MULAN_STRING_ERRORS
#define MULAN_STRING_ERRORS

#include <string>

//CUT-START

namespace mls {
	
	class InvalidTemplateState : public std::exception {
			const std::string err;
		public:
			InvalidTemplateState(std::string errString);
			const char* what() const noexcept override;
	};
	
};

//CUT-END

#endif //!MULAN_STRING_ERRORS
