#pragma once
#ifndef MULAN_STRING_TEMPLATE
#define MULAN_STRING_TEMPLATE

#include <utility>
#include <string_view>
#include <string>
#include <vector>
#include <map>
#include <variant>

#include "mls_locale.h"
#include "preparser.h"
#include "errors.h"
//CUT-START

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

//CUT-END

#endif //!MULAN_STRING_TEMPLATE
