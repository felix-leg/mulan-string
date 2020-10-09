
#ifdef TEST
#	undef TEST
#	define BASIC_PARSE_TEST
#	define BOOST_TEST_DYN_LINK
#	define BOOST_TEST_MODULE basic_parse_module
#	include <boost/test/unit_test.hpp>

#	include <iostream>
#endif

/**
 * This module contains a function `basicParseString(template)` and all necessary
 * classes and functions, for parsing a template string into a list of template elements. 
 */

#include "mulanstr_config.hpp"
#include "basic_parse.hpp"

#if __has_include(<version>)
#	include <version>
#endif

#if __has_include(<string_view>)
#	if __cpp_lib_starts_ends_with
#		define CAN_USE_STARTS_ENDS_WITH
#		include <string_view>
#	endif
#endif

namespace mls {
	
	//#####StringElement class
	ElementType StringElement::getType() {
		return ElementType::STRING;
	}
	std::string StringElement::getString() {
		return strElem;
	}
	
	//###### SubstElement class
	ElementType SubstElement::getType() {
		return ElementType::SUBSTITUTION;
	}
	std::string SubstElement::getVariableName() {
		return varName;
	}
	
	//####### FunctElement class
	ElementType FunctElement::getType() {
		return ElementType::FUNCTION;
	}
	std::string FunctElement::getFunctionName() {
		return fnName;
	}
	
	//###### FunctAndSubstElement class
	ElementType FunctAndSubstElement::getType() {
		return ElementType::FUNCTION_WITH_SUBSTITUTION;
	}
	std::string FunctAndSubstElement::getFunctionName() {
		return fnName;
	}
	
	//######## OneFunctArg class
	FunctionArgs OneFunctArg::getArgType() {
		return FunctionArgs::ONE_ARG;
	}
	std::string OneFunctArg::getValue() {
		return argValue;
	}
	
	//######## TabFunctArg class
	FunctionArgs TabFunctArg::getArgType() {
		return FunctionArgs::TABLE_ARGS;
	}
	
	//######## MapFunctArg class
	FunctionArgs MapFunctArg::getArgType() {
		return FunctionArgs::MAP_ARGS;
	}
	
	//######## BasicTagParser class
	
	/**
	 * Parses one tag
	 */
	class BasicTagParser {
		public:
			BasicTagParser(std::string& templateString, std::size_t startPos):
			cPos(startPos), str(templateString) {};
			
			/// is at the start of a tag?
			bool hasNext() {
				std::size_t pos = str.find(TAG_START,cPos);
				if( pos == std::string::npos ) return false;
				if( pos != cPos ) return false;
				return true;
			}
			/// is there a text to extract?
			bool hasText() {
				std::size_t pos = str.find(TAG_START,cPos);
				if( pos == std::string::npos ) {
					return cPos < str.size();
				}
				if( pos > cPos ) return true;
				return false;
			}
			/// extract text
			std::string getText() {
				std::size_t pos = str.find(TAG_START,cPos);
				std::string returnStr;
				if( pos == std::string::npos ) {
					returnStr = str.substr(cPos);
					cPos = std::string::npos;
				} else {
					returnStr = str.substr(cPos, pos - cPos);
					cPos = pos;
				}
				return returnStr;
			}
			std::size_t getLastPosition() {
				return cPos;
			}
			
