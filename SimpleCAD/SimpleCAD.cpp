
// SimpleCAD.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define OLC_PGE_APPLICATION
//#include <iostream>
#include "olcPixelGameEngine.h"

// Forward Declaration
struct sShape;

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


	// Since they don't have access to the WorldToScreen, ScreenToWorld function, provide local implementation
	static float fWorldScale;
	static olc::vf2d vWorldOffset;

	void WorldToScreen(const olc::vf2d& v, int& nScreenX, int& nScreenY)
	{
		nScreenX = (int)((v.x - vWorldOffset.x) * fWorldScale);
		nScreenY = (int)((v.y - vWorldOffset.y) * fWorldScale);

	}
	// virtual function so derived class must provide implementation to override this
	// This function takes a pointer of a PixelGameEngine object because sShape is defined outside the PixelGameEngine
	virtual void DrawYourself(olc::PixelGameEngine* pge) = 0;

	// sShape is responsible for creating new node, it takes in position of world space, create the node
	sNode* GetNextNode(const olc::vf2d &p)
	{	
		// Check if the shape has already reached max nodes defined, return nullptr if it has as nothing new can be drawn
		if (vecNodes.size() == nMaxNodes)
			return nullptr;

		// else create a new node and add to shape node vector
		sNode n;
		n.parent = this;
		n.pos = p;
		vecNodes.push_back(n);
		return &vecNodes[vecNodes.size() - 1];
	}

	void DrawNodes(olc::PixelGameEngine *pge)
	{
		for (auto &n : vecNodes)
		{
			int sx, sy;
			WorldToScreen(n.pos, sx, sy);
			pge->FillCircle(sx, sy, 2, olc::RED);
		}
	}
};

// Initialise static members of sShape
float sShape::fWorldScale = 1.0f;
olc::vf2d sShape::vWorldOffset = { 0,0 };

// Line struct, inherits from sSHape struct
struct sLine : public sShape
{
	// Constructor
	sLine()
	{
		nMaxNodes = 2;
		// GetNextNodes() returns a pointer to a vector element which is bad
		// However, we can reserve memory space to prevent memory fragmentation
		vecNodes.reserve(nMaxNodes);
	}

	// Own implementation of DrawYourself() to override virtual method in sShape
	void DrawYourself(olc::PixelGameEngine* pge) override
	{
		int sx, sy, ex, ey;
		// Convert coordinates of vecNodes in WorldSpace to Screen Space
		WorldToScreen(vecNodes[0].pos, sx, sy);
		WorldToScreen(vecNodes[1].pos, ex, ey);
		// Draw line with method in pge object
		pge->DrawLine(sx, sy, ex, ey,col);
	}
};

class Example : public olc::PixelGameEngine {
public:
	Example() {
		sAppName = "SimpleCAD";
	}

private:
	olc::vf2d vOffset = { 0.0f,0.0f };
	olc::vf2d vStartPan = { 0.0f,0.0f };
	float fScale = 10.0f;
	float fGrid = 1.0f;

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

	// A pointer to a shape that is currently being defined
	// by the placment of nodes
	sShape* tempShape = nullptr;

	// A list of pointers to all shapes which have been drawn
	// so far
	std::list<sShape*> listShapes;

	// A pointer to a node that is currently selected. Selected 
	// nodes follow the mouse cursor
	sNode* selectedNode = nullptr;

	// "Snapped" mouse location
	olc::vf2d vCursor = { 0, 0 };


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

		// User interaction to draw line
		if (GetKey(olc::Key::L).bPressed)
		{
			tempShape = new sLine();
			// Place first node at the location of keypress
			selectedNode = tempShape->GetNextNode(vCursor);
			
			// Get Second Node with mouse
			selectedNode = tempShape->GetNextNode(vCursor);
		}

		if (selectedNode != nullptr)
		{
			// Set the pos of selectedNode to the snapped vCursor location
			selectedNode->pos = vCursor;
		}

		// Place the next node when left mouse is pressed
		if (GetMouse(0).bReleased)
		{
			// Check if we can place more nodes, if not, set colour to white for drawing
			if (tempShape != nullptr)
			{
				selectedNode = tempShape->GetNextNode(vCursor);
				if (selectedNode == nullptr)
				{
					tempShape->col = olc::WHITE;
					listShapes.push_back(tempShape);
					tempShape = nullptr; // Thanks @howlevergreen /Disord
				}

			}
			else
			{
				selectedNode = nullptr;
			}
		}
		
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

		// Update static variables of sShape which scales the same as the ones
		// defined in pge
		sShape::fWorldScale = fScale;
		sShape::vWorldOffset = vOffset;

		// Draw All Existing Shapes
		for (auto& shape : listShapes)
		{
			shape->DrawYourself(this);
			shape->DrawNodes(this);
		}

		// Draw shape defined currently
		if (tempShape != nullptr)
		{
			tempShape->DrawYourself(this);
			tempShape->DrawNodes(this);
		}
		
		// Draw cursor
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

