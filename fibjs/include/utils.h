/***************************************************************************
 *   Copyright (C) 2012 by Leo Hoo                                         *
 *   lion@9465.net                                                         *
 *                                                                         *
 ***************************************************************************/

#ifndef _fj_utils_H_
#define _fj_utils_H_

/**
 @author Leo Hoo <lion@9465.net>
 */

#include <errno.h>

#ifdef _WIN32

#include <ws2tcpip.h>
#include <winsock2.h>
#include <mstcpip.h> 

#ifndef IPV6_V6ONLY
#define IPV6_V6ONLY 27
#endif

#include <windows.h>

#ifndef EWOULDBLOCK
#define EINPROGRESS			WSAEWOULDBLOCK
#endif

#ifndef ETIMEDOUT
#define ETIMEDOUT       138
#endif

#define SHUT_RD		SD_READ
#define SHUT_WR		SD_SEND
#define SHUT_RDWR	SD_BOTH

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>

typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define closesocket close

#ifndef TCP_KEEPIDLE
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif

#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif

#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#include <v8/v8.h>
#include <string>
#include <math.h>

#include "qstring.h"
#include "date.h"
#include "obj_ptr.h"
#include "Variant.h"

#include <exlib/fiber.h>
#include <exlib/lockfree.h>

namespace fibjs
{

typedef int result_t;

#define CALL_RETURN_NULL		100000

#define CALL_E_MAX              -100000
// Invalid number of parameters.
#define CALL_E_BADPARAMCOUNT    (CALL_E_MAX - 1)
// Parameter not optional.
#define CALL_E_PARAMNOTOPTIONAL (CALL_E_MAX - 2)
// The input parameter is not a valid type.
#define CALL_E_BADVARTYPE       (CALL_E_MAX - 3)
// Invalid argument.
#define CALL_E_INVALIDARG       (CALL_E_MAX - 4)
// The argument could not be coerced to the specified type.
#define CALL_E_TYPEMISMATCH     (CALL_E_MAX - 5)
// Value is out of range.
#define CALL_E_OUTRANGE         (CALL_E_MAX - 6)

#define CALL_E_MIN_ARG			(CALL_E_OUTRANGE - 1)

// Constructor cannot be called as a function.
#define CALL_E_CONSTRUCTOR      (CALL_E_MAX - 7)
// Object is not an instance of declaring class.
#define CALL_E_NOTINSTANCE      (CALL_E_MAX - 8)
// Invalid procedure call.
#define CALL_E_INVALID_CALL		 (CALL_E_MAX - 9)
// Index was out of range.
#define CALL_E_BADINDEX         (CALL_E_MAX - 10)
// Memory overflow error.
#define CALL_E_OVERFLOW         (CALL_E_MAX - 11)
// Operation now in progress.
#define CALL_E_PENDDING			(CALL_E_MAX - 12)
// Operation now support synchronous call.
#define CALL_E_NOSYNC			(CALL_E_MAX - 13)
// Exception occurred.
#define CALL_E_EXCEPTION		(CALL_E_MAX - 14)

#define CALL_E_MIN              -100100

#define PROPERTY_ENTER() \
	result_t hr = 0;do{

#define METHOD_OVER(c, o) \
	}while(0); \
	if(hr > CALL_E_MIN_ARG && hr < CALL_E_MAX)do{hr = 0;\
    int argc; \
    argc = args.Length(); \
    if((c) >= 0 && argc > (c)){hr = CALL_E_BADPARAMCOUNT;break;} \
    if((o) > 0 && argc < (o)){hr = CALL_E_PARAMNOTOPTIONAL;break;}

#define METHOD_ENTER(c, o) \
    result_t hr = CALL_E_BADPARAMCOUNT; do{\
    METHOD_OVER(c, o)

#define CONSTRUCT_ENTER(c, o) \
    if (!args.IsConstructCall())return ThrowResult(CALL_E_CONSTRUCTOR); \
    METHOD_ENTER(c, o)

#define METHOD_INSTANCE(cls) \
    obj_ptr<cls> pInst = (cls*)cls::class_info().getInstance(args.This()); \
    if(pInst == NULL)return ThrowResult(CALL_E_NOTINSTANCE); \
    scope l(pInst);

#define PROPERTY_INSTANCE(cls) \
    obj_ptr<cls> pInst = (cls*)cls::class_info().getInstance(info.This()); \
    if(pInst == NULL){hr = CALL_E_NOTINSTANCE;break;} \
    scope l(pInst);

#define PROPERTY_SET_LEAVE() \
    }while(0); \
    if(hr < 0)ThrowResult(hr);

#define METHOD_RETURN() \
    }while(0); \
    if(hr == CALL_RETURN_NULL)return v8::Undefined(); \
    if(hr >= 0)return ReturnValue(vr); \
    return ThrowResult(hr);

#define METHOD_VOID() \
    }while(0); \
    if(hr >= 0)return v8::Undefined(); \
    return ThrowResult(hr);

