

/*
*
*	Copyright (C) 2016 Luxon Jean-Pierre
*	gumichan01.olympe.in
*
*
*	Luxon Jean-Pierre (Gumichan01)
*	luxon.jean.pierre@gmail.com
*
*/

#include "utf8_string.hpp"
#include <cstring>
#include <stdexcept>


UTF8string::UTF8string() : utf8length(0){}

UTF8string::UTF8string(const std::string &str)
 : utf8data(str)
{
    if(!utf8_is_valid_())
        throw std::invalid_argument("Invalid UTF-8 string\n");

    utf8length = utf8_length_();
}


UTF8string::UTF8string(const UTF8string &u8str)
{
    utf8data = u8str.utf8data;
    utf8length = u8str.utf8length;
}


const UTF8string& UTF8string::operator =(const char * str)
{
    utf8data = str;

    if(!utf8_is_valid_())
        throw std::invalid_argument("Invalid UTF-8 string\n");

    utf8length = utf8_length_();
    return *this;
}


const UTF8string& UTF8string::operator =(const std::string str)
{
    utf8data = str;

    if(!utf8_is_valid_())
        throw std::invalid_argument("Invalid UTF-8 string\n");

    utf8length = utf8_length_();
    return *this;
}


const UTF8string& UTF8string::operator =(const UTF8string u8str)
{
    utf8data = u8str.utf8data;
    utf8length = u8str.utf8length;
    return *this;
}


const UTF8string& UTF8string::operator +=(const std::string str)
{
    utf8data += str;

    if(!utf8_is_valid_())
        throw std::invalid_argument("Invalid UTF-8 string\n");

    utf8length = utf8_length_();
    return *this;
}


const UTF8string& UTF8string::operator +=(const UTF8string u8str)
{
    utf8data += u8str.utf8data;
    utf8length = utf8_length_();
    return *this;
}


void UTF8string::utf8_clear()
{
    utf8data.clear();
    utf8length = 0;
}

bool UTF8string::utf8_empty() const
{
    return utf8length == 0;
}


UTF8string UTF8string::utf8_substr(size_t pos,size_t len) const
{
    if(pos > utf8length)
        return std::string();

    // Length of the substring (number of code points)
    const size_t n = (len == std::string::npos || (pos + len) > utf8length) ?
                        (utf8length - pos) : (pos + len - pos);
    // First position in bytes
    size_t beginsz = 0;
    const size_t u8size = utf8_size();

    for(size_t i = 0; beginsz < u8size && i < pos; i++)
    {
        beginsz += utf8_codepoint_len_(beginsz);
    }

    std::string s;
    size_t u8count = 0;

    for(size_t j = beginsz; j < u8size && u8count < n;)
    {
        const utf8_len_t cplen = utf8_codepoint_len_(j);
        size_t i = j;

        while(i < (j+cplen))
        {
            s.push_back(utf8data[i++]);
        }

        j += cplen;
        u8count++;
    }

    return s;
}


size_t UTF8string::utf8_find(const UTF8string& str, size_t pos) const
{
    // Go to the position
    size_t beginsz = 0;
    const size_t u8size = utf8_size();

    for(size_t i = 0; beginsz < u8size && i < pos; i++)
    {
        beginsz += utf8_codepoint_len_(beginsz);
    }

    // Look for the string
    while(beginsz < u8size)
    {
        UTF8string tmp = utf8_substr(pos,str.utf8_length());

        if(tmp == str)
            return pos;
        else
        {
            beginsz += utf8_codepoint_len_(pos);
            pos++;
        }
    }

    return std::string::npos;
}


utf8_len_t UTF8string::utf8_codepoint_len_(size_t j) const
{
    if (0xf0 == (0xf8 & utf8data[j]))
    {
        return 4;
    }
    else if (0xe0 == (0xf0 & utf8data[j]))
    {
        return 3;
    }
    else if (0xc0 == (0xe0 & utf8data[j]))
    {
        return 2;
    }
    else
        return 1;
}


utf8_len_t UTF8string::utf8_size() const
{
    return utf8data.size();
}

