#ifndef JSONMARSHAL_H
#define JSONMARSHAL_H

// 结构体转json序列化实现 =》

#include <stdint.h>
#include <json/json.h>
#include <string>
#include <vector>
#include <set>
#include <map>

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
        T t;
        js[i] >> t;
        vec.push_back(t);
    }
    return js;
}

template <typename T>
inline Json::Value & operator << (Json::Value & js, const std::set<T> & set)
{
    for (typename std::set<T>::const_iterator i = set.begin(); i != set.end(); ++i)
    {
        Json::Value jsItem;
        jsItem << (*i);
        js.append(jsItem);
    }
    return js;
}

template <typename T>
inline const Json::Value & operator >> (const Json::Value & js, std::set<T> & set)
{
    for (size_t i = 0; i < js.size(); ++i)
    {
        T t;
        js[i] >> t;
        set.push_back(t);
    }
    return js;
}

template <typename T>
inline Json::Value & operator << (Json::Value & js, const std::map<std::string, T> & map)
{
    for (typename std::map<std::string, T>::const_iterator i = map.begin(); i != map.end(); ++i)
    {
        js[i->first] << i->second;
    }
    return js;
}

template <typename T>
inline const Json::Value & operator >> (const Json::Value & js, std::map<std::string, T> & map)
{
    Json::Value::Members members = js.getMemberNames();
    for (Json::Value::Members::const_iterator i = members.begin(); i != members.end(); ++i)
    {
        std::string strKey = (*i);
        T t;
        js[strKey] >> t;
        map[strKey] = t;
    }
    return js;
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
