
#ifndef WIDE_H
#define WIDE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#ifdef USE_UNICODE

#define CERR std::wcerr
#define COUT std::wcout
#define CIN std::wcin

typedef wchar_t Char;

#define String std::wstring

#define W(X) L ## X

#else

#define CERR std::cerr
#define COUT std::cout
#define CIN std::cin

typedef char Char;

#define String std::string

#define OStream std::ostream
#define IStream std::istream
#define OFStream std::ofstream
#define IFStream std::ifstream

#define OStringStream std::ostringstream

#define W(X) X

#endif // USE_UNICODE

#endif // WIDE_H
