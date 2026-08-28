//****************************************************************************
// FILE:    ImageUtils.cpp
//
// DESC:    C++ image processing definitions
//
// AUTHOR:  Russ Barker
//


#include <cstring>

class CImage
{
public:
	int m_nPixelSize;
	int m_nWidth;
	int m_nHeight;
	unsigned char *m_pData;

	CImage() :
		m_nPixelSize(24),
		m_nWidth(0),
		m_nHeight(0),
		m_pData(NULL)
	{
	}
};

class CPixel
{
public:
	int m_nSize;
	int m_nY;
	int m_nU;
	int m_nV;

	CPixel() :
		m_nSize(24),
		m_nY(0),
		m_nU(0),
		m_nV(0)
	{
	}
};

static unsigned char PixelComponent(int value)
{
	if (value < 0)
		return 0;
	if (value > 255)
		return 255;
	return static_cast<unsigned char>(value);
}

bool SetPixel(CImage &target, int x, int y, const CPixel &source)
{
	if (!target.m_pData || x < 0 || y < 0 ||
		x >= target.m_nWidth || y >= target.m_nHeight ||
		target.m_nPixelSize != source.m_nSize ||
		target.m_nPixelSize < 24 || (target.m_nPixelSize % 8) != 0)
	{
		return false;
	}

	const int bytesPerPixel = target.m_nPixelSize / 8;
	unsigned char *pixel = target.m_pData +
		((y * target.m_nWidth + x) * bytesPerPixel);

	pixel[0] = PixelComponent(source.m_nY);
	pixel[1] = PixelComponent(source.m_nU);
	pixel[2] = PixelComponent(source.m_nV);
	return true;
}

bool OverlayImage(CImage &target, int x, int y, const CImage &source)
{
	if (!target.m_pData || !source.m_pData || x < 0 || y < 0 ||
		source.m_nWidth < 0 || source.m_nHeight < 0 ||
		target.m_nPixelSize != source.m_nPixelSize ||
		target.m_nPixelSize <= 0 || (target.m_nPixelSize % 8) != 0 ||
		x + source.m_nWidth > target.m_nWidth ||
		y + source.m_nHeight > target.m_nHeight)
	{
		return false;
	}

	const size_t bytesPerPixel = static_cast<size_t>(target.m_nPixelSize / 8);
	const size_t sourceRowBytes =
		static_cast<size_t>(source.m_nWidth) * bytesPerPixel;
	const size_t targetRowBytes =
		static_cast<size_t>(target.m_nWidth) * bytesPerPixel;

	for (int row = 0; row < source.m_nHeight; ++row)
	{
		unsigned char *targetRow = target.m_pData +
			(static_cast<size_t>(y + row) * targetRowBytes) +
			(static_cast<size_t>(x) * bytesPerPixel);
		const unsigned char *sourceRow = source.m_pData +
			(static_cast<size_t>(row) * sourceRowBytes);
		std::memcpy(targetRow, sourceRow, sourceRowBytes);
	}

	return true;
}
