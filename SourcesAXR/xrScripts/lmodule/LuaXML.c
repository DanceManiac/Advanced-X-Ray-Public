/** 
LuaXML License

LuaXml is licensed under the terms of the MIT license reproduced below,
the same as Lua itself. This means that LuaXml is free software and can be
used for both academic and commercial purposes at absolutely no cost.

Copyright (C) 2007-2010 Gerald Franz, www.viremo.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "cstdafx.h"
#include "../luajit/lua.h"
#include "../luajit/lualib.h"
#include "../luajit/lauxlib.h"


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static const char ESC=27;
static const char OPN=28;
static const char CLS=29;

//--- auxliary functions -------------------------------------------

static const char* char2code(unsigned char ch, char buf[8]) {
	unsigned char i=0;
	buf[i++]='&'; 
	buf[i++]='#';
	if(ch>99) buf[i++]=ch/100+48;
	if(ch>9) buf[i++]=(ch%100)/10+48;
	buf[i++]=ch%10+48;
	buf[i++]=';';
	buf[i]=0;
	return buf;
}

static size_t find(const char* s, const char* pattern, size_t start) {
	const char* found =strstr(s+start, pattern);
	return found ? found-s : strlen(s);
}

//--- internal tokenizer -------------------------------------------

typedef struct Tokenizer_s  {
	/// stores string to be tokenized
	const char* s;
	/// stores size of string to be tokenized
	size_t s_size;
	/// stores current read position
	size_t i;
	/// stores current read context
	int tagMode;
	/// stores next token, if already determined
	const char* m_next;
	/// size of next token
	size_t m_next_size;
	/// pointer to current token
	char* m_token;
	/// size of current token
	size_t m_token_size;
	/// capacity of current token
	size_t m_token_capacity;
} Tokenizer;

Tokenizer* Tokenizer_new(const char* str, size_t str_size) {
	Tokenizer *tok = (Tokenizer*)xr_malloc_C(sizeof(Tokenizer));
	memset(tok, 0, sizeof(Tokenizer));
	tok->s_size = str_size;
	tok->s = str;
	return tok;
}

void Tokenizer_xr_delete(Tokenizer* tok) {
	xr_free_C(tok->m_token);
	xr_free_C(tok);
}

//void Tokenizer_print(Tokenizer* tok) { printf("  @%u %s\n", tok->i, !tok->m_token ? "(null)" : (tok->m_token[0]==ESC)?"(esc)" : (tok->m_token[0]==OPN)?"(open)": (tok->m_token[0]==CLS)?"(close)" : tok->m_token); fflush(stdout); }

static const char* Tokenizer_set(Tokenizer* tok, const char* s, size_t size) {
	if(!size||!s) return 0;
	xr_free_C(tok->m_token);
	tok->m_token = (char*)xr_malloc_C(size+1);
	strncpy(tok->m_token,s, size);
	tok->m_token[size] = 0;
	tok->m_token_size = tok->m_token_capacity = size;
	//Tokenizer_print(tok);
	return tok->m_token;
}

static void Tokenizer_append(Tokenizer* tok, char ch) {
	if(tok->m_token_size+1>=tok->m_token_capacity) {
		tok->m_token_capacity = (tok->m_token_capacity==0) ? 16 : tok->m_token_capacity*2;
		tok->m_token = (char*)xr_realloc_C(tok->m_token, tok->m_token_capacity);
	}
	tok->m_token[tok->m_token_size]=ch;
	tok->m_token[++tok->m_token_size]=0;
}

const char* Tokenizer_next(Tokenizer* tok) {
	const char* ESC_str = "\033";
	const char* OPEN_str = "\034";
	const char* CLOSE_str = "\035";

	
	if(tok->m_token) {
		xr_free_C(tok->m_token);
		tok->m_token = 0;
		tok->m_token_size=tok->m_token_capacity = 0;
	}
	
	int quotMode=0;
	int tokenComplete = 0;
	while(tok->m_next_size || (tok->i < tok->s_size)) {

		if(tok->m_next_size) {
			Tokenizer_set(tok, tok->m_next, tok->m_next_size);
			tok->m_next=0;
			tok->m_next_size=0;
			return tok->m_token;
		}

		switch(tok->s[tok->i]) {
		    case '"':
		    case '\'':
			if(tok->tagMode) {
				if(!quotMode) quotMode=tok->s[tok->i];
				else if(quotMode==tok->s[tok->i]) quotMode=0;
			}
			Tokenizer_append(tok, tok->s[tok->i]);
			break;
		    case '<':
			if(!quotMode&&(tok->i+4<tok->s_size)&&(strncmp(tok->s+tok->i,"<!--",4)==0)) // strip comments
			    tok->i=find(tok->s, "-->", tok->i+4)+2;
			else if(!quotMode&&(tok->i+9<tok->s_size)&&(strncmp(tok->s+tok->i,"<![CDATA[",9)==0)) { // interpet CDATA
			    size_t b=tok->i+9;
			    tok->i=find(tok->s, "]]>",b)+3;
			    if(!tok->m_token_size) return Tokenizer_set(tok, tok->s+b, tok->i-b-3);
			    tokenComplete = 1;
			    tok->m_next = tok->s+b;
			    tok->m_next_size = tok->i-b-3;
			    --tok->i;
			}
			else if(!quotMode&&(tok->i+1<tok->s_size)&&((tok->s[tok->i+1]=='?')||(tok->s[tok->i+1]=='!'))) // strip meta information
			    tok->i=find(tok->s, ">", tok->i+2);
			else if(!quotMode&&!tok->tagMode) {
				if((tok->i+1<tok->s_size)&&(tok->s[tok->i+1]=='/')) {
					tok->m_next=ESC_str;
					tok->m_next_size = 1;
					tok->i=find(tok->s, ">", tok->i+2);
				}
				else {
					tok->m_next = OPEN_str;
					tok->m_next_size = 1;
					tok->tagMode=1; 
				}
				tokenComplete = 1;
			}
			else Tokenizer_append(tok, tok->s[tok->i]);
			break;
		    case '/': 
			if(tok->tagMode&&!quotMode) {
				tokenComplete = 1;
				if((tok->i+1 < tok->s_size) && (tok->s[tok->i+1]=='>')) {
					tok->tagMode=0;
					tok->m_next=ESC_str;
					tok->m_next_size = 1;
					++tok->i;
				}
				else Tokenizer_append(tok, tok->s[tok->i]);
			}                    
			else Tokenizer_append(tok, tok->s[tok->i]);
			break;
		    case '>': 
			if(!quotMode&&tok->tagMode) {
				tok->tagMode=0;
				tokenComplete = 1;
				tok->m_next = CLOSE_str;
				tok->m_next_size = 1;
			}                    
			else Tokenizer_append(tok, tok->s[tok->i]);
			break;
		    case ' ':
		    case '\r':
		    case '\n':
		    case '\t':
			if(tok->tagMode&&!quotMode) {
			    if(tok->m_token_size) tokenComplete=1;
			}
			else if(tok->m_token_size) Tokenizer_append(tok, tok->s[tok->i]);
			break;
		    default: Tokenizer_append(tok, tok->s[tok->i]);
		}
		++tok->i;
		if((tok->i>=tok->s_size)||(tokenComplete&&tok->m_token_size)) {
			tokenComplete=0;
			while(tok->m_token_size&&isspace(tok->m_token[tok->m_token_size-1])) // trim whitespace
				tok->m_token[--tok->m_token_size]=0;
			if(tok->m_token_size) break;
		}
	}
	//Tokenizer_print(tok);
	return tok->m_token;
}

//--- local variables ----------------------------------------------

/// stores number of special character codes
static size_t sv_code_size=0;
/// stores currently allocated capacity for special character codes
static size_t sv_code_capacity=16;
/// stores code table for special characters
static char** sv_code=0; 

//--- public methods -----------------------------------------------

static void Xml_pushDecode(lua_State* L, const char* s, size_t s_size) {
	size_t start=0, pos;
	if(!s_size) s_size=strlen(s);
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	const char* found = strstr(s, "&#");
	if(!found) pos = s_size;
	else pos = found-s;
	while(found&&(pos+5<s_size)&&(*(found+5)==';')&&isdigit(*(found+2))&&isdigit(*(found+3))&&isdigit(*(found+4)) ) {
		if(pos>start) luaL_addlstring(&b,s+start, pos-start);
		luaL_addchar(&b, 100*(s[pos+2]-48)+10*(s[pos+3]-48)+(s[pos+4]-48));
		start=pos+6;
		found = strstr(found+6, "&#");
		if(!found) pos = s_size;
		else pos = found-s;
	}
	if(pos>start) luaL_addlstring(&b,s+start, pos-start);
	luaL_pushresult(&b);
	size_t i;
	for(i=sv_code_size-1; i<sv_code_size; i-=2) {
		luaL_gsub(L, lua_tostring(L,-1), sv_code[i], sv_code[i-1]);
		lua_remove(L,-2);
	}
}

int Xml_eval(lua_State *L) {
	char* str = 0;
	size_t str_size=0;
	if(lua_isuserdata(L,1)) str = (char*)lua_touserdata(L,1);
	else {
		const char * sTmp = luaL_checklstring(L,1,&str_size);
		str = (char*)xr_malloc_C(str_size+1);
		memcpy(str, sTmp, str_size);
		str[str_size]=0;
	}
	Tokenizer* tok = Tokenizer_new(str, str_size ? str_size : strlen(str));
	lua_settop(L,0);
	const char* token=0;
	int firstStatement = 1;
	while((token=Tokenizer_next(tok))!=0) if(token[0]==OPN) { // new tag found
		if(lua_gettop(L)) {
			int newIndex=(int)lua_objlen(L,-1)+1;
			lua_pushnumber(L,newIndex);
			lua_newtable(L);
			lua_settable(L, -3);
			lua_pushnumber(L,newIndex);
			lua_gettable(L,-2);
		}
		else {
			if (firstStatement) {
				lua_newtable(L); 
				firstStatement = 0;
			}
			else return lua_gettop(L);
		}
		// set metatable:    
		lua_newtable(L);
		lua_pushliteral(L, "__index");
		lua_getglobal(L, "xml");
		lua_settable(L, -3);
			
		lua_pushliteral(L, "__tostring"); // set __tostring metamethod
		lua_getglobal(L, "xml");
		lua_pushliteral(L,"str");
		lua_gettable(L, -2);
		lua_remove(L, -2);
		lua_settable(L, -3);			
		lua_setmetatable(L, -2);
		
		// parse tag and content:
		lua_pushnumber(L,0); // use index 0 for storing the tag
		lua_pushstring(L, Tokenizer_next(tok));
		lua_settable(L, -3);
		
		while(((token = Tokenizer_next(tok))!=0)&&(token[0]!=CLS)&&(token[0]!=ESC)) { // parse tag header
			size_t sepPos=find(token, "=", 0);
			if(token[sepPos]) { // regular attribute
				const char* aVal =token+sepPos+2;
				lua_pushlstring(L, token, sepPos);
				size_t lenVal = strlen(aVal)-1;
				if(!lenVal) Xml_pushDecode(L, "", 0);
				else Xml_pushDecode(L, aVal, lenVal);
				lua_settable(L, -3);
			}
		}            
		if(!token||(token[0]==ESC)) {
			if(lua_gettop(L)>1) lua_settop(L,-2); // this tag has no content, only attributes
			else break;
		}
	}
	else if(token[0]==ESC) { // previous tag is over
		if(lua_gettop(L)>1) lua_settop(L,-2); // pop current table
		else break;
	}
	else { // read elements
		lua_pushnumber(L, (lua_Number)lua_objlen(L,-1)+1);
		Xml_pushDecode(L, token, 0);
		lua_settable(L, -3);
	}
	Tokenizer_xr_delete(tok);
	xr_free_C(str);
	return lua_gettop(L);
}

int Xml_load (lua_State *L) {
	const char * filename = luaL_checkstring(L,1);
	FILE * file=fopen(filename,"r");
	if(!file) return luaL_error(L,"LuaXml ERROR: \"%s\" file error or file not found!",filename);

	fseek (file , 0 , SEEK_END);
	size_t sz = ftell (file);
	rewind (file);
	char* buffer = (char*)xr_malloc_C(sz+1);
	sz = fread (buffer,1,sz,file);
	fclose(file);
	buffer[sz]=0;
	lua_pushlightuserdata(L,buffer);
	lua_replace(L,1);
	return Xml_eval(L);
};

int Xml_registerCode(lua_State *L) {
    const char * decoded = luaL_checkstring(L,1);
    const char * encoded = luaL_checkstring(L,2);
    
    size_t i;
    for(i=0; i<sv_code_size; i+=2)
        if(strcmp(sv_code[i],decoded)==0)
            return luaL_error(L,"LuaXml ERROR: code already exists.");
	if(sv_code_size+2>sv_code_capacity) {
		sv_code_capacity*=2;
		sv_code = (char**)xr_realloc_C(sv_code, sv_code_capacity*sizeof(char*));
	}
	sv_code[sv_code_size]=(char*)xr_malloc_C(strlen(decoded)+1);
	strcpy(sv_code[sv_code_size++], decoded);
	sv_code[sv_code_size]=(char*)xr_malloc_C(strlen(encoded)+1);
	strcpy(sv_code[sv_code_size++],encoded);
    return 0;
}

int Xml_encode(lua_State *L) {
	if(lua_gettop(L)!=1) return 0;
	luaL_checkstring(L,-1);
	size_t i;
	for(i=0; i<sv_code_size; i+=2) {
		luaL_gsub(L, lua_tostring(L,-1), sv_code[i], sv_code[i+1]);
		lua_remove(L,-2);
	}
	char buf[8];
	const char* s=lua_tostring(L,1);
	size_t start, pos;
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	for(start=pos=0; s[pos]!=0; ++pos) if(s[pos]<0) {
		if(pos>start) luaL_addlstring(&b,s+start, pos-start);
		luaL_addstring(&b,char2code((unsigned char)(s[pos]),buf));
		start=pos+1;
	}
	if(pos>start) luaL_addlstring(&b,s+start, pos-start);
	luaL_pushresult(&b);
	lua_remove(L,-2);
    return 1;
}

static const struct luaL_Reg funcs_xml[] = {
	{ "load", Xml_load },
	{ "eval", Xml_eval },
	{ "encode", Xml_encode },
	{ "registerCode", Xml_registerCode },
	{ NULL, NULL }
};

int luaopen_LuaXML_lib (lua_State* L) 
{
	luaL_register(L, "xml", funcs_xml);
	// register default codes:
	if(!sv_code)
	{
		sv_code=(char**)xr_malloc_C(sv_code_capacity*sizeof(char*));
		sv_code[sv_code_size++]="&";
		sv_code[sv_code_size++]="&amp;";
		sv_code[sv_code_size++]="<";
		sv_code[sv_code_size++]="&lt;";
		sv_code[sv_code_size++]=">";
		sv_code[sv_code_size++]="&gt;";
		sv_code[sv_code_size++]="\"";
		sv_code[sv_code_size++]="&quot;";
		sv_code[sv_code_size++]="'";
		sv_code[sv_code_size++]="&apos;";
	}
	return 1;
}
