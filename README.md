#  *Mu*lti *Lan*guage *String*
An i18n and l10n library for C++. It aims at being easy to use for both programmer and translator, yet
still powerful. 

[The latest release: 1.1](https://github.com/felix-leg/mulan-string/releases/download/v1.1/mulan-string-1.1.tar.gz)

## How MuLan-String works
The idea of MLS is based on *template strings*. From the programmer's point of view the usage comes down to:
```c++
std::string filesSelectedString = _("%{num}% file%{num!P:,s}% selected.").apply("num", count).get();
```

From the translator's point of view **MuLan-String** offers tools to produce templates which are
sensitive to **genders**, **cases**, and **pluralization rules** of the language transtlator translates into. 

## What is in the project
* the **mulanstring** folder: contains the library and is meant to be copied into your compiler's include directory
* the **example** folder: has got an example project using the library
* the **manual** folder: in it there is The project's manual in the PDF format. It covers the usage of the library. 

## The project so far
The project is in the 1.0 version now. However the project is very fresh and there is still room for improvement!

For now the project's lacks are:
* **Contributors:** the project needs contributors! If you want to add something to it, feel free to ask.
* **Supported languages list:** the list is still short, so everyone who think there is some language not included in the list, can contribute to the project
* **Features:** the aim of the project is to be a powerful library being able to meet different language requirements. For it, it needs contributors with ideas of features the project must have to cope with this goal.
