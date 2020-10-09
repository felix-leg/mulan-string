#ifdef TEST
#	undef TEST
#	define TEMPLATE_OBJECT_TEST
#	define BOOST_TEST_DYN_LINK
#	define BOOST_TEST_MODULE basic_parse_module
#	include <boost/test/unit_test.hpp>

#endif
#	include <iostream>

/**
 * This module contains the Template class responsible for parsing template
 * and generating output string from it. 
 * In module there are also all classes for different functions that can exist
 * in the string template.
*/

#include "mulanstr_config.hpp"
#include "template_object.hpp"
#include "basic_parse.hpp"
#include "language_features.hpp"

namespace mls {
	
	// ---------- StringTemplateElement class
	StringTemplateElement::StringTemplateElement(StringElement* element) {
		theString = element->getString();
	}
	StringTemplateElement::StringTemplateElement(std::string str) {
		theString = str;
	}
	TemplateElement* StringTemplateElement::copy() {
		return new StringTemplateElement(theString);
	}
	std::string StringTemplateElement::execute(Template& parent) {
		return theString;
	}
	
	// ---------- SubstitutionTemplateElement class
	SubstitutionTemplateElement::SubstitutionTemplateElement(std::string valName):
		valueName(valName) {};
	
	std::string SubstitutionTemplateElement::execute(Template& parent) {
		try{
			return parent.findArgument(valueName).get();
		}
		catch(RequiredVariableNotFound e) {
			return std::to_string( parent.findNumber(valueName) );
		}
	}
	
	TemplateElement* SubstitutionTemplateElement::copy() {
		return new SubstitutionTemplateElement(valueName);
	}
	
	// ----------------- CaseSetterElement class
	CaseSetterElement::CaseSetterElement(std::string varName, std::string caseName):
		variableName(varName), caseName(caseName) {};
	
	std::string CaseSetterElement::execute(Template& parent) {
		return parent.findArgument(variableName).apply("__CASE__",caseName).get();
	}
	
	TemplateElement* CaseSetterElement::copy() {
		return new CaseSetterElement(variableName,caseName);
	}
	
	// ---------- CaseWriterElement class
	
	CaseWriterElement::CaseWriterElement(std::map<std::string,std::string> &paramsList):
		params(paramsList) {};
	
	std::string CaseWriterElement::execute(Template& parent) {
		const std::string caseName = parent.findArgument("__CASE__").get();
		const auto& found = params.find(caseName);
		if( found == params.end() )
			return "";
		else
			return found->second;
	}
	
	TemplateElement* CaseWriterElement::copy() {
		return new CaseWriterElement(params);
	}
	
	// ------------- GenderWriterElement class
	GenderWriterElement::GenderWriterElement(std::string varName, std::map<std::string, std::string> &list):
		variableName(varName), argList(list) {};
	
	std::string GenderWriterElement::execute(Template& parent) {
		std::string gender = parent.findArgument(variableName).getGender();
		if( gender == "" ) return "";
		
		if( auto found = argList.find(gender); found != argList.end() ) {
			return found->second;
		}
		
		return "";
	}
	
	TemplateElement* GenderWriterElement::copy() {
		return new GenderWriterElement(variableName,argList);
	}
	
	// ------------- PluralWriterElement class
	PluralWriterElement::PluralWriterElement(std::string varName, std::map<std::string, std::string> &list):
		variableName(varName), argList(list) {};
	
	std::string PluralWriterElement::execute(Template& parent) {
		long number = parent.findNumber(variableName);
		std::string numberAsStr = std::to_string(number);
		std::string numClass = parent.languageFeature->pluralizer(number,numberAsStr);
		
		if( auto found = argList.find(numClass); found != argList.end() )
			return found->second;
		else
			return "";
	}
	
	TemplateElement* PluralWriterElement::copy() {
		return new PluralWriterElement(variableName, argList);
	}
	
