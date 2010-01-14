
#include "StdAfx.h"
#include "TString.h"

std::string W2AConvert(std::wstring t) {
	std::string s;
	int len;

    // Get the converted length
    len = WideCharToMultiByte(CP_UTF8, 0, t.c_str(), t.length(), NULL, 0, NULL, NULL);
	s.resize(len);
    // Convert string
    WideCharToMultiByte(CP_UTF8, 0, t.c_str(), t.length(), (char *)s.c_str(), s.length(), NULL, NULL);

	return s;
};

std::wstring A2WConvert(std::string t) {
	std::wstring s;
	int len;

    // Get the converted length
    len = MultiByteToWideChar(CP_UTF8, 0, t.c_str(), t.length(), NULL, 0);
	s.resize(len);
    // Convert string
    MultiByteToWideChar(CP_UTF8, 0, t.c_str(), t.length(), (wchar_t *)s.c_str(), s.length());

	return s;
};

std::wstring T2WConvert(TString t) {
#ifndef _UNICODE
	return A2WConvert(t);
#else
	return t;
#endif
};

std::string T2AConvert(TString t) {
#ifdef _UNICODE
	return W2AConvert(t);
#else
	return t;
#endif
};

TString W2TConvert(std::wstring t) {
#ifndef _UNICODE
	return W2AConvert(t);
#else
	return t;
#endif
};

TString A2TConvert(std::string t) {
#ifdef _UNICODE
	return A2WConvert(t);
#else
	return t;
#endif
};
