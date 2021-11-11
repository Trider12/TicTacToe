#include "MainApp.hpp"

int main() {
	for (MainApp app; app.isRunning(); app.update());

	return 0;
}