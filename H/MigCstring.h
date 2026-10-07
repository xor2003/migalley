#pragma once
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cstdarg>
#include <cstdio>
#include <cstring>

// RERUN: Robust CString replacement that mimics MFC layout.
// By holding only a single char* pointer, sizeof(CString) == sizeof(char*).
// This allows CString objects to be passed directly to variadic functions (printf, CSprintf)
// expecting "%s", which matches the behavior of the original MFC CString on 32-bit builds.
class CString {
private:
    char* m_pchData;

    void Alloc(int nLen) {
        m_pchData = new char[nLen + 1];
        m_pchData[0] = '\0';
    }

    void Free() {
        if (m_pchData) delete[] m_pchData;
        m_pchData = nullptr;
    }

    void Init(const char* s) {
        if (!s) s = "";
        int len = strlen(s);
        m_pchData = new char[len + 1];
        strcpy(m_pchData, s);
    }

public:
    // Constructors
    CString() { Init(nullptr); }
    CString(const CString& other) { Init(other.m_pchData); }
    CString(const std::string& s) { Init(s.c_str()); }
    CString(const char* s) { Init(s); }
    
    CString(char ch, int nRepeat = 1) {
        if (nRepeat < 0) nRepeat = 0;
        m_pchData = new char[nRepeat + 1];
        memset(m_pchData, ch, nRepeat);
        m_pchData[nRepeat] = '\0';
    }
    
    CString(int i) { Init(std::to_string(i).c_str()); }
    CString(long l) { Init(std::to_string(l).c_str()); }
    CString(unsigned long ul) { Init(std::to_string(ul).c_str()); }

    ~CString() { Free(); }

    // RERUN Added for OVERLAY
    operator const char*() const { return m_pchData; }
    const char* c_str() const { return m_pchData; }
    const char* data() const { return m_pchData; }

    // Assignment operators
    CString& operator=(const CString& rhs) { if (this != &rhs) { Free(); Init(rhs.m_pchData); } return *this; }
    CString& operator=(const std::string& rhs) { Free(); Init(rhs.c_str()); return *this; }
    CString& operator=(const char* rhs) { Free(); Init(rhs); return *this; }
    CString& operator=(char ch) { Free(); m_pchData = new char[2]; m_pchData[0] = ch; m_pchData[1] = '\0'; return *this; }
    CString& operator=(int i) { *this = std::to_string(i); return *this; }
    CString& operator=(long l) { *this = std::to_string(l); return *this; }
    CString& operator=(unsigned long ul) { *this = std::to_string(ul); return *this; }

    // Length / empty
    int GetLength() const { return (int)strlen(m_pchData); }
    bool IsEmpty() const { return m_pchData[0] == '\0'; }
    void Empty() { Free(); Init(nullptr); }

    // Character access
    char GetAt(int nIndex) const { return m_pchData[nIndex]; }
    void SetAt(int nIndex, char ch) { m_pchData[nIndex] = ch; }

    // Concatenation
    CString& operator+=(const CString& rhs) {
        int newLen = GetLength() + rhs.GetLength();
        char* newData = new char[newLen + 1];
        strcpy(newData, m_pchData);
        strcat(newData, rhs.m_pchData);
        Free();
        m_pchData = newData;
        return *this;
    }
    CString& operator+=(char ch) {
        int len = GetLength();
        char* newData = new char[len + 2];
        strcpy(newData, m_pchData);
        newData[len] = ch;
        newData[len + 1] = '\0';
        Free();
        m_pchData = newData;
        return *this;
    }
    CString& operator+=(const unsigned char* psz) { return *this += reinterpret_cast<const char*>(psz); }
    CString& operator+=(const char* psz) {
        if (!psz) return *this;
        int newLen = GetLength() + strlen(psz);
        char* newData = new char[newLen + 1];
        strcpy(newData, m_pchData);
        strcat(newData, psz);
        Free();
        m_pchData = newData;
        return *this;
    }

