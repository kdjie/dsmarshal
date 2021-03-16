# dsmarshal
一个跨平台的C++结构体对象序列化与反序列化实现，且源码为HeaderOnly方式，方便朋友们使用。

### 例子1：
```
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

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
g++ -o test1 test1.cpp -Idsmarshal库所在目录

### 例子2：
```
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

// 引用头文件
#include <dsmarshal/dspacket.h>

int main(int argc, char* argv[])
{
    using namespace dakuang;

    std::string strName = "abc123";
    int nAge = 20;
    std::vector<std::string> vecBooks;
    vecBooks.push_back("a");
    vecBooks.push_back("b");
    vecBooks.push_back("c");

    DSPack pack;
    pack << strName << nAge << vecBooks;

    DSUnpack unpack(pack.data(), pack.size());

    std::string strName2;
    int nAge2;
    std::vector<std::string> vecBooks2;
    unpack >> strName2 >> nAge2 >> vecBooks2;

    return 0;
}
```

###### 编译：
g++ -o test2 test2.cpp -Idsmarshal库所在目录

### 主要类及方法：

#### DSPackBuffer
本类为序列化压包缓冲区，内部实现为按块自动分配内存，每块的大小默认为4K。

##### 主要方法：
char * data(); <br>
返回缓冲区指针。

size_t size() const; <br>
返回缓冲区数据长度。

void reserve(size_t nSize); <br>
扩充内部容量为指定大小。

void resize(size_t nSize); <br>
扩充或改变缓冲区数据长度为指定大小。

void append(const char * pData, size_t nSize); <br>
向缓冲区尾部插入指定长度的数据。

void replace(size_t nPos, const char * pData, size_t nSize); <br>
替换缓冲区指定位置、指定长度的内存。

#### DSPack
本类为序列化压包操作实现，但是自己不管理缓冲区，需要在定义时指定DSPackBuffer缓冲区对象。

##### 主要方法：
DSPack(DSPackBuffer & pb, size_t off = 0); <br>
定义序列化压包操作对象。

const char * data() const; <br>
返回本对象指向的缓冲区数据指针。

size_t size(); <br>
返回本对象指向的缓冲区数据长度。

DSPack & push(const void * pData, size_t nSize); <br>
向本对象指向的缓冲区压入指定长度的数据。

DSPack & push_uint8(uint8_t u8); <br>
DSPack & push_uint16(uint16_t u16); <br>
DSPack & push_uint32(uint32_t u32); <br>
DSPack & push_uint64(uint64_t u64); <br>
向本对象指向的缓冲区压入不同类型的整型数据。

DSPack & push_string(const void * pData, size_t nSize); <br>
向本对象指向的缓冲区压入定长度的字符串，但限制最大长度为64K。

DSPack & push_string32(const void * pData, size_t nSize); <br>
向本对象指向的缓冲区压入指定长度的字符串，但限制最大长度为0xFFFFFF。

DSPack & push_string(const std::string & str); <br>
向本对象指向的缓冲区压入std::string的字符串，但限制最大长度为64K。

#### DSUnpack
本类为反序列化解包操作实现，需要在定义时给定一段缓存区。

##### 主要方法：
DSUnpack(const void * pData, size_t nSize); <br>
定义反序列化解包操作对象。

void reset(const void * pData, size_t nSize) const; <br>
重置解包缓存区。

const char * data() const; <br>
返回待解包的缓存区地址。

size_t size() const; <br>
返回待解包的缓存区长度。

bool empty() const; <br>
检查待解包的数据长度是否为空。

void finish() const; <br>
检查是否完成解包。

const char * pop_fetch_ptr(size_t nSize, bool bPeek = false) const; <br>
从缓冲区解出指定长度的数据，如果bPeek为true，表示仅查看。

uint8_t pop_uint8(bool bPeek = false) const; <br>
uint16_t pop_uint16(bool bPeek = false) const; <br>
uint32_t pop_uint32(bool bPeek = false) const; <br>
uint64_t pop_uint64(bool bPeek = false) const; <br>
从缓冲区解出指定类型的整数，如果bPeek为true，表示仅查看。

std::string pop_string() const; <br>
从缓冲区解出以push_string()方式压入的字符串。

std::string pop_string32() const; <br>
从缓冲区解出以push_string32()方式压入的字符串。

#### Marshallable
本类为抽像基类，主要定义了序列化与反序列化的方法。

##### 主要方法：
virtual void marshal(DSPack &) const = 0; <br>
virtual void unmarshal(const DSUnpack &) = 0;

###### Marshallable对象序列化与反序列化
定义了两个方法：<br>
inline void Object2String(const Marshallable & obj, std::string & str); <br>
将对象序列化为字符串流。

inline bool String2Object(const std::string & str, Marshallable & obj); <br>
从字序串流反序列化对象。

### 基于std::string更轻量级的实现

在本开源目录simplemarshal下有个simplemarshal.h，它采用std::string做为压包缓冲，从形式上更加轻量，也更稳定。<br>
除了相关的类名称不一样，在用法上跟DS是相同的。使用时，请将simplemarshal.h拷贝到工程目录下，并将其include引入。<br>
