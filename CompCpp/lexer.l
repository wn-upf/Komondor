/*  -*- c-basic-offset:4; tab-width:4; indent-tabs-mode:t -*- */
%x incl
%option noyywrap

%{

#define YY_NO_UNPUT

#include "token.h"

#define YY_DECL CToken* yylex()

#define DEFAULT_ACTION 	if(CToken::Current()->type!=UNKNOWN) \
							{ CToken::New(CTranslation::Current()); CToken::Current()->Push(yytext); return CToken::Current(); } \
						else CToken::Current()->Push(yytext);

%}

intsuffix                               ([uU][lL]?)|([lL][uU]?)
fracconst                               ([0-9]*\.[0-9]+)|([0-9]+\.)
exppart                                 [eE][-+]?[0-9]+
floatsuffix                             [fFlL]
chartext                                ([^'])|(\\.)
stringtext                              ([^"])|(\\.)

%%

#include[\t\f\v\r ]*\"             { BEGIN(incl); }

<incl>[^ \t\n\r\"]+\"     { 
                            string filename = yytext;
                            filename.erase(filename.size()-1,1);
                            string pathname=CTranslation::Current()->filename;
                            string::size_type pos = pathname.rfind ('/');
                            if(pos==string::npos)
                            	pathname="";
                            else
                            	pathname.erase(pos+1);
                            filename = pathname + filename;
                            CTranslation::Current()->include_stack = YY_CURRENT_BUFFER;
                            CTranslation::New();
                            CTranslation::Current()->filename = filename;

                            if(g_debug)printf("include file '%s'\n",filename.c_str());
							yyin = fopen( filename.c_str(), "r" );
							if ( ! yyin )
                            {
                                CTranslation::Error(NULL, 1, "cannot open include file '%s'\n", filename.c_str());
								return NULL;
                            }
                            else
                            {
							    yy_switch_to_buffer(yy_create_buffer( yyin, YY_BUF_SIZE ) );
							    BEGIN(INITIAL);
                                CTranslation::Current()->lineno = 1;
                                CToken* t = CToken::New(CTranslation::Current());
                                t -> line_info = true;
                                return t;
                            }
                        }

<incl>"\n"              { BEGIN(INITIAL); CToken::Current()->Push("\n"); }

<<EOF>>                 {
	                        if ( !CTranslation::Delete() )
							{
								yyterminate();
								return NULL;
							}
							else
							{
								yy_delete_buffer( YY_CURRENT_BUFFER );
								yy_switch_to_buffer( (YY_BUFFER_STATE)CTranslation::Current()->include_stack);
                                CToken* t = CToken::New(CTranslation::Current());
                                t -> line_info = true;
                                return t;
							}
                        }

"#"					    {   
                            CToken* t = CToken::New(CTranslation::Current());
                            int c1, c2;
	                        c1 = 0;
	                        c2 = yyinput();
							t->Push('#');
                            while (c2 != '\n' || c1 == '\\' )
                            {
							    t->Push(c2);
								if (c2 == '\n')CTranslation::Current()->lineno++; 
                         		c1 = c2;
                         		c2 = yyinput();
							}
							if (c2 == '\n')CTranslation::Current()->lineno++; 
							t->Push(c2);
							return t;
                        }

"\n"					{ 
                            CToken::Current()->Push("\n"); 
                            CTranslation::Current()->lineno++; 
                        }
"\r"					{}
[\t\f\v ]+				{ 
                            CToken::Current()->Push(yytext);
                        }

"#cxxdef"				{
							string text = "#define";
							string id,id1,id2;
							int state=0;
							int c;
							c = yyinput();
							while(c!='\n')
							{
								text += c;
								switch(state)
								{
								case 0:
								case 2:
									if(c=='_'||isalpha(c))
									{
										state++;
										id+=c;
									}
									else if(!isspace(c))
									{
										state=5;
									}
									break;
								case 1:
								case 3:
									if(c=='_'||isalnum(c))
									{
										id+=c;
									}
									else if (isspace(c))
									{
										if(state==1)
											id1=id;
										else
											id2=id;
										state++;
										id="";
									}
									else
									{
										state=5;
									}
									break;
								case 4:
									if(!isspace(c))
										state=5;
									break;
								default:
									break;
								}
								c = yyinput();
							}
							if(state==3)
							{
								id2=id;
								state++;
							}
							text += c;
                            CTranslation::Current()->lineno++; 
							if(state==4)
							{
                            	std::map<string, string>::iterator iter=g_macros.find(id1);
                            	if(iter==g_macros.end())
                            	    g_macros[id1]=id2;
							}
							else
							{
								if(CToken::Current()->type!=UNKNOWN) 
								{
									CToken::New(CTranslation::Current()); 
									CToken::Current()->Push(text.c_str());
									return CToken::Current(); 
								}
								else 
									CToken::Current()->Push(yytext);
							}
						}

"/*"					{   
                            int c1, c2;
	                        c1 = yyinput();
	                        c2 = yyinput();
                            while (c2 != EOF && !(c1 == '*' && c2 == '/'))
                            {
                                if (c1 == '\n')
                                {
                                    CToken::Current()->Push("\n"); 
                                    CTranslation::Current()->lineno++; 
                                }   
                         		c1 = c2;
                         		c2 = yyinput();
							}
	                    }


"//"					{   
                            int c;
                            while ((c = yyinput()) != EOF && c != '\n');
                            CToken::Current()->Push("\n"); 
                            CTranslation::Current()->lineno++; 
                        }

"{"					    {  CToken* t = CToken::New( CTranslation::Current(), '{' ); t->Push("{"); return t; }
"}"					    {  CToken* t = CToken::New( CTranslation::Current(), '}' ); t->Push("}"); return t; }
"("					    {  CToken* t = CToken::New( CTranslation::Current(), '(' ); t->Push("("); return t; }
")"					    {  CToken* t = CToken::New( CTranslation::Current(), ')' ); t->Push(")"); return t; }
"["					    {  CToken* t = CToken::New( CTranslation::Current(), '[' ); t->Push("["); return t; }
"]"					    {  CToken* t = CToken::New( CTranslation::Current(), ']' ); t->Push("]"); return t; }
"<"					    {  CToken* t = CToken::New( CTranslation::Current(), '<' ); t->Push("<"); return t; }
">"					    {  CToken* t = CToken::New( CTranslation::Current(), '>' ); t->Push(">"); return t; }
";"					    {  CToken* t = CToken::New( CTranslation::Current(), ';' ); t->Push(";"); return t; }
"."					    {  CToken* t = CToken::New( CTranslation::Current(), '.' ); t->Push("."); return t; }
"~"                     {  CToken* t = CToken::New( CTranslation::Current(), '~' ); t->Push("~"); return t; }
","                     {  CToken* t = CToken::New( CTranslation::Current(), ',' ); t->Push(","); return t; }

":"					    {  CToken* t = CToken::New( CTranslation::Current(), ':' );     t->Push(":");       return t; }
"[]"					{  CToken* t = CToken::New( CTranslation::Current(), ARRAY );     t->Push("[]");       return t; }
"::"					{  CToken* t = CToken::New( CTranslation::Current(), SCOPE );     t->Push("::");       return t; }
".*"					{  CToken* t = CToken::New( CTranslation::Current(), DOTSTAR );   t->Push(".*");       return t; }
"->"					{  CToken* t = CToken::New( CTranslation::Current(), ARROW );     t->Push("->");       return t; }
"->*"					{  CToken* t = CToken::New( CTranslation::Current(), ARROWSTAR ); t->Push("->*");      return t; }

"outport"	 	        {  CToken* t = CToken::New( CTranslation::Current(), OUTPORT );   t->Push("outport");         return t; }
"inport"				{  CToken* t = CToken::New( CTranslation::Current(), INPORT );    t->Push("inport");         return t; }
"component"				{  CToken* t = CToken::New( CTranslation::Current(), COMPONENT ); t->Push("class");    return t; }
"class"				    {  CToken* t = CToken::New( CTranslation::Current(), CLASS ); t->Push("class");    return t; }
"typename"			    {  CToken* t = CToken::New( CTranslation::Current(), TYPENAME ); t->Push("typename");    return t; }
"enum"				    {  CToken* t = CToken::New( CTranslation::Current(), CLASS ); t->Push("enum");    return t; }
"struct"				{  CToken* t = CToken::New( CTranslation::Current(), CLASS ); t->Push("struct");    return t; }
"connect"				{  CToken* t = CToken::New( CTranslation::Current(), CONNECT );   t->Push("connect");  return t; }
"template"				{  CToken* t = CToken::New( CTranslation::Current(), TEMPLATE );  t->Push("template"); return t; }
"virtual"				{  CToken* t = CToken::New( CTranslation::Current(), FSPECIFIER );  t->Push("virtual"); return t; }
"inline"				{  CToken* t = CToken::New( CTranslation::Current(), FSPECIFIER );  t->Push("inline"); return t; }
"explicit"				{  CToken* t = CToken::New( CTranslation::Current(), FSPECIFIER );  t->Push("explicit"); return t; }

[a-zA-Z_][a-zA-Z_0-9]*	{
                            CToken* t = CToken::New( CTranslation::Current(), VARID );
                           	std::map<string, string>::iterator iter;
							iter=g_macros.find(yytext);
							if(iter!=g_macros.end())
								t->Push( (*iter).second.c_str() );
							else
								t->Push(yytext);
							return t;
                        }

"0"[xX][0-9a-fA-F]+{intsuffix}?         { DEFAULT_ACTION; }
"0"[0-7]+{intsuffix}?                   { DEFAULT_ACTION; }
[0-9]+{intsuffix}?                      { DEFAULT_ACTION; }

{fracconst}{exppart}?{floatsuffix}?     { DEFAULT_ACTION; }
[0-9]+{exppart}{floatsuffix}?           { DEFAULT_ACTION; }

"'"{chartext}*"'"                       { DEFAULT_ACTION; }
"L'"{chartext}*"'"                      { DEFAULT_ACTION; }

"\""{stringtext}*"\""                   { DEFAULT_ACTION; }
"L\""{stringtext}*"\""                  { DEFAULT_ACTION; }

.			                            { DEFAULT_ACTION; }

%%