			/// !!! extracting tag - main method
			TemplateElementBase* getNext() {
				advance( TAG_START.length() );
				
				if( isAtTheEndOfATag() ) //too soon, we have to have some content to read
					throw BadlyFormatedTemplateString();
				
				//comments are in form %{# comment #}%
				if( viewed() == '#') {
					advance();
					while( viewed() != '#') advance();
					advance();//pass the hash character
					//after the hash character we must be at the end tag string
					if( !isAtTheEndOfATag() ) throw BadlyFormatedTemplateString();
					advance( TAG_END.length() );
					
					return nullptr;//comments have no its own type
				}
				
				//no var but function call? (in form "%{+FUNC_NAME}%")
				if( viewed() == '+') {
					advance();
					auto funcCall = parseFunctionAndArgs<FunctElement>();
					advance( TAG_END.length() );
					return funcCall;
				}
				//get variable name
				std::string varName;
				while( viewed() != '!' && !isAtTheEndOfATag() ) {
					varName += viewed();
					advance();
				}
				//var with function call? (in form "%{var!FUNC_NAME}%")
				if( viewed() == '!') {
					advance();
					auto funcCall = parseFunctionAndArgs<FunctAndSubstElement>();
					funcCall->varName = varName;
					advance( TAG_END.length() );
					return funcCall;
				} else {				
					advance( TAG_END.length() );
					return new SubstElement(varName);
				}
			}
		private:
			std::string& str;//template string
			std::size_t cPos;//current position in the template string
			
			/**
			 * Gets current pair (template string, position) as string view
			 */
			#ifdef CAN_USE_STARTS_ENDS_WITH
			std::string_view toBeParsedAsSV() {
				return std::string_view(str.data() + cPos, str.length()-cPos);
			}
			#endif
			
			/** move string pointer by `howMuch` characters 
			 * and throw if out of the template string
			 */
			void advance(std::size_t howMuch = 1) {
				cPos += howMuch;
				if( cPos > str.length() ) {
					throw BadlyFormatedTemplateString();
				}
			}
			///get the character at the string pointer
			char viewed() {
				return str[cPos];
			}
			/// check if reached end of a tag
			bool isAtTheEndOfATag() {
				#ifdef CAN_USE_STARTS_ENDS_WITH
				return toBeParsedAsSV().starts_with(TAG_END);
				#else				
				std::size_t pos = str.find(TAG_END,cPos);
				if( pos == std::string::npos ) return false;
				return pos == cPos;
				#endif
			}
			/**
			 * parse the function call part
			 */
			template<class ReturnType>
			ReturnType* parseFunctionAndArgs() {
				//we have to have something to extract, so no premature endings
				if( isAtTheEndOfATag() ) 
					throw BadlyFormatedTemplateString();
				
				std::string functionName;
				//function name can have got only uppercase characters
				while( viewed() >= 'A' && viewed() <= 'Z' ) {
					functionName += viewed();
					advance();
				}
				//no empty function name!
				if( functionName.empty() ) throw BadlyFormatedTemplateString();
				
				//prepare return element
				ReturnType* returnValue = new ReturnType(functionName);
				
				try{
					/* Now we must decide if it is function: 
					* with one parameter	%{var!ONE=one}%
					* with one tabular parameter	%{var!TAB:one,two,three}%
					* or with multiple parameters	%{var!MAP one={first} two={second}}%
					*/
					switch( viewed() ) {
						case '=': // one parameter
							advance();
							returnValue->arguments = new OneFunctArg(parseParam());
							
							//now we should be at the end of a tag, if not...
							if( !isAtTheEndOfATag() )
								throw BadlyFormatedTemplateString();
							
							break;
						
						case ':': // one tabular parameter
						{
							advance();
							returnValue->arguments = new TabFunctArg();
							std::vector<std::string>& paramsTable = 
								static_cast<TabFunctArg*>(returnValue->arguments)->paramsTable;
							std::string parmValue;
							bool hasSeenComma = false;
							while( !isAtTheEndOfATag() ) {
								if( viewed() == ',') {
									hasSeenComma = true;
									paramsTable.push_back(parmValue);
									parmValue.clear();
								} else {
									parmValue += viewed();
								}
								advance();
							}
							
							//in this type of args list comma is required
							if( !hasSeenComma )
								throw BadlyFormatedTemplateString();
							
							//add the last parameter
							paramsTable.push_back(parmValue);
						}
							break;
						
						case ' ': //a map of parameters
							{
								advance();
								
								//we can't yet be at the end of the tag
								if( isAtTheEndOfATag() )
									throw BadlyFormatedTemplateString();
								
								returnValue->arguments = new MapFunctArg();
								std::map<std::string,std::string>& paramsMap = 
									static_cast<MapFunctArg*>(returnValue->arguments)->paramsMap;
								
								std::string paramName, paramValue;
								while( !isAtTheEndOfATag() ) {
									//params should be in the form param1=value1 or param2={value2}
									if( viewed() != '=') {
										paramName += viewed();
										advance();
									} else {
										//no empty param names
										if( paramName.empty() )
											throw BadlyFormatedTemplateString();
										advance();
										paramValue = parseParam();
										if(viewed() == ' ') advance();
										paramsMap.insert_or_assign(paramName,paramValue);
										paramName = "";
										paramValue = "";
									}
								}
							}
							
							break;
						default: //unknown arg type if not at the end of a tag
							if( !isAtTheEndOfATag() )
								throw BadlyFormatedTemplateString();
					}
				}catch(BadlyFormatedTemplateString e) {
					delete returnValue;
					throw e;
				}
				
				return returnValue;
			}
			
