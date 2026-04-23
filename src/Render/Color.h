#pragma once

struct Color
{
	float r, g, b, a;

	static XMFLOAT4 Lerp(const XMFLOAT4& c0, const XMFLOAT4& c1, float t)
	{
		return XMFLOAT4(
			c0.x + (c1.x - c0.x) * t,
			c0.y + (c1.y - c0.y) * t,
			c0.z + (c1.z - c0.z) * t,
			c0.w + (c1.w - c0.w) * t
		);
	}

	static XMFLOAT4 Clear;
	static XMFLOAT4 Black;
	static XMFLOAT4 LightBlack;
	static XMFLOAT4 Grey;
	static XMFLOAT4 Silver;
	static XMFLOAT4 GhostWhite;
	static XMFLOAT4 White;
	static XMFLOAT4 DarkRed;
	static XMFLOAT4 Red;
	static XMFLOAT4 Orange;
	static XMFLOAT4 Coral;
	static XMFLOAT4 Brown;
	static XMFLOAT4 GhostPurple;
	static XMFLOAT4 Magenta;
	static XMFLOAT4 DarkPink;
	static XMFLOAT4 Pink;
	static XMFLOAT4 LightPink;
	static XMFLOAT4 SkyeBlue;
	static XMFLOAT4 Blue;
	static XMFLOAT4 Turquoise;
	static XMFLOAT4 Cyan;
	static XMFLOAT4 DarkBlue;
	static XMFLOAT4 BlueViolet;
	static XMFLOAT4 Chartreuse;
	static XMFLOAT4 Green;
	static XMFLOAT4 DarkGreen;
	static XMFLOAT4 Lime;
	static XMFLOAT4 Olive;
	static XMFLOAT4 Yellow;
	static XMFLOAT4 Mustard;
	static XMFLOAT4 Gold;

	void Set(XMFLOAT4 newColor);
	void Set(float r, float g, float b, float a);

	static XMFLOAT4 NormalizeColor(XMFLOAT4 color);
};