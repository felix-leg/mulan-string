#ifndef MULAN_STRING
#define MULAN_STRING
/**
MIT License

Copyright (c) 2022 Przemysław Chojnacki

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

**/


#include <string>
#include <charconv>
#include <cmath>
#include <string>
#include <string_view>
#include <initializer_list>
#include <utility>
#include <vector>
#include <map>
#include <sstream>
#include <cctype>
#include <variant>



#include <libintl.h>
#include <locale.h>




namespace mls {
	
	class InvalidTemplateState : public std::exception {
			const std::string err;
		public:
			InvalidTemplateState(std::string errString);
			const char* what() const noexcept override;
	};
	
};



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



namespace mls::preparse {
	
	class ParsedTemplateFunction {
		public:
			char name[2];
			std::string varName;
			
			enum class Type {
				VAR_ONLY,
				ONE_ARG,
				TABLE_ARG,
				HASH_ARG
			};
			virtual Type getType() const =0;
			
			virtual ~ParsedTemplateFunction() {};
	};
	
	class VarOnlyFunction : public ParsedTemplateFunction {
		public:
			virtual Type getType() const override;
	};
	
	class OneArgFunction : public ParsedTemplateFunction {
		public:
			virtual Type getType() const override;
			
			std::string argument;
			
			OneArgFunction(std::string_view templateContent);
	};
	
	class TableArgFunction : public ParsedTemplateFunction {
		public:
			virtual Type getType() const override;
			
			std::vector<std::string> arguments;
			
			TableArgFunction(std::string_view templateContent);
	};
	
	class HashArgFunction : public ParsedTemplateFunction {
		public:
			virtual Type getType() const override;
			
			std::map<std::string, std::string> arguments;
			
			HashArgFunction(std::string_view templateContent);
	};
	
	std::pair<std::vector<char*>, std::vector<ParsedTemplateFunction*>>
		preparse_template(std::string_view templateString);
	
};



namespace mls {
	
	class Template;
	typedef std::variant<std::string_view, long, double, Template*> variableValue;
	
	///interface for template functions
	class TemplateFunction {
		public:
			virtual std::string produceString(std::map<std::string_view, variableValue> &variables) = 0;
	};//!class TemplateFunction
	
	///Template class used to make string out of a template string and a locale
	class Template {
		public:
			///default ctor
			Template();
			///no copy ctor
			Template(const Template& other) = delete;
			///move ctor
			Template(Template&& other);
			///standard ctor
			Template(const std::string templateString, locale::Locale& locale);
			~Template();
			
			//apply function group
			
			///set a raw string for a variable
			Template& apply(std::string_view varName, std::string_view rawString);
			///set an integer number for a variable
			Template& apply(std::string_view varName, long number);
			///set a real number for a variable
			Template& applyReal(std::string_view varName, double number);
			///set another template for a variable
			Template& apply(std::string_view varName, Template& t);
			
			//end apply
			
			///runs the template and produces a result string
			std::string get();
			
			std::string getGender();
		private:
			locale::Locale *myLocale;
			std::string genderID;
			std::vector<char*> stringsList;
			std::map<std::string_view, variableValue> variables;
			std::vector<TemplateFunction*> functionsList;
			
	};//!class Template
	
};



namespace mls::backend {
	
	class IntlNotInitialized : public std::exception {};
	
	/**
	 * @brief initialize the GetText utility
	 * 
	 * @param packageName Name of the GetText unit to pass to `bindtextdomain(...)`
	 * @param wantedLocale Force select a specific locale. Pass `nullptr` to use system locale
	 * @param folderLookup Setup a specific folder to look for `*.mo` files. Pass `nullptr` to use the default `/usr/local/share/locale`
	 */
	void init(
		const char* packageName, 
		const char* wantedLocale = nullptr, 
		const char* folderLookup = nullptr
		);
};

namespace mls {
	
	///Get template for `msgid` string
	Template translate(const char* msgid);
	
	///Get template for `msgid` string in a .mo `catalog`
	Template translate(const char* catalog, const char* msgid);
	
};

#ifndef MULANSTR_DONT_USE_UNDERSCORE

#	define _(msgid) mls::translate(msgid)
#	define _c(catalog, msgid) mls::translate(catalog, msgid)

#endif





#ifdef MULAN_STRING_IMPLEMENTATION




namespace mls {
	
	InvalidTemplateState::InvalidTemplateState(std::string errString): err{errString} {};
	const char* InvalidTemplateState::what() const noexcept {
		return err.c_str();
	}
	
};



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



namespace mls::preparse {
	
	//------------ Configuration
	
	#ifdef MULANSTR_TAG_START
	constexpr std::string_view TEMPLATE_START{MULANSTR_TAG_START};
	#else
	constexpr std::string_view TEMPLATE_START{"%{"};
	#endif
	
	#ifdef MULANSTR_TAG_END
	constexpr std::string_view TEMPLATE_END{MULANSTR_TAG_END};
	#else
	constexpr std::string_view TEMPLATE_END{"}%"};
	#endif
	
