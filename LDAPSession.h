//
//
// C++ Interface: $MODULE$
//
// Description: 
//
//
// Author: Oleksandr Shneyder AKA nCryer <oleksandr.shneyder@treuchtlingen.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LDAPSESSION_H
#define LDAPSESSION_H
#define LDAP_DEPRECATED 1
#include "x2goclientconfig.h"

#ifdef USELDAP
#include <ldap.h>
#include <string>
#include <list>
using namespace std;

struct LDAPExeption
{
   LDAPExeption(string type,string str){err_type=type;err_str=str;}
   string err_type;
   string err_str;
};

class ByteArray
{
public:
     ByteArray();
     ByteArray(const ByteArray&);
     ~ByteArray();
     const char* getData(){return data;}
     string asString(){return data;}
     int length(){return size;}
     void load(const char*,int);     
     void fromStdStr(const string&);
     void operator =(const ByteArray&);
private:
     void _delete();
     char* data;
     int size;
};


struct LDAPBinValue
{
    string attr;
    list<ByteArray> value;
};

struct LDAPStringValue
{
    string attr;
    list<string> value;
};

typedef  list<LDAPStringValue> LDAPStringEntry;
typedef  list<LDAPBinValue> LDAPBinEntry;
#endif

class LDAPSession
{
#ifdef USELDAP
public:
    LDAPSession(string,int,string,string, bool simple=false, bool start_tls=true);
    ~LDAPSession();
    void addStringValue(string dn,const list<LDAPStringValue>& values);
    void remove(string);
    static list<string> getStringAttrValues(const LDAPStringEntry& entry,string attr);
    static list<ByteArray> getBinAttrValues(const LDAPBinEntry& entry,string attr);
    void modifyStringValue(string dn,const list<LDAPStringValue>& values);
    void stringSearch(string dn,const list<string> &attributes,string searchParam,list<LDAPStringEntry> &result);
    void binSearch(string dn,const list<string> &attributes,string searchParam,list<LDAPBinEntry> &result);
    
private:
    LDAP* ld;
#endif
};

#endif
