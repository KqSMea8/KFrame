﻿/**
 * URI decode encode接口
 *
 * @version 3.0.0
 * @author open.qq.com
 * @copyright (c) 2013, Tencent Corporation. All rights reserved.
 * @ History:
 *               3.0.0 | jixingguan | 2013-05-16 10:33:04 | initialize
 */


#ifndef _KF_URLCharacter_H__
#define _KF_URLCharacter_H__


#include <string>

namespace KFURLCharacter {

    enum errorCode {
        ERR_NULL_PARAMS = -100,
        ERR_BUFFER_TOO_SMALL = -99,
        ERR_ENCODE = -98,
        OK = 0
    };

    enum charsetCheck {
        NO_CHECK = 0,
        UTF8_CHECK = 1,
        GBK_CHECK = 2
    };

    struct encodeURIDefineNode {
        size_t len;
        char ta[4];
    };


    struct decodeURIDefineNode {
        int highVal;
        int lowVal;
    };


    /***********************************************************
    **函数名 encodeURIValue
    **输入:    sourceStr  源字符串
    **
    **输出:  无
    **
    **返回: 编码后的结果串
    **描述:  对URI中的参数进行标准编码
    **		   规范见 RFC 1738
    **************************************************************/
    inline std::string encodeURIValue( const std::string& sourceStr );

    /***********************************************************
    **函数名 encodeURIValue
    **输入:    sourceStr  源字符串
    **
    **输出:  resultStr  编码后的字符串
    **
    **返回:  0           成功
    		    其他  失败
    **描述:  对URI中的参数进行标准编码带返回值版本
    **		   规范见 RFC 1738
    **************************************************************/
    inline int encodeURIValue( std::string& resultStr, const std::string& sourceStr );


    /***********************************************************
    **函数名 encodeURIValue
    **输入:     sourceStr  源字符串
    **			resultBufferSize 结果缓冲区的最小容量

    **
    **输出:  resultBuffer  编码后的字符串
    **
    **返回:  0           成功
    		    其他  失败
    **描述:  对URI中的参数进行标准编码带返回值版本
    **		   规范见 RFC 1738
    **************************************************************/
    inline int encodeURIValue( char* resultBuffer, const char* sourceStr, size_t resultBufferSize );

    /***********************************************************
    **函数名 decodeURIValue
    **输入:     sourceStr  源字符串
    **
    **输出:
    **
    **返回:  解码后的字符串
    **描述:  对URI中的参数进行标准解码
    **		   规范见 RFC 1738
    **************************************************************/
    inline std::string decodeURIValue( const std::string& sourceStr );

    /***********************************************************
    **函数名 decodeURIValue
    **输入: 	sourceStr  源字符串
    **
    **输出: 	resultStr  解码后的字符串
    **
    **返回:  0 成功
    		   其他失败
    **描述:  对URI中的参数进行标准解码，带返回值版本
    **		   规范见 RFC 1738
    **************************************************************/
    inline int decodeURIValue( std::string& resultStr, const std::string& sourceStr );

    /***********************************************************
    **函数名 decodeURIValue
    **输入: 	sourceStr  源字符串
    			resultBufferSize 结果缓冲区的最小容量
    **
    **输出: 	resultStr  解码后的字符串
    **
    **返回:  0 成功
    		   其他失败
    **描述:  对URI中的参数进行标准解码，带返回值的C版本
    **		   规范见 RFC 1738
    **************************************************************/
    inline int decodeURIValue( char* resultBuffer, const char* sourceStr, size_t resultBufferSize );

};


/**
	 * 用以编码的字符映射表
	 * @type {static encodeURIDefineNode}
	 */
