#include "glue/SphericalHarmonics.hpp"
#ifdef _WIN32
#include <SDL.h>
#endif
#include <iostream>

void testDir(const glue::vec3 &dir) {
	float theta, phi;
	glue::dirToSphericalAngles(dir, &theta, &phi);
	glue::vec3 newDir = glue::sphericalAngleToDir(theta, phi);
	std::cout << "Dir:\n" << dir << "\nTheta: " << theta << " Phi: " << phi << "\nNew dir:\n" << newDir << "\n\n" << std::endl;
}

int main(int argc, char *argv[])
{
	glue::vec3 dir(1, 0, 0);
	testDir(dir);
	dir = glue::vec3(0, 1, 0);
	testDir(dir);
	dir = glue::vec3(0, 0, 1);
	testDir(dir);
	dir = glue::vec3(-1, 0, 0);
	testDir(dir);
	dir = glue::vec3(0, -1, 0);
	testDir(dir);
	dir = glue::vec3(0, 0, -1);
	testDir(dir);

	dir = glue::vec3(1, 0, -1);
	dir.normalize();
	testDir(dir);
	dir = glue::vec3(0, -1, 1);
	dir.normalize();
	testDir(dir);
	dir = glue::vec3(-1, 0, 1);
	dir.normalize();
	testDir(dir);
	dir = glue::vec3(0, 1, -1);
	dir.normalize();
	testDir(dir);

	std::cin.get();
	
	return 0;
}