			std::string parseParam() {
				std::string parmValue;
				bool brackedInInnerTag = false;
				
				if( isAtTheStartOfInnerTag() ) {
					brackedInInnerTag = true;
					advance( INNER_TAG_START.length() );
				}
				
				while( !isAtTheEndOfATag() ) {
					//a space breaks reading the value unless the value is contained inside
					//an inner brace tag
					if( !brackedInInnerTag && viewed() == ' ') break;
					if( brackedInInnerTag && isAtTheEndOfInnerTag() ) {
						advance( INNER_TAG_END.length() );
						return parmValue;
					}
					parmValue += viewed();
					advance();
				}
				
				//if was inside inner brace, but not seen the end brace...
				if( brackedInInnerTag )
					throw BadlyFormatedTemplateString();
				
				if( parmValue.empty() && !brackedInInnerTag )
					throw BadlyFormatedTemplateString();
				
				return parmValue;
			}
			//used by parseParam()
			bool isAtTheStartOfInnerTag() {
				#ifdef CAN_USE_STARTS_ENDS_WITH
				return toBeParsedAsSV().starts_with(INNER_TAG_START);
				#else
				std::size_t pos = str.find(INNER_TAG_START,cPos);
				if( pos == std::string::npos ) return false;
				return pos == cPos;
				#endif
			}
			bool isAtTheEndOfInnerTag() {
				#ifdef CAN_USE_STARTS_ENDS_WITH
				return toBeParsedAsSV().starts_with(INNER_TAG_END);
				#else
				std::size_t pos = str.find(INNER_TAG_END,cPos);
				if( pos == std::string::npos ) return false;
				return pos == cPos;
				#endif
			}
	};
	
	//######### basicParseString
	TemplateElementsList basicParseString(std::string templateString)
	{
		TemplateElementsList outputList;
		
		//Check if template contains any tags
		if( auto start = templateString.find(TAG_START); start == std::string::npos  )
		{
			//Tag not found -> return only StringElement
			outputList.emplace_back(new StringElement(templateString));
		}
		else
		{
			//there are some tags
			
			//get prefix and put it into outputList if not empty
			if( start > 0 ) {
				outputList.emplace_back(new StringElement(templateString.substr(0,start)));
			}
			
			BasicTagParser parser(templateString,start);
			while( parser.hasNext() ) {
				if( auto nextElement = parser.getNext(); nextElement != nullptr)
					outputList.emplace_back( nextElement );
				if( parser.hasText() ) {
					outputList.emplace_back( new StringElement(parser.getText()) );
				}
			}
			//detect suffix and add it to the outputList
			if(std::size_t lastPos = parser.getLastPosition(); lastPos != std::string::npos) {
				std::string suffix = templateString.substr(lastPos);
				if( !suffix.empty() ) {
					outputList.emplace_back(new StringElement(suffix));
				}
			}
		}
		
		return outputList;
	}
}; //end namespace mls


