#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE template_preparser_module
#include <boost/test/unit_test.hpp>

#include <preparser.h>

template<typename T>
void clearVector(const std::vector<T> &theVector) {
	for(auto elem : theVector) {
		delete elem;
	}
}

BOOST_AUTO_TEST_CASE( testNonTemplate ) {
	std::string nonTemplate{"NonTemplate"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(nonTemplate);
	
	if( charList.size() == 1 && fnList.size() == 0 ) {
		std::string compStr{charList[0]};
		clearVector(charList);
		
		BOOST_TEST_REQUIRE( nonTemplate == compStr );
	} else {
		clearVector(charList);
		clearVector(fnList);
		BOOST_ERROR( "Wrong size" );
	}
}

BOOST_AUTO_TEST_CASE( testComments ) {
	std::string commentTemplate{"ab%{#comment#}%cd"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(commentTemplate);
	if( charList.size() == 2 && fnList.size() == 1 ) {
		std::string prefix{charList[0]};
		std::string suffix{charList[1]};
		clearVector(charList);
		
		if( fnList[0] != nullptr ) {
			delete fnList[0];
			BOOST_ERROR( "A function returned when expected a null pointer" );
		}
		BOOST_TEST_REQUIRE( prefix == "ab" );
		BOOST_TEST_REQUIRE( suffix == "cd" );
	} else {
		clearVector(charList);
		clearVector(fnList);
		BOOST_ERROR( "Wrong size" );
	}
}

BOOST_AUTO_TEST_CASE( testVarOnly ) {
	std::string_view varTemplate{"a%{var}%b"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(varTemplate);
	
	if( charList.size() == 2 && fnList.size() == 1) {
		std::string prefix{charList[0]};
		std::string suffix{charList[1]};
		clearVector(charList);
		
		auto fn = fnList[0];
		if( fn == nullptr ) {
			BOOST_ERROR( "Null pointer where expected a parsed function" );
		} else {
			auto varName = fn->varName;
			delete fn;
			
			BOOST_TEST_REQUIRE( varName == "var" );
			BOOST_TEST_REQUIRE( prefix == "a" );
			BOOST_TEST_REQUIRE( suffix == "b" );
		}
	} else {
		clearVector(charList);
		clearVector(fnList);
		BOOST_ERROR( "Wrong size" );
	}
}

BOOST_AUTO_TEST_CASE( testNoVarFunctionOneArg ) {
	std::string_view testTemplate{"a%{+F=b}%c"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(testTemplate);
	
	if( charList.size() == 2 && fnList.size() == 1) {
		std::string prefix{charList[0]};
		std::string suffix{charList[1]};
		clearVector(charList);
		
		auto fn = fnList[0];
		if( fn == nullptr ) {
			BOOST_ERROR( "Null pointer where expected a parsed function" );
		} else {
			auto varName = fn->varName;
			char fnName1 = fn->name[0];
			char fnName2 = fn->name[1];
			auto type = fn->getType();
			std::string arg{""};
			if(type == mls::preparse::ParsedTemplateFunction::Type::ONE_ARG) {
				arg = static_cast<mls::preparse::OneArgFunction*>(fn)->argument;
			}
			delete fn;
			
			BOOST_TEST_REQUIRE( varName == "" );
			BOOST_TEST_REQUIRE( fnName1 == 'F' );
			BOOST_TEST_REQUIRE( fnName2 == '\0' );
			BOOST_TEST_REQUIRE( (type == mls::preparse::ParsedTemplateFunction::Type::ONE_ARG) );
			BOOST_TEST_REQUIRE( arg == "b" );
			BOOST_TEST_REQUIRE( prefix == "a" );
			BOOST_TEST_REQUIRE( suffix == "c" );
		}
	} else {
		clearVector(charList);
		clearVector(fnList);
		BOOST_ERROR( "Wrong size" );
	}
}

BOOST_AUTO_TEST_CASE( testWithVarFunctionOneArg ) {
	std::string_view testTemplate{"a%{var!F=b}%c"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(testTemplate);
	
	if( charList.size() == 2 && fnList.size() == 1) {
		std::string prefix{charList[0]};
		std::string suffix{charList[1]};
		clearVector(charList);
		
		auto fn = fnList[0];
		if( fn == nullptr ) {
			BOOST_ERROR( "Null pointer where expected a parsed function" );
		} else {
			auto varName = fn->varName;
			char fnName1 = fn->name[0];
			char fnName2 = fn->name[1];
			auto type = fn->getType();
			std::string arg{""};
			if(type == mls::preparse::ParsedTemplateFunction::Type::ONE_ARG) {
				arg = static_cast<mls::preparse::OneArgFunction*>(fn)->argument;
			}
			delete fn;
			
			BOOST_TEST_REQUIRE( varName == "var" );
			BOOST_TEST_REQUIRE( fnName1 == 'F' );
			BOOST_TEST_REQUIRE( fnName2 == '\0' );
			BOOST_TEST_REQUIRE( (type == mls::preparse::ParsedTemplateFunction::Type::ONE_ARG) );
			BOOST_TEST_REQUIRE( arg == "b" );
			BOOST_TEST_REQUIRE( prefix == "a" );
			BOOST_TEST_REQUIRE( suffix == "c" );
		}
	} else {
		clearVector(charList);
		clearVector(fnList);
		BOOST_ERROR( "Wrong size" );
	}
}

BOOST_AUTO_TEST_CASE( testNoVarFunctionOneArgWrongFormat ) {
	std::string_view testTemplate{"a%{+F=}%c"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(testTemplate);
	
	if( charList.size() == 2 && fnList.size() == 1) {
		clearVector(charList);
		
		auto fn = fnList[0];
		if( fn != nullptr ) {
			delete fn;
			BOOST_ERROR( "Function returned on wrong template" );
		}
	} else {
		clearVector(charList);
		clearVector(fnList);
	}
}

BOOST_AUTO_TEST_CASE( testNoVarFunctionTableArg ) {
	std::string_view testTemplate{"a%{+F:b,c}%d"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(testTemplate);
	
	if( charList.size() == 2 && fnList.size() == 1) {
		std::string prefix{charList[0]};
		std::string suffix{charList[1]};
		clearVector(charList);
		
		auto fn = fnList[0];
		if( fn == nullptr ) {
			BOOST_ERROR( "Null pointer where expected a parsed function" );
		} else {
			auto varName = fn->varName;
			char fnName1 = fn->name[0];
			char fnName2 = fn->name[1];
			auto type = fn->getType();
			std::vector<std::string> args;
			if(type == mls::preparse::ParsedTemplateFunction::Type::TABLE_ARG) {
				args = static_cast<mls::preparse::TableArgFunction*>(fn)->arguments;
			}
			delete fn;
			
			BOOST_TEST_REQUIRE( varName == "" );
			BOOST_TEST_REQUIRE( fnName1 == 'F' );
			BOOST_TEST_REQUIRE( fnName2 == '\0' );
			BOOST_TEST_REQUIRE( (type == mls::preparse::ParsedTemplateFunction::Type::TABLE_ARG) );
			BOOST_TEST_REQUIRE( args.size() == 2 );
			BOOST_TEST_REQUIRE( args[0] == "b" );
			BOOST_TEST_REQUIRE( args[1] == "c" );
			BOOST_TEST_REQUIRE( prefix == "a" );
			BOOST_TEST_REQUIRE( suffix == "d" );
		}
	} else {
		clearVector(charList);
		clearVector(fnList);
		BOOST_ERROR( "Wrong size" );
	}
}

BOOST_AUTO_TEST_CASE( testWithVarFunctionTableArg ) {
	std::string_view testTemplate{"a%{var!F:b,c}%d"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(testTemplate);
	
	if( charList.size() == 2 && fnList.size() == 1) {
		std::string prefix{charList[0]};
		std::string suffix{charList[1]};
		clearVector(charList);
		
		auto fn = fnList[0];
		if( fn == nullptr ) {
			BOOST_ERROR( "Null pointer where expected a parsed function" );
		} else {
			auto varName = fn->varName;
			char fnName1 = fn->name[0];
			char fnName2 = fn->name[1];
			auto type = fn->getType();
			std::vector<std::string> args;
			if(type == mls::preparse::ParsedTemplateFunction::Type::TABLE_ARG) {
				args = static_cast<mls::preparse::TableArgFunction*>(fn)->arguments;
			}
			delete fn;
			
			BOOST_TEST_REQUIRE( varName == "var" );
			BOOST_TEST_REQUIRE( fnName1 == 'F' );
			BOOST_TEST_REQUIRE( fnName2 == '\0' );
			BOOST_TEST_REQUIRE( (type == mls::preparse::ParsedTemplateFunction::Type::TABLE_ARG) );
			BOOST_TEST_REQUIRE( args.size() == 2 );
			BOOST_TEST_REQUIRE( args[0] == "b" );
			BOOST_TEST_REQUIRE( args[1] == "c" );
			BOOST_TEST_REQUIRE( prefix == "a" );
			BOOST_TEST_REQUIRE( suffix == "d" );
		}
	} else {
		clearVector(charList);
		clearVector(fnList);
		BOOST_ERROR( "Wrong size" );
	}
}

BOOST_AUTO_TEST_CASE( testNoVarFunctionTableArgWrongFormat ) {
	std::string_view testTemplate{"a%{+F:}%c"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(testTemplate);
	
	if( charList.size() == 2 && fnList.size() == 1) {
		clearVector(charList);
		
		auto fn = fnList[0];
		if( fn != nullptr ) {
			delete fn;
			BOOST_ERROR( "Function returned on wrong template" );
		}
	} else {
		clearVector(charList);
		clearVector(fnList);
	}
}

BOOST_AUTO_TEST_CASE( testNoVarFunctionHashArg ) {
	std::string_view testTemplate{"a%{+F b={B} c={C}}%d"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(testTemplate);
	
	if( charList.size() == 2 && fnList.size() == 1) {
		std::string prefix{charList[0]};
		std::string suffix{charList[1]};
		clearVector(charList);
		
		auto fn = fnList[0];
		if( fn == nullptr ) {
			BOOST_ERROR( "Null pointer where expected a parsed function" );
		} else {
			auto varName = fn->varName;
			char fnName1 = fn->name[0];
			char fnName2 = fn->name[1];
			auto type = fn->getType();
			std::map<std::string, std::string> args;
			if(type == mls::preparse::ParsedTemplateFunction::Type::HASH_ARG) {
				args = static_cast<mls::preparse::HashArgFunction*>(fn)->arguments;
			}
			delete fn;
			
			BOOST_TEST_REQUIRE( varName == "" );
			BOOST_TEST_REQUIRE( fnName1 == 'F' );
			BOOST_TEST_REQUIRE( fnName2 == '\0' );
			BOOST_TEST_REQUIRE( (type == mls::preparse::ParsedTemplateFunction::Type::HASH_ARG) );
			BOOST_TEST_REQUIRE( args.size() == 2 );
			BOOST_TEST_REQUIRE( args["b"] == "B" );
			BOOST_TEST_REQUIRE( args["c"] == "C" );
			BOOST_TEST_REQUIRE( prefix == "a" );
			BOOST_TEST_REQUIRE( suffix == "d" );
		}
	} else {
		clearVector(charList);
		clearVector(fnList);
		BOOST_ERROR( "Wrong size" );
	}
}

BOOST_AUTO_TEST_CASE( testWithVarFunctionHashArg ) {
	std::string_view testTemplate{"a%{var!F b={B} c={C}}%d"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(testTemplate);
	
	if( charList.size() == 2 && fnList.size() == 1) {
		std::string prefix{charList[0]};
		std::string suffix{charList[1]};
		clearVector(charList);
		
		auto fn = fnList[0];
		if( fn == nullptr ) {
			BOOST_ERROR( "Null pointer where expected a parsed function" );
		} else {
			auto varName = fn->varName;
			char fnName1 = fn->name[0];
			char fnName2 = fn->name[1];
			auto type = fn->getType();
			std::map<std::string, std::string> args;
			if(type == mls::preparse::ParsedTemplateFunction::Type::HASH_ARG) {
				args = static_cast<mls::preparse::HashArgFunction*>(fn)->arguments;
			}
			delete fn;
			
			BOOST_TEST_REQUIRE( varName == "var" );
			BOOST_TEST_REQUIRE( fnName1 == 'F' );
			BOOST_TEST_REQUIRE( fnName2 == '\0' );
			BOOST_TEST_REQUIRE( (type == mls::preparse::ParsedTemplateFunction::Type::HASH_ARG) );
			BOOST_TEST_REQUIRE( args.size() == 2 );
			BOOST_TEST_REQUIRE( args["b"] == "B" );
			BOOST_TEST_REQUIRE( args["c"] == "C" );
			BOOST_TEST_REQUIRE( prefix == "a" );
			BOOST_TEST_REQUIRE( suffix == "d" );
		}
	} else {
		clearVector(charList);
		clearVector(fnList);
		BOOST_ERROR( "Wrong size" );
	}
}

BOOST_AUTO_TEST_CASE( testNoVarFunctionHashArgWrongFormat ) {
	std::string_view testTemplate{"a%{+F }%c"};
	
	auto[charList, fnList] = mls::preparse::preparse_template(testTemplate);
	
	if( charList.size() == 2 && fnList.size() == 1) {
		clearVector(charList);
		
		auto fn = fnList[0];
		if( fn != nullptr ) {
			delete fn;
			BOOST_ERROR( "Function returned on wrong template" );
		}
	} else {
		clearVector(charList);
		clearVector(fnList);
	}
}