	#ifdef MULANSTR_INNER_TAG_START
	constexpr std::string_view INNER_TAG_START{MULANSTR_INNER_TAG_START};
	#else
	constexpr std::string_view INNER_TAG_START{"{"};
	#endif
	
	#ifdef MULANSTR_INNER_TAG_END
	constexpr std::string_view INNER_TAG_END{MULANSTR_INNER_TAG_END};
	#else
	constexpr std::string_view INNER_TAG_END{"}"};
	#endif
	
	//-------------- Utility functions (ref)
	
	std::pair<std::string_view, std::string_view> 
		breakInHalfOn( std::string_view source, std::string_view delimiter );
	
	bool contains( std::string_view container, std::string_view searched );
	
	char *copyStringView( const std::string_view theView );
	std::string copyStringViewAsString( const std::string_view theView );
	
	std::string_view trim(std::string_view theView);
	
	//-------------- The main function
	
	ParsedTemplateFunction* parse_template_contents(std::string_view content);
	
	std::pair<std::vector<char*>, std::vector<ParsedTemplateFunction*>>
		preparse_template(std::string_view templateString) 
	{
		std::vector<char*> resultStrings;
		std::vector<ParsedTemplateFunction*> resultFunctions;
		
		//check if we have got a template string
		if( !contains(templateString, TEMPLATE_START) ) {
			//not a template - just copy
			resultStrings.push_back( copyStringView(templateString) );
		} else {
			//a rightful template
			while( ! templateString.empty() ) {
				auto[prefix, firstHalf] = breakInHalfOn(templateString, TEMPLATE_START);
				resultStrings.push_back( copyStringView(prefix) );
				
				if( !contains(firstHalf, TEMPLATE_END) ) {//Wrong template
					#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
					//clean the result vars
					for(char* el : resultStrings) {
						delete[] el;
					}
					for(auto el : resultFunctions) {
						delete el;
					}
					throw InvalidTemplateState{"No ending marker"};
					#else
					return {resultStrings, resultFunctions};
					#endif
				}
				
				auto[templateContent, suffix] = breakInHalfOn(firstHalf, TEMPLATE_END);
				
				try{
					resultFunctions.push_back( parse_template_contents(templateContent) );
				}
				catch(InvalidTemplateState e) {
					//clean the result vars
					for(char* el : resultStrings) {
						delete[] el;
					}
					for(auto el : resultFunctions) {
						delete el;
					}
					throw e;
				}
				
				if( contains(suffix, TEMPLATE_START) ) {//next template?
					templateString = suffix;
				} else {
					//add the last string
					resultStrings.push_back( copyStringView(suffix) );
					templateString = std::string_view();
				}
			}
		}
		
		return {resultStrings, resultFunctions};
	}
	
	constexpr std::string_view VAR_FN_DIVIDER{"!"};
	constexpr std::string_view NO_VAR_FN{"+"};
	constexpr char COMMENT_MARKER = '#';
	
	constexpr std::string_view FN_ARGS_DIV_ALL{"=: "};
	constexpr char FN_ARGS_DIV_ONE = '=';
	constexpr char FN_ARGS_DIV_TABLE = ':';
	constexpr char FN_ARGS_DIV_HASH = ' ';
	
	ParsedTemplateFunction* parse_template_contents(std::string_view content) {
		std::string varName{""};
		bool hasFn = false;
		//check for comments
		if( content.starts_with(COMMENT_MARKER) && content.ends_with(COMMENT_MARKER) ) {
			return nullptr; 
		}
		//extract var name
		if( content.starts_with(NO_VAR_FN) ) {
			//this is a no-var function
			content.remove_prefix(NO_VAR_FN.size());
			hasFn = true;
		} else {
			//look for name
			if( contains(content, VAR_FN_DIVIDER) ) {
				auto[varNameVS, theRest] = breakInHalfOn(content, VAR_FN_DIVIDER);
				varName = copyStringViewAsString(varNameVS);
				content = theRest;
				hasFn = true;
			} else {
				//only var name
				varName = copyStringViewAsString(content);
			}
		}
		//return the right function class
		if( !hasFn ) {
			VarOnlyFunction* result = new VarOnlyFunction();
			result->varName = std::move(varName);
			result->name[0] = result->name[1] = '\0';
			
			return result;
		} else {
			auto nameArgsDivider = content.find_first_of(FN_ARGS_DIV_ALL);
			
			if( nameArgsDivider == std::string_view::npos ) {
				//no required element - error
				#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
				throw InvalidTemplateState("No function-parameters divider");
				#else
				return nullptr;
				#endif
			}
			
			if( nameArgsDivider != 1 && nameArgsDivider != 2 ) {
				//wrong function name length
				#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
				throw InvalidTemplateState("Wrong function name length");
				#else
				return nullptr;
				#endif
			}
			
			ParsedTemplateFunction* result;
			try{
				switch( content[nameArgsDivider] ) {
					case FN_ARGS_DIV_ONE:
						result = new OneArgFunction(content.substr(nameArgsDivider+1));
						break;
					case FN_ARGS_DIV_TABLE:
						result = new TableArgFunction(content.substr(nameArgsDivider+1));
						break;
					case FN_ARGS_DIV_HASH:
						result = new HashArgFunction(content.substr(nameArgsDivider+1));
						break;
					default: //shouldn't reach
						#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
						throw InvalidTemplateState("Unknown function-parameters divider");
						#else
						return nullptr;
						#endif
				}
			} catch(InvalidTemplateState e) {
				#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
				throw e;
				#else
				return nullptr;
				#endif
			}
			result->varName = std::move(varName);
			result->name[0] = content[0];
			if( nameArgsDivider == 2 )
				result->name[1] = content[1];
			else
				result->name[1] = '\0';
			
			return result;
		}
	}
	
