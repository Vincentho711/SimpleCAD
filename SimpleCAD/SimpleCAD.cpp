// SimpleCAD.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define OLC_PGE_APPLICATION
#include <iostream>
#include "olcPixelGameEngine.h"

// Define a node
struct sNode
{
	sShape* parent;
	olc::vf2d pos;
};

struct sShape
{
	std::vector<sNode> vecNodes;
	uint32_t nMaxNodes = 0;
	olc::Pixel col = olc::GREEN;

	// virtual function so derived class must provide implementation to override this
	// This function takes a pointer in the PixelGameEngine because sShape is defined outside the PixelGameEngine
	virtual void DrawYourself(olc::PixelGameEngine* pge) = 0;

	// sShape is responsible for creating new node, it takes in position of world space, create the node
	sNode* GetNextNode(const olc::vf2d& p)
	{	
		// Check if the shape has already reached max nodes defined, return nullptr if it has as nothing new can be drawn
		if (vecNodes.size() == nMaxNodes)
		{
			return nullptr;
		}
		// else create a new node and add to shape node vector
		sNode n;
		n.parent = this;
		n.pos = p;
		vecNodes.emplace_back(n);
		return &vecNodes[vecNodes.size() - 1];
	}
};

class Example : public olc::PixelGameEngine {
public:
	Example() {
		sAppName = "Example";
	}

private:
	olc::vf2d vOffset = { 0.0f,0.0f };
	olc::vf2d vStartPan = { 0.0f,0.0f };
	float fScale = 10.0f;
	float fGrid = 1.0f;

	// "Snapped" mouse location
	olc::vf2d vCursor = { 0, 0 };

	//Convert coordinates from World Space to Screen Space
	void WorldToScreen(const olc::vf2d &v, int &nScreenX, int &nScreenY)
	{
		nScreenX = (int)((v.x - vOffset.x) * fScale);
		nScreenY = (int)((v.y - vOffset.y) * fScale);
	}

	//Convert coordinates from Screen Space to World Space
	void ScreenToWorld(int nScreenX, int nScreenY, olc::vf2d &v)
	{
		v.x = (float)(nScreenX) / fScale + vOffset.x;
		v.y = (float)(nScreenY) / fScale + vOffset.y;
	}



public:
	bool OnUserCreate() override {
		// Configure world space (0,0) to be middle of screen space
		vOffset = { (float)(-ScreenWidth() / 2) / fScale, (float)(-ScreenHeight() / 2) / fScale };
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override {
		olc::vf2d vMouse = { (float)GetMouseX(),(float)GetMouseY() };

		//Panning by dragging mouse
		if (GetMouse(2).bPressed)
		{
			//Store the startPan location
			vStartPan = vMouse;
		}
		if (GetMouse(2).bHeld)
		{
			vOffset -= (vMouse - vStartPan) / fScale;
			vStartPan = vMouse;
		}

		//Zooming using key Q and L
		olc::vf2d vMouseBeforeZoom;
		ScreenToWorld((int)vMouse.x, (int)vMouse.y, vMouseBeforeZoom);

		if (GetKey(olc::Key::Q).bHeld || GetMouseWheel() > 0)
		{
			fScale *= 1.05f;
		}

		if (GetKey(olc::Key::A).bHeld || GetMouseWheel() < 0)
		{
			fScale *= 0.95f;
		}

		olc::vf2d vMouseAfterZoom;
		ScreenToWorld((int)vMouse.x, (int)vMouse.y, vMouseAfterZoom);
		vOffset += (vMouseBeforeZoom - vMouseAfterZoom);
		
		// Snap mouse cursor to nearest grid interval
		vCursor.x = floorf((vMouseAfterZoom.x + 0.5f) * fGrid);
		vCursor.y = floorf((vMouseAfterZoom.y + 0.5f) * fGrid);

		// Clear Screen
		Clear(olc::VERY_DARK_BLUE);

		int sx, sy;
		int ex, ey;

		// Get visible world
		olc::vf2d vWorldTopLeft, vWorldBottomRight;
		ScreenToWorld(0, 0, vWorldTopLeft);
		ScreenToWorld(ScreenWidth(), ScreenHeight(), vWorldBottomRight);

		// Get values just beyond screen boundaries
		vWorldTopLeft.x = floor(vWorldTopLeft.x);
		vWorldTopLeft.y = floor(vWorldTopLeft.y);
		vWorldBottomRight.x = ceil(vWorldBottomRight.x);
		vWorldBottomRight.y = ceil(vWorldBottomRight.y);

		// Draw Grid dots
		for (float x = vWorldTopLeft.x; x < vWorldBottomRight.x; x += fGrid)
		{
			for (float y = vWorldTopLeft.y; y < vWorldBottomRight.y; y += fGrid)
			{
				WorldToScreen({ x, y }, sx, sy);
				Draw(sx, sy, olc::BLUE);
			}
		}

		// Draw World Axis
		WorldToScreen({ 0,vWorldTopLeft.y }, sx, sy);
		WorldToScreen({ 0,vWorldBottomRight.y }, ex, ey);
		DrawLine(sx, sy, ex, ey, olc::GREY, 0xF0F0F0F0);
		WorldToScreen({ vWorldTopLeft.x,0 }, sx, sy);
		WorldToScreen({ vWorldBottomRight.x,0 }, ex, ey);
		DrawLine(sx, sy, ex, ey, olc::GREY, 0xF0F0F0F0);

		//Draw cursor
		WorldToScreen(vCursor, sx, sy);
		DrawCircle(sx, sy, 3,olc::YELLOW);

		//Draw cursor location
		DrawString(15, 15, "X=" + std::to_string(vCursor.x) + ", Y=" + std::to_string(-vCursor.y), olc::YELLOW,2);
		return true;
	}
};

int main()
{
	Example demo;
	if (demo.Construct(800, 480, 1, 1, false))
		demo.Start();
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