	// ---------- Template class
	Template::Template(std::string string_template, Feature* langFeature):
	languageFeature(langFeature) {
		auto parsedString = basicParseString(string_template);
		
		for(auto &basic_element : parsedString) {
			switch( basic_element->getType() ) {
				case ElementType::STRING:
					elements.emplace_back( 
						new StringTemplateElement(dynamic_cast<StringElement*>(basic_element.get())) 
						);
					break;
				
				case ElementType::SUBSTITUTION: {
					elements.emplace_back(
						new SubstitutionTemplateElement(
							dynamic_cast<SubstElement*>(basic_element.get())->getVariableName()
						)	
					);
				}
				break;
				
				case ElementType::FUNCTION: {
					FunctElement* fun = dynamic_cast<FunctElement*>(basic_element.get());
					TemplateElement* newElem = makeFunction(fun->getFunctionName(),fun->arguments,"");
					if( newElem != nullptr ) {
						elements.emplace_back( newElem );
					}
				}
				break;
				
				case ElementType::FUNCTION_WITH_SUBSTITUTION: {
					FunctAndSubstElement* fun = dynamic_cast<FunctAndSubstElement*>(basic_element.get());
					TemplateElement* newElem = makeFunction(fun->getFunctionName(),fun->arguments,fun->varName);
					if( newElem != nullptr ) {
						elements.emplace_back( newElem );
					}
				}
				break;
					
				default:
					break;
			}
		}
	}
	
	/**
	 * Converst an argument list from table to map based on a ';'-separated list
	 * which describes the position in arg-list -> the index-name in map
	 */
	std::map<std::string,std::string> Template::convertTableToMap(std::vector<std::string> &table, const char* replacementList) {
		std::map<std::string,std::string> paramsMap;
		for(unsigned int n=0; n < table.size(); ++n) {
			std::string argName = mls::getNthOnSClist(n,replacementList);
			paramsMap.insert_or_assign(argName, table[n]);
		}
		return paramsMap;
	}
			
	
	/**
	 * Makes decision of what a template element produce form function name.
	*/
	TemplateElement* Template::makeFunction(std::string name, FunctArgsBase* arguments, std::string varName) {
		
		//FUNCTION: +SG -> SetGender = sets gender in template having that function
		if( name == "SG" ) {
			//this function gets only one argument
			if( arguments == nullptr )
				throw WrongFunctionArguments();
			if( arguments->getArgType() != FunctionArgs::ONE_ARG )
				throw WrongFunctionArguments();
			
			/*this*/gender = static_cast<OneFunctArg*>(arguments)->getValue();
			
			//this function doesn't generate template element
			return nullptr;
		}
		
		//FUNCTION: var!G a=b || var!G:a,b -> GenderWriter = writes output based on the variable's gender
		if( name == "G" ) {
			//GenderWriter requires a valid variable name
			if( varName == "" )
				throw RequiredVariableNotFound();
			//GenderWriter requires either a list or a map arguments
			if( arguments == nullptr )
				throw WrongFunctionArguments();
				
			switch( arguments->getArgType() ) {
				case FunctionArgs::MAP_ARGS: {
					auto &list = static_cast<MapFunctArg*>(arguments)->paramsMap;
					return new GenderWriterElement(varName,list);
				}
				break;
				
				case FunctionArgs::TABLE_ARGS: {
					auto &table = static_cast<TabFunctArg*>(arguments)->paramsTable;
					auto params = convertTableToMap(table, languageFeature->genderList);
					return new GenderWriterElement(varName, params);
				}
				break;
				
				default:
					throw WrongFunctionArguments();
			}
			
			
		}
		
		//FUNCTION: var!C=a -> CaseSetter = sets case on a given variable
		//FUNCTION: +C a=b ... || +C:a,b -> CaseWriter = outputs text based on case given
		if( name == "C" ) {
			if( varName != "" ) { //CaseSetter
				//CaseSetter requires only one argument
				if( arguments == nullptr )
					throw WrongFunctionArguments();
				if( arguments->getArgType() != FunctionArgs::ONE_ARG )
					throw WrongFunctionArguments();
				
				const std::string caseName = static_cast<OneFunctArg*>(arguments)->getValue();
				return new CaseSetterElement(varName, caseName);
			}
			else //CaseWriter
			{
				//CaseWriter requires either a table or a map arguments
				if( arguments == nullptr )
					throw WrongFunctionArguments();
				
				//CaseWriter searches for this
				this->arguments.insert({"__CASE__",Template("",languageFeature)});
				
				switch( arguments->getArgType() ) {
					case FunctionArgs::MAP_ARGS: {
						auto &list = static_cast<MapFunctArg*>(arguments)->paramsMap;
						return new CaseWriterElement(list);
					}
					break;
					
					case FunctionArgs::TABLE_ARGS: {
						auto &table = static_cast<TabFunctArg*>(arguments)->paramsTable;
						auto params = convertTableToMap(table, languageFeature->casesList);
						return new CaseWriterElement(params);
					}
					break;
					
					default:
						throw WrongFunctionArguments();
				}
				
				
			}
		}
		
		//FUNCTION: var!P:a,b || var!P: one=a -> PluralWriter = gets text based on a number
		if( name == "P" ) {
			//P requires a valid variable name
			if( varName == "" )
				throw RequiredVariableNotFound();
			//P requires a table or a map as its arguments
			if( arguments == nullptr )
				throw WrongFunctionArguments();
			
			switch( arguments->getArgType() ) {
				case FunctionArgs::MAP_ARGS: {
						auto &list = static_cast<MapFunctArg*>(arguments)->paramsMap;
						return new PluralWriterElement(varName, list);
					}
					break;
					
					case FunctionArgs::TABLE_ARGS: {
						auto &table = static_cast<TabFunctArg*>(arguments)->paramsTable;
						auto params = convertTableToMap(table, languageFeature->pluralsList);
						return new PluralWriterElement(varName, params);
					}
					break;
					
					default:
						throw WrongFunctionArguments();
			}
		}
		
		return nullptr;
	}
	
