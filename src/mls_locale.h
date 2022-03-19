#pragma once
#ifndef MULAN_STRING_LOCALE
#define MULAN_STRING_LOCALE

#include <string>
#include <string_view>
#include <initializer_list>
#include <utility>
#include <vector>
#include <map>

// cSpell: words pluralizer
//CUT-START

namespace mls::locale {
	
	class NumberFormat {
		public:
			NumberFormat(
				std::string groupChar, 
				std::string fractionChar,
				std::initializer_list<unsigned char> schema
				);
			
			std::string formatInteger(long integer) const;
			std::string formatReal(double real, short precision = -1) const;
		private:
			std::string integerGroupingChar;
			std::string fractionSeparator;
			std::vector<unsigned char> groupingSchema;
	};
	
	typedef std::string (*pluralizer)(long, std::string&);
	
	class Locale {
		std::string_view myName;
		pluralizer pluralFunction;
		std::vector<const char*> pluralsList;
		std::vector<const char*> casesList;
		std::vector<const char*> gendersList;
		std::map<std::string, NumberFormat> numFormats;
		public:
			Locale(
				std::string_view name,
				std::initializer_list<const char*> pluralForms,
				pluralizer pluralFn,
				std::initializer_list<const char*> cases,
				std::initializer_list<const char*> genders,
				std::initializer_list<std::pair<std::string, NumberFormat>> numberFormats
			);
			
			bool isTheLocale(std::string_view localeName) const;
			
			const std::vector<const char*>& getCasesList() const;
			const std::vector<const char*>& getGendersList() const;
			const std::vector<const char*>& getPluralsList() const;
			
			std::string getPluralID(long number) const;
			NumberFormat * getNumberFormat(std::string_view name);
	};
	
	extern std::vector<Locale> localesList;
	Locale& getLocale(std::string_view localeName);
};

//CUT-END

#endif //!MULAN_STRING_LOCALE
