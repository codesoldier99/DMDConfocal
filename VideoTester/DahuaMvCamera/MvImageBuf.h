#ifndef __DAHUA_MULTICAMERAS_MVIMAGEBUF_H__
#define __DAHUA_MULTICAMERAS_MVIMAGEBUF_H__

#include "Infra/IntTypes.h"
#include "Memory/SharedPtr.h"
#include "GenICam/PixelType.h"
#include "GenICam/Frame.h"

namespace Dahua{

class CMvImageBuf
{
public:
	CMvImageBuf(Dahua::GenICam::CFrame const& frame)
		: m_pDataBuf(NULL)
	{
		if ( frame.getImageSize() > 0 )
		{
			if(frame.getImagePixelFormat() == Dahua::GenICam::gvspPixelMono8)
			{
				m_pDataBuf = new(std::nothrow) uint8_t[frame.getImageSize()];
			}
			else
			{
				m_pDataBuf = new(std::nothrow) uint8_t[frame.getImageWidth() * frame.getImageHeight() * 3];
			}		
			if(m_pDataBuf)
			{
				m_iWidth = frame.getImageWidth();
				m_iHeight = frame.getImageHeight();
				m_paddingX = frame.getImagePadddingX();
				m_paddingY = frame.getImagePadddingY();
				m_iDataSize = frame.getImageSize();
				m_pixelFormat = frame.getImagePixelFormat();
			}
		}
	}

	~CMvImageBuf()
	{
		if ( NULL != m_pDataBuf )
		{
			delete[] m_pDataBuf; 
		}
	}

public:
	uint8_t* bufPtr()
	{
		return m_pDataBuf;
	}

	uint32_t	bufSize()
	{
		return m_iDataSize;
	}

	uint32_t imageWidth()
	{
		return m_iWidth;
	}

	uint32_t imageHeight()
	{
		return m_iHeight;
	}

	uint32_t imagePaddingX()
	{
		return m_paddingX;
	}

	uint32_t imagePaddingY()
	{
		return m_paddingY;
	}

	uint64_t imagePixelFormat()
	{
		return m_pixelFormat;
	}

	void setImageWidth(uint32_t iWidth)
	{
		m_iWidth = iWidth;
	}

	void setImageHeight(uint32_t iHeight)
	{
		m_iHeight = iHeight;
	}

	void setImagePixelFormat(uint64_t iPixelFormat)
	{
		m_pixelFormat = iPixelFormat;
	}

private:
	uint8_t*						m_pDataBuf;
	uint32_t						m_iDataSize;
	uint32_t						m_iWidth;
	uint32_t						m_iHeight;
	uint32_t						m_paddingX;
	uint32_t						m_paddingY;
	uint64_t						m_pixelFormat;
};

typedef Memory::TSharedPtr<CMvImageBuf> CMvImageBufPtr;

} // namespace Dahua

#endif // __DAHUA_MULTICAMERAS_MVIMAGEBUF_H__
