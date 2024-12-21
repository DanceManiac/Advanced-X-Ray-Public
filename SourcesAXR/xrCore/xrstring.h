#pragma once

#include <locale>
#include <algorithm>

#pragma pack(push,4)
//////////////////////////////////////////////////////////////////////////
typedef const char*		str_c;

//////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4200)

static std::unordered_map<char, char> lowerCaseTable = {
	{'А', 'а'}, {'Б', 'б'}, {'В', 'в'}, {'Г', 'г'}, {'Д', 'д'},
	{'Е', 'е'}, {'Ё', 'ё'}, {'Ж', 'ж'}, {'З', 'з'}, {'И', 'и'},
	{'Й', 'й'}, {'К', 'к'}, {'Л', 'л'}, {'М', 'м'}, {'Н', 'н'},
	{'О', 'о'}, {'П', 'п'}, {'Р', 'р'}, {'С', 'с'}, {'Т', 'т'},
	{'У', 'у'}, {'Ф', 'ф'}, {'Х', 'х'}, {'Ц', 'ц'}, {'Ч', 'ч'},
	{'Ш', 'ш'}, {'Щ', 'щ'}, {'Ъ', 'ъ'}, {'Ы', 'ы'}, {'Ь', 'ь'},
	{'Э', 'э'}, {'Ю', 'ю'}, {'Я', 'я'}
};

static std::unordered_map<xr_string, xr_string> lowerCaseTableUtf8 = {
	{"\xD0\x90", "\xD0\xB0"}, // А -> а
	{"\xD0\x91", "\xD0\xB1"}, // Б -> б
	{"\xD0\x92", "\xD0\xB2"}, // В -> в
	{"\xD0\x93", "\xD0\xB3"}, // Г -> г
	{"\xD0\x94", "\xD0\xB4"}, // Д -> д
	{"\xD0\x95", "\xD0\xB5"}, // Е -> е
	{"\xD0\x96", "\xD0\xB6"}, // Ж -> ж
	{"\xD0\x97", "\xD0\xB7"}, // З -> з
	{"\xD0\x98", "\xD0\xB8"}, // И -> и
	{"\xD0\x99", "\xD0\xB9"}, // Й -> й
	{"\xD0\x9A", "\xD0\xBA"}, // К -> к
	{"\xD0\x9B", "\xD0\xBB"}, // Л -> л
	{"\xD0\x9C", "\xD0\xBC"}, // М -> м
	{"\xD0\x9D", "\xD0\xBD"}, // Н -> н
	{"\xD0\x9E", "\xD0\xBE"}, // О -> о
	{"\xD0\x9F", "\xD0\xBF"}, // П -> п
	{"\xD0\xA0", "\xD1\x80"}, // Р -> р
	{"\xD0\xA1", "\xD1\x81"}, // С -> с
	{"\xD0\xA2", "\xD1\x82"}, // Т -> т
	{"\xD0\xA3", "\xD1\x83"}, // У -> у
	{"\xD0\xA4", "\xD1\x84"}, // Ф -> ф
	{"\xD0\xA5", "\xD1\x85"}, // Х -> х
	{"\xD0\xA6", "\xD1\x86"}, // Ц -> ц
	{"\xD0\xA7", "\xD1\x87"}, // Ч -> ч
	{"\xD0\xA8", "\xD1\x88"}, // Ш -> ш
	{"\xD0\xA9", "\xD1\x89"}, // Щ -> щ
	{"\xD0\xAA", "\xD1\x8A"}, // Ъ -> ъ
	{"\xD0\xAB", "\xD1\x8B"}, // Ы -> ы
	{"\xD0\xAC", "\xD1\x8C"}, // Ь -> ь
	{"\xD0\xAD", "\xD1\x8D"}, // Э -> э
	{"\xD0\xAE", "\xD1\x8E"}, // Ю -> ю
	{"\xD0\xAF", "\xD1\x8F"}, // Я -> я
};

struct		XRCORE_API	str_value
{
	u32					dwReference		;
	u32					dwLength		;
	u32					dwCRC			;
	str_value*          next            ;
	char				value		[]	;
};

struct		XRCORE_API	str_value_cmp	{ // less
	IC bool		operator ()	(const str_value* A, const str_value* B) const	{ return A->dwCRC<B->dwCRC;	};
};

