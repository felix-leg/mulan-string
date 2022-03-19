/**
 * @file locale.cpp
 * @brief Locale class with options for each supported language
 * 
 */

#include "mls_locale.h"
#include <charconv>
#include <cmath>

// cSpell: words pluralizer
//CUT-START

namespace mls::locale {
	
	Locale::Locale(
		std::string_view name,
		std::initializer_list<const char*> pluralForms,
		pluralizer pluralFn,
		std::initializer_list<const char*> cases,
		std::initializer_list<const char*> genders,
		std::initializer_list<std::pair<std::string, NumberFormat>> numberFormats
	) {
		myName = name;
		pluralFunction = pluralFn;
		for(const char* pl : pluralForms) {
			pluralsList.push_back(pl);
		}
		pluralsList.shrink_to_fit();
		for(const char* caseName : cases) {
			casesList.push_back(caseName);
		}
		casesList.shrink_to_fit();
		for(const char* gender : genders) {
			gendersList.push_back(gender);
		}
		gendersList.shrink_to_fit();
		for(auto format : numberFormats) {
			numFormats.emplace( format.first, format.second );
		}
	}
	
	bool Locale::isTheLocale(std::string_view localeName) const {
		return myName == localeName;
	}
	
	const std::vector<const char*>& Locale::getCasesList() const {
		return casesList;
	}
	
	const std::vector<const char*>& Locale::getGendersList() const {
		return gendersList;
	}
	
	const std::vector<const char*>& Locale::getPluralsList() const {
		return pluralsList;
	}
	
	NumberFormat * Locale::getNumberFormat(std::string_view name) {
		for(auto& format : numFormats) {
			if( format.first == name ) {
				return &(format.second);
			}
		}
		//no match
		return nullptr;
	}
	
	std::string Locale::getPluralID(long number) const {
		std::string asString = std::to_string(number);
		return pluralFunction(number, asString);
	}
	
	NumberFormat::NumberFormat(
		std::string groupChar, 
		std::string fractionChar,
		std::initializer_list<unsigned char> schema
		) {
		integerGroupingChar = groupChar;
		fractionSeparator = fractionChar;
		for(unsigned char s : schema) {
			groupingSchema.push_back(s);
		}
		groupingSchema.shrink_to_fit();
	}
	
	std::string NumberFormat::formatInteger(long integer) const {
		char intStr[50];
		bool negative = integer < 0;
		auto[lastPtr, err] = std::to_chars(&intStr[0], &intStr[49], std::abs(integer), 10);
		
		std::string result;
		
		if( groupingSchema.size() == 0u ) {
			//no grouping
			result = std::string(&intStr[0], lastPtr - &intStr[0]);
		} else {
			unsigned char currentGroup = groupingSchema[0];
			unsigned char groupNo = 0u;
			char* charGroup;
			
			while( lastPtr != &intStr[0] ) {
				if( groupNo < groupingSchema.size() - 1 ) {
					++groupNo;
					currentGroup = groupingSchema[groupNo];
				}
				
				if( lastPtr - &intStr[0] <= currentGroup ) {
					result = std::string(&intStr[0], lastPtr - &intStr[0]) + result;
					lastPtr = &intStr[0];
					break;
				} else {
					charGroup = lastPtr - currentGroup;
					result = std::string(charGroup, currentGroup) + result;
					lastPtr = charGroup;
				}
				
				if( lastPtr != &intStr[0]) {
					result = integerGroupingChar + result;
				}
			}
		}
		
		if(negative)
			return std::string("-") + result;
		else
			return result;
	}
	
	std::string NumberFormat::formatReal(double real, short precision) const {
		long integerPart = static_cast<long>(real);
		real -= integerPart; //only fractions
		
		if( precision == 0 ) {
			return formatInteger(integerPart);
		}
		
		if( precision > 0 ) {
			const double modifer = std::pow(10.0, precision);
			real = std::round( real * modifer ) / modifer;
		}
		
		std::string result = formatInteger(integerPart);
		
		if( real == 0.0 )
			return result;
		
		char realStr[50];
		auto[lastPtr, err] = std::to_chars(&realStr[0], &realStr[49], real, std::chars_format::fixed);
		
		char* dotPos = &realStr[0];
		while( dotPos != lastPtr ) {
			if( *dotPos == '.' ) {
				++dotPos;
				break;
			}
			++dotPos;
		}
		
		result = result + fractionSeparator + std::string(dotPos, lastPtr - dotPos);
		return result;
	}
	
	// helper functions used by pluralizer functions
	bool hasEnding (std::string const &fullString, std::string_view ending) {
		if (fullString.length() >= ending.length()) {
			return (fullString.compare (fullString.length() - ending.length(), ending.length(), ending) == 0);
		} else {
			return false;
		}
	}

	int twoDigitsEnding(std::string &number) {
		if(number.length() >= 2)
			return std::stoi(number.substr(number.length() - 2));
		else
			return  std::stoi(number);
	}
	/*
	
	===================== Plurals ======================
	based on: https://developer.mozilla.org/en-US/docs/Mozilla/Localization/Localization_and_Plurals
	
	*/

	// cSpell: disable

