#ifndef __DSPACKET_H__
#define __DSPACKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "dstypes.h"
#include "dsbuffer.h"

namespace dakuang
{

// 本地字节序 -> 网络字节序
inline uint16_t DS_HTONS(uint16_t u16)
{
    return ( (u16 << 8) | (u16 >> 8) );
}
inline uint32_t DS_HTONL(uint32_t u32)
{
    return ( (uint32_t(DS_HTONS(uint16_t(u32))) << 16) | DS_HTONS(uint16_t(u32 >> 16)) );
}
inline uint64_t DS_HTONLL(uint64_t u64)
{
    return ( (uint64_t(DS_HTONL(uint32_t(u64))) << 32) | DS_HTONL(uint32_t(u64 >> 32)) );
}

// 网络字节序 -> 本地字节序
#define DS_NTOHS DS_HTONS
#define DS_NTOHL DS_HTONL
#define DS_NTOHLL DS_HTONLL

// 定义异常类型
struct DSError
        : public std::runtime_error
{
    DSError(const std::string & w) : std::runtime_error(w) {}
};

// 定义压包缓冲区
class DSPackBuffer
{
private:
    // 最大1G的压包缓冲区
    typedef DSBuffer<BLOCK_ALLOC_4K, 1024 * 256> DSBuffer_t;
    DSBuffer_t m_buffer;

public:
    char * data()
    {
        return m_buffer.data();
    }
    size_t size() const
    {
        return m_buffer.size();
    }

    void reserve(size_t nSize)
    {
        if (m_buffer.reserve(nSize))
            return;

        throw DSError("[DSPackBuffer::reserve] reserve buffer overflow");
    }

    void resize(size_t nSize)
    {
        if (m_buffer.resize(nSize))
            return;

        throw DSError("[DSPackBuffer::resize] resize buffer overflow");
    }

    void append(const char * pData, size_t nSize)
    {
        if (m_buffer.append(pData, nSize))
            return;

        throw DSError("[DSPackBuffer::append] append buffer overflow");
    }

    void replace(size_t nPos, const char * pData, size_t nSize)
    {
        if (m_buffer.replace(nPos, pData, nSize))
            return;

        throw DSError("[DSPackBuffer::replace] replace buffer overflow");
    }
};

// 定义序列化操作类
class DSPack
{
private:
    DSPackBuffer & m_buffer;
    size_t m_offset;

    DSPack (const DSPack & o);
    DSPack & operator = (const DSPack& o);

public:
    static uint16_t xhtons(uint16_t u16) { return DS_HTONS(u16); }
    static uint32_t xhtonl(uint32_t u32) { return DS_HTONL(u32); }
    static uint64_t xhtonll(uint64_t u64) { return DS_HTONLL(u64); }

    DSPack(DSPackBuffer & pb, size_t off = 0)
        : m_buffer(pb)
    {
        m_offset = pb.size() + off;
        m_buffer.resize(m_offset);
    }
    virtual ~DSPack() {}

    const char * data() const { return m_buffer.data() + m_offset; }
    size_t size() const { return m_buffer.size() - m_offset; }

    DSPack & push(const void * pData, size_t nSize)
    {
        m_buffer.append((const char *)pData, nSize);
        return *this;
    }

    DSPack & push_uint8(uint8_t u8) { return push(&u8, 1); }
    DSPack & push_uint16(uint16_t u16) { u16 = xhtons(u16); return push(&u16, 2); }
    DSPack & push_uint32(uint32_t u32) { u32 = xhtonl(u32); return push(&u32, 4); }
    DSPack & push_uint64(uint64_t u64) { u64 = xhtonll(u64); return push(&u64, 8); }