struct		XRCORE_API	str_hash_function {
	IC u32		operator ()	(str_value const* const value) const	{ return value->dwCRC;	};
};

#pragma warning(default : 4200)

struct str_container_impl;
class IWriter;
//////////////////////////////////////////////////////////////////////////
class		XRCORE_API	str_container
{
private:
	xrCriticalSection					cs;
	str_container_impl*                 impl;
public:
						str_container	();
						~str_container  ();

	str_value*			dock			(str_c value);
	void				clean			();
	void				dump			();
	void				dump			(IWriter* W);
	void				verify			();
	u32					stat_economy	();
#ifdef PROFILE_CRITICAL_SECTIONS
						str_container	():cs(MUTEX_PROFILE_ID(str_container)){}
#endif // PROFILE_CRITICAL_SECTIONS
};
XRCORE_API	extern		str_container*	g_pStringContainer;

//////////////////////////////////////////////////////////////////////////
class					shared_str
{
private:
	str_value*			p_;
protected:
	// ref-counting
	void				_dec		()								{	if (0==p_) return;	p_->dwReference--; 	if (0==p_->dwReference)	p_=0;						}
public:
	void				_set		(str_c rhs) 					{	str_value* v = g_pStringContainer->dock(rhs); if (0!=v) v->dwReference++; _dec(); p_ = v;	}
	void				_set		(shared_str const &rhs)			{	str_value* v = rhs.p_; if (0!=v) v->dwReference++; _dec(); p_ = v;							}
//	void				_set		(shared_str const &rhs)			{	str_value* v = g_pStringContainer->dock(rhs.c_str()); if (0!=v) v->dwReference++; _dec(); p_ = v;							}
	


	const str_value*	_get		()	const						{	return p_;																					}
public:
	// construction
						shared_str	()								{	p_ = 0;											}
						shared_str	(str_c rhs) 					{	p_ = 0;	_set(rhs);								}
						shared_str	(shared_str const &rhs)			{	p_ = 0;	_set(rhs);								}
						~shared_str	()								{	_dec();											}

	// assignment & accessors
	shared_str&			operator=	(str_c rhs)						{	_set(rhs);	return (shared_str&)*this;			}
	shared_str&			operator=	(shared_str const &rhs)			{	_set(rhs);	return (shared_str&)*this;			}
	str_c				operator*	() const						{	return p_?p_->value:0;							}
	bool				operator!	() const						{	return p_ == 0;									}
	char				operator[]	(size_t id)						{	return p_->value[id];							}
	str_c				c_str		() const						{	return p_?p_->value:0;							}
	str_c				data		() const						{ return p_ ? p_->value : "";						}

	// misc func
	u32					size		()						const	{	if (0==p_) return 0; else return p_->dwLength;	}
	void				swap		(shared_str & rhs)				{	str_value* tmp = p_; p_ = rhs.p_; rhs.p_ = tmp;	}
	bool				equal		(const shared_str & rhs) const	{	return (p_ == rhs.p_);							}
	shared_str& __cdecl	printf		(const char* format, ...)		{
		string4096 	buf;
		va_list		p;
		va_start	(p,format);
		int vs_sz	= _vsnprintf(buf,sizeof(buf)-1,format,p); buf[sizeof(buf)-1]=0;
		va_end		(p);
		if (vs_sz)	_set(buf);	
		return 		(shared_str&)*this;
	}
};

// res_ptr == res_ptr
// res_ptr != res_ptr
// const res_ptr == ptr
// const res_ptr != ptr
// ptr == const res_ptr
// ptr != const res_ptr
// res_ptr < res_ptr
// res_ptr > res_ptr
IC bool operator	==	(shared_str const & a, shared_str const & b)		{ return a._get() == b._get();					}
IC bool operator	!=	(shared_str const & a, shared_str const & b)		{ return a._get() != b._get();					}
IC bool operator	<	(shared_str const & a, shared_str const & b)		{ return a._get() <  b._get();					}
IC bool operator	>	(shared_str const & a, shared_str const & b)		{ return a._get() >  b._get();					}