	//-------------- Utility functions
	
	std::pair<std::string_view, std::string_view> 
		breakInHalfOn( std::string_view source, std::string_view delimiter )
	{
		auto delimPos = source.find( delimiter );
		if( delimPos != source.npos ) {
			return {
				source.substr(0, delimPos),
				source.substr(delimPos + delimiter.size())
			};
		} else {
			return {
				source,
				std::string_view()
			};
		}
	}
	
	bool contains( std::string_view container, std::string_view searched )
	{
		auto pos = container.find( searched );
		if( pos == container.npos )
			return false;
		else
			return true;
	}
	
	char *copyStringView( const std::string_view theView )
	{
		char *result = new char[theView.size() + 1];
		theView.copy( result, theView.size() );
		result[theView.size()] = 0;
		return result;
	}
	
	std::string copyStringViewAsString( const std::string_view theView ) {
		char* copy = copyStringView( theView );
		std::string strCopy{copy};
		delete[] copy;
		return strCopy;
	}
	
	std::string_view trim(std::string_view theView) {
		auto pos = theView.find_first_not_of(" ");
		if( pos != std::string_view::npos ) {
			theView.remove_prefix(pos);
		}
		pos = theView.find_first_of(" ");
		if( pos != std::string_view::npos ) {
			theView.remove_suffix(theView.size() - pos);
		}
		return theView;
	}
	
	//-------------- class methods
	
	ParsedTemplateFunction::Type VarOnlyFunction::getType() const {
		return ParsedTemplateFunction::Type::VAR_ONLY;
	}
	
	ParsedTemplateFunction::Type OneArgFunction::getType() const {
		return ParsedTemplateFunction::Type::ONE_ARG;
	}
	
	ParsedTemplateFunction::Type TableArgFunction::getType() const {
		return ParsedTemplateFunction::Type::TABLE_ARG;
	}
	
	ParsedTemplateFunction::Type HashArgFunction::getType() const {
		return ParsedTemplateFunction::Type::HASH_ARG;
	}
	
	OneArgFunction::OneArgFunction(std::string_view templateContent) {
		if( templateContent.empty() ) {
			throw InvalidTemplateState("Empty argument");
		}
		argument = copyStringViewAsString(templateContent);
	}
	
	TableArgFunction::TableArgFunction(std::string_view templateContent) {
		if( templateContent.empty() ) {
			throw InvalidTemplateState("Empty argument list");
		}
		
		std::string_view::size_type pos{0};
		while( pos != std::string_view::npos ) {
			pos = templateContent.find(",");
			if( pos != std::string_view::npos ) {
				arguments.push_back(
					copyStringViewAsString(
						templateContent.substr(0, pos)
					)
				);
				templateContent.remove_prefix(pos+1);
			} else {
				arguments.push_back(
					copyStringViewAsString( templateContent )
				);
			}
		}
	}
	
	HashArgFunction::HashArgFunction(std::string_view templateContent) {
		if( templateContent.empty() ) {
			throw InvalidTemplateState("Empty argument hash");
		}
		
		std::string_view::size_type pos;
		while( !templateContent.empty() ) {
			pos = templateContent.find_first_not_of(" ");
			if( pos == std::string_view::npos ) {//no more arguments
				break;
			} else {
				templateContent.remove_prefix(pos-1);
			}
			
			std::string argName{""};
			pos = templateContent.find("=");
			if( pos == std::string_view::npos ) {
				//no required element
				throw InvalidTemplateState("No required \"=\" sign");
			}
			argName = copyStringViewAsString(
				trim( templateContent.substr(0, pos) )
			);
			templateContent.remove_prefix(pos);
			
			std::string argContent{""};
			pos = templateContent.find(INNER_TAG_START);
			if( pos == std::string_view::npos ) {
				//no required element
				throw InvalidTemplateState("No inner tag start");
			}
			templateContent.remove_prefix(pos + INNER_TAG_START.size());
			pos = templateContent.find(INNER_TAG_END);
			if( pos == std::string_view::npos ) {
				//no required element
				throw InvalidTemplateState("No inner tag end");
			}
			argContent = copyStringViewAsString(
				templateContent.substr(0, pos)
			);
			templateContent.remove_prefix(pos + INNER_TAG_END.size());
			
			arguments.emplace(argName, argContent);
		}
	}
	
};