//------------- TESTS
#ifdef BASIC_PARSE_TEST
using namespace mls;
using namespace std;

BOOST_AUTO_TEST_CASE( testStringParseNoTemplate )
{
	auto list = basicParseString("No template");
	BOOST_TEST_REQUIRE( list.size() == 1 );
	BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::STRING) );
	StringElement* str = dynamic_cast<StringElement*>(list[0].get());
	BOOST_TEST_REQUIRE( str != nullptr);
	BOOST_TEST_REQUIRE( str->getString() == string("No template"));
}

BOOST_AUTO_TEST_CASE( testStringParseSimpleVariable )
{
	auto list = basicParseString("prefix%{variable}%suffix");
	BOOST_TEST_REQUIRE( list.size() == 3 );
	//test for strings
	BOOST_TEST_CHECKPOINT("test for strings");
	BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::STRING) );
	BOOST_TEST_REQUIRE( (list[2]->getType() == ElementType::STRING) );
	StringElement* str = dynamic_cast<StringElement*>(list[0].get());
	BOOST_TEST_REQUIRE( str != nullptr );
	BOOST_TEST_REQUIRE( str->getString() == string("prefix") );
	str = dynamic_cast<StringElement*>(list[2].get());
	BOOST_TEST_REQUIRE( str != nullptr );
	BOOST_TEST_REQUIRE( str->getString() == string("suffix") );
	//test for substitution
	BOOST_TEST_CHECKPOINT("test for substitution");
	BOOST_TEST_REQUIRE( (list[1]->getType() == ElementType::SUBSTITUTION) );
	SubstElement* sub = dynamic_cast<SubstElement*>(list[1].get());
	BOOST_TEST_REQUIRE( sub != nullptr );
	BOOST_TEST_REQUIRE( sub->getVariableName() == string("variable") );
}

BOOST_AUTO_TEST_CASE( testStringParseSimpleTwoVariable )
{
	auto list = basicParseString("prefix%{variable1}%infix%{variable2}%suffix");
	BOOST_TEST_REQUIRE( list.size() == 5 );
	//test for strings
	BOOST_TEST_CHECKPOINT("test for strings");
	BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::STRING) );
	BOOST_TEST_REQUIRE( (list[2]->getType() == ElementType::STRING) );	
	BOOST_TEST_REQUIRE( (list[4]->getType() == ElementType::STRING) );
	StringElement* str = dynamic_cast<StringElement*>(list[0].get());
	BOOST_TEST_REQUIRE( str != nullptr );
	BOOST_TEST_REQUIRE( str->getString() == string("prefix") );
	str = dynamic_cast<StringElement*>(list[2].get());
	BOOST_TEST_REQUIRE( str != nullptr );
	BOOST_TEST_REQUIRE( str->getString() == string("infix") );
	str = dynamic_cast<StringElement*>(list[4].get());
	BOOST_TEST_REQUIRE( str != nullptr );
	BOOST_TEST_REQUIRE( str->getString() == string("suffix") );
	//test for substitution
	BOOST_TEST_CHECKPOINT("test for substitution");
	BOOST_TEST_REQUIRE( (list[1]->getType() == ElementType::SUBSTITUTION) );
	SubstElement* sub = dynamic_cast<SubstElement*>(list[1].get());
	BOOST_TEST_REQUIRE( sub != nullptr );
	BOOST_TEST_REQUIRE( sub->getVariableName() == string("variable1") );
	BOOST_TEST_REQUIRE( (list[3]->getType() == ElementType::SUBSTITUTION) );
	sub = dynamic_cast<SubstElement*>(list[3].get());
	BOOST_TEST_REQUIRE( sub != nullptr );
	BOOST_TEST_REQUIRE( sub->getVariableName() == string("variable2") );
}

