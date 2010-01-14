/// A handy little set of typedef and functions to help deal with CHAR <-> TCHAR <-> WCHAR strings
#ifndef _TSTRING_H_
#define _TSTRING_H_

typedef std::basic_string<TCHAR> TString;

std::string W2AConvert(std::wstring t);
std::wstring A2WConvert(std::string t);
std::wstring T2WConvert(TString t);
std::string T2AConvert(TString t);
TString W2TConvert(std::wstring t);
TString A2TConvert(std::string t);

#endif // _TSTRING_H_