#define CONSTRUCT_RETURN() \
    }while(0); \
    if(hr >= 0)return vr->wrap(args.This()); \
    return ThrowResult(hr);

#define ARG_String(n) \
    v8::String::Utf8Value tv##n(args[n]); \
    const char* v##n = *tv##n; \
    if(v##n == NULL){hr = CALL_E_INVALIDARG;break;}

#define OPT_ARG_String(n, d) \
    v8::Local<v8::Value> tvv##n; \
    if(n < argc)tvv##n = args[n]; \
    v8::String::Utf8Value tv##n(tvv##n); \
    const char* v##n = (n) < argc ? *tv##n : (d); \
    if(v##n == NULL){hr = CALL_E_INVALIDARG;break;}

#define PROPERTY_VAL_String() \
    v8::String::Utf8Value tv0(value); \
    const char* v0 = *tv0; \
    if(v0 == NULL){hr = CALL_E_INVALIDARG;break;}

#define PROPERTY_VAL(t) \
    t v0; \
    hr = SafeGetValue(value, v0); \
    if(hr < 0)break;

#define ARG(t, n) \
    t v##n; \
    hr = SafeGetValue(args[n], v##n); \
    if(hr < 0)break;

#define OPT_ARG(t, n, d) \
    t v##n; \
    if((n) < argc){ \
        hr = SafeGetValue(args[n], v##n); \
        if(hr < 0)break; \
    }else v##n = (d);

#define CLONE_CLASS(n, cls) \
		{obj_ptr<cls> v; hr = get_##n(v); if(hr < 0)return hr; \
		retVal->Set(v8::String::NewSymbol(#n), ReturnValue(v));}

#define CLONE_String(n) \
		{std::string v; hr = get_##n(v); if(hr < 0)return hr; \
		retVal->Set(v8::String::NewSymbol(#n), ReturnValue(v));}

#define CLONE(n, t) \
	{t v; hr = get_##n(v); if(hr < 0)return hr; \
	retVal->Set(v8::String::NewSymbol(#n), ReturnValue(v));}

#define EVENT_SUPPORT() \
		public: \
			virtual result_t on(const char* ev, v8::Handle<v8::Function> func) \
			{	return object_base::on(ev, func);} \
			virtual result_t on(v8::Handle<v8::Object> map) \
			{	return object_base::on(map);} \
			virtual result_t once(const char* ev, v8::Handle<v8::Function> func) \
			{	return object_base::once(ev, func);} \
			virtual result_t once(v8::Handle<v8::Object> map) \
			{	return object_base::once(map);} \
			virtual result_t off(const char* ev, v8::Handle<v8::Function> func) \
			{	return object_base::off(ev, func);} \
			virtual result_t off(v8::Handle<v8::Object> map) \
			{	return object_base::off(map);} \
			virtual result_t trigger(const char* ev, const v8::Arguments& args) \
			{	return object_base::trigger(ev, args);}

#define FIBER_FREE() \
		public: \
			virtual void enter() \
			{} \
			virtual void leave() \
			{} \

#ifdef _MSC_VER
#define isnan _isnan
#endif

inline result_t SafeGetValue(v8::Handle<v8::Value> v, double& n)
{
	if (v.IsEmpty() || !v->IsNumber())
		return CALL_E_INVALIDARG;

	n = v->NumberValue();
	if (isnan(n))
		return CALL_E_INVALIDARG;

	return 0;
}

inline result_t SafeGetValue(v8::Handle<v8::Value> v, int64_t& n)
{
	double num;

	result_t hr = SafeGetValue(v, num);
	if (hr < 0)
		return hr;

	if (num < -9007199254740992ll || num > 9007199254740992ll)
		return CALL_E_OUTRANGE;

	n = (int64_t) num;

	return 0;
}

inline result_t SafeGetValue(v8::Handle<v8::Value> v, int32_t& n)
{
	double num;

	result_t hr = SafeGetValue(v, num);
	if (hr < 0)
		return hr;

	if (num < -2147483648ll || num > 2147483647ll)
		return CALL_E_OUTRANGE;

	n = (int32_t) num;

	return 0;
}

inline result_t SafeGetValue(v8::Handle<v8::Value> v, bool& n)
{
	if (v.IsEmpty())
		return CALL_E_INVALIDARG;

	n = v->BooleanValue();
	return 0;
}

inline result_t SafeGetValue(v8::Handle<v8::Value> v, date_t& d)
{
	if (v.IsEmpty())
		return CALL_E_INVALIDARG;

	if (!v->IsDate())
		return CALL_E_INVALIDARG;

	d = v;
	return 0;
}

template<class T>
inline result_t SafeGetValue(v8::Handle<v8::Value> v, obj_ptr<T>& vr)
{
	vr = (T*) T::class_info().getInstance(v);
	if (vr == NULL)
		return CALL_E_INVALIDARG;

	return 0;
}

inline result_t SafeGetValue(v8::Handle<v8::Value> v,
		v8::Handle<v8::Object>& vr)
{
	if (v.IsEmpty())
		return CALL_E_INVALIDARG;

	if (!v->IsObject())
		return CALL_E_INVALIDARG;

	vr = v8::Handle<v8::Object>::Cast(v);
	return 0;
}

inline result_t SafeGetValue(v8::Handle<v8::Value> v, v8::Handle<v8::Array>& vr)
{
	if (v.IsEmpty())
		return CALL_E_INVALIDARG;

	if (!v->IsArray())
		return CALL_E_INVALIDARG;

	vr = v8::Handle<v8::Array>::Cast(v);
	return 0;
}

inline result_t SafeGetValue(v8::Handle<v8::Value> v, v8::Handle<v8::Value>& vr)
{
	vr = v;
	return 0;
}

inline result_t SafeGetValue(v8::Handle<v8::Value> v,
		v8::Handle<v8::Function>& vr)
{
	if (!v->IsFunction())
		return CALL_E_INVALIDARG;
	vr = v8::Handle<v8::Function>::Cast(v);
	return 0;
}

inline v8::Handle<v8::Value> ReturnValue(int32_t v)
{
	return v8::Int32::New(v);
}

inline v8::Handle<v8::Value> ReturnValue(bool v)
{
	return v ? v8::True() : v8::False();
}

inline v8::Handle<v8::Value> ReturnValue(double v)
{
	return v8::Number::New(v);
}

inline v8::Handle<v8::Value> ReturnValue(int64_t v)
{
	return v8::Number::New((double) v);
}

inline v8::Handle<v8::Value> ReturnValue(std::string& str)
{
	return v8::String::New(str.c_str(), (int) str.length());
}

inline v8::Handle<v8::Value> ReturnValue(date_t& v)
{
	return v;
}

inline v8::Handle<v8::Value> ReturnValue(v8::Handle<v8::Object>& obj)
{
	return obj;
}

inline v8::Handle<v8::Value> ReturnValue(v8::Handle<v8::Array>& array)
{
	return array;
}

inline v8::Handle<v8::Value> ReturnValue(v8::Handle<v8::Value>& value)
{
	return value;
}

inline v8::Handle<v8::Value> ReturnValue(v8::Handle<v8::Function>& func)
{
	return func;
}

template<class T>
inline v8::Handle<v8::Value> ReturnValue(obj_ptr<T>& obj)
{
	v8::Handle<v8::Object> retVal;

	if (obj)
		obj->ValueOf(retVal);
	else
		return v8::Undefined();

	return retVal;
}

inline v8::Handle<v8::Value> ThrowError(const char* msg)
{
	return v8::ThrowException(v8::Exception::Error(v8::String::New(msg)));
}

inline v8::Handle<v8::Value> ThrowTypeError(const char* msg)
{
	return v8::ThrowException(v8::Exception::TypeError(v8::String::New(msg)));
}

inline v8::Handle<v8::Value> ThrowRangeError(const char* msg)
{
	return v8::ThrowException(v8::Exception::RangeError(v8::String::New(msg)));
}

inline result_t LastError()
{
#ifdef _WIN32
	return - (int)GetLastError();
#else
	return -errno;
#endif
}

inline result_t SocketError()
{
#ifdef _WIN32
	return - WSAGetLastError();
#else
	return -errno;
#endif
}

extern v8::Isolate* isolate;
extern exlib::Service* g_pService;

std::string traceInfo();
std::string getResultMessage(result_t hr);
v8::Handle<v8::Value> ThrowResult(result_t hr);
void ReportException(v8::TryCatch* try_catch, bool rt);
std::string JSON_stringify(v8::Handle<v8::Value> v);

#ifdef _WIN32

#define PATH_SLASH	'\\'

inline bool isPathSlash(char ch)
{
	return ch == '/' || ch == '\\';
}

#else

#define PATH_SLASH	'/'

inline bool isPathSlash(char ch)
{
	return ch == '/';
}

#endif

#define URL_SLASH	'/'

inline bool isUrlSlash(char ch)
{
	return ch == '/';
}

}

#endif

