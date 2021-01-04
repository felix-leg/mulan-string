#pragma once

#ifndef MULANSTR_TEMPLATE_OBJECT_HPP
#define MULANSTR_TEMPLATE_OBJECT_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>

#include "basic_parse.hpp"
#include "language_features.hpp"

namespace mls {
	
	// ----------- Exceptions
	
	class WrongFunctionCall : public std::exception {};
	class WrongFunctionArguments : public std::exception {};
	class RequiredVariableNotFound : public std::exception {};
	
#	ifdef MULANSTR_THROW_ON_INVALID_TEMPLATE
	class InvalidTemplateState : public std::exception {};
#	endif
	
	// -----------
	
	class Template; //forward declaration
	/**
	 * Base class for any types of element a template can have.
	*/
	class TemplateElement {
		public:
			virtual std::string execute(Template&) = 0;
			virtual TemplateElement* copy() = 0;
	};
	
	/**
	 * Class for simple strings elements
	*/
	class StringTemplateElement : public TemplateElement {
		public:
			StringTemplateElement(StringElement* basic_element );
			virtual std::string execute(Template&) override;
			virtual TemplateElement* copy() override;
		private:
			std::string theString;
			StringTemplateElement(std::string other);
	};
	
	/**
	 * Simple substitution
	*/
	class SubstitutionTemplateElement : public TemplateElement {
		public:
			SubstitutionTemplateElement(std::string valName);
			virtual std::string execute(Template&) override;
			virtual TemplateElement* copy() override;
		private:
			std::string valueName;
	};
	
	/**
	 * Class-function for CaseSetter
	*/
	class CaseSetterElement : public TemplateElement /*TemplateElementWithVariable*/ {
		public:
			CaseSetterElement(std::string varName, std::string caseName);
			virtual std::string execute(Template&) override;
			virtual TemplateElement* copy() override;
			//virtual void assignVariable(std::string name) override;
		private:
			std::string variableName = "";
			std::string caseName = "";
	};
	
	/**
	 * Class-function for CaseWriter
	*/
	class CaseWriterElement : public TemplateElement {
		public:
			CaseWriterElement(std::map<std::string,std::string> &paramsList);
			virtual std::string execute(Template&) override;
			virtual TemplateElement* copy() override;
		private:
			std::map<std::string,std::string> params;
	};
	
	/**
	 * Class-function for GenderWriter
	*/
	class GenderWriterElement : public TemplateElement {
		public:
			GenderWriterElement(std::string varName, std::map<std::string, std::string> &list);
			virtual std::string execute(Template&) override;
			virtual TemplateElement* copy() override;
		private:
			std::map<std::string, std::string> argList;
			std::string variableName;
	};
	
	/**
	 * Class-function for PluralWriter
	*/
	class PluralWriterElement : public TemplateElement {
		public:
			PluralWriterElement(std::string varName, std::map<std::string, std::string>& list);
			virtual std::string execute(Template&) override;
			virtual TemplateElement* copy() override;
		private:
			std::map<std::string, std::string> argList;
			std::string variableName;
	};
	
	/**
	 * The main class responsible for managing argument applying and
	 * generating output string.
	 */
	class Template {
		public:
			Template(std::string string_template, Feature* langFeature);
			Template(const Template& other);
			Template(Template&& other) = default;
			Template& operator=(const Template& other);
			
			///default contructor making an invalid template object (to use when the valid object is obtained later)
			Template();
			
			///assings variable value
			Template& apply(std::string name, std::string value);
			Template& apply(std::string name, Template value);
			Template& apply(std::string name, long value);
			
			///commands the class to produce output string
			std::string get();
			
			///gives gender string associated with this template
			std::string getGender();
		private:
			std::vector<std::unique_ptr<TemplateElement>> elements;
			typedef std::variant<long, Template> ArgType;
			std::map<std::string,ArgType> arguments;
			
			std::string gender = "";
			Feature* languageFeature;
			
			std::map<std::string,std::string> convertTableToMap(std::vector<std::string>& table, const char* replacementList);
			TemplateElement* makeFunction(std::string name, FunctArgsBase* arguments, std::string varName);
			Template& findArgument(std::string name);
			long findNumber(std::string name);
			
		friend class SubstitutionTemplateElement;
		friend class CaseSetterElement;
		friend class CaseWriterElement;
		friend class GenderWriterElement;
		friend class PluralWriterElement;
	};
	
}//end namespace mls


#endif /* MULANSTR_TEMPLATE_OBJECT_HPP */
//file end