namespace mls {
	
	//------------- Functions classes (ref)
	
	/**
	 * @brief Just puts the variable content
	 * 
	 */
	class VariablePutter : public TemplateFunction {
		private:
			const std::string variable;
		public:
			VariablePutter(std::string &&varName): variable{varName} {};
			
			virtual std::string produceString(std::map<std::string_view, variableValue> &variables) override;
	};
	
	/**
	 * @brief Outputs a result based on the gender of the variable
	 * 
	 * Template: `%{varname!G:output1,output2,...}%`
	 * 	OR
	 * Template: `%{varname!G gender1={output1} gender2={output2} ...}%`
	 */
	class GenderFunction : public TemplateFunction {
		private:
			const std::string variable;
			std::map<std::string, std::string> outputTexts;
		public:
			GenderFunction(
				std::string &&varName,
				std::vector<std::string> &listOfOutputs,
				const locale::Locale &locale
			);
			GenderFunction(
				std::string &&varName,
				std::map<std::string, std::string> &mapOfOutputs
			);
			
			virtual std::string produceString(std::map<std::string_view, variableValue> &variables) override;
	};
	
	/**
	 * @brief Writes an output based on the `__CASE__` variable
	 * 
	 */
	class CaseWriterFunction : public TemplateFunction {
		private:
			std::map<std::string, std::string> outputTexts;
		public:
			CaseWriterFunction(
				std::vector<std::string> &listOfOutputs,
				const locale::Locale &locale
			);
			CaseWriterFunction(
				std::map<std::string, std::string> &mapOfOutputs
			);
			
			virtual std::string produceString(std::map<std::string_view, variableValue> &variables) override;
	};
	
	/**
	 * @brief Sets the `__CASE__` variable in the sub-template
	 * 
	 */
	class CaseChooserFunction : public TemplateFunction {
		private:
			const std::string caseID;
			const std::string variable;
		public:
			CaseChooserFunction(
				std::string &&varName,
				std::string &&case_id
			);
			
		virtual std::string produceString(std::map<std::string_view, variableValue> &variables) override;
	};
	
	/**
	 * @brief Pluralizes a word based on a numeric value in the variable
	 * 
	 */
	class PluralFunction : public TemplateFunction {
		private:
			const locale::Locale &theLocale;
			const std::string variable;
			std::map<std::string, std::string> outputTexts;
		public:
			PluralFunction(
				std::string &&varName,
				std::vector<std::string> listOfOutputs,
				const locale::Locale &locale
			);
			PluralFunction(
				std::string &&varName,
				std::map<std::string, std::string> &mapOfOutputs,
				const locale::Locale &locale
			);
			
			virtual std::string produceString(std::map<std::string_view, variableValue> &variables) override;
	};
	
	/**
	 * @brief Formats an integer according to the locale's formater.
	 * 
	 */
	class IntegerFormaterFunction : public TemplateFunction {
		private:
			const locale::NumberFormat &formater;
			const std::string variable;
		public:
			IntegerFormaterFunction(
				std::string &&varName,
				std::string &formatName,
				locale::Locale &theLocale
			);
			
			virtual std::string produceString(std::map<std::string_view, variableValue> &variables) override;
	};
	
	/**
	 * @brief Formats a real number according to the locale's formater.
	 * 
	 */
	class RealFormaterFunction : public TemplateFunction {
		private:
			const std::string variable;
			short precision;
			locale::NumberFormat *formater;
			
			bool isAllNumber(std::string &str);
		public:
			RealFormaterFunction(
				std::string &&varName,
				std::vector<std::string> &listOfOptions,
				locale::Locale &theLocale
			);
			RealFormaterFunction(
				std::string &&varName,
				std::map<std::string, std::string> &hashOfOptions,
				locale::Locale &theLocale
			);
			
			virtual std::string produceString(std::map<std::string_view, variableValue> &variables) override;
	};
	
	//------------- Template class
	
	Template::Template():myLocale{nullptr}, genderID{""} {
		//do nothing else
	}
	
	Template::Template(Template&& other):genderID{""} {
		myLocale = other.myLocale;
		other.myLocale = nullptr;
		
		for(char* aString : other.stringsList) {
			stringsList.push_back(aString);
		}
		other.stringsList.clear();
		
		for(auto aFunction : other.functionsList) {
			functionsList.push_back(aFunction);
		}
		other.functionsList.clear();
	}
	
