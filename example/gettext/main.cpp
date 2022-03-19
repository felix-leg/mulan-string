/**
 * This is an example file demonstrating usage of MuLan String library
*/

//include the right header (taken from the project's main folder)
#define MULAN_STRING_IMPLEMENTATION
#define MULANSTR_THROW_ON_INVALID_TEMPLATE
#include <mulanstring-gettext.hpp>

//the rest of our program
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv) {
	//initialize the backend. 
	//as we use GNU GetText, we need to provide the right parameters
	mls::backend::init("example", nullptr, "./locale");
	
	//now we can use translations
	
	string simpleText = _("Simple text").get();
	cout << "'Simple text' translated as: '" << simpleText << "'\n";
	
	string substText = _("%{var}% text").apply("var","Test").get();
	cout << "'%{var}% text' with var='Test' translated as: '" << substText << "'\n";
	
	auto fileTempl = _("%{num}% file%{num!P:,s}% modifed");
	string fileText = fileTempl.apply("num",1).get();
	cout << "'%{num}% file%{num!P:,s}% modifed' with num=1 translated as: '" << fileText << "'\n";
	fileText = fileTempl.apply("num",3).get();
	cout << "'%{num}% file%{num!P:,s}% modifed' with num=3 translated as: '" << fileText << "'\n";
	fileText = fileTempl.apply("num",8).get();
	cout << "'%{num}% file%{num!P:,s}% modifed' with num=8 translated as: '" << fileText << "'\n";
	
	return 0;
}
