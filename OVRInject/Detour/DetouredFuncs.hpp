#pragma once
#include "targetver.h"


#include "DetourManager.hpp"

#include "MinHook.h"
#include "../Log.hpp"

#define __REPLACE__SIGNATURE__ 0x12345678

namespace OVRInject
{
	struct Detour
	{
		Detour(void* from)
		{
			this->from = from;
		};

		virtual ~Detour()
		{
			Deactivate();
			MH_RemoveHook(from);
		};

		void Activate() {
			if (MH_CreateHook(from, target, (void**)(original)) != MH_OK)
			{
				LOGSTRF("Failed to create hook!\n");
				return;
			}

			if (MH_EnableHook(from) != MH_OK)
			{
				LOGSTRF("Failed to enable hook!\n");
				return;
			}

			LOGSTRF("Successfully created hook from 0x%p to 0x%p Original: 0x%p\n", from, target, original);
		};

		void Deactivate() {
			MH_DisableHook(from);
		};

	protected:
		void* from;
		void* target;
		void* original;
	};
}