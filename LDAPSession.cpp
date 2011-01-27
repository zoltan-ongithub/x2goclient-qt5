#include "LDAPSession.h"
#include "x2goclientconfig.h"
#ifdef USELDAP
#include <stdlib.h>


ByteArray::ByteArray()
{
   size=0;
   data=0;
}

ByteArray::ByteArray(const ByteArray& src)
{
   size=0;
   data=0;
   *this=src;
}

ByteArray::~ByteArray()
{
  _delete();
}

void ByteArray::load(const char* buf,int len)
{
   _delete();
   if(len>0)
   {
      size=len;
      data=new char[size+1];
      if(!data)
        throw string("Not enouth memory");
      memcpy(data,buf,len);
      data[size]=0;
   }
}

void ByteArray::fromStdStr(const string& src)
{
    load(src.c_str(),src.size());
}

void ByteArray::operator =(const ByteArray& src)
{
    load(src.data,src.size);
}

void ByteArray::_delete()
{
   if(data)
   {
      delete []data;    
      size=0;     
   }
}

/////////////////////////////////////


LDAPSession::LDAPSession(string server,int port,string bindDN, string pass, bool simple, bool start_tls)
{
     ld=ldap_init(server.c_str(),port);
     if(!ld)
        throw LDAPExeption("ldap_init","Can't init LDAP library");
     int ver=3;
     int errc=ldap_set_option(ld,LDAP_OPT_PROTOCOL_VERSION,&ver);     
     if(errc != LDAP_SUCCESS)
        throw LDAPExeption("ldap_set_option",ldap_err2string(errc));
     if(start_tls)
     {
        errc=ldap_start_tls_s(ld,NULL,NULL);
        if(errc != LDAP_SUCCESS)
            throw LDAPExeption("ldap_start_tls_s",ldap_err2string(errc));
     }
     if(!simple)
     {
         errc=ldap_bind_s(ld,bindDN.c_str(),pass.c_str(),LDAP_AUTH_SIMPLE);
         if(errc != LDAP_SUCCESS)
             throw LDAPExeption("ldap_bind_s",ldap_err2string(errc));
     }
     else
     {
         errc=ldap_simple_bind_s(ld,bindDN.c_str(),pass.c_str());
         if(errc != LDAP_SUCCESS)
             throw LDAPExeption("ldap_simple_bind_s",ldap_err2string(errc));
     }
}

void LDAPSession::addStringValue(string dn,const list<LDAPStringValue>& values)
{
   list<LDAPStringValue>::const_iterator it=values.begin();
   list<LDAPStringValue>::const_iterator end=values.end();
   int i=0;
   LDAPMod** mods=(LDAPMod**)malloc(sizeof(LDAPMod*)*values.size()+1);

   for(;it!=end;++it)
   {
       mods[i]=(LDAPMod*)malloc(sizeof(LDAPMod));
       mods[i]->mod_op=LDAP_MOD_ADD;
       mods[i]->mod_type=(char*)malloc(sizeof(char)*(*it).attr.length());
       strcpy(mods[i]->mod_type,(*it).attr.c_str());
       
       list<string>::const_iterator sit=(*it).value.begin();
       list<string>::const_iterator send=(*it).value.end();
       int j=0;
       mods[i]->mod_vals.modv_strvals=(char**)malloc(sizeof(char*)*(*it).value.size()+1);
       for(;sit!=send;++sit)
       {
           mods[i]->mod_vals.modv_strvals[j]=(char*)malloc(sizeof(char)*(*sit).length());
           strcpy(mods[i]->mod_vals.modv_strvals[j],(*sit).c_str());
           ++j;       
       }  
       mods[i]->mod_vals.modv_strvals[j]=0l;
       ++i;       
   }
   mods[i]=0l;

   int errc= ldap_add_s(ld,dn.c_str(),mods);
   if(errc != LDAP_SUCCESS)
        throw LDAPExeption("ldap_add_s",ldap_err2string(errc));
	
   ldap_mods_free(mods,1);
}


void LDAPSession::modifyStringValue(string dn,const list<LDAPStringValue>& values)
{
   list<LDAPStringValue>::const_iterator it=values.begin();
   list<LDAPStringValue>::const_iterator end=values.end();
   int i=0;
   LDAPMod** mods=(LDAPMod**)malloc(sizeof(LDAPMod*)*values.size()+1);

   for(;it!=end;++it)
   {
       mods[i]=(LDAPMod*)malloc(sizeof(LDAPMod));
       mods[i]->mod_op=LDAP_MOD_REPLACE;
       mods[i]->mod_type=(char*)malloc(sizeof(char)*(*it).attr.length());
       strcpy(mods[i]->mod_type,(*it).attr.c_str());
       
       list<string>::const_iterator sit=(*it).value.begin();
       list<string>::const_iterator send=(*it).value.end();
       int j=0;
       mods[i]->mod_vals.modv_strvals=(char**)malloc(sizeof(char*)*(*it).value.size()+1);
       for(;sit!=send;++sit)
       {
           mods[i]->mod_vals.modv_strvals[j]=(char*)malloc(sizeof(char)*(*sit).length());
           strcpy(mods[i]->mod_vals.modv_strvals[j],(*sit).c_str());
           ++j;       
       }  
       mods[i]->mod_vals.modv_strvals[j]=0l;
       ++i;       
   }
   mods[i]=0l;

   int errc= ldap_modify_s(ld,dn.c_str(),mods);
   if(errc != LDAP_SUCCESS)
        throw LDAPExeption("ldap_modify_s",ldap_err2string(errc));
	
   ldap_mods_free(mods,1);
}