    // Friend concatenation operators
    friend CString operator+(const CString& lhs, const CString& rhs) { CString s(lhs); s += rhs; return s; }
    friend CString operator+(const CString& lhs, const char* rhs) { CString s(lhs); s += rhs; return s; }
    friend CString operator+(const char* lhs, const CString& rhs) { CString s(lhs); s += rhs; return s; }
    friend CString operator+(const CString& lhs, char rhs) { CString s(lhs); s += rhs; return s; }
    friend CString operator+(char lhs, const CString& rhs) { CString s(lhs); s += rhs; return s; }

    // Comparison
    int Compare(const char* s) const { return strcmp(m_pchData, s ? s : ""); }
    int CompareNoCase(const char* s) const {
        return strcasecmp(m_pchData, s ? s : "");
    }
    
    bool operator==(const char* s) const { return Compare(s) == 0; }
    bool operator!=(const char* s) const { return Compare(s) != 0; }
    bool operator<(const char* s) const { return Compare(s) < 0; }
    bool operator==(const CString& s) const { return Compare(s.m_pchData) == 0; }
    bool operator!=(const CString& s) const { return Compare(s.m_pchData) != 0; }
    bool operator<(const CString& s) const { return Compare(s.m_pchData) < 0; }

    // Substrings
    CString Mid(int first, int count) const {
        int len = GetLength();
        if (first < 0) first = 0;
        if (first >= len) return CString();
        if (count < 0 || first + count > len) count = len - first;
        std::string s(m_pchData + first, count);
        return CString(s.c_str());
    }
    CString Mid(int first) const { return Mid(first, GetLength() - first); }
    CString Left(int count) const {
        // MFC clamps nCount to [0,len]; the Mid() fallback treated a
        // negative count as "rest of string", returning the whole thing.
        int len = GetLength();
        if (count < 0) count = 0;
        if (count > len) count = len;
        return Mid(0, count);
    }
    CString Right(int count) const {
        int len = GetLength();
        if (count > len) count = len;
        return Mid(len - count, count);
    }

    // Case conversion
    void MakeUpper() { for (char* p = m_pchData; *p; ++p) *p = toupper((unsigned char)*p); }
    void MakeLower() { for (char* p = m_pchData; *p; ++p) *p = tolower((unsigned char)*p); }
    void MakeReverse() { std::reverse(m_pchData, m_pchData + GetLength()); }

    // Trimming
    void TrimLeft() {
        const char* p = m_pchData;
        while (*p && isspace((unsigned char)*p)) ++p;
        if (p != m_pchData) {
            int newLen = strlen(p);
            char* newData = new char[newLen + 1];
            strcpy(newData, p);
            Free();
            m_pchData = newData;
        }
    }
    void TrimRight() {
        int len = GetLength();
        while (len > 0 && isspace((unsigned char)m_pchData[len - 1]))
            m_pchData[--len] = '\0';
    }
    void Trim() { TrimLeft(); TrimRight(); }

    // Replace / Remove
    int Replace(char oldCh, char newCh) {
        int count = 0;
        for (char* p = m_pchData; *p; ++p) if (*p == oldCh) { *p = newCh; ++count; }
        return count;
    }
    int Replace(const char* oldStr, const char* newStr) {
        std::string s = m_pchData;
        int count = 0;
        size_t pos = 0, oldLen = strlen(oldStr);
        size_t newLen = strlen(newStr);
        while ((pos = s.find(oldStr, pos)) != std::string::npos) {
            s.replace(pos, oldLen, newStr);
            pos += newLen;
            ++count;
        }
        *this = s.c_str();
        return count;
    }
    int Remove(char ch) {
        std::string s = m_pchData;
        auto it = std::remove(s.begin(), s.end(), ch);
        s.erase(it, s.end());
        int removed = GetLength() - s.length();
        *this = s.c_str();
        return removed;
    }

    // Find
    int Find(char ch, int start = 0) const {
        const char* p = strchr(m_pchData + start, ch);
        return (p) ? (int)(p - m_pchData) : -1;
    }
    int ReverseFind(char ch) const {
        const char* p = strrchr(m_pchData, ch);
        return (p) ? (int)(p - m_pchData) : -1;
    }
    int Find(const char* sub, int start = 0) const {
        const char* p = strstr(m_pchData + start, sub);
        return (p) ? (int)(p - m_pchData) : -1;
    }

