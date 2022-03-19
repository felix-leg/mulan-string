/**
 * @file template.cpp
 * @brief Main template class for MuLan String
 * 
 */

#include <sstream>
#include <cctype>

#include "template.h"

//CUT-START

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

//CUT-END