	Template::Template(const std::string templateString, locale::Locale& locale):
		myLocale{&locale} {
		using namespace preparse;
		using Type = ParsedTemplateFunction::Type;
		auto[charList, fnList] = preparse_template(templateString);
		
		try{
			for(char* aString : charList) {
				stringsList.push_back(aString);
			}
			
			for(auto& functionDesc : fnList) {
				//test for comments
				if( functionDesc == nullptr ) {
					functionsList.push_back(nullptr);
					continue;
				}
				//select function
				TemplateFunction* function = nullptr;
				switch( functionDesc->name[0] ) {
					case '\0': //Variable put
						if( functionDesc->getType() != Type::VAR_ONLY) {
							throw InvalidTemplateState{"Wrong type of function"};
						}
						function = new VariablePutter(std::move(functionDesc->varName));
						break;
					case 'S': //Setter of...
						switch( functionDesc->name[1] ) {
							case 'G': //gender
								if( functionDesc->getType() == Type::ONE_ARG ) {
									genderID = static_cast<OneArgFunction*>(
										functionDesc
									)->argument;
								} else {
									throw InvalidTemplateState("Gender setter has invalid argument type");
								}
								break;
							default:
								throw InvalidTemplateState(std::string{"Unknown function: "} + functionDesc->name[0] + functionDesc->name[1]);
						}
						break;
					case 'G': //Gender chooser
						if( functionDesc->varName.empty() ) {
							throw InvalidTemplateState("Variable name can't be empty");
						}
						if( functionDesc->getType() == Type::TABLE_ARG ) {
							function = new GenderFunction(
								std::move(functionDesc->varName),
								static_cast<TableArgFunction*>(functionDesc)->arguments,
								*myLocale
							);
						} else if( functionDesc->getType() == Type::HASH_ARG ) {
							function = new GenderFunction(
								std::move(functionDesc->varName),
								static_cast<HashArgFunction*>(functionDesc)->arguments
							);
						} else {
							throw InvalidTemplateState("Gender set called with wrong type of arguments");
						}
						break;
					case 'C': //case...
						if( functionDesc->varName.empty() ) {// ...writer
							if( functionDesc->getType() == Type::TABLE_ARG ) {
								function = new CaseWriterFunction(
									static_cast<TableArgFunction*>(functionDesc)->arguments,
									*myLocale
								);
							} else if( functionDesc->getType() == Type::HASH_ARG ) {
								function = new CaseWriterFunction(
									static_cast<HashArgFunction*>(functionDesc)->arguments
								);
							} else {
								throw InvalidTemplateState("Case writer called with wrong type of arguments");
							}
						} else {// ...chooser
							if( functionDesc->getType() == Type::ONE_ARG ) {
								function = new CaseChooserFunction(
									std::move(functionDesc->varName),
									std::move(static_cast<OneArgFunction*>(functionDesc)->argument)
								);
							} else {
								throw InvalidTemplateState("Case chooser called with wrong type of arguments");
							}
						}
						break;
					case 'P'://Plural function
						if( functionDesc->varName.empty() ) {
							throw InvalidTemplateState("Variable name empty");
						}
						
						if( functionDesc->getType() == Type::TABLE_ARG ) {
							function = new PluralFunction(
								std::move(functionDesc->varName),
								static_cast<TableArgFunction*>(functionDesc)->arguments,
								*myLocale
							);
						} else if( functionDesc->getType() == Type::HASH_ARG ) {
							function = new PluralFunction(
								std::move(functionDesc->varName),
								static_cast<HashArgFunction*>(functionDesc)->arguments,
								*myLocale
							);
						} else {
							throw InvalidTemplateState("Plural function called with invalid arguments type");
						}
						break;
					case 'I': //Integer
						if( functionDesc->varName.empty() ) {
							throw InvalidTemplateState("Variable name empty");
						}
						
						if( functionDesc->getType() == Type::ONE_ARG ) {
							function = new IntegerFormaterFunction(
								std::move(functionDesc->varName),
								static_cast<OneArgFunction*>(functionDesc)->argument,
								*myLocale
							);
						} else {
							throw InvalidTemplateState("Integer function called with invalid arguments list");
						}
						break;
					case 'R': //Real
						if( functionDesc->varName.empty() ) {
							throw InvalidTemplateState("Variable name empty");
						}
						
						if( functionDesc->getType() == Type::TABLE_ARG ) {
							function = new RealFormaterFunction(
								std::move(functionDesc->varName),
								static_cast<TableArgFunction*>(functionDesc)->arguments,
								*myLocale
							);
						} else if( functionDesc->getType() == Type::HASH_ARG ) {
							function = new RealFormaterFunction(
								std::move(functionDesc->varName),
								static_cast<HashArgFunction*>(functionDesc)->arguments,
								*myLocale
							);
						} else {
							throw InvalidTemplateState("Real function called with invalid arguments list");
						}
						break;
					default:
						throw InvalidTemplateState(std::string{"Unknown function: "} + functionDesc->name[0] + functionDesc->name[1]);
				}
				//-----
				delete functionDesc;
				functionDesc = nullptr;
				functionsList.push_back(function);
			}
		} catch(InvalidTemplateState e) {
			//clear all data
			for(char* aString : stringsList) {
				delete[] aString;
			}
			stringsList.clear();
			for(auto functionDesc : fnList) {
				delete functionDesc;
			}
			for(auto function : functionsList) {
				delete function;
			}
			functionsList.clear();
			//return an error
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw e;
			#else
			return;
			#endif
		}
	}
	