    // Format (printf-style)
    void Format(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        FormatV(fmt, args);
        va_end(args);
    }

    void FormatV(const char* fmt, va_list args)
    {
        char buf[2048];
        vsnprintf(buf, sizeof(buf), fmt, args);
        *this = buf;
    }

    // Buffer access (stubbed)
    char* GetBuffer(int nMinBufLength = -1) {
        if (nMinBufLength < 0) nMinBufLength = GetLength();
        if (nMinBufLength > GetLength()) {
            char* newData = new char[nMinBufLength + 1];
            strcpy(newData, m_pchData);
            Free();
            m_pchData = newData;
        }
        return m_pchData;
    }
    void ReleaseBuffer(int nNewLength = -1) {
        if (nNewLength == -1) nNewLength = strlen(m_pchData);
        m_pchData[nNewLength] = '\0';
    }

    // --- Extended helpers from your original header ---

    // Split into parts by delimiter
    CString Part(char cDelimiter, int nIndex) const {
        std::string s = m_pchData;
        size_t start = 0, end;
        int idx = 0;
        while ((end = s.find(cDelimiter, start)) != std::string::npos) {
            if (idx == nIndex)
                return CString(s.substr(start, end - start).c_str());
            start = end + 1;
            ++idx;
        }
        if (idx == nIndex)
            return CString(s.substr(start).c_str());
        return CString(); // empty if not found
    }

    int PartCount(char cDelimiter) const {
        if (IsEmpty()) return 0;
        int count = 1;
        for (char* p = m_pchData; *p; ++p) if (*p == cDelimiter) count++;
        return count;
    }

    int PartBegin(char cDelimiter, int nIndex) const {
        std::string s = m_pchData;
        size_t start = 0, end;
        int idx = 0;
        while ((end = s.find(cDelimiter, start)) != std::string::npos) {
            if (idx == nIndex)
                return static_cast<int>(start);
            start = end + 1;
            ++idx;
        }
        if (idx == nIndex)
            return static_cast<int>(start);
        return -1;
    }

    // Escape/unescape (basic URL-style %xx encoding)
    void Escape() {
        std::ostringstream oss;
        for (char* p = m_pchData; *p; ++p) {
            unsigned char c = *p;
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                oss << c;
            else {
                char buf[4];
                snprintf(buf, sizeof(buf), "%%%02X", c);
                oss << buf;
            }
        }
        *this = oss.str().c_str();
    }

    void UnEscape() {
        std::string out;
        std::string in = m_pchData;
        for (size_t i = 0; i < in.size(); ++i) {
            // Both chars after % must be hex. "%x" sscanf alone would
            // parse "%4G" as 4 (eating the G) and "%GG" left val
            // uninitialised — a garbage byte. Non-escapes pass through.
            if (in[i] == '%' && i + 2 < in.size()
                    && isxdigit((unsigned char)in[i + 1])
                    && isxdigit((unsigned char)in[i + 2])) {
                unsigned int val = 0;
                sscanf(in.substr(i+1, 2).c_str(), "%x", &val);
                out.push_back(static_cast<char>(val));
                i += 2;
            } else {
                out.push_back(in[i]);
            }
        }
        *this = out.c_str();
    }

    int LoadString(int resID);
};

// RERUN: Template wrapper for CSprintf to automatically unwrap CString objects.
// On Linux x64, passing a CString (class with destructor) to varargs (...) is passed
// via memory/hidden pointer, not as the raw char* pointer, causing printf %s to read garbage.
// This wrapper intercepts calls and converts CString arguments to const char*.

template<typename T> inline const T& _CSprintf_Arg(const T& t) { return t; }
inline const char* _CSprintf_Arg(const CString& s) { return s.c_str(); }

template<typename... Args>
inline CString CSprintf(const char* fmt, const Args&... args) {
    CString s;
    s.Format(fmt, _CSprintf_Arg(args)...);
    return s;
}

#if 0 // RERUN now defined in original location RDIALOG.H
inline	CString	LoadResString(int resnum)
{
	CString s;
	s.LoadString(resnum);
	return s;
};
#endif
#define	RESSTRING(name)	LoadResString(IDS_##name)