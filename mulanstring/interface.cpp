
#include "interface.hpp"
#include "backend.hpp"
#include <string>

#include <iostream>

namespace mls {
	
	///strips the charset and flavour from the locale string
	std::string parseLocaleString(const char* locale) {
		size_t stop = 0;
		while( locale[stop] != '\0' ) {
			if( locale[stop] == '.' ) break; //charset begin
			if( locale[stop] == '@' ) break; //flavour begin
			++stop;
		}
		std::string strLoc(locale, stop);
		
		if( strLoc == "C" || strLoc == "POSIX" )
			return std::string("en_US");
		else 
			return strLoc;
	}
	
	void initLocale(const char* locale) {
		std::string strLoc = parseLocaleString(locale);
		
		if( auto found = FeaturesByLang.find(strLoc); found != FeaturesByLang.end() ) {
			defaultLanguage = &(found->second);
		} else {
			defaultLanguage = &FeaturesByLang["en_US"];
		}
	}
	
	Template translate(const char* message) {
		if( defaultLanguage == nullptr)
			throw LibraryNotInitialized();
		
		return Template{ backend::getTranslation(message), defaultLanguage};
	}
	Template translate(const char* catalogue, const char* message) {
		if( defaultLanguage == nullptr)
			throw LibraryNotInitialized();
		
		return Template{ backend::getTranslation(catalogue, message), defaultLanguage};
	}
	Template translateWithLocale(const char* message, const char* locale) {
		std::string strLoc = parseLocaleString(locale);
		Feature* language;
		
		if( auto found = FeaturesByLang.find(strLoc.c_str()); found != FeaturesByLang.end() ) {
			language = &(found->second);
		} else {
			language = &FeaturesByLang["en_US"];
		}
		
		return Template{ backend::getTranslation(message), language};
	}
	Template translateWithLocale(const char* catalogue, const char* message, const char* locale) {
		std::string strLoc = parseLocaleString(locale);
		Feature* language;
		
		if( auto found = FeaturesByLang.find(strLoc.c_str()); found != FeaturesByLang.end() ) {
			language = &(found->second);
		} else {
			language = &FeaturesByLang["en_US"];
		}
		
		return Template{ backend::getTranslation(catalogue, message), language};
	}
}