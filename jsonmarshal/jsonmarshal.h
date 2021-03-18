#ifndef JSONMARSHAL_H
#define JSONMARSHAL_H

// 结构体转json序列化实现 =》

#include <stdint.h>
#include <json/json.h>
#include <string>
#include <vector>
#include <typeinfo>

namespace dakuang
{

// 基础数据类型的序列化与反序列化 =>

inline Json::Value & operator << (Json::Value & js, bool b)
{
    js = b;
    return js;
}

inline Json::Value & operator << (Json::Value & js, uint8_t u8)
{
    js = (Json::UInt)u8;
    return js;
}

inline Json::Value & operator << (Json::Value & js, uint16_t u16)
{
    js = (Json::UInt)u16;
    return js;
}

inline Json::Value & operator << (Json::Value & js, uint32_t u32)
{
    js = (Json::UInt)u32;
    return js;
}

inline Json::Value & operator << (Json::Value & js, int8_t i8)
{
    js = (Json::Int)i8;
    return js;
}

inline Json::Value & operator << (Json::Value & js, int16_t i16)
{
    js = (Json::Int)i16;
    return js;
}

inline Json::Value & operator << (Json::Value & js, int32_t i32)
{
    js = (Json::Int)i32;
    return js;
}

inline Json::Value & operator << (Json::Value & js, const std::string & str)
{
    js = str;
    return js;
}

inline const Json::Value & operator >> (const Json::Value & js, bool & b)
{
    b = js.asBool();
    return js;
}

inline const Json::Value & operator >> (const Json::Value & js, uint8_t & u8)
{
    u8 = (uint8_t)js.asUInt();
    return js;
}

inline const Json::Value & operator >> (const Json::Value & js, uint16_t & u16)
{
    u16 = (uint16_t)js.asUInt();
    return js;
}

inline const Json::Value & operator >> (const Json::Value & js, uint32_t & u32)
{
    u32 = (uint32_t)js.asUInt();
    return js;
}

inline const Json::Value & operator >> (const Json::Value & js, int8_t & i8)
{
    i8 = (int8_t)js.asInt();
    return js;
}

inline const Json::Value & operator >> (const Json::Value & js, int16_t & i16)
{
    i16 = (int16_t)js.asInt();
    return js;
}

inline const Json::Value & operator >> (const Json::Value & js, int32_t & i32)
{
    i32 = (int32_t)js.asInt();
    return js;
}

inline const Json::Value & operator >> (const Json::Value & js, std::string & str)
{
    str = js.asString();
    return js;
}

// 容器类型的序列化与反序列化 =>

template <typename T>
inline Json::Value & operator << (Json::Value & js, const std::vector<T> & vec)
{
    for (typename std::vector<T>::const_iterator i = vec.begin(); i != vec.end(); ++i)
    {
        Json::Value jsItem;
        jsItem << (*i);
        js.append(jsItem);
    }
    return js;
}

template <typename T>
inline const Json::Value & operator >> (const Json::Value & js, std::vector<T> & vec)
{
    for (size_t i = 0; i < js.size(); ++i)
    {
        if (typeid (T) == typeid(bool) )
        {
            bool t = (bool)js[i].asBool();
            vec.push_back(t);
        }
        else if (typeid (T) == typeid(uint8_t)
                || typeid (T) == typeid(uint16_t)
                || typeid (T) == typeid(uint32_t)
                || typeid (T) == typeid(unsigned int) )
        {
            T t = (T)js[i].asUInt();
            vec.push_back(t);
        }
        else if (typeid (T) == typeid(int8_t)
                 || typeid (T) == typeid(int16_t)
                 || typeid (T) == typeid(int32_t)
                 || typeid (T) == typeid(int) )
        {
            T t = (T)js[i].asInt();
            vec.push_back(t);
        }
//        else if (typeid (T) == typeid(std::string) )
//        {
//            T t = (T)js[i].asString();
//            vec.push_back(t);
//        }
    }

    return js;
}

template <>
inline const Json::Value & operator >> (const Json::Value & js, std::vector<std::string> & vec)
{
    for (size_t i = 0; i < js.size(); ++i)
    {
        vec.push_back( js[i].asString() );
    }
}

// 结构类型的序列化与反序列化 =>

struct JsonMarshallable
{
    virtual ~JsonMarshallable() {}

    virtual void marshal(Json::Value &) const = 0;
    virtual void unmarshal(const Json::Value &) = 0;
};

Json::Value & operator << (Json::Value & js, const JsonMarshallable & obj)
{
    obj.marshal(js);
    return js;
}

const Json::Value & operator >> (const Json::Value & js, JsonMarshallable & obj)
{
    obj.unmarshal(js);
    return js;
}

}

#endif // JSONMARSHAL_H