    DSPack & push_string(const void * pData, size_t nSize)
    {
        if (nSize > 0xFFFF) throw DSError("[DSPack::push_string] string too big");
        return push_uint16(uint16_t(nSize)).push(pData, nSize);
    }
    DSPack & push_string32(const void * pData, size_t nSize)
    {
        if (nSize > 0xFFFFFFFF) throw DSError("[DSPack::push_string32] string too big");
        return push_uint32(uint32_t(nSize)).push(pData, nSize);
    }

    DSPack & push_string(const std::string & str) { return push_string(str.data(), str.size()); }
    DSPack & push_string(const StringPtr & SP) { return push_string(SP.data(), SP.size()); }

    DSPack & replace(size_t nPos, const void * pData, size_t nSize)
    {
        m_buffer.replace(nPos, (const char*)pData, nSize);
        return *this;
    }

    DSPack & replace_uint8(size_t nPos, uint8_t u8) { return replace(nPos, &u8, 1); }
    DSPack & replace_uint16(size_t nPos, uint8_t u16) { u16 = xhtons(u16); return replace(nPos, &u16, 2); }
    DSPack & replace_uint32(size_t nPos, uint8_t u32) { u32 = xhtonl(u32); return replace(nPos, &u32, 4); }
    DSPack & replace_uint64(size_t nPos, uint8_t u64) { u64 = xhtonll(u64); return replace(nPos, &u64, 8); }
    DSPack & replace_string(size_t nPos, const void * pData, size_t nSize) { return replace_uint16(nPos, uint16_t(nSize)).replace(nPos + 2, pData, nSize); }
    DSPack & replace_string32(size_t nPos, const void * pData, size_t nSize) { return replace_uint32(nPos, uint32_t(nSize)).replace(nPos + 4, pData, nSize); }
};

// 定义反序列化操作类
class DSUnpack
{
private:
    mutable const char * m_pData;
    mutable size_t m_nSize;

public:
    static uint16_t xntohs(uint16_t u16) { return DS_NTOHS(u16); }
    static uint32_t xntohl(uint32_t u32) { return DS_NTOHL(u32); }
    static uint64_t xntohll(uint64_t u64) { return DS_NTOHLL(u64); }

    DSUnpack(const void * pData, size_t nSize)
    {
        reset(pData, nSize);
    }
    virtual ~DSUnpack()
    {
        reset(NULL, 0);
    }

    operator const void *() const { return m_pData; }
    bool operator!() const { return (NULL == m_pData); }

    void reset(const void * pData, size_t nSize) const
    {
        m_pData = (const char *)pData;
        m_nSize = nSize;
    }

    const char * data() const { return m_pData; }
    size_t size() const	  { return m_nSize; }

    bool empty() const	  { return size() == 0; }

    void finish() const
    {
        if (!empty())
            throw DSError("[DSUnpack::finish] too much data");
    }

    const char * pop_fetch_ptr(size_t nSize, bool bPeek = false) const
    {
        if (m_nSize < nSize)
            throw DSError("[DSUnpack::pop_fetch_ptr] not enough data");

        const char * pData = m_pData;

        if (!bPeek)
        {
            m_pData += nSize;
            m_nSize -= nSize;
        }

        return pData;
    }

    uint8_t pop_uint8(bool bPeek = false) const { return *(uint8_t*)pop_fetch_ptr(1, bPeek); }
    uint16_t pop_uint16(bool bPeek = false) const { uint16_t u16 = *(uint16_t*)pop_fetch_ptr(2, bPeek); return xntohs(u16); }
    uint32_t pop_uint32(bool bPeek = false) const { uint32_t u32 = *(uint32_t*)pop_fetch_ptr(4, bPeek); return xntohl(u32); }
    uint64_t pop_uint64(bool bPeek = false) const { uint64_t u64 = *(uint64_t*)pop_fetch_ptr(8, bPeek); return xntohll(u64); }

    const char * pop_string(size_t & nSize) const
    {
        nSize = pop_uint16();
        return pop_fetch_ptr(nSize);
    }
    const char * pop_string32(size_t & nSize) const
    {
        nSize = pop_uint32();
        return pop_fetch_ptr(nSize);
    }

