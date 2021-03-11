#ifndef __DSBUFFER_H__
#define __DSBUFFER_H__

#include <stdlib.h>
#include <string.h>
#include <new>

namespace dakuang
{
// 内存分配器定义 =>

template <unsigned int BlockSize>
struct SBlockAllocator_MallocFree
{
    enum { blockSize = BlockSize };

    static char * ordered_malloc(size_t BlockCount)
    {
        return (char *)malloc(BlockSize * BlockCount);
    }
    static void ordered_free(char * const pBlock, size_t)
    {
        free(pBlock);
    }
};

template <unsigned int BlockSize>
struct SBlockAllocator_NewDelete
{
    enum { blockSize = BlockSize };

    static char * ordered_malloc(size_t BlockCount)
    {
        return new (std::nothrow) char[BlockSize * BlockCount];
    }
    static void ordered_free(char * const pBlock, size_t)
    {
        delete[] pBlock;
    }
};

#define USE_NEW_ALLOCATOR

#ifdef  USE_NEW_ALLOCATOR
typedef SBlockAllocator_NewDelete<1 * 1024> BLOCK_ALLOC_1K;
typedef SBlockAllocator_NewDelete<2 * 1024> BLOCK_ALLOC_2K;
typedef SBlockAllocator_NewDelete<4 * 1024> BLOCK_ALLOC_4K;
typedef SBlockAllocator_NewDelete<8 * 1024> BLOCK_ALLOC_8K;
typedef SBlockAllocator_NewDelete<16 * 1024> BLOCK_ALLOC_16K;
typedef SBlockAllocator_NewDelete<32 * 1024> BLOCK_ALLOC_32K;
#else
typedef SBlockAllocator_MallocFree<1 * 1024> BLOCK_ALLOC_1K;
typedef SBlockAllocator_MallocFree<2 * 1024> BLOCK_ALLOC_2K;
typedef SBlockAllocator_MallocFree<4 * 1024> BLOCK_ALLOC_4K;
typedef SBlockAllocator_MallocFree<8 * 1024> BLOCK_ALLOC_8K;
typedef SBlockAllocator_MallocFree<16 * 1024> BLOCK_ALLOC_16K;
typedef SBlockAllocator_MallocFree<32 * 1024> BLOCK_ALLOC_32K;
#endif

// 数据序列化缓冲 =>

template <typename BlockAllocator = BLOCK_ALLOC_4K, unsigned int MaxBlockCount = 1024>
class DSBuffer
{
private:
    char * m_pData;
    size_t m_nSize;
    size_t m_nBlockCount;

public:
    enum { maxBlockCount = MaxBlockCount };

    typedef BlockAllocator allocator;

    DSBuffer() : m_pData(NULL), m_nSize(0), m_nBlockCount(0) {}
    virtual ~DSBuffer() { __free(); }

    char * data() { return m_pData; }
    size_t size() const { return m_nSize; }

    bool empty() const	 { return size() == 0; }
    size_t blockCount() const	 { return m_nBlockCount; }
    size_t blockSize() const { return allocator::blockSize; }
    size_t capacity() const  { return allocator::blockSize * m_nBlockCount; }
    size_t maxCapacity() const	 { return allocator::blockSize * maxBlockCount; }
    size_t curFreeSize() const { return capacity() - size(); }
    size_t maxFreeSize() const	 { return maxCapacity() - size(); }

