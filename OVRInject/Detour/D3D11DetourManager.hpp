#pragma once

#include "DetourManager.hpp"

namespace OVRInject
{
	class D3D11DetourManager : public DetourManager
	{
	public:
		D3D11DetourManager();
		~D3D11DetourManager();
	};
}