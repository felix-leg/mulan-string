/**
 * @file preparser.cpp
 * @brief Preparser prepares template string to further process
 * 
 */

#include "preparser.h"
#include "errors.h"

//CUT-START

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

//CUT-END
