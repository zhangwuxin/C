#include <iconv.h>
#include <string.h>
#include <stdint.h>

#include "charset.h"

int boss::CodeConvert(const char * from_charset, const char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    char **pin = &inbuf;
    char **pout = &outbuf;
    iconv_t cd = iconv_open(to_charset, from_charset);
    if ( cd == (iconv_t)(-1) ) return -1;
    memset(outbuf, 0, outlen);
    int iRet = iconv(cd, pin, (size_t *) & inlen, pout, (size_t *) & outlen);
    iconv_close(cd);
    if ( iRet == -1 ) return -1;
    return 0;
}

int boss::u2g(const char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    char * pIn = const_cast<char *>(inbuf);
    return CodeConvert("UTF-8", "GBK//IGNORE", pIn, inlen, outbuf, outlen);
}

int boss::g2u(const char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    char * pIn = const_cast<char *>(inbuf);
    return CodeConvert("GBK", "UTF-8//IGNORE", pIn, inlen, outbuf, outlen);
}
std::string boss::u2g(const std::string & sSrc)
{
    int iLen = sSrc.length();
    char * pBuffer = new char [iLen + 1];
    bzero(pBuffer, iLen + 1);
    u2g(sSrc.c_str(), sSrc.length(), pBuffer, iLen);
    std::string sDest = pBuffer;
    delete [] pBuffer;
    pBuffer = NULL;
    return sDest;
}
std::string boss::g2u(const std::string & sSrc)
{
    int iLen = sSrc.length();
    uint32_t dwULen = iLen * 2;
    char * pBuffer = new char [dwULen]; //u为g的1.5倍
    bzero(pBuffer, dwULen);
    g2u(sSrc.c_str(), sSrc.length(), pBuffer, dwULen);
    std::string sDest = pBuffer;
    delete [] pBuffer;
    pBuffer = NULL;
    return sDest;
}

//全角转半角 只处理0-9 A-Z a-z
int boss::toDBC(const std::string & sSrc,std::string & sOut)
{
    if ( sSrc.empty() )return 0;
    unsigned char c1 = 0;
    std::string::size_type len = sSrc.size();
    std::string sDest;
    for ( std::string::size_type i = 0;i < len;i++ )
    {
        c1 = sSrc[i];
        if ( c1 > 128 && i + 1 >= len )
        {
            return -1;//双字节后面应该还有
        }

        if ( c1 == 0xA3 )//判断是否为全角字符  0xA3
        {
            unsigned char c2 = sSrc[i+1];
            if( (c2>=0xB0 && c2<=0xB9)
                ||(c2>=0xC1 && c2<=0xDA)
                ||(c2>=0xE1 && c2<=0xFA) )
            sDest += sSrc[++i] - 128;
            continue;
        }
        
        if ( c1 > 128 )     //其他中文不动
        {
            sDest += c1;
            sDest += sSrc[++i];
            continue;
        }        
        sDest += sSrc[i];   //英文不动
    }
    sOut = sDest;
    return 0;
}

//参考 http://ff.163.com/newflyff/gbk-list/
//清理符号
int boss::CleanSymbol(const std::string & sSrc,std::string & sOut)
{
    if ( sSrc.empty() )return 0;
    unsigned char c1 = 0;
    std::string::size_type len = sSrc.size();
    std::string sDest;
    for ( std::string::size_type i = 0;i < len; ++i )
    {
        c1 = sSrc[i];
        if ( c1 > 128 && i + 1 >= len )
        {
            return -1;//双字节后面应该还有
        }

        if ( c1>=0xA1 && c1<=0xA9 )//字符
        {
            ++i;
            continue;
        }
        
        if ( c1 > 128 )     //其他中文不动
        {
            sDest += c1;
            sDest += sSrc[++i];
            continue;
        }

        if(isalnum(c1))
        {
           sDest += sSrc[i];  
        }
    }
    sOut = sDest;
    return 0;
    
}


//把URI字符串采用UTF-8编码格式转化成escape格式的字符串。不会被此方法编码的字符：! @ # $& * ( ) = : / ; ? + '
std::string boss::EncodeURI(const std::string &sContent)
{
	std::string sResult = "";
	char c;
	char *pcDigits = "0123456789ABCDEF";

	for (size_t i = 0; i < sContent.size(); i++)
	{
		c = sContent[i];

		if (('0'<=c && c<='9') || ('a'<=c && c<='z') || ('A'<=c && c<='Z') || strchr("!@#$&*()=:/;?+'", c) != NULL)
		{
			sResult += c;
		}
		else
		{
			sResult += "%";
			sResult += pcDigits[(c >> 4) & 0x0f];
			sResult += pcDigits[c & 0x0f];
		}
	}
	return sResult;
}


std::string boss::DecodeURI(const std::string &sContent)
{
	std::string sResult = sContent;
	int x, y;

	for (x = 0, y = 0; sContent[y]; x++, y++)
	{
		if((sResult[x] = sContent[y]) == '%')
		{
			sResult[x] = X2C(&sContent[y+1]);
			y += 2;
		}
	}

	return sResult.substr(0, x);
}

//把URI字符串采用UTF-8编码格式转化成escape格式的字符串。不会被此方法编码的字符：! * ( )
std::string boss::EncodeURIComponent(const std::string &sContent)
{
	std::string sResult = "";
	char c;
	char *pcDigits = "0123456789ABCDEF";

	for (size_t i = 0; i < sContent.size(); i++)
	{
		c = sContent[i];

		if (('0'<=c && c<='9') || ('a'<=c && c<='z') || ('A'<=c && c<='Z') || strchr("!*()", c) != NULL)
		{
			sResult += c;
		}
		else
		{
			sResult += "%";
			sResult += pcDigits[(c >> 4) & 0x0f];
			sResult += pcDigits[c & 0x0f];
		}
	}
	return sResult;
}

std::string boss::DecodeURIComponent(const std::string &sContent)
{
	std::string sResult = sContent;
	int x, y;

	for (x = 0, y = 0; sContent[y]; x++, y++)
	{
		if((sResult[x] = sContent[y]) == '%')
		{
			sResult[x] = X2C(&sContent[y+1]);
			y += 2;
		}
	}

	return sResult.substr(0, x);
}


char boss::X2C(const char *pcWhat)
{
	char cDigit;

	cDigit = (toupper(pcWhat[0]) >= 'A' ? (toupper(pcWhat[0]) - 'A') + 10 : (pcWhat[0] - '0'));
	cDigit *= 16;
	cDigit += (toupper(pcWhat[1]) >= 'A' ? (toupper(pcWhat[1]) - 'A') + 10 : (pcWhat[1] - '0'));

	return cDigit;
}


std::string boss::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string boss::base64_decode(std::string const& encoded_string) {
  size_t in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