static KFURLCharacter::encodeURIDefineNode URIEncodeMap[256] = {
    {3, "%00"},
    {3, "%01"},
    {3, "%02"},
    {3, "%03"},
    {3, "%04"},
    {3, "%05"},
    {3, "%06"},
    {3, "%07"},
    {3, "%08"},
    {3, "%09"},
    {3, "%0A"},
    {3, "%0B"},
    {3, "%0C"},
    {3, "%0D"},
    {3, "%0E"},
    {3, "%0F"},
    {3, "%10"},
    {3, "%11"},
    {3, "%12"},
    {3, "%13"},
    {3, "%14"},
    {3, "%15"},
    {3, "%16"},
    {3, "%17"},
    {3, "%18"},
    {3, "%19"},
    {3, "%1A"},
    {3, "%1B"},
    {3, "%1C"},
    {3, "%1D"},
    {3, "%1E"},
    {3, "%1F"},
    {3, "%20"},
    {1, "!"},
    {3, "%22"},
    {3, "%23"},
    {3, "%24"},
    {3, "%25"},
    {3, "%26"},
    {1, "'"},
    {1, "("},
    {1, ")"},
    {1, "*"},
    {3, "%2B"},
    {3, "%2C"},
    {1, "-"},
    {1, "."},
    {3, "%2F"},
    {1, "0"},
    {1, "1"},
    {1, "2"},
    {1, "3"},
    {1, "4"},
    {1, "5"},
    {1, "6"},
    {1, "7"},
    {1, "8"},
    {1, "9"},
    {3, "%3A"},
    {3, "%3B"},
    {3, "%3C"},
    {3, "%3D"},
    {3, "%3E"},
    {3, "%3F"},
    {3, "%40"},
    {1, "A"},
    {1, "B"},
    {1, "C"},
    {1, "D"},
    {1, "E"},
    {1, "F"},
    {1, "G"},
    {1, "H"},
    {1, "I"},
    {1, "J"},
    {1, "K"},
    {1, "L"},
    {1, "M"},
    {1, "N"},
    {1, "O"},
    {1, "P"},
    {1, "Q"},
    {1, "R"},
    {1, "S"},
    {1, "T"},
    {1, "U"},
    {1, "V"},
    {1, "W"},
    {1, "X"},
    {1, "Y"},
    {1, "Z"},
    {3, "%5B"},
    {3, "%5C"},
    {3, "%5D"},
    {3, "%5E"},
    {1, "_"},
    {3, "%60"},
    {1, "a"},
    {1, "b"},
    {1, "c"},
    {1, "d"},
    {1, "e"},
    {1, "f"},
    {1, "g"},
    {1, "h"},
    {1, "i"},
    {1, "j"},
    {1, "k"},
    {1, "l"},
    {1, "m"},
    {1, "n"},
    {1, "o"},
    {1, "p"},
    {1, "q"},
    {1, "r"},
    {1, "s"},
    {1, "t"},
    {1, "u"},
    {1, "v"},
    {1, "w"},
    {1, "x"},
    {1, "y"},
    {1, "z"},
    {3, "%7B"},
    {3, "%7C"},
    {3, "%7D"},
    {1, "~"},
    {3, "%7F"},
    {3, "%80"},
    {3, "%81"},
    {3, "%82"},
    {3, "%83"},
    {3, "%84"},
    {3, "%85"},
    {3, "%86"},
    {3, "%87"},
    {3, "%88"},
    {3, "%89"},
    {3, "%8A"},
    {3, "%8B"},
    {3, "%8C"},
    {3, "%8D"},
    {3, "%8E"},
    {3, "%8F"},
    {3, "%90"},
    {3, "%91"},
    {3, "%92"},
    {3, "%93"},
    {3, "%94"},
    {3, "%95"},
    {3, "%96"},
    {3, "%97"},
    {3, "%98"},
    {3, "%99"},
    {3, "%9A"},
    {3, "%9B"},
    {3, "%9C"},
    {3, "%9D"},
    {3, "%9E"},
    {3, "%9F"},
    {3, "%A0"},
    {3, "%A1"},
    {3, "%A2"},
    {3, "%A3"},
    {3, "%A4"},
    {3, "%A5"},
    {3, "%A6"},
    {3, "%A7"},
    {3, "%A8"},
    {3, "%A9"},
    {3, "%AA"},
    {3, "%AB"},
    {3, "%AC"},
    {3, "%AD"},
    {3, "%AE"},
    {3, "%AF"},
    {3, "%B0"},
    {3, "%B1"},
    {3, "%B2"},
    {3, "%B3"},
    {3, "%B4"},
    {3, "%B5"},
    {3, "%B6"},
    {3, "%B7"},
    {3, "%B8"},
    {3, "%B9"},
    {3, "%BA"},
    {3, "%BB"},
    {3, "%BC"},
    {3, "%BD"},
    {3, "%BE"},
    {3, "%BF"},
    {3, "%C0"},
    {3, "%C1"},
    {3, "%C2"},
    {3, "%C3"},
    {3, "%C4"},
    {3, "%C5"},
    {3, "%C6"},
    {3, "%C7"},
    {3, "%C8"},
    {3, "%C9"},
    {3, "%CA"},
    {3, "%CB"},
    {3, "%CC"},
    {3, "%CD"},
    {3, "%CE"},
    {3, "%CF"},
    {3, "%D0"},
    {3, "%D1"},
    {3, "%D2"},
    {3, "%D3"},
    {3, "%D4"},
    {3, "%D5"},
    {3, "%D6"},
    {3, "%D7"},
    {3, "%D8"},
    {3, "%D9"},
    {3, "%DA"},
    {3, "%DB"},
    {3, "%DC"},
    {3, "%DD"},
    {3, "%DE"},
    {3, "%DF"},
    {3, "%E0"},
    {3, "%E1"},
    {3, "%E2"},
    {3, "%E3"},
    {3, "%E4"},
    {3, "%E5"},
    {3, "%E6"},
    {3, "%E7"},
    {3, "%E8"},
    {3, "%E9"},
    {3, "%EA"},
    {3, "%EB"},
    {3, "%EC"},
    {3, "%ED"},
    {3, "%EE"},
    {3, "%EF"},
    {3, "%F0"},
    {3, "%F1"},
    {3, "%F2"},
    {3, "%F3"},
    {3, "%F4"},
    {3, "%F5"},
    {3, "%F6"},
    {3, "%F7"},
    {3, "%F8"},
    {3, "%F9"},
    {3, "%FA"},
    {3, "%FB"},
    {3, "%FC"},
    {3, "%FD"},
    {3, "%FE"},
    {3, "%FF"}
};

