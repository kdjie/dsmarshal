# dsmarshal
一个跨平台的结构体对象序列化与反序列化实现，且源码为HeaderOnly方式，方便朋友们使用。

### 例子：
```
#include <stdio.h>
#include <stdlib.h>

// 引用头文件
#include <dsmarshal/dspacket.h>

int main(int argc, char* argv[])
{
    using namespace dakuang;

    // 定义待序列化的结象结构，要求继承Marshallable
    struct SUser : public Marshallable
    {
        std::string strName;
        int nAge;
        std::vector<std::string> vecBooks;

        // 实现结构内部序列化
        virtual void marshal(DSPack & p) const
        {
            p << strName << nAge << vecBooks;
        }
        // 实现结构内部反序列化
        virtual void unmarshal(const DSUnpack & up)
        {
            up >> strName >> nAge >> vecBooks;
        }

        void show()
        {
            printf("name:[%s] \n", strName.c_str());
            printf("age:[%d] \n", nAge);
            printf("books: \n");
            for (std::vector<std::string>::iterator iter = vecBooks.begin(); iter != vecBooks.end(); ++iter)
            {
                printf("\t %s \n", (*iter).c_str());
            }
        }
    };

    // 定义对象
    SUser stUser1;
    stUser1.strName = "abc123";
    stUser1.nAge = 20;
    stUser1.vecBooks.push_back("a");
    stUser1.vecBooks.push_back("b");
    stUser1.vecBooks.push_back("c");

    // 序列化对象
    std::string strStream;
    Object2String(stUser1, strStream);

    printf("marshal stream size:[%d] \n", strStream.size());

    // 反序列化对象
    SUser stUser2;
    String2Object(strStream, stUser2);

    printf("unmarshal object: \n");
    stUser2.show();

    return 0;
}
```

###### 编译：
g++ -o test test.cpp -Idsmarshal库所在目录

### 主要类及方法：

###### DSPackBuffer
本类为序列化压包缓冲区，内部实现为按块自动分配内存，每块的大小默认为4K。

主要方法：
char * data();
返回缓冲区指针。

size_t size() const;
返回缓冲区数据长度。

void reserve(size_t nSize);
扩充内部容量为指定大小。

void resize(size_t nSize);
扩充或改变缓冲区数据长度为指定大小。

void append(const char * pData, size_t nSize);
向缓冲区尾部插入指定长度的数据。

void append(const char * pData);
向缓冲区尾部插入以C风格结尾的数据。

void replace(size_t nPos, const char * pData, size_t nSize);
替换缓冲区指定位置、指定长度的内存。

###### DSPack
本类为序列化压包操作实现，但是自己不管理缓冲区，需要在定义时指定DSPackBuffer缓冲区对象。

主要方法：
DSPack(DSPackBuffer & pb, size_t off = 0);
定义序列化压包操作对象。

char * data();
返回本对象指向的缓冲区数据指针。

size_t size();
返回本对象指向的缓冲区数据长度。

DSPack & push(const void * pData, size_t nSize);
向本对象指向的缓冲区压入指定长度的数据。

DSPack & push(const void * pData);
向本对象指向的缓冲区压入以C风格结尾的数据。

DSPack & push_uint8(uint8_t u8);
DSPack & push_uint16(uint16_t u16);
DSPack & push_uint32(uint32_t u32);
DSPack & push_uint64(uint64_t u64);
向本对象指向的缓冲区压入不同类型的整型数据。

DSPack & push_string(const void * pData, size_t nSize);
向本对象指向的缓冲区压入定长度的字符串，但限制最大长度为64K。

DSPack & push_string32(const void * pData, size_t nSize);
向本对象指向的缓冲区压入指定长度的字符串，但限制最大长度为0xFFFFFF。

DSPack & push_string(const void * pData);
向本对象指向的缓冲区压入以C风格结尾的字符串，但限制最大长度为64K。

DSPack & push_string(const std::string & str);
向本对象指向的缓冲区压入std::string的字符串，但限制最大长度为64K。

###### DSUnpack
本类为反序列化解包操作实现，需要在定义时给定一段缓存区。

主要方法：
DSUnpack(const void * pData, size_t nSize);
定义反序列化解包操作对象。

void reset(const void * pData, size_t nSize) const;
重置解包缓存区。

const char * data();
返回待解包的缓存区地址。

size_t size() const;
返回待解包的缓存区长度。

bool empty() const;
检查待解包的数据长度是否为空。

void finish() const;
检查是否完成解包。

uint8_t pop_uint8(bool bPeek = false) const;
uint16_t pop_uint16(bool bPeek = false) const;
uint32_t pop_uint32(bool bPeek = false) const;
uint64_t pop_uint64(bool bPeek = false) const;
从缓冲区解出指定类型的整数，如果bPeek为true，表示仅查看。

const char * pop_fetch_ptr(size_t nSize) const;
从缓冲区解出指定长度的数据。

std::string pop_fetch_string(size_t nSize) const;
从缓冲区解出指定长度的字符串。

std::string pop_string() const;
从缓冲区解出以push_string()方式压入的字符串。

std::string pop_string32() const;
从缓冲区解出以push_string32()方式压入的字符串。

###### Marshallable
本类为抽像基类，主要定义了序列化与反序列化的方法。

主要方法：
virtual void marshal(DSPack &) const = 0;
virtual void unmarshal(const DSUnpack &) = 0;

###### Marshallable对象序列化与反序列化
定义了两个方法：
inline void Object2String(const Marshallable & obj, std::string & str);
将对象序列化为字符串流。

inline bool String2Object(const std::string & str, Marshallable & obj);
从字序串流反序列化对象。
