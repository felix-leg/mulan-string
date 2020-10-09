#pragma once

#ifndef MULANSTR_LANGUAGE_FEATURES_HPP
#define MULANSTR_LANGUAGE_FEATURES_HPP

#include <map>
#include <string>

namespace mls {
	
	struct Feature {
			const char* langName;
			const char* decimalSeparator;
			const char* thousandsSeparator;
			///function returning name of a plural based on a number
			const char* (*pluralizer)(long numberAsInt, std::string numberAsString); 
			const char* casesList; //normal order of cases as a string with ';'-separated list
			const char* genderList; //normal order of genders as a string with ';'-separated list
			const char* pluralsList; //normal order of plurals as a string with ';'-separated list
	};
	
	///returns the Nth element on a semicolon separated list
	std::string getNthOnSClist(const unsigned int n, const char* list, const char* defaultValue = "");
	
	extern std::map<std::string, Feature> FeaturesByLang;
	
}

#endif /* MULANSTR_LANGUAGE_FEATURES_HPP */
//file end