// externally visible standart functionality
IC void swap			(shared_str & lhs, shared_str & rhs)				{ lhs.swap(rhs);		}
IC u32	xr_strlen		(shared_str & a)									{ return a.size();		}
IC int	xr_strcmp		(const shared_str & a, const char* b)				{ return xr_strcmp(*a,b);	}
IC int	xr_strcmp		(const char* a, const shared_str & b)				{ return xr_strcmp(a,*b);	}
IC int	xr_strcmp		(const shared_str & a, const shared_str & b)		{ 
	if (a.equal(b))		return 0;
	else				return xr_strcmp(*a,*b);
}
IC void	xr_strlwr		(xr_string& src)									{ for(xr_string::iterator it=src.begin(); it!=src.end(); it++) *it=xr_string::value_type(tolower(*it));}
IC void	xr_strlwr		(shared_str& src)									{ if (*src){LPSTR lp=xr_strdup(*src); xr_strlwr(lp); src=lp; xr_free(lp);} }

// Преобразование char* в Utf8 xr_string
IC xr_string toUtf8(const char* s)
{
	static xr_vector<wchar_t> buf;
	int n = MultiByteToWideChar(CP_ACP, 0, s, -1, nullptr, 0);
	buf.resize(n);
	MultiByteToWideChar(CP_ACP, 0, s, -1, &buf[0], buf.size());
	xr_string result;
	n = WideCharToMultiByte(CP_UTF8, 0, &buf[0], buf.size(), nullptr, 0, nullptr, nullptr);
	result.resize(n);
	n = WideCharToMultiByte(CP_UTF8, 0, &buf[0], buf.size(), &result[0], result.size(), nullptr, nullptr);
	return result;
}

// Преобразование Utf8 xr_string в нижний регистр
IC void ToLowerUtf8(xr_string& src)
{
	std::locale loc;
	for (xr_string::iterator it = src.begin(); it != src.end(); ++it)
	{
		*it = std::tolower(*it, loc);
	}
}

// Преобразование Utf8 shared_str в нижний регистр
IC void ToLowerUtf8(shared_str& src)
{
	if (*src)
	{
		xr_string tempStr = src.c_str();

		std::locale loc;
		std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), [&loc](char c)
			{
				return std::tolower(c, loc);
			});

		src = shared_str(tempStr.c_str());
	}
}

// Преобразование кириллической Utf8 xr_string в нижний регистр
IC void ToLowerUtf8RU(xr_string& str)
{
	xr_string result;

	for (size_t i = 0; i < str.size();)
	{
		if ((str[i] & 0xC0) == 0xC0)
		{
			int charLength = 1;

			if ((str[i] & 0xF0) == 0xF0)
				charLength = 4;
			else if ((str[i] & 0xE0) == 0xE0)
				charLength = 3;
			else if ((str[i] & 0xC0) == 0xC0)
				charLength = 2;

			xr_string utf8Char = str.substr(i, charLength);

			auto it = lowerCaseTableUtf8.find(utf8Char);

			if (it != lowerCaseTableUtf8.end())
				result += it->second;
			else
				result += utf8Char;

			i += charLength;
		}
		else
		{
			result += std::tolower(str[i]);
			i++;
		}
	}

	str = result;
}

// Преобразование кириллической Utf8 shared_str в нижний регистр
IC void ToLowerUtf8RU(shared_str& str)
{
	xr_string tempStr = str.c_str();
	ToLowerUtf8RU(tempStr);
	str = tempStr.c_str();
}

// Поиск подстроки в кириллической Utf8 xr_string
IC size_t xr_string_find(const xr_string& str, const xr_string& substr)
{
	xr_string strLower = str;
	xr_string substrLower = substr;

	for (char& ch : strLower)
	{
		auto it = lowerCaseTable.find(ch);

		if (it != lowerCaseTable.end())
			ch = it->second;
		else
			ch = std::tolower(ch);
	}

	for (char& ch : substrLower)
	{
		auto it = lowerCaseTable.find(ch);

		if (it != lowerCaseTable.end())
			ch = it->second;
		else
			ch = std::tolower(ch);
	}

	return strLower.find(substrLower);
}

#pragma pack(pop)

inline bool SplitFilename(std::string& str)
{
	size_t found = str.find_last_of("/\\");
	if (found != std::string::npos)
	{
		str.erase(found);
		return true;
	}
	return false;
}