    std::string pop_string() const
    {
        size_t nSize = pop_uint16();
        const char* pData = pop_fetch_ptr(nSize);
        return std::string(pData, nSize);
    }
    std::string pop_string32() const
    {
        size_t nSize = pop_uint32();
        const char* pData = pop_fetch_ptr(nSize);
        return std::string(pData, nSize);
    }

    StringPtr pop_StringPtr() const
    {
        StringPtr SP;
        SP.m_nSize = pop_uint16();
        SP.m_pData = pop_fetch_ptr(SP.m_nSize);
        return SP;
    }
    StringPtr pop_StringPtr32() const
    {
        StringPtr SP;
        SP.m_nSize = pop_uint32();
        SP.m_pData = pop_fetch_ptr(SP.m_nSize);
        return SP;
    }
};


// 基础数据类型的序列化与反序列化 =>

inline DSPack & operator << (DSPack & p, bool b)
{
    p.push_uint8(b ? 1 : 0);
    return p;
}

inline DSPack & operator << (DSPack & p, uint8_t u8)
{
    p.push_uint8(u8);
    return p;
}

inline DSPack & operator << (DSPack & p, uint16_t u16)
{
    p.push_uint16(u16);
    return p;
}

inline DSPack & operator << (DSPack & p, uint32_t u32)
{
    p.push_uint32(u32);
    return p;
}

inline DSPack & operator << (DSPack & p, uint64_t u64)
{
    p.push_uint64(u64);
    return p;
}

inline DSPack & operator << (DSPack & p, int8_t i8)
{
    p.push_uint8((uint8_t)i8);
    return p;
}

inline DSPack & operator << (DSPack & p, int16_t i16)
{
    p.push_uint16((uint16_t)i16);
    return p;
}

inline DSPack & operator << (DSPack & p, int32_t i32)
{
    p.push_uint32((uint32_t)i32);
    return p;
}

inline DSPack & operator << (DSPack & p, int64_t i64)
{
    p.push_uint64((uint64_t)i64);
    return p;
}

inline DSPack & operator << (DSPack & p, const std::string & str)
{
    p.push_string(str);
    return p;
}

inline DSPack & operator << (DSPack & p, const StringPtr & SP)
{
    p.push_string(SP);
    return p;
}

inline const DSUnpack & operator >> (const DSUnpack & up, bool & b)
{
    b = (up.pop_uint8() == 0) ? false : true;
    return up;
}

inline const DSUnpack & operator >> (const DSUnpack & up, uint8_t & u8)
{
    u8 = up.pop_uint8();
    return up;
}

inline const DSUnpack & operator >> (const DSUnpack & up, uint16_t & u16)
{
    u16 = up.pop_uint16();
    return up;
}

inline const DSUnpack & operator >> (const DSUnpack & up, uint32_t & u32)
{
    u32 = up.pop_uint32();
    return up;
}

inline const DSUnpack & operator >> (const DSUnpack & up, uint64_t & u64)
{
    u64 = up.pop_uint64();
    return up;
}

inline const DSUnpack & operator >> (const DSUnpack & up, int8_t & i8)
{
    i8 = (int8_t)up.pop_uint8();
    return up;
}

inline const DSUnpack & operator >> (const DSUnpack & up, int16_t & i16)
{
    i16 = (int16_t)up.pop_uint16();
    return up;
}

inline const DSUnpack & operator >> (const DSUnpack & up, int32_t & i32)
{
    i32 = (int32_t)up.pop_uint32();
    return up;
}

inline const DSUnpack & operator >> (const DSUnpack & up, int64_t & i64)
{
    i64 = (int64_t)up.pop_uint64();
    return up;
}

inline const DSUnpack & operator >> (const DSUnpack & up, std::string & str)
{
    str = up.pop_string();
    return up;
}

inline const DSUnpack & operator >> (const DSUnpack & up, StringPtr & SP)
{
    SP = up.pop_StringPtr();
    return up;
}

// 容器类型的序列化与反序列化 =>

template < typename ContainerClass >
inline void marshal_container(DSPack & p, const ContainerClass & c)
{
    p.push_uint32( uint32_t(c.size()) );
    for (typename ContainerClass::const_iterator i = c.begin(); i != c.end(); ++i)
    {
        p << *i;
    }
}

template < typename OutputIterator >
inline void unmarshal_container(const DSUnpack & up, OutputIterator i)
{
    for (uint32_t count = up.pop_uint32(); count > 0; --count)
    {
        typename OutputIterator::container_type::value_type tmp;
        up >> tmp;
        *i = tmp;
        ++i;
    }
}

template < typename OutputContainer >
inline void unmarshal_container2(const DSUnpack & p, OutputContainer & c)
{
    for (uint32_t count = p.pop_uint32(); count > 0; --count)
    {
        typename OutputContainer::value_type tmp;
        p >> tmp;
        c.push_back(tmp);
    }
}

template <class T1, class T2>
inline DSPack & operator << (DSPack & p, const std::pair<T1, T2> & pair)
{
    p << pair.first << pair.second;
    return p;
}

template <class T1, class T2>
inline const DSUnpack & operator >> (const DSUnpack & up, std::pair<const T1, T2> & pair)
{
    const T1 & m = pair.first;
    T1 & m2 = const_cast<T1 &>(m);
    up >> m2 >> pair.second;
    return up;
}

template <class T1, class T2>
inline const DSUnpack & operator >> (const DSUnpack & up, std::pair<T1, T2> & pair)
{
    up >> pair.first >> pair.second;
    return up;
}

template <class T>
inline DSPack & operator << (DSPack & p, const std::vector<T> & vec)
{
    marshal_container(p, vec);
    return p;
}

template <class T>
inline const DSUnpack & operator >> (const DSUnpack & up, std::vector<T> & vec)
{
    unmarshal_container(up, std::back_inserter(vec));
    return up;
}

template <class T>
inline DSPack & operator << (DSPack & p, const std::set<T> & set)
{
    marshal_container(p, set);
    return p;
}

template <class T>
inline const DSUnpack & operator >> (const DSUnpack & up, std::set<T> & set)
{
    unmarshal_container(up, std::inserter(set, set.begin()));
    return up;
}

template <class T1, class T2>
inline DSPack & operator << (DSPack & p, const std::map<T1, T2> & map)
{
    marshal_container(p, map);
    return p;
}

template <class T1, class T2>
inline const DSUnpack & operator >> (const DSUnpack & up, std::map<T1, T2> & map)
{
    unmarshal_container(up, std::inserter(map, map.begin()));
    return up;
}

// 结构类型的序列化与反序列化 =>

struct Marshallable
{
    virtual ~Marshallable() {}

    virtual void marshal(DSPack &) const = 0;
    virtual void unmarshal(const DSUnpack &) = 0;
};

struct Voidmable : public Marshallable
{
    virtual void marshal(DSPack &) const {}
    virtual void unmarshal(const DSUnpack &) {}
};

inline DSPack & operator << (DSPack & p, const Marshallable & m)
{
    m.marshal(p);
    return p;
}

inline const DSUnpack & operator >> (const DSUnpack & p, Marshallable & m)
{
    m.unmarshal(p);
    return p;
}

inline void Object2String(const Marshallable & obj, std::string & str)
{
    DSPackBuffer buffer;
    DSPack pack(buffer);

    obj.marshal(pack);
    str.assign(pack.data(), pack.size());
}

inline bool String2Object(const std::string & str, Marshallable & obj)
{
    try
    {
        DSUnpack unpack(str.data(), str.size());

        obj.unmarshal(unpack);
    }
    catch (const DSError & e)
    {
        return false;
    }

    return true;
}

}

#endif //__DSPACKET_H__
