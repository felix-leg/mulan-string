#pragma once

#ifndef MULANSTR_BASIC_PARSE_HPP
#define MULANSTR_BASIC_PARSE_HPP

#include <vector>
#include <string>
#include <memory>
#include <map>

namespace mls {
	
	class BadlyFormatedTemplateString: public std::exception {};
	
	enum class ElementType {
		STRING,
		SUBSTITUTION,
		FUNCTION,
		FUNCTION_WITH_SUBSTITUTION,
	};
	enum class FunctionArgs {
		//NO_ARGS,
		ONE_ARG,
		TABLE_ARGS,
		MAP_ARGS
	};
	
	/// Base class for many versions of argument pass to function
	class FunctArgsBase {
		public:
			virtual FunctionArgs getArgType() = 0;
			virtual ~FunctArgsBase() {};
	};
	
	class OneFunctArg: public FunctArgsBase {
		public:
			virtual FunctionArgs getArgType() override;	
			
			OneFunctArg(std::string value):argValue(value) {};
			std::string getValue();
		private:
			std::string argValue;
	};
	
	class TabFunctArg: public FunctArgsBase {
		public:
			virtual FunctionArgs getArgType() override;
			std::vector<std::string> paramsTable;
	};
	
	class MapFunctArg: public FunctArgsBase {
		public:
			virtual FunctionArgs getArgType() override;
			std::map<std::string,std::string> paramsMap;	
	};
	
	/// Base class for everything, that can exist in template string
	class TemplateElementBase {
		public:
			virtual ElementType getType() = 0;
			virtual ~TemplateElementBase() {};
	};
	
	/// class for simple text outside of tags
	class StringElement: public TemplateElementBase {
		public:
			StringElement(std::string str):strElem(str) {};
			virtual ElementType getType() override;	
			std::string getString();
		private:
			std::string strElem;
	};
	
	/// tag with just a substitution of a variable
	class SubstElement: public TemplateElementBase {
		public:
			SubstElement(std::string variable):varName(variable) {};
			virtual ElementType getType() override;
			std::string getVariableName();
		private:
			std::string varName;
	};
	
	/// tag with only function call
	class FunctElement: public TemplateElementBase {
		public:
			FunctElement(std::string functionName):fnName(functionName),arguments(nullptr) {};
			~FunctElement() { delete arguments; };
			
			virtual ElementType getType() override;
			std::string getFunctionName();
			FunctArgsBase* arguments;
		private:
			std::string fnName;
	};
	
	/// tag with substitution and function call
	class FunctAndSubstElement: public TemplateElementBase {
		public:
			FunctAndSubstElement(std::string functionName):fnName(functionName),arguments(nullptr) {};
			~FunctAndSubstElement() { delete arguments; };
			virtual ElementType getType() override;
			std::string getFunctionName();
			FunctArgsBase* arguments;
			std::string varName;
		private:
			std::string fnName;
	};
	
	using TemplateElementsList = std::vector<std::unique_ptr<TemplateElementBase>>;
	TemplateElementsList basicParseString(std::string templateString);
}; //end namespace mls

#endif //file end
