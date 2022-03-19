#pragma once
#ifndef MULAN_STRING_PREPARSER
#define MULAN_STRING_PREPARSER

#include <string_view>
#include <string>
#include <utility>
#include <vector>
#include <map>

//CUT-START

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

//CUT-END

#endif //!MULAN_STRING_PREPARSER