	Template::~Template() {
		for(char* aString : stringsList) {
			delete[] aString;
		}
		for(auto function : functionsList) {
			delete function;
		}
	}
	
	Template& Template::apply(std::string_view varName, std::string_view rawString) {
		if( myLocale == nullptr ) {
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("Template not initialized");
			#else
			return *this;
			#endif
		}
		variables.insert_or_assign( varName, variableValue{rawString} );
		return *this;
	}
	
	Template& Template::apply(std::string_view varName, long number) {
		if( myLocale == nullptr ) {
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("Template not initialized");
			#else
			return *this;
			#endif
		}
		variables.insert_or_assign( varName, variableValue{number} );
		return *this;
	}
	
	Template& Template::applyReal(std::string_view varName, double number) {
		if( myLocale == nullptr ) {
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("Template not initialized");
			#else
			return *this;
			#endif
		}
		variables.insert_or_assign( varName, variableValue{number} );
		return *this;
	}
	
	Template& Template::apply(std::string_view varName, Template& t) {
		if( myLocale == nullptr ) {
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("Template not initialized");
			#else
			return *this;
			#endif
		}
		variables.insert_or_assign( varName, variableValue{&t} );
		return *this;
	}
	
	std::string Template::get() {
		if( myLocale == nullptr ) {
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("Template not initialized");
			#else
			return "";
			#endif
		}
		std::ostringstream output;
		std::size_t i=0u;
		
		for(i=0u; i<functionsList.size(); ++i) {
			if( stringsList[i] != nullptr ) {
				output << stringsList[i];
			}
			if( functionsList[i] != nullptr ) {
				output << functionsList[i]->produceString(variables);
			}
		}
		//the last string
		if( stringsList[i] != nullptr ) {
			output << stringsList[i];
		}
		
		//clear variables data
		variables.clear();
		
		//return the result
		return output.str();
	}
	
	std::string Template::getGender() {
		return genderID;
	}
	
	//------------- Methods for functions classes
	
	std::string VariablePutter::produceString(std::map<std::string_view, variableValue> &variables) {
		if( !variables.contains(variable) ) {
			//error
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("The variable doesn't exists: " + variable);
			#else
			return "";
			#endif
		}
		
		auto content = variables.at(variable);
		
		if( std::holds_alternative<std::string_view>(content) ) {
			//raw strings are returned as is
			auto sv = std::get<std::string_view>(content);
			char* sc = new char[sv.size()+1];
			sv.copy(sc, sv.size());
			sc[sv.size()] = '\0';
			std::string result{sc};
			delete[] sc;
			
			return result;
		}
		
		if( std::holds_alternative<Template*>(content) ) {
			return std::get<Template*>(content)->get();
		}
		
		//simple number formating
		if( std::holds_alternative<long>(content) ) {
			return std::to_string(
				std::get<long>(content)
			);
		}
		
		if( std::holds_alternative<double>(content) ) {
			return std::to_string(
				std::get<double>(content)
			);
		}
		
		//other types of data are incompatible with this function
		#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
		throw InvalidTemplateState("Unsupported type of data for variable: " + variable);
		#else
		return "";
		#endif
	}
	
	GenderFunction::GenderFunction(
		std::string &&varName,
		std::vector<std::string> &listOfOutputs,
		const locale::Locale &locale
	):variable{varName} {
		std::size_t i;
		
		if( listOfOutputs.size() != locale.getGendersList().size() ) {
			throw InvalidTemplateState("Invalid list of genders");
		}
		
		for(i=0u; i<listOfOutputs.size(); ++i) {
			outputTexts.emplace(
				locale.getGendersList()[i],
				listOfOutputs[i]
			);
		}
	}
	
	GenderFunction::GenderFunction(
		std::string &&varName,
		std::map<std::string, std::string> &mapOfOutputs
	):variable{varName} {
		outputTexts = std::move(mapOfOutputs);
	}
	
	std::string GenderFunction::produceString(std::map<std::string_view, variableValue> &variables) {
		if( !variables.contains(variable) ) {
			//error
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("The variable doesn't exists: " + variable);
			#else
			return "";
			#endif
		}
		
		auto content = variables.at(variable);
		
		if( std::holds_alternative<Template*>(content) ) {
			auto subTemplateGender = std::get<Template*>(content)->getGender();
			
			if( !outputTexts.contains(subTemplateGender) ) {
				//error
				#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
				throw InvalidTemplateState("Unknown gender: " + subTemplateGender);
				#else
				return "";
				#endif
			}
			
			return outputTexts[subTemplateGender];
		}
		
		//other types are incompatible with this function
		#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
		throw InvalidTemplateState("Unsupported type of the variable");
		#else
		return "";
		#endif
	}
	
	CaseWriterFunction::CaseWriterFunction(
		std::vector<std::string> &listOfOutputs,
		const locale::Locale &locale
	) {
		if( listOfOutputs.size() != locale.getCasesList().size() ) {
			throw InvalidTemplateState("Invalid list of cases");
		}
		
		for(std::size_t i=0u; i<listOfOutputs.size(); ++i) {
			outputTexts.emplace(
				locale.getCasesList()[i],
				listOfOutputs[i]
			);
		}
	}
	