/**
 * 用以解码的字符映射表
 * @type {static decodeURIDefineNode}
 *
 */
static KFURLCharacter::decodeURIDefineNode URIDecodeMap[103] = {
    {0, 0}, //0
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0}, //20
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0}, //30
    {0, 0},
    {0, 0}, //32
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0}, //40
    {0, 0},
    {0, 0},
    {0, 20}, //43 + --> ' '
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {16, 1},
    {32, 2},
    {48, 3},
    {64, 4},
    {80, 5},
    {96, 6},
    {112, 7},
    {128, 8},
    {144, 9},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {160, 10},
    {176, 11},
    {192, 12},
    {208, 13},
    {224, 14},
    {240, 15},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {160, 10},
    {176, 11},
    {192, 12},
    {208, 13},
    {224, 14},
    {240, 15}
};

/***********************************************************
**函数名 encodeURIValue
**输入:    sourceStr  源字符串
**
**输出:  无
**
**返回: 编码后的结果串
**描述:  对URI中的参数进行标准编码
**         规范见 RFC 1738
**************************************************************/
std::string KFURLCharacter::encodeURIValue( const std::string& sourceStr ) {
    std::string re;

    encodeURIValue( re, sourceStr );

    return re;
}

/***********************************************************
**函数名 encodeURIValue
**输入:    sourceStr  源字符串
**
**输出:  resultStr  编码后的字符串
**
**返回:  0           成功
            其他  失败
**描述:  对URI中的参数进行标准编码带返回值版本
**         规范见 RFC 1738
**************************************************************/
int KFURLCharacter::encodeURIValue( std::string& resultStr, const std::string& sourceStr ) {

    size_t len = sourceStr.size() * 3 + 10;
    char* buff = new char[len];

    int res = encodeURIValue( buff, sourceStr.c_str(), len );
    resultStr = buff;

    delete [] buff;

    return res;
}