BOOST_AUTO_TEST_CASE( testStringParseFunctionCallWithNoVarAssign )
{
	auto list = basicParseString("%{+SG=male}%");
	BOOST_TEST_REQUIRE( list.size() == 1 );
	
	BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::FUNCTION) );
	FunctElement* func = dynamic_cast<FunctElement*>(list[0].get());
	BOOST_TEST_REQUIRE( func != nullptr );
	BOOST_TEST_REQUIRE( func->getFunctionName() == string("SG") );
	BOOST_TEST_REQUIRE( func->arguments != nullptr );
	BOOST_TEST_REQUIRE( (func->arguments->getArgType() == FunctionArgs::ONE_ARG) );
	OneFunctArg* argType = dynamic_cast<OneFunctArg*>( func->arguments );
	BOOST_TEST_REQUIRE( argType != nullptr );
	BOOST_TEST_REQUIRE( argType->getValue() == string("male") );
}

BOOST_AUTO_TEST_CASE( testStringParseFunctionCallWithNoVarTable )
{
	auto list = basicParseString("%{+C:ąg,ągowi,ągami}%");
	BOOST_TEST_REQUIRE( list.size() == 1 );
	BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::FUNCTION) );
	FunctElement* func = dynamic_cast<FunctElement*>(list[0].get());
	BOOST_TEST_REQUIRE( func != nullptr );
	BOOST_TEST_REQUIRE( func->getFunctionName() == string("C") );
	BOOST_TEST_REQUIRE( func->arguments != nullptr );
	BOOST_TEST_REQUIRE( (func->arguments->getArgType() == FunctionArgs::TABLE_ARGS) );
	TabFunctArg* argType = dynamic_cast<TabFunctArg*>( func->arguments );
	BOOST_TEST_REQUIRE( argType != nullptr );
	std::vector<std::string> paramsCompare = { "ąg", "ągowi", "ągami" };
	BOOST_TEST_REQUIRE( argType->paramsTable.size() == paramsCompare.size() );
	for(unsigned int i=0; i<paramsCompare.size(); i++) {
		BOOST_TEST_CHECKPOINT("Checking for '" << paramsCompare[i] << "'");
		BOOST_TEST_REQUIRE( argType->paramsTable[i] == paramsCompare[i] );
	}
}

BOOST_AUTO_TEST_CASE( testStringParseFunctionCallWithNoVarMap )
{
	auto list = basicParseString("%{+G neut={} male=on female={ona}}%");
	BOOST_TEST_REQUIRE( list.size() == 1 );
	BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::FUNCTION) );
	FunctElement* func = dynamic_cast<FunctElement*>(list[0].get());
	BOOST_TEST_REQUIRE( func != nullptr );
	BOOST_TEST_REQUIRE( func->getFunctionName() == string("G") );	
	BOOST_TEST_REQUIRE( func->arguments != nullptr );
	BOOST_TEST_REQUIRE( (func->arguments->getArgType() == FunctionArgs::MAP_ARGS) );
	MapFunctArg* argType = dynamic_cast<MapFunctArg*>( func->arguments );
	BOOST_TEST_REQUIRE( argType != nullptr );
	std::map<std::string,std::string> paramsCompare = {
		{"male", "on"}, {"female","ona"}, {"neut",""}
	};
	BOOST_TEST_REQUIRE( argType->paramsMap.size() == paramsCompare.size() );
	for(auto& param: paramsCompare) {
		BOOST_TEST_CHECKPOINT("Checking for '" << param.first << "'='" << param.second << "'");
		auto argFound = argType->paramsMap.find(param.first);
		if( argFound != argType->paramsMap.end() ) {
			BOOST_TEST_REQUIRE( argFound->second == param.second );
		} else {
			BOOST_ERROR("Required parameter in the map not found");
		}
	}
}

