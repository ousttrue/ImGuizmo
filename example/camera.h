#pragma once


struct Camera
{
	bool isPerspective = false;
	bool firstFrame = true;

	// for perspective
	float fov = 27.f;

	// for orthographic
	float viewWidth = 10.f;

	float cameraProjection[16];

	// view
	float camYAngle = 165.f / 180.f * 3.14159f;
	float camXAngle = 0.f / 180.f * 3.14159f;
	float camDistance = 8.f;
	float cameraView[16] =
		{1.f, 0.f, 0.f, 0.f,
		 0.f, 1.f, 0.f, 0.f,
		 0.f, 0.f, 1.f, 0.f,
		 0.f, 0.f, 0.f, 1.f};

	void Update(float width, float height);
	void Gui();
};