	CaseWriterFunction::CaseWriterFunction(
		std::map<std::string, std::string> &mapOfOutputs
	) {
		outputTexts = std::move(mapOfOutputs);
	}
	
	std::string CaseWriterFunction::produceString(std::map<std::string_view, variableValue> &variables) {
		if( !variables.contains("__CASE__") ) {
			//not an error
			return "";
		}
		
		auto content = variables.at("__CASE__");
		
		if( std::holds_alternative<std::string_view>(content) ) {
			auto sv = std::get<std::string_view>(content);
			
			for(auto&[key, value] : outputTexts) {
				if( key == sv ) {
					return value;
				}
			}
			//other cases are errors
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("Wrong case");
			#else
			return "";
			#endif
		}
		//other types of data are errors
		#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
		throw InvalidTemplateState("Unsupported type of the __CASE__ variable");
		#else
		return "";
		#endif
	}
	
	CaseChooserFunction::CaseChooserFunction(
		std::string &&varName,
		std::string &&case_id
	):variable{varName}, caseID{case_id} { }
	
	std::string CaseChooserFunction::produceString(std::map<std::string_view, variableValue> &variables) {
		if( !variables.contains(variable) ) {
			//error
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("The variable doesn't exists: " + variable);
			#else
			return "";
			#endif
		}
		
		auto content = variables.at(variable);
		
		if( std::holds_alternative<Template*>(content) ) {
			return std::get<Template*>(content)->apply("__CASE__", caseID).get();
		}
		
		//other types of data are incompatible with this function
		#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
		throw InvalidTemplateState("Invalid type of variable: " + variable);
		#else
		return "";
		#endif
	}
	
	PluralFunction::PluralFunction(
		std::string &&varName,
		std::vector<std::string> listOfOutputs,
		const locale::Locale &locale
	):variable{varName}, theLocale{locale} {
		if( listOfOutputs.size() != locale.getPluralsList().size() ) {
			throw InvalidTemplateState("Wrong number of arguments");
		}
		
		for(std::size_t i=0u; i<listOfOutputs.size(); ++i) {
			outputTexts.emplace(
				locale.getPluralsList()[i],
				listOfOutputs[i]
			);
		}
	}
	
	PluralFunction::PluralFunction(
		std::string &&varName,
		std::map<std::string, std::string> &mapOfOutputs,
		const locale::Locale &locale
	):variable{varName}, theLocale{locale} {
		outputTexts = std::move(mapOfOutputs);
	}
	
	std::string PluralFunction::produceString(std::map<std::string_view, variableValue> &variables) {
		if( !variables.contains(variable) ) {
			//error
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("The variable doesn't exists: " + variable);
			#else
			return "";
			#endif
		}
		
		long number;
		auto content = variables.at(variable);
		if( std::holds_alternative<long>(content) ) {
			number = std::get<long>(content);
		} else if( std::holds_alternative<double>(content) ) {
			//keep only the integer part
			number = static_cast<long>(std::get<double>(content));
		} else {
			//error
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("Invalid type of the variable: " + variable);
			#else
			return "";
			#endif
		}
		
		if( number < 0 ) number *= -1;
		
		std::string pluralID = theLocale.getPluralID(number);
		if( ! outputTexts.contains(pluralID) ) {
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("Unknown plural type: " + pluralID);
			#else
			return "";
			#endif
		} else {
			return outputTexts[pluralID];
		}
	}
	
	IntegerFormaterFunction::IntegerFormaterFunction(
		std::string &&varName,
		std::string &formatName,
		locale::Locale &theLocale
	):variable{varName}, formater{*(theLocale.getNumberFormat(formatName))} {}
	
	std::string IntegerFormaterFunction::produceString(std::map<std::string_view, variableValue> &variables) {
		if( !variables.contains(variable) ) {
			//error
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("The variable doesn't exists: " + variable);
			#else
			return "";
			#endif
		}
		
		long number;
		auto content = variables.at(variable);
		
		if( std::holds_alternative<long>(content) ) {
			number = std::get<long>(content);
		} else if( std::holds_alternative<double>(content) ) {
			//keep only the integer part
			number = static_cast<long>(std::get<double>(content));
		} else {
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("The variable has improper content: " + variable);
			#else
			return "";
			#endif
		}
		
		return formater.formatInteger(number);
	}
	
	bool RealFormaterFunction::isAllNumber(std::string &str) {
		//IMPORTANT: we don check for *negative* numbers
		for(auto c : str) {
			if( !std::isdigit(c) ) {
				return false;
			}
		}
		return true;
	}
	
