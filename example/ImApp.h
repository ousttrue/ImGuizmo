#pragma once

#include "glutil.h"

class Impl;
namespace ImApp
{

struct Config
{
	Config() : mWidth(1280), mHeight(720), mFullscreen(false)
	{
	}
	int mWidth;
	int mHeight;
	bool mFullscreen;
};

struct ImApp
{
	static ImApp *mInstance;
	static ImApp *Instance()
	{
		return mInstance;
	}

	Impl *m_impl = nullptr;
	ImApp();
	~ImApp();
	int Init(const Config &config = Config());
	void NewFrame();
	void EndFrame();
	void Finish();
	bool Done();
};

} // namespace ImApp