BOOST_AUTO_TEST_CASE( testStringParseFunctionCallWithVarAssign )
{
	auto list = basicParseString("%{var!SG=male}%");
	BOOST_TEST_REQUIRE( list.size() == 1 );
	
	BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::FUNCTION_WITH_SUBSTITUTION) );
	FunctAndSubstElement* func = dynamic_cast<FunctAndSubstElement*>(list[0].get());
	BOOST_TEST_REQUIRE( func != nullptr );
	BOOST_TEST_REQUIRE( func->varName == string("var") );
	BOOST_TEST_REQUIRE( func->getFunctionName() == string("SG") );
	BOOST_TEST_REQUIRE( func->arguments != nullptr );
	BOOST_TEST_REQUIRE( (func->arguments->getArgType() == FunctionArgs::ONE_ARG) );
	OneFunctArg* argType = dynamic_cast<OneFunctArg*>( func->arguments );
	BOOST_TEST_REQUIRE( argType != nullptr );
	BOOST_TEST_REQUIRE( argType->getValue() == string("male") );
}

BOOST_AUTO_TEST_CASE( testStringParseFunctionCallWithVarTable )
{
	auto list = basicParseString("%{var!C:ąg,ągowi,ągami}%");
	BOOST_TEST_REQUIRE( list.size() == 1 );
	BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::FUNCTION_WITH_SUBSTITUTION) );
	FunctAndSubstElement* func = dynamic_cast<FunctAndSubstElement*>(list[0].get());
	BOOST_TEST_REQUIRE( func != nullptr );
	BOOST_TEST_REQUIRE( func->varName == string("var") );
	BOOST_TEST_REQUIRE( func->getFunctionName() == string("C") );
	BOOST_TEST_REQUIRE( func->arguments != nullptr );
	BOOST_TEST_REQUIRE( (func->arguments->getArgType() == FunctionArgs::TABLE_ARGS) );
	TabFunctArg* argType = dynamic_cast<TabFunctArg*>( func->arguments );
	BOOST_TEST_REQUIRE( argType != nullptr );
	std::vector<std::string> paramsCompare = { "ąg", "ągowi", "ągami" };
	BOOST_TEST_REQUIRE( argType->paramsTable.size() == paramsCompare.size() );
	for(unsigned int i=0; i<paramsCompare.size(); i++) {
		BOOST_TEST_CHECKPOINT("Checking for '" << paramsCompare[i] << "'");
		BOOST_TEST_REQUIRE( argType->paramsTable[i] == paramsCompare[i] );
	}
}

BOOST_AUTO_TEST_CASE( testStringParseFunctionCallWithVarMap )
{
	auto list = basicParseString("%{var!G male=on female={ona}}%");
	BOOST_TEST_REQUIRE( list.size() == 1 );
	BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::FUNCTION_WITH_SUBSTITUTION) );
	FunctAndSubstElement* func = dynamic_cast<FunctAndSubstElement*>(list[0].get());
	BOOST_TEST_REQUIRE( func != nullptr );
	BOOST_TEST_REQUIRE( func->varName == string("var") );
	BOOST_TEST_REQUIRE( func->getFunctionName() == string("G") );	
	BOOST_TEST_REQUIRE( func->arguments != nullptr );
	BOOST_TEST_REQUIRE( (func->arguments->getArgType() == FunctionArgs::MAP_ARGS) );
	MapFunctArg* argType = dynamic_cast<MapFunctArg*>( func->arguments );
	BOOST_TEST_REQUIRE( argType != nullptr );
	std::map<std::string,std::string> paramsCompare = {
		{"male", "on"}, {"female","ona"}
	};
	BOOST_TEST_REQUIRE( argType->paramsMap.size() == paramsCompare.size() );
	for(auto& param: paramsCompare) {
		BOOST_TEST_CHECKPOINT("Checking for '" << param.first << "'='" << param.second << "'");
		auto argFound = argType->paramsMap.find(param.first);
		if( argFound != argType->paramsMap.end() ) {
			BOOST_TEST_REQUIRE( argFound->second == param.second );
		} else {
			BOOST_ERROR("Required parameter in the map not found");
		}
	}
}

