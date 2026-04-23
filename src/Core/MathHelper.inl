template<typename T>
T MathHelper::Min(const T& a, const T& b)
{
	return a < b ? a : b;
}

template<typename T>
T MathHelper::Max(const T& a, const T& b)
{
	return a > b ? a : b;
}

template<typename T>
T MathHelper::Lerp(const T& a, const T& b, float t)
{
	return a + (b - a) * t;
}

template<typename T>
T MathHelper::Clamp(const T& x, const T& low, const T& high)
{
	return x < low ? low : (x > high ? high : x);
}