	///Tries to find variable of template type by name from template's list
	Template& Template::findArgument(std::string name) {
		if( auto arg = arguments.find(name); arg == arguments.end() ) 
			throw RequiredVariableNotFound();
		else
			if( std::holds_alternative<Template>(arg->second) )
				return std::get<Template>(arg->second);
			else
				throw RequiredVariableNotFound();
	}
	
	///Tries to find variable of numeric type by name from template's list
	long Template::findNumber(std::string name) {
		if( auto arg = arguments.find(name); arg == arguments.end() ) 
			throw RequiredVariableNotFound();
		else
			if( std::holds_alternative<long>(arg->second) )
				return std::get<long>(arg->second);
			else
				throw RequiredVariableNotFound();
	}
	
	Template::Template(const Template& other) {
		for(const auto& other_element : other.elements ) {
			elements.emplace_back( other_element->copy() );
		}
		for(const auto& other_argument : other.arguments ) {
			arguments.insert(other_argument);
		}
		gender = other.gender;
		languageFeature = other.languageFeature;
	}
	
	Template& Template::operator=(const Template& other) {
		if(this == &other) return *this;
		
		elements.clear();
		for(const auto& other_element : other.elements ) {
			elements.emplace_back( other_element->copy() );
		}
		arguments = other.arguments;
		gender = other.gender;
		languageFeature = other.languageFeature;
		
		return *this;
	}
	
	Template& Template::apply(std::string name, std::string value) {
		return apply(name, Template(value,languageFeature));
	}
	
	Template& Template::apply(std::string name, Template value) {
		arguments.insert_or_assign(name,value);
		
		return *this;
	}
	
	Template& Template::apply(std::string name, long value) {
		arguments.insert_or_assign(name, value);
		
		return *this;
	}
	
	std::string Template::get() {
		std::string output;
		
		for(const auto& element : elements) {
			//std::cout << "output =" << output << std::endl;
			output += element->execute(*this);
		}
		
		return output;
	}
	
	std::string Template::getGender() {
		return gender;
	}
	
}//end namespace mls

//--------------------TESTS
#ifdef TEMPLATE_OBJECT_TEST

using namespace mls;
using namespace std;

#include "basic_parse.cpp"
#include "language_features.cpp"

