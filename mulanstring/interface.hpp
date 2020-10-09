#pragma once

#ifndef MULANSTR_INTERFACE_HPP
#define MULANSTR_INTERFACE_HPP

#include "language_features.hpp"
#include "template_object.hpp"

namespace mls {
	
	class LibraryNotInitialized : std::exception {};
	
	#ifdef MULANSTR_MAIN_HPP
	extern Feature* defaultLanguage;
	#else
	Feature* defaultLanguage = nullptr;
	#endif
	
	///Initializes the MLS library with default language
	void initLocale(const char* locale);
	
	///gets translation from the backend and returns MLS template object
	Template translate(const char* message);
	Template translate(const char* catalogue, const char* message);
	Template translateWithLocale(const char* message, const char* locale);
	Template translateWithLocale(const char* catalogue, const char* message, const char* locale);
}

#endif /* MULANSTR_INTERFACE_HPP */
//file end
