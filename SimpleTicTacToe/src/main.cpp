#include "MainApp.hpp"

int main()
{
	for (MainApp app; app.isAppRunning(); app.update());

	return 0;
}