#ifndef __DSTYPES_H__
#define __DSTYPES_H__

#include <sys/types.h>

#ifndef __cplusplus

typedef unsigned char bool;
#define     true    1
#define     false   0

#endif

#ifdef __GNUC__

#include <stdint.h>

#elif defined(_MSC_VER)

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed __int64 int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

#endif

namespace dakuang
{

	struct StringPtr
	{
		const char * m_pData;
		size_t m_nSize;

		StringPtr(const char * pData = NULL, size_t nSize = 0)
			: m_pData(pData)
			, m_nSize(nSize)
		{
		}
		StringPtr(const StringPtr & SP)
		{
			m_pData = SP.m_pData;
			m_nSize = SP.m_nSize;
		}

		StringPtr& operator = (const StringPtr & SP)
		{
			m_pData = SP.m_pData;
			m_nSize = SP.m_nSize;
			return *this;
		}

		void set(const char * pData, size_t nSize)
		{
			m_pData = pData;
			m_nSize = nSize;
		}

		bool empty() const
		{
			return (m_nSize == 0);
		}

		const char * data() const
		{
			return m_pData;
		}

		size_t size() const
		{
			return m_nSize;
		}
	};

}

#endif // __DSTYPES_H__