bool UTF8string::utf8_is_valid_()
{
    auto it = utf8data.begin();
    const auto itend = utf8data.end();

    while(it < itend)
    {
        if((0xF8 & *it) == 0xF0)
        {
            // The UTF-8 codepoint begin with 0b11110xxx -> 4-byte codepoint
            // If the iterator reach the end of the string before the
            // end of the 4-byte codepoint -> invalid string
            if((it + 1) == itend || (it + 2) == itend || (it + 3) == itend)
            {
                return false;
            }

            // Each of the following bytes is a value
            // between 0x80 and 0xBF
            if(((0xC0 & *(it + 1)) != 0x80) || ((0xC0 & *(it + 2)) != 0x80)
               || ((0xC0 & *(it + 3)) != 0x80))
            {
                return false;
            }

            // If the first byte of the sequence is 0xF0
            // then the first continuation byte must be between 90 and BF
            // otherwise, if the byte is 0xF4
            // then the first continuation byte must be between 80 and 8F
            if(*it == '\xF0')
            {
                if(*(it + 1) < '\x90' || *(it + 1) > '\xBF')
                    return false;
            }
            else if(*it == '\xF4')
            {
                if(*(it + 1) < '\x80' || *(it + 1) > '\x8F')
                    return false;
            }

            it += 4;    // Jump to the next codepoint
        }
        else if((0xF0 & *it) == 0xE0)
        {
            // The UTF-8 codepoint begin with 0b1110xxxx -> 3-byte codepoint
            if((it + 1) == itend || (it + 2) == itend)
            {
                return false;
            }

            // Each of the following bytes starts with
            // 0b10xxxxxx in a valid string
            if(((0xC0 & *(it + 1)) != 0x80) || ((0xC0 & *(it + 2)) != 0x80))
            {
                return false;
            }

            // If the first byte of the sequence is 0xE0
            // then the first continuation byte must be between A0 and BF
            // otherwise, if the byte is 0xF4
            // then the first continuation byte must be between 80 and 9F
            if(*it == '\xE0')
            {
                if(*(it + 1) < '\xA0' || *(it + 1) > '\xBF')
                    return false;
            }
            else if(*it == '\xED')
            {
                if(*(it + 1) < '\x80' || *(it + 1) > '\x9F')
                    return false;
            }

            it += 3;
        }
        else if((0xE0 & *it) == 0xC0)
        {
            // The UTF-8 codepoint begin with 0b110xxxxx -> 2-byte codepoint
            if((it + 1) == itend)
            {
                return false;
            }

            // The following byte starts with 0b10xxxxxx in a valid string
            if((0xC0 & *(it + 1)) != 0x80)
            {
                return false;
            }

            it += 2;
        }
        else if((0x80 & *it) == 0x00)
        {
            // The UTF-8 codepoint begin with 0b0xxxxxxx -> 1-byte codepoint
            it += 1;
        }
        else
        {
            // Invalid codepoint
            return false;
        }
    }

    return true;
}


utf8_len_t UTF8string::utf8_length_() const
{
    auto end_data = utf8data.end();
    auto it = utf8data.begin();
    utf8_len_t len = 0;

    while(it != end_data)
    {
        byte_t byte = *it;

        if (0xf0 == (0xf8 & byte))
        {
            // 4-byte utf8 character
            // (0b11110xxx 0bxxxxxxxx 0bxxxxxxxx 0bxxxxxxxx)
            it += 4;
        }
        else if (0xe0 == (0xf0 & byte))
        {
            // 3-byte utf8 code point (0b110xxxxx 0bxxxxxxxx 0bxxxxxxxx)
            it += 3;
        }
        else if (0xc0 == (0xe0 & byte))
        {
            // 2-byte utf8 code point (0b110xxxxx 0bxxxxxxxx)
            it += 2;
        }
        else
        {
            // 1-byte utf8 code point (0b0xxxxxxx)
            it += 1;
        }

        // We want the number of characters (utf-8 code point)
        len += 1;
    }

    return len;
}

utf8_len_t UTF8string::utf8_length() const
{
    return utf8length;
}

const char * UTF8string::utf8_str() const
{
    return utf8data.c_str();
}


std::string::iterator UTF8string::utf8_begin() noexcept
{
    return utf8data.begin();
}


std::string::iterator UTF8string::utf8_end() noexcept
{
    return utf8data.end();
}


bool operator ==(const UTF8string &str1, const UTF8string &str2)
{
    const std::string s1 = str1.utf8_str();
    const std::string s2 = str2.utf8_str();

    return s1 == s2;
}

bool operator !=(const UTF8string &str1, const UTF8string &str2)
{
    return !(str1 == str2);
}


bool operator <=(const UTF8string &str1, const UTF8string &str2)
{
    const std::string s1 = str1.utf8_str();
    const std::string s2 = str2.utf8_str();

    return s1 <= s2;
}


bool operator >=(const UTF8string &str1, const UTF8string &str2)
{
    const std::string s1 = str1.utf8_str();
    const std::string s2 = str2.utf8_str();

    return s1 >= s2;
}


bool operator <(const UTF8string &str1, const UTF8string &str2)
{
    const std::string s1 = str1.utf8_str();
    const std::string s2 = str2.utf8_str();

    return s1 < s2;
}


bool operator >(const UTF8string &str1, const UTF8string &str2)
{
    const std::string s1 = str1.utf8_str();
    const std::string s2 = str2.utf8_str();

    return s1 > s2;
}


std::ostream & operator <<(std::ostream &os, const UTF8string &str)
{
    os << str.utf8_str();
    return os;
}


std::istream & operator >>(std::istream &is, UTF8string &str)
{
    std::string tmp;
    is >> tmp;
    str = tmp;
    return is;
}