    inline bool reserve(size_t nSize);
    inline bool resize(size_t nSize, char cChar = 0);
    inline bool append(const char * pData, size_t nSize);
    inline bool replace(size_t nPos, const char * pData, size_t nSize);
    inline bool erase(size_t nPos, size_t nSize = size_t(-1), bool bFree = true);

protected:
    char * __tail() { return m_pData + m_nSize; }
    inline void __free();
    inline bool __increaseCapacity(size_t nSize);

private:
    DSBuffer(const DSBuffer &) {}
    void operator = (const DSBuffer &) {}
};

template <typename BlockAllocator, unsigned int MaxBlockCount>
inline bool DSBuffer<BlockAllocator, MaxBlockCount >::reserve(size_t nSize)
{
    // 容量不够扩容
    return (nSize <= capacity() || __increaseCapacity(nSize - capacity()));
}

template <typename BlockAllocator, unsigned int MaxBlockCount>
inline bool DSBuffer<BlockAllocator, MaxBlockCount >::resize(size_t nSize, char cChar)
{
    if (nSize > size())
    {
        size_t nIncreaseSize = nSize - size();

        // 空间不够扩容
        if (nIncreaseSize > curFreeSize() &&  !__increaseCapacity(nIncreaseSize - curFreeSize()))
            return false;

        memset(__tail(), cChar, nIncreaseSize);
    }

    m_nSize = nSize;

    return true;
}

template <typename BlockAllocator, unsigned int MaxBlockCount>
inline bool DSBuffer<BlockAllocator, MaxBlockCount >::append(const char * pData, size_t nSize)
{
    if (nSize == 0)
        return true;

    // 空间不够扩容
    if (nSize > curFreeSize() && !__increaseCapacity(nSize - curFreeSize()))
        return false;

    memcpy(__tail(), pData, nSize);
    m_nSize += nSize;

    return true;
}

template <typename BlockAllocator, unsigned int MaxBlockCount>
inline bool DSBuffer<BlockAllocator, MaxBlockCount >::replace(size_t nPos, const char * pData, size_t nSize)
{
    if (nSize == 0)
        return true;

    // 替换区在当前数据之外
    if (nPos >= size())
        return append(pData, nSize);

    // 替换区在当前数据之内，并且超过了当前长度，需要增加空间
    if (nPos + nSize > size())
    {
        if (!resize(nPos + nSize))
            return false;
    }

    memcpy(m_pData + nPos, pData, nSize);

    return true;
}

template <typename BlockAllocator, unsigned int MaxBlockCount>
inline bool DSBuffer<BlockAllocator, MaxBlockCount >::erase(size_t nPos, size_t nSize, bool bFree)
{
    if (nSize == 0)
        return true;

    // 删除区在当前数据之外
    if (nPos >= size())
        return true;

    // 替换区在当前数据之内，并且超过了当前长度，需要减小空间
    if (nSize == size_t(-1) || nPos + nSize >= size())
    {
        resize(nPos);
    }
    // 没有超过当前长度，搬移内存
    else
    {
        size_t nMoveSize = size() - (nPos + nSize);
        memcpy(m_pData + nPos, m_pData + nPos + nSize, nMoveSize);
    }

    // 释放空间
    if (empty() && bFree)
    {
        __free();
    }
}

template <typename BlockAllocator, unsigned int MaxBlockCount>
inline void DSBuffer<BlockAllocator, MaxBlockCount >::__free()
{
    if (m_nBlockCount > 0)
    {
        allocator::ordered_free(m_pData, m_nBlockCount);

        m_pData = NULL;
        m_nSize = 0;
        m_nBlockCount = 0;
    }
}

template <typename BlockAllocator, unsigned int MaxBlockCount>
inline bool DSBuffer<BlockAllocator, MaxBlockCount >::__increaseCapacity(size_t nSize)
{
    if (nSize == 0)
        return true;

    size_t nIncreaseBlockCount = nSize / blockSize();
    if ((nSize % blockSize()) != 0)
        nIncreaseBlockCount++;

    if (m_nBlockCount + nIncreaseBlockCount > maxBlockCount)
        return false;

    char * pNew = (char*)(allocator::ordered_malloc( m_nBlockCount + nIncreaseBlockCount ));
    if (pNew == NULL)
        return false;

    if (m_nBlockCount > 0)
    {
        memcpy(pNew, m_pData, m_nSize);
        allocator::ordered_free(m_pData, m_nBlockCount);
    }

    m_pData = pNew;
    m_nBlockCount = m_nBlockCount + nIncreaseBlockCount;

    return true;
}

}

#endif // __DSBUFFER_H__