Feature* defaultLang = &FeaturesByLang["en_US"];
Feature* langWithGenders = &FeaturesByLang["pl_PL"];
Feature* langWithCases = &FeaturesByLang["pl_PL"];

BOOST_AUTO_TEST_CASE( testSimpleStringShouldGenerateTheSameString )
{
	Template t{"simple string", defaultLang};
	string output = t.get();
	BOOST_TEST_REQUIRE( (output == string("simple string")) );
}

BOOST_AUTO_TEST_CASE( testSetGenderDoesntGenerateStringButSetsTemplateGender )
{
	Template t{"%{+SG=male}%", defaultLang};
	string output = t.get();
	BOOST_TEST_REQUIRE( output == string("") );
	BOOST_TEST_REQUIRE( t.getGender() == string("male") );
}

BOOST_AUTO_TEST_CASE( testCaseSystem )
{
	Template stringWithCase{"pociąg%{+C nom={} gen={u}}%", defaultLang};
	Template stringSetter{"ten %{train!C=nom}%, ławka %{train!C=gen}%", defaultLang};
	
	string output = stringSetter.apply("train",stringWithCase).get();
	BOOST_TEST_REQUIRE( output == string("ten pociąg, ławka pociągu") );
}


BOOST_AUTO_TEST_CASE( testCaseSystemWithParamsAsTable )
{
	Template stringWithCase{"pociąg%{+C:,u,owi,,iem,u,}%", langWithCases};
	Template stringSetter{"ten %{train!C=nom}%, ławka %{train!C=gen}%", langWithCases};
	
	string output = stringSetter.apply("train",stringWithCase).get();
	BOOST_TEST_REQUIRE( output == string("ten pociąg, ławka pociągu") );
}


BOOST_AUTO_TEST_CASE( testGenderSystem )
{
	Template isBig{"%{item}% magn%{item!G m={us} f={a} n={um}}% est", defaultLang};
	Template well{"%{+SG=m}%Puteus", defaultLang};
	Template factory{"%{+SG=f}%Officina", defaultLang};
	Template square{"%{+SG=n}%Forum", defaultLang};
	
	string output = isBig.apply("item",well).get();
	BOOST_TEST_REQUIRE( output == string("Puteus magnus est") );
	
	output = isBig.apply("item",factory).get();
	BOOST_TEST_REQUIRE( output == string("Officina magna est") );
	
	output = isBig.apply("item",square).get();
	BOOST_TEST_REQUIRE( output == string("Forum magnum est") );
}

BOOST_AUTO_TEST_CASE( testGenderSystemWithParamsAsTable )
{
	Template isBig{"%{item}% jest wielk%{item!G:i,a,ie}%", langWithGenders};
	Template male{"%{+SG=m}%Budynek", langWithGenders};
	Template female{"%{+SG=f}%Ławka", langWithGenders};
	Template neuter{"%{+SG=n}%Drzewo", langWithGenders};
	
	string output = isBig.apply("item",male).get();
	BOOST_TEST_REQUIRE( output == string("Budynek jest wielki") );
	
	output = isBig.apply("item",female).get();
	BOOST_TEST_REQUIRE( output == string("Ławka jest wielka") );
	
	output = isBig.apply("item",neuter).get();
	BOOST_TEST_REQUIRE( output == string("Drzewo jest wielkie") );
}

BOOST_AUTO_TEST_CASE( testPluralForms )
{
	Template numFiles{"%{num}% file%{num!P one={} other={s}}%", defaultLang};
	
	string output = numFiles.apply("num",1).get();
	BOOST_TEST_REQUIRE( output == string("1 file") );
	
	output = numFiles.apply("num",2).get();
	BOOST_TEST_REQUIRE( output == string("2 files") );
}

BOOST_AUTO_TEST_CASE( testPluralFormsWithParamsAsTable )
{
	Template numFiles{"%{num}% file%{num!P:,s}%", defaultLang};
	
	string output = numFiles.apply("num",1).get();
	BOOST_TEST_REQUIRE( output == string("1 file") );
	
	output = numFiles.apply("num",2).get();
	BOOST_TEST_REQUIRE( output == string("2 files") );
}

#endif //end tests
