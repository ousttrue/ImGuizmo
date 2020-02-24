#pragma once

#include "glutil.h"

class Impl;
namespace ImApp
{

struct ImApp
{
	Impl *m_impl = nullptr;
	ImApp();
	~ImApp();
	int Init();
	bool NewFrame();
	void EndFrame();
	void Finish();
};

} // namespace ImApp