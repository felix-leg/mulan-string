
#include "interface.hpp"
#include "backend.hpp"
#include <string>


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
		for(auto& feature : FeaturesByLang) {
			unsigned int nameNo = 0;
			std::string nameStr;
			while( (nameStr = getNthOnSClist(nameNo,feature.second.langName)) != "" ) {
				if( nameStr == strLoc ) {
					//locale found
					defaultLanguage = &(feature.second);
					return;
				}
				++nameNo;
			}
		}
		//fallback
		defaultLanguage = &FeaturesByLang["American English"];
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
		Feature* language = nullptr;
		
		for(auto& feature : FeaturesByLang) {
			unsigned int nameNo = 0;
			std::string nameStr;
			while( (nameStr = getNthOnSClist(nameNo,feature.second.langName)) != "" ) {
				if( nameStr == strLoc ) {
					//locale found
					language = &(feature.second);
					break;
				}
				++nameNo;
			}
			if( language != nullptr ) break;
		}
		//fallback
		if( language == nullptr )
			language = &FeaturesByLang["American English"];
		
		return Template{ backend::getTranslation(message), language};
	}
	Template translateWithLocale(const char* catalogue, const char* message, const char* locale) {
		std::string strLoc = parseLocaleString(locale);
		Feature* language = nullptr;
		
		for(auto& feature : FeaturesByLang) {
			unsigned int nameNo = 0;
			std::string nameStr;
			while( (nameStr = getNthOnSClist(nameNo,feature.second.langName)) != "" ) {
				if( nameStr == strLoc ) {
					//locale found
					language = &(feature.second);
					break;
				}
				++nameNo;
			}
			if( language != nullptr ) break;
		}
		//fallback
		if( language == nullptr )
			language = &FeaturesByLang["American English"];
		
		return Template{ backend::getTranslation(catalogue, message), language};
	}
}