	//Families: Asian (Chinese, Japanese, Korean), Persian, Turkic/Altaic (Turkish), Thai, Lao
	std::string PluralRule0(long numberAsInt, std::string &numberAsString) {
		return "other";
	}

	//Families: Germanic, Finno-Ugric, Language isolate, Latin/Greek, Semitic, Romanic, Vietnamese
	std::string PluralRule1(long numberAsInt, std::string &numberAsString) {
		if(numberAsInt == 1) return "one";
		return "other";
	}

	//Families: Romanic (French, Brazilian Portuguese), Lingala
	std::string PluralRule2(long numberAsInt, std::string &numberAsString) {
		if( numberAsInt == 1 || numberAsInt == 0) return "zero_one";
		return "other";
	}

	//Families: Baltic (Latvian, Latgalian)
	std::string PluralRule3(long numberAsInt, std::string &numberAsString) {
		if( hasEnding(numberAsString,"0")) return "zero";
		if( hasEnding(numberAsString,"1") && !hasEnding(numberAsString,"11")) return "one";
		return "other";
	}

	//Families: Celtic (Scottish Gaelic)
	std::string PluralRule4(long numberAsInt, std::string &numberAsString) {
		if( numberAsInt == 1 || numberAsInt == 11 ) return "one";
		if( numberAsInt == 2 || numberAsInt == 12 ) return "two";
		if( 
			(numberAsInt >= 3 && numberAsInt <= 10)
			||
			(numberAsInt >= 13 && numberAsInt <= 19)
			) return "three";
		return "other";
	}

	//Families: Romanic (Romanian)
	std::string PluralRule5(long numberAsInt, std::string &numberAsString) {
		if( numberAsInt == 1 ) return "one";
		int ending = twoDigitsEnding(numberAsString);
		if( numberAsInt == 0 || (ending >= 1 && ending <= 19) ) return "few";
		return "other";
	}

	//Families: Baltic (Lithuanian)
	std::string PluralRule6(long numberAsInt, std::string &numberAsString) {
		if( numberAsInt != 11 && hasEnding(numberAsString,"1") ) return "one";
		int ending = twoDigitsEnding(numberAsString);
		if( hasEnding(numberAsString,"0") ||
			(ending >= 11 && ending <= 19)
		) return "few";
		return "other";
	}

	//Families: Belarusian, Russian, Ukrainian
	std::string PluralRule7(long numberAsInt, std::string &numberAsString) {
		if( hasEnding(numberAsString,"1") && numberAsInt != 11 ) return "one";
		int twoDigits = twoDigitsEnding(numberAsString);
		if(
			!( twoDigits >= 12 && twoDigits <= 14 )
			&&
			( hasEnding(numberAsString,"2") || hasEnding(numberAsString,"3") || hasEnding(numberAsString,"4") )
		) return "few";
		return "other";
	}

	//Families: Slavic (Slovak, Czech)
	std::string PluralRule8(long numberAsInt, std::string &numberAsString) {
		if( numberAsInt == 1 ) return "one";
		if( numberAsInt >=2 && numberAsInt <= 4) return "few";
		return "other";
	}

	//Families: Slavic (Polish)
	std::string PluralRule9(long numberAsInt, std::string &numberAsString) {
		if( numberAsInt == 1 ) return "one";
		if(
			!( hasEnding(numberAsString,"12") || hasEnding(numberAsString,"13") || hasEnding(numberAsString,"14") )
			&&
			( hasEnding(numberAsString,"2") || hasEnding(numberAsString,"3") || hasEnding(numberAsString,"4") )
		) return "few";
		return "other";
	}

	//TODO: make rules for the 10-19 positions
	
	/*
	
	=========================== Supported locales =====================
	
	*/
	
	std::vector<Locale> localesList{
		//British English
		{"en_GB", {"one","other"}, PluralRule1, {}, {}, {
			{"general", {",", ".", {}}},
			{"grouped", {",", ".", {3}}}
		}},
		//American English
		{"en_US", {"one","other"}, PluralRule1, {}, {}, {
			{"general", {",", ".", {}}},
			{"grouped", {",", ".", {3}}}
		}},
		//Polish
		/*
		(nom)inative = mianownik(kto? co?)
		(gen)etive = dopełniacz(kogo? czego?)
		(dat)ive = celownik (komu? czemu?)
		(acc)usative = biernik (kogo? co?)
		(ins)trumental = narzędnik (z kim? z czym?)
		(loc)ative = miejscownik (o kim? o czym?)
		(voc)ative = wołacz (O!)
		*/
		{"pl_PL", {"one","few","other"}, PluralRule9, 
		{"nom","gen","dat","acc","ins","loc","voc"}, {"m","f","n"}, {
			{"general", {" ", ",", {}}},
			{"grouped", {" ", ",", {3}}}
		}}
	};
		
	Locale& getLocale(std::string_view localeName) {
		Locale* found = nullptr;
		for( auto& locale : localesList ) {
			if( locale.isTheLocale(localeName) ) {
				found = &locale;
			}
		}
		
		if( found != nullptr )
			return *found;
		else
			return getLocale("en_US");
	}
	
};

//CUT-END