/***********************************************************
**函数名 encodeURIValue
**输入:     sourceStr  源字符串
**          resultBufferSize 结果缓冲区的最小容量

**
**输出:  resultBuffer  编码后的字符串
**
**返回:  0           成功
            其他  失败
**描述:  对URI中的参数进行标准编码带返回值版本
**         规范见 RFC 1738
**************************************************************/
int KFURLCharacter::encodeURIValue( char* resultBuffer, const char* sourceStr, size_t resultBufferSize ) {
    if ( sourceStr == NULL || resultBuffer == NULL ) {
        return ERR_NULL_PARAMS;
    }

    register size_t blen = resultBufferSize - 1,
                    c = 0;
    register unsigned char* p = ( unsigned char* )sourceStr;
    register encodeURIDefineNode* tmp = NULL;
    int res = OK;


    while ( *p != '\0' ) {
        tmp = &URIEncodeMap[*p];
        if ( c + ( *tmp ).len > blen ) {
            res = ERR_BUFFER_TOO_SMALL;
            break;
        } else {
            memcpy( resultBuffer + c, ( *tmp ).ta, ( *tmp ).len );
            c += ( *tmp ).len;
        }
        ++p;
    }
    *( resultBuffer + c ) = '\0';

    return res;
}

/***********************************************************
**函数名 decodeURIValue
**输入:     sourceStr  源字符串
**
**输出:
**
**返回:  解码后的字符串
**描述:  对URI中的参数进行标准解码
**         规范见 RFC 1738
**************************************************************/
std::string KFURLCharacter::decodeURIValue( const std::string& sourceStr ) {
    std::string re;

    decodeURIValue( re, sourceStr );

    return re;
}

/***********************************************************
**函数名 decodeURIValue
**输入:     sourceStr  源字符串
**
**输出:     resultStr  解码后的字符串
**
**返回:  0 成功
           其他失败
**描述:  对URI中的参数进行标准解码，带返回值版本
**         规范见 RFC 1738
**************************************************************/
int KFURLCharacter::decodeURIValue( std::string& resultStr, const std::string& sourceStr ) {

    size_t len = sourceStr.size() + 10;
    char* buff = new char[len];

    int res = decodeURIValue( buff, sourceStr.c_str(), len );
    resultStr = buff;

    delete [] buff;

    return res;
}

/***********************************************************
**函数名 decodeURIValue
**输入:     sourceStr  源字符串
            resultBufferSize 结果缓冲区的最小容量
**
**输出:     resultStr  解码后的字符串
**
**返回:  0 成功
           其他失败
**描述:  对URI中的参数进行标准解码，带返回值的C版本
**         规范见 RFC 1738
**************************************************************/
int KFURLCharacter::decodeURIValue( char* resultBuffer, const char* sourceStr, size_t resultBufferSize ) {
    if ( sourceStr == NULL || resultBuffer == NULL ) {
        return ERR_NULL_PARAMS;
    }

    register size_t blen = resultBufferSize - 1,
                    c = 0;

    register unsigned char* p = ( unsigned char* )sourceStr;
    register decodeURIDefineNode* tmph = NULL;
    register decodeURIDefineNode* tmpl = NULL;
    int res = OK;

    while ( *p != '\0' ) {
        if ( c + 1 > blen ) {
            res = ERR_BUFFER_TOO_SMALL;
            break;
        }

        if ( ( '%' == *p ) && isxdigit( *( p + 1 ) ) && isxdigit( *( p + 2 ) ) ) {
            tmph = &URIDecodeMap[*( p + 1 )];
            tmpl = &URIDecodeMap[*( p + 2 )];
            *( resultBuffer + c ) = ( *tmph ).highVal + ( *tmpl ).lowVal;
            p += 3;
        } else {
            *( resultBuffer + c ) = ( *p == '+' ) ? 0x20 : *p;
            ++p;
        }

        c += 1;
    }
    *( resultBuffer + c ) = '\0';

    return res;
}

#endif //!_KF_URLCharacter_H__