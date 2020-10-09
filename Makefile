#this makefile builds tests


%.test: %.cpp
	g++ --std=c++17 -g -DTEST -o $@ $< -lboost_unit_test_framework