BOOST_AUTO_TEST_CASE( testForComments )
{
	auto list = basicParseString("prefix%{# comment #}%suffix");
	BOOST_TEST_REQUIRE( list.size() == 2 );
	BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::STRING) );
	StringElement* str = dynamic_cast<StringElement*>(list[0].get());
	BOOST_TEST_REQUIRE( str != nullptr);
	BOOST_TEST_REQUIRE( str->getString() == string("prefix"));
	BOOST_TEST_REQUIRE( (list[1]->getType() == ElementType::STRING) );
	str = dynamic_cast<StringElement*>(list[1].get());
	BOOST_TEST_REQUIRE( str != nullptr);
	BOOST_TEST_REQUIRE( str->getString() == string("suffix"));
}

BOOST_AUTO_TEST_CASE( testIfThrowsOnEmptyTag )
{
	try{
		auto list = basicParseString("%{}%");
		BOOST_ERROR("Parser shouldn't accept empty tag");
	}
	catch(BadlyFormatedTemplateString e) { return; };
	BOOST_ERROR("Wrong exception");
}

BOOST_AUTO_TEST_CASE( testEmptyParametersListNoVar )
{
	try{
		auto list = basicParseString("%{+NO}%");
		BOOST_TEST_REQUIRE( list.size() == 1 );
		BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::FUNCTION) );
		FunctElement* func = dynamic_cast<FunctElement*>(list[0].get());
		BOOST_TEST_REQUIRE( func != nullptr );
		BOOST_TEST_REQUIRE( func->getFunctionName() == string("NO") );
		BOOST_TEST_REQUIRE( func->arguments == nullptr );
	}
	catch(BadlyFormatedTemplateString e) {
		BOOST_ERROR("Template '%{+NO}%' not accepted");
	}
}

BOOST_AUTO_TEST_CASE( testEmptyParametersListWithVar )
{
	try{
		auto list = basicParseString("%{var!NO}%");
		BOOST_TEST_REQUIRE( list.size() == 1 );
		BOOST_TEST_REQUIRE( (list[0]->getType() == ElementType::FUNCTION_WITH_SUBSTITUTION) );
		FunctAndSubstElement* func = dynamic_cast<FunctAndSubstElement*>(list[0].get());
		BOOST_TEST_REQUIRE( func != nullptr );
		BOOST_TEST_REQUIRE( func->getFunctionName() == string("NO") );
		BOOST_TEST_REQUIRE( func->arguments == nullptr );
	}
	catch(BadlyFormatedTemplateString e) {
		BOOST_ERROR("Template '%{var!NO}%' not accepted");
	}
}

BOOST_AUTO_TEST_CASE( testOnBadlyFormatedInput )
{
	const char* inputs[] = {
		"prefix%{",
		"prefix%{+",
		"prefix%{+NM",
		"prefix%{+C=",
		"prefix%{+C=a",
		"prefix%{+C={b}%",
		"prefix%{+C:",
		"prefix%{+C:}%",
		//"prefix%{+C:,a}%", //ACCEPTABLE: params may be omnited 
		"prefix%{+C a",
		"prefix%{+C a=b",
		"prefix%{+C a={b}%",
		"prefix%{+C a={b}suffix",
		"prefix%{var!",
		"prefix%{var!NM",
		"prefix%{var!C=",
		"prefix%{var!C=a",
		"prefix%{var!C={b}%",
		"prefix%{var!C:",
		"prefix%{var!C:}%",
		"prefix%{var!C a",
		"prefix%{var!C a=b",
		"prefix%{var!C a={b}%",
		"prefix%{var!c a={b}suffix",
		"%{#"
	};
	
	for(const char* input : inputs) {
		try{
			auto list = basicParseString( std::string(input) );
			BOOST_ERROR("Parser shouldn't accept: " << input);
		}
		catch(BadlyFormatedTemplateString e) { continue; };
	}
}

// tests end
#endif