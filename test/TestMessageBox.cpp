#include "glue/MessageBox.hpp"
#include <SDL.h>

int main(int argc, char *argv[])
{
	while (glue::showMessageBoxRetryCancel("TEST MESSAGE", "Retry, or cancel?"));
	glue::showMessageBoxOk("TEST MESSAGE 2", "Hit OK to quit...");

	return 0;
}