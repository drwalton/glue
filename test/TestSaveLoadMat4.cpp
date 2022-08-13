#include "glue/Files.hpp"
#include <iostream>

int main()
{
	glue::mat4 test;
	test <<
		1, 2, 3, 4,
		5, 6, 7, 8,
		9, 10, 11, 12,
		13, 14, 15, 16;
	glue::saveMat4ToFile("Test.mat4", test);
	glue::mat4 loaded = glue::loadMat4FromFile("Test.mat4");

	std::cout << "Initial matrix: \n" << test;
	std::cout << "\n\nAfter loading: \n" << loaded;
	std::cin.get();

	return 0;
}
