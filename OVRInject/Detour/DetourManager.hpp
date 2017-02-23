#pragma once
#include "targetver.h"


#include "DetouredFuncs.hpp"

#include <vector>

namespace OVRInject
{
	class DetourManager
	{
	public:
		DetourManager();
		virtual ~DetourManager();

		template <typename T>
		Detour* MakeDetour(void* from)
		{
			Detour* detouredFunc = new T(from);
			detouredFunc->Activate();

			detouredFuncs.push_back(detouredFunc);

			return detouredFunc;
		};

	protected:
		std::vector<Detour*> detouredFuncs;
	};
};