void LDAPSession::remove(string dn)
{
   int errc= ldap_delete_s(ld,dn.c_str());
   if(errc != LDAP_SUCCESS)
        throw LDAPExeption("ldap_delete_s",ldap_err2string(errc));
}

void LDAPSession::binSearch(string dn,const list<string> &attributes,string searchParam,list<LDAPBinEntry>& result)
{
   char** attr;
   attr=(char**)malloc(sizeof(char*)*attributes.size()+1);
   int i=0;
   list<string>::const_iterator it=attributes.begin();
   list<string>::const_iterator end=attributes.end();
   for(;it!=end;++it)
   {
       attr[i]=(char*)malloc(sizeof(char)*(*it).length());
       strcpy(attr[i],(*it).c_str());
       ++i;
   }
   attr[i]=0l;
   LDAPMessage* res;
   int errc=ldap_search_s(ld,dn.c_str(),LDAP_SCOPE_SUBTREE,searchParam.c_str(),attr,0,&res);
   if(errc != LDAP_SUCCESS)
   {
	i=0;
	it=attributes.begin();
	for(;it!=end;++it)
	{
    	    free(attr[i]);
    	    ++i;
	}
	free(attr);
        throw LDAPExeption("ldap_search_s",ldap_err2string(errc));
   }
   LDAPMessage *entry=ldap_first_entry(ld,res);
   while(entry)
   {
        LDAPBinEntry binEntry;
        it=attributes.begin();
        for(;it!=end;++it)
        {
	    LDAPBinValue val;
	    val.attr=(*it);
            berval **atr=ldap_get_values_len(ld,entry,(*it).c_str());
	    int count=ldap_count_values_len(atr);
	    for(i=0;i<count;i++)
	    {
	       ByteArray arr;
	       arr.load(atr[i]->bv_val,atr[i]->bv_len);
	       val.value.push_back(arr);
	    }
            ldap_value_free_len(atr);
	    binEntry.push_back(val);
	}	
	entry=ldap_next_entry(ld,entry);
	result.push_back(binEntry);
   }				
   free(res);																	    
   i=0;
   it=attributes.begin();
   for(;it!=end;++it)
   {
       free(attr[i]);
       ++i;
   }
   free(attr);
}

void LDAPSession::stringSearch(string dn,const list<string> &attributes,string searchParam,list<LDAPStringEntry>& result)
{
   char** attr;
   attr=(char**)malloc(sizeof(char*)*attributes.size()+1);
   int i=0;
   list<string>::const_iterator it=attributes.begin();
   list<string>::const_iterator end=attributes.end();
   for(;it!=end;++it)
   {
       attr[i]=(char*)malloc(sizeof(char)*(*it).length()+1);
       strcpy(attr[i],(*it).c_str());
       ++i;
   }
   attr[i]=0l;
   LDAPMessage* res;
   int errc=ldap_search_s(ld,dn.c_str(),LDAP_SCOPE_SUBTREE,searchParam.c_str(),attr,0,&res);
   if(errc != LDAP_SUCCESS)
   {
	i=0;
	it=attributes.begin();
	for(;it!=end;++it)
	{
    	    free(attr[i]);
    	    ++i;
	}
	free(attr);
        throw LDAPExeption("ldap_search_s",ldap_err2string(errc));
   }
   LDAPMessage *entry=ldap_first_entry(ld,res);
   while(entry)
   {
        LDAPStringEntry stringEntry;
        it=attributes.begin();
        for(;it!=end;++it)
        {
	    LDAPStringValue val;
	    val.attr=(*it);
            char **atr=ldap_get_values(ld,entry,(*it).c_str());
	    int count=ldap_count_values(atr);
	    for(i=0;i<count;i++)
	    {
	       val.value.push_back(atr[i]);	         
	    }
            ldap_value_free(atr);
	    stringEntry.push_back(val);
	}	
	entry=ldap_next_entry(ld,entry);
	result.push_back(stringEntry);
   }				
   free(res);																	    
   i=0;
   it=attributes.begin();
   for(;it!=end;++it)
   {
       free(attr[i]);
       ++i;
   }
   free(attr);
}

list<string> LDAPSession::getStringAttrValues(const LDAPStringEntry& entry,string attr)
{
    list<LDAPStringValue>::const_iterator it=entry.begin();
    list<LDAPStringValue>::const_iterator end=entry.end();
    list<string> lst;
    for(;it!=end;++it)
    {
        if((*it).attr==attr)
	    return (*it).value;
    }
    return lst;
}

list<ByteArray> LDAPSession::getBinAttrValues(const LDAPBinEntry& entry,string attr)
{
    list<LDAPBinValue>::const_iterator it=entry.begin();
    list<LDAPBinValue>::const_iterator end=entry.end();
    list<ByteArray> lst;
    for(;it!=end;++it)
    {
        if((*it).attr==attr)
	    return (*it).value;
    }
    return lst;
}

LDAPSession::~LDAPSession()
{
  ldap_unbind(ld);
}
#endif
