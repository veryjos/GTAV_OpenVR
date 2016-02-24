#include "DetourManager.hpp"

using namespace OVRInject;

DetourManager::DetourManager()
{
	if (MH_Initialize() != MH_OK) {
		throw std::exception("Failed to initialize MinHook");
	}
}

DetourManager::~DetourManager()
{
	for (auto f : detouredFuncs)
	{
		f->Deactivate();
		delete f;
	}

	MH_Uninitialize();
}