	RealFormaterFunction::RealFormaterFunction(
		std::string &&varName,
		std::vector<std::string> &listOfOptions,
		locale::Locale &theLocale
	):variable{varName} {
		formater = nullptr;
		if( listOfOptions.size() == 1u ) {
			//only formater
			formater = theLocale.getNumberFormat(listOfOptions[0]);
			precision = -1;
		} else if( listOfOptions.size() == 2u ) {
			//formater and precision
			formater = theLocale.getNumberFormat(listOfOptions[0]);
			if( ! isAllNumber(listOfOptions[1]) ) {
				throw InvalidTemplateState("Invalid precision: " + listOfOptions[1]);
			} else {
				precision = static_cast<short>(std::stoi(listOfOptions[1]));
			}
		}
		
		if( formater == nullptr ) {
			throw InvalidTemplateState("Unknow formater type");
		}
	}
	
	RealFormaterFunction::RealFormaterFunction(
		std::string &&varName,
		std::map<std::string, std::string> &hashOfOptions,
		locale::Locale &theLocale
	):variable{varName} {
		formater = nullptr;
		if( hashOfOptions.contains("format") ) {
			formater = theLocale.getNumberFormat(hashOfOptions["format"]);
		} else {
			formater = theLocale.getNumberFormat("general");
		}
		
		precision = -1;
		if( hashOfOptions.contains("prec") ) {
			if( isAllNumber(hashOfOptions["prec"]) ) {
				precision = static_cast<short>(
					std::stoi(hashOfOptions["prec"])
				);
			} else {
				throw InvalidTemplateState("Invalid number for precision: " + hashOfOptions["prec"]);
			}
		}
		
		if( formater == nullptr ) {
			throw InvalidTemplateState("Unknown formater type");
		}
	}
	
	std::string RealFormaterFunction::produceString(std::map<std::string_view, variableValue> &variables) {
		if( !variables.contains(variable) ) {
			//error
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("The variable doesn't exists: " + variable);
			#else
			return "";
			#endif
		}
		
		double number;
		auto content = variables.at(variable);
		
		if( std::holds_alternative<double>(content) ) {
			number = std::get<double>(content);
		} else if( std::holds_alternative<long>(content) ) {
			//keep only the integer part
			number = static_cast<double>(std::get<long>(content));
		} else {
			#ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
			throw InvalidTemplateState("The variable has improper content: " + variable);
			#else
			return "";
			#endif
		}
		
		return formater->formatReal(number, precision);
	}
};



namespace mls::backend {
	
	locale::Locale *defaultLocale = nullptr;
	
	locale::Locale *getLocaleFromName(std::string_view searchLocaleName) {
		static std::map<std::string, std::vector<std::string>> LOCALES{
			//English - United Kingdom
			{"en_GB", {"en_GB","en-GB","English_United Kingdom"}},
			//English - American
			{"en_US", {"en_US","en-US","English_United States"}},
			//Polish
			{"pl_PL", {"pl_PL","pl-PL","Polish_Poland"}}
		};
		
		//Search for supported locales
		for(auto&[locale_name, locale_list] : LOCALES) {
			for(auto& some_locale_name : locale_list) {
				if( some_locale_name == searchLocaleName ) {
					return &(locale::getLocale(locale_name));
				}
			}
		}
		//unable to find the locale
		return nullptr;
	}
	
	void init(const char* packageName, const char* wantedLocale, const char* folderLookup) {
		const char* theLocale;
		
		if( wantedLocale == nullptr ) {
			theLocale = setlocale(LC_ALL, "");
		} else {
			setlocale(LC_ALL, wantedLocale);
			theLocale = wantedLocale;
		}
		
		std::string_view correctLocale{theLocale};
		//check for presence of unwanted suffixes or wrong locale names
		if( correctLocale == "C" || correctLocale == "POSIX" ) {
			correctLocale = "en_US";
		}
		auto dot = correctLocale.find_last_of(".");
		if( dot != std::string_view::npos ) {
			correctLocale.remove_suffix( correctLocale.size() - dot );
		}
		auto at = correctLocale.find_last_of("@");
		if( at != std::string_view::npos ) {
			correctLocale.remove_suffix( correctLocale.size() - at );
		}
		
		defaultLocale = getLocaleFromName(correctLocale);
		if( defaultLocale == nullptr ) {//unable to find supported locale?
			defaultLocale = &(locale::getLocale("en_US"));//fallback to English
			setlocale(LC_ALL, "C");//Force non-translated strings
		}
		
		if( folderLookup == nullptr ) {
			bindtextdomain(packageName, "/usr/local/share/locale");
		} else {
			bindtextdomain(packageName, folderLookup);
		}
		
		bind_textdomain_codeset(packageName, "utf-8");
		textdomain(packageName);
	}
	
};

namespace mls {
	
	Template translate(const char* msgid) {
		if( backend::defaultLocale == nullptr ) {
			throw backend::IntlNotInitialized();
		}
		return Template{std::string{ gettext(msgid) }, *backend::defaultLocale};
	}
	
	Template translate(const char* catalog, const char* msgid) {
		if( backend::defaultLocale == nullptr ) {
			throw backend::IntlNotInitialized();
		}
		return Template{std::string{ dgettext(catalog, msgid) }, *backend::defaultLocale};
	}
	
};





#endif
#endif


