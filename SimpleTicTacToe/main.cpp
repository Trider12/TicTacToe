#include "MainApp.hpp"

int main()
{
	for (std::unique_ptr<IMainApp> app = std::make_unique<MainApp>(); app->isAppRunning(); app->update());

	return 0;
}