#include "pch.h"
#include "Color.h"

XMFLOAT4 Color::Clear			 = XMFLOAT4(0, 0, 0, 0);
XMFLOAT4 Color::Black			 = XMFLOAT4(0, 0, 0, 255);
XMFLOAT4 Color::LightBlack		 = XMFLOAT4(20, 20, 20, 255);
XMFLOAT4 Color::Grey			 = XMFLOAT4(128, 128, 128, 255);
XMFLOAT4 Color::Silver			 = XMFLOAT4(192, 192, 192, 255);
XMFLOAT4 Color::GhostWhite		 = XMFLOAT4(248, 248, 255, 255);
XMFLOAT4 Color::White			 = XMFLOAT4(255, 255, 255, 255);
XMFLOAT4 Color::DarkRed			 = XMFLOAT4(139, 0, 0, 255);
XMFLOAT4 Color::Red				 = XMFLOAT4(255, 0, 0, 255);
XMFLOAT4 Color::Orange			 = XMFLOAT4(255, 165, 0, 255);
XMFLOAT4 Color::Coral			 = XMFLOAT4(255, 127, 80, 255);
XMFLOAT4 Color::Brown			 = XMFLOAT4(165, 42, 42, 255);
XMFLOAT4 Color::GhostPurple	     = XMFLOAT4(172, 126, 194, 255);
XMFLOAT4 Color::Magenta			 = XMFLOAT4(255, 0, 255, 255);
XMFLOAT4 Color::DarkPink		 = XMFLOAT4(231, 84, 128, 255);
XMFLOAT4 Color::Pink			 = XMFLOAT4(255, 105, 180, 255);
XMFLOAT4 Color::LightPink		 = XMFLOAT4(255, 182, 193, 255);
XMFLOAT4 Color::DarkBlue		 = XMFLOAT4(0, 0, 139, 255);
XMFLOAT4 Color::BlueViolet		 = XMFLOAT4(123, 104, 238, 255);
XMFLOAT4 Color::Blue			 = XMFLOAT4(0, 0, 255, 255);
XMFLOAT4 Color::SkyeBlue		 = XMFLOAT4(0, 191, 255, 255);
XMFLOAT4 Color::Turquoise		 = XMFLOAT4(64, 224, 208, 255);
XMFLOAT4 Color::Cyan			 = XMFLOAT4(0, 255, 255, 255);
XMFLOAT4 Color::DarkGreen		 = XMFLOAT4(0, 108, 0, 255);
XMFLOAT4 Color::Green			 = XMFLOAT4(0, 128, 0, 255);
XMFLOAT4 Color::Chartreuse		 = XMFLOAT4(127, 255, 0, 255);
XMFLOAT4 Color::Lime			 = XMFLOAT4(50, 205, 50, 255);
XMFLOAT4 Color::Olive			 = XMFLOAT4(128, 128, 0, 255);
XMFLOAT4 Color::Yellow			 = XMFLOAT4(255, 255, 0, 255);
XMFLOAT4 Color::Mustard			 = XMFLOAT4(255, 219, 88, 255);
XMFLOAT4 Color::Gold			 = XMFLOAT4(255, 215, 0, 255);

void Color::Set(XMFLOAT4 newColor)
{
	r = newColor.x;
	g = newColor.y;
	b = newColor.z;
	a = newColor.w;
}

void Color::Set(float newR, float newG, float newB, float newA)
{
	r = newR;
	g = newG;
	b = newB;
	a = newA;
}

XMFLOAT4 Color::NormalizeColor(XMFLOAT4 color)
{
	XMFLOAT4 newColor(0.f, 0.f, 0.f, 0.f);

	newColor.x = color.x / 255;
	newColor.y = color.y / 255;
	newColor.z = color.z / 255;
	newColor.w = color.w / 255;

	return newColor;
}