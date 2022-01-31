#include <string>

using std::string;

#include "token.h"

extern CToken* yylex();
extern FILE* yyin;
string g_header;
bool g_lineinfo;
bool g_debug;
std::map<string,string> g_macros;

class ETEOF
{
};

class ETUnmatch
{
public:
    ETUnmatch(int _exp, CToken* _enc) : exp(_exp), enc(_enc) { }
    string to_string (int c)
    {
        char buff[10];
        if(c<MIN_TOKEN)
            sprintf(buff,"'%c'",c);
        else
            sprintf(buff,"%d",c);
        return string(buff);
    }
    int exp;
    CToken* enc;
};

class Parser
{
public:
    int parse(const char*);

private:
    int NextToken(bool if_throw=true) ;
    CToken* MatchAny();
    CToken* Match(int t);
    CToken* MatchPair(int,int);
    CToken* MatchPair(int,int,string&);

    CToken* parse_template();
    CToken* parse_component();
    CToken* parse_connect();  
    CToken* parse_varid();
	CToken* parse_compid();
	CToken* parse_typename();
    CToken* parse_port(string &, bool&);
    CToken* parse_funct_body();
    
    CToken* parse_arglist(string&, string&);
    
    std::vector<CToken*> m_tokens;
    std::vector<CToken*> m_program;
    std::vector<CToken*>::iterator m_iter;
    CToken* m_template;
    ComponentClass* m_current_component;
    ComponentTable* m_component_table;
    
    FILE* m_outfile;
    void WriteProgram();
	string m_hfilename;
};

int Parser::NextToken(bool if_throw)
{
	if(m_iter==m_tokens.end())
	{   
	    if(if_throw)
            throw ETEOF();
        else
            return 0;
    }
    else
        return (*m_iter)->type;
}
CToken* Parser::MatchAny()
{
    CPort* port;
    CToken* t = *m_iter;
    m_iter++;
    if(m_iter!=m_tokens.end()&&(*m_iter)->type==VARID)
	{
	    if(m_current_component!=NULL)
	    {
	        for(unsigned int i=0;i<m_current_component->tmpl_args.size();i++)
	        {
	            if( (*m_iter)->GetID() == m_current_component->tmpl_args[i])
	            {
	                (*m_iter)->type=ARGID;
	                //printf("line: %d, argid: %s\n",(*m_iter)->lineno,(*m_iter)->text.c_str());
	                break;
	            }
	        }
	    }
		if(t->type!=SCOPE&&t->type!='.'&&t->type!=DOTSTAR&&t->type!=ARROWSTAR&&t->type!=ARROW)
		{
			if(ComponentClass::Lookup((*m_iter)->GetID())!=NULL)
			{
			    //printf("convert '%s' to COMPID\n",(*m_iter)->GetID().c_str());
				(*m_iter)->type=COMPID;
		    }
			if(m_current_component!=NULL && (port=m_current_component->LookupPort((*m_iter)->GetID()))!=NULL )
			{
			    //printf("convert '%s' to PORTID\n",(*m_iter)->GetID().c_str());
				(*m_iter)->type=PORTID;
		    }
		}   
    }
    return t;
};

CToken* Parser::Match(int t) 
{ 
    if(t!=NextToken()) 
        throw ETUnmatch(t,*m_iter);
    else
        return MatchAny();
}

CToken* Parser::MatchPair(int left, int right)
{
    CToken *s, *t;
	s=t=Match(left);
    int level=0;
    while(level!=0||NextToken()!=right)
    {
        if(NextToken()==right)level--;
        if(NextToken()==left)level++;
        t->sibling=MatchAny();
		t=t->sibling;
    }
    t->sibling=MatchAny();
    t=t->sibling;
    return s;
}
CToken* Parser::MatchPair(int left, int right, string& text)
{
    CToken *s, *t;
	s=t=Match(left);
	text = t->text;
    int level=0;
    while(level!=0||NextToken()!=right)
    {
        if(NextToken()==right)level--;
        if(NextToken()==left)level++;
        t->sibling=MatchAny();
		t=t->sibling;
		text += t->text;
    }
    t->sibling=MatchAny();
    t=t->sibling;
	text += t->text;
    return s;
}

int Parser::parse(const char* filename)
{
    yyin= fopen(filename,"r");
    if(yyin==NULL)
    {
        printf("error: cannot open file '%s'\n", filename);
        return 2;
    }

    CTranslation::New();
    CTranslation::Current()->filename=filename;
    CToken::New(CTranslation::Current());

    // read all tokens into m_tokens

    while( (yylex()) != NULL )
    {
        m_tokens.push_back(CToken::Current());
    }

    /*for(unsigned int i=0;i<m_tokens.size();i++)
    {
        m_tokens[i]->Dump(); 
    }*/

    fclose(yyin);

    g_header =  "#include <assert.h> \n #include<vector> \n"
                "template <class T> class compcxx_array { public: \n"
                "virtual ~compcxx_array() { for (typename std::vector<T*>::iterator i=m_elements.begin();i!=m_elements.end();i++) delete (*i); } \n"
                "void SetSize(unsigned int n) { for(unsigned int i=0;i<n;i++)m_elements.push_back(new T); } \n"
                "T& operator [] (unsigned int i) { assert(i<m_elements.size()); return(*m_elements[i]); } \n"
                "unsigned int size() { return m_elements.size();} \n"
                "private: std::vector<T*> m_elements; }; \n"
                "class compcxx_component; \n"
                "template <class T> class compcxx_functor {public: \n"
                "void Connect(compcxx_component&_c, T _f){ c.push_back(&_c); f.push_back(_f); } \n"
                "protected: std::vector<compcxx_component*>c; std::vector<T> f; }; \n"
                "class compcxx_component { public: \n";

    string ofilename = filename;
    m_hfilename=ofilename;
    unsigned int pos = ofilename.rfind('.');
    ofilename.erase(pos+1);
    m_hfilename.erase(pos);
    m_hfilename = string("compcxx_") + m_hfilename + ".h";
    ofilename+="cxx";
    //printf("output file name:%s\n",ofilename.c_str());
    m_outfile=fopen(ofilename.c_str(),"w");
    if(m_outfile==NULL)
    {
        printf("cannot open output file '%s'\n",ofilename.c_str());
        return 1;
    }
    
    // starting parsing here

    m_iter=m_tokens.begin();
    int type;
    bool line_info=true;
    CToken* token;
    
    
 parsing:

    m_current_component = NULL;
    m_component_table = NULL;
    
    try 
    {
        while( (type=NextToken(false))!=0)
        {
    	    assert(m_current_component==NULL);
    	    assert(m_component_table==NULL);
            switch(type)
            {
            case TEMPLATE:
                token=parse_template();
                break;
            case COMPONENT:
                token=parse_component();
                break;
            case CONNECT:
                token=MatchAny();
                CTranslation::Error(token,4, "connect statements must appear within a function\n");
			    break;        	
            case INPORT:
            case OUTPORT:
                token=MatchAny();
                CTranslation::Error(token,5, "port must be declared within a component\n");
                break;
            case COMPID:
        	    token= parse_compid() ;
        	    break;
            case VARID:
        	    token = parse_varid();
        	    break;
        	case TYPENAME:
        		token = parse_typename();
        		break;
            default:
                token=MatchAny();
            }
            if(token!=NULL)
            {
                if(line_info==true)
                {
                    token->line_info=true;
                    line_info=false;
                }
                m_program.push_back(token);
                
            }
            else
                line_info=true;
        }
    }
    catch( ETUnmatch& e )
    {
        CTranslation::Error(e.enc, 3, "syntax error -- expected: %s, encountered: %s\n", 
            e.to_string(e.exp).c_str(), e.to_string(e.enc->type).c_str());
        line_info=true;
        goto parsing;
    }
    catch( ETEOF & )
    {
        CTranslation::Error(NULL, 2, "unexpected end-of-file\n");
    }
    
    if(CTranslation::ErrorCount())
    {
        printf("%d error(s).\n",CTranslation::ErrorCount());
    }
    
    WriteProgram();
    fclose(m_outfile);
    
    m_outfile=fopen(m_hfilename.c_str(),"w+");
    if(m_outfile==NULL)
    {
        printf("cannot open %s\n",m_hfilename.c_str());
        return 1;
    }
    fprintf(m_outfile,"%s};\n",g_header.c_str());
    fclose(m_outfile);
    return CTranslation::ErrorCount();
}

CToken* Parser::parse_template()
{
    CToken* t =  Match(TEMPLATE);
    t->AppendChild( MatchPair('<','>') );
    if(NextToken()==TEMPLATE)
    {
        t->AppendChild(Match(TEMPLATE));
        t->AppendChild(MatchPair('<','>'));
    }
    return t;
}

CToken* Parser::parse_component()
{
    int token;
    string port_name, comp_name;
    string return_type, full_list, arg_list;
    bool array;
    
    CToken* comp = Match(COMPONENT);
    CToken* comp_decl = CToken::New(comp,component_declaration);
    CToken* tmpl = NULL;
    CToken* p = NULL;
    
    if(!m_program.empty()&&m_program.back()->type==TEMPLATE)
    {
    	tmpl = m_program.back();
    	m_program.pop_back();
    	comp_decl->AppendChild(tmpl);
    	comp->lineno=tmpl->lineno;
    	tmpl->text = string("/*") + tmpl->text;
    	assert(tmpl->last_child!=NULL);
    	tmpl->last_child->text += "*/";
    }
  	comp->line_info=true;

    comp_decl->AppendChild(comp);
    CToken* t = Match(VARID);
    t->type=MAINID;
    comp_decl->AppendChild(t);
    comp_name = t->GetID();
    
    m_current_component = ComponentClass::Lookup(comp_name);
    if(m_current_component!=NULL)
    {
    	CTranslation::Error(t,6,"component '%s' has already been defined\n",comp_name.c_str());
    	comp_name+='_';
    }
   	m_current_component = ComponentClass::New(comp,comp_name);
   	if(g_debug)printf("component %s\n",comp_name.c_str());
   	if(tmpl!=NULL)m_current_component->AnalyzeTemplate(tmpl);
   	
   	m_component_table = m_current_component;
    if(NextToken()==';')
    {
        CTranslation::Error(t,7,"foward component declaration not necessary\n");
        comp_decl->AppendChild(MatchAny());
        return comp;
    }
    
    if(NextToken()==':')
    {
        CToken* t = Match(':');
        t->text += "public " COMPCXX "component, ";
        comp_decl->AppendChild(t);
    }
    else
    {
        CToken* t = CToken::New(comp,UNKNOWN);
        t->text = ": public " COMPCXX "component ";
        comp_decl->AppendChild(t);
    }
    
    while(NextToken()!='{')
        comp_decl->AppendChild(MatchAny());
    comp_decl->AppendChild(Match('{'));
    
    while( (token=NextToken())!='}')
    {
        switch(token)
        {
    	case COMPONENT:
        	CTranslation::Error(*m_iter,8,"nested component not permitted\n");
        	while(NextToken()!=';')
                MatchAny();
	        MatchAny();
            break;
    	case INPORT:
        	t = parse_port( port_name, array);
        	comp_decl->AppendChild(t);
        	break;
    	case OUTPORT:
        	t = parse_port( port_name, array);
        	p = CToken::New(t, outport_declaration);
        	p->text = port_name;
        	p->AppendChild(t);
        	comp_decl->AppendChild(p);
            if(m_current_component!=NULL&&m_current_component->composite&&array)
            {
              	CTranslation::Error(t,19,"a composite component cannot have outport arrays\n");
            }
        	break;
    	case CONNECT:
            MatchAny();
            CTranslation::Error(NULL,4, "connect statements must appear within a function\n");
            break;
    	case COMPID:
        	comp_decl->AppendChild(parse_compid());
            break;
       	case VARID:
        	comp_decl->AppendChild(parse_varid());
            break;
        case CLASS:
			while(NextToken()!=';'&&NextToken()!='{')
				comp_decl->AppendChild(MatchAny());
			if(NextToken()=='{')
	        	comp_decl->AppendChild(MatchPair('{','}'));
	       	else
	       		comp_decl->AppendChild(Match(';'));
			break;        	
       	default:
            comp_decl->AppendChild(MatchAny());
        }
    };
    
    CToken* tend = Match('}');
   	if(g_debug)printf("end component\n");
    comp_decl->AppendChild(CToken::New(tend,end_of_component_declaration));
    comp_decl->AppendChild(tend);
    if(NextToken()==';')
        comp_decl->AppendChild(Match(';'));
    m_current_component->AddToken(comp_decl);
    m_current_component=NULL;
    m_component_table=NULL;
	return NULL;  
}
CToken*  Parser::parse_port(string& port_name, bool& array)
{
    CToken *port, *token;
    CToken *args;
    CToken *name=NULL;
    
    port_name="";
    array=false;
    string return_type="";
    string full_list="";
    string arg_list="";

    CToken *return_type_begin=NULL, *return_type_end=NULL;
    CToken *full_list_begin=NULL, *full_list_end=NULL;

    if(NextToken()==INPORT)
    {
    	port=Match(INPORT);
    }
    else
    {
    	port=Match(OUTPORT);
    }
    
    if(NextToken()==ARRAY)
    {
    	CToken * t =Match(ARRAY);
    	t->text = "";
        if(port->type==INPORT)
        {
            CTranslation::Error(port,17,"inport array not support\n");
        }
        array=true;
    }

    while(NextToken()!='(')
    {
    	token=MatchAny();
    	if(return_type_begin==NULL)return_type_begin=token;
    	if(name==NULL&&NextToken()=='(')
    	{
    		port_name=token->GetID();
			name=token;
			return_type_end=name;
    	}
    	else
    	{
    	    return_type += token->text;
    	}
    	port->AppendChild(token);
    }
    args=parse_arglist(full_list,arg_list);
    port->AppendChild(args);
    full_list_begin=args;
    
    //printf("%s\n%s\n",full_list.c_str(),arg_list.c_str());
    
    if(NextToken()=='{')
    {
        token=MatchPair('{','}');
        CTranslation::Error(token,9,"in-class port implementation not allowed\n");
        return port;
    }

    CToken* tend = Match(';');
	port->AppendChild(tend);
	full_list_end=tend;
	
	CPort* port_class;
	if(port->type==INPORT)
	{
        port_class = m_current_component->AddPort(port,port_name,INPORT);
        port_class -> array = array;
	}
	else
	{
        port_class = m_current_component->AddPort(port,port_name,OUTPORT);
        port_class -> array = array;
        port_class -> functor_type = m_current_component->class_name + "_" + port_name + "_f_t";
        if(array)
            port_class -> functor_name = port_name;
        else
            port_class -> functor_name = port_name + "_f";
    }
    port_class -> full_list_begin = full_list_begin;
    port_class -> full_list_end = full_list_end;
    port_class -> return_type_begin = return_type_begin;
    port_class -> return_type_end = return_type_end;
    port_class -> arg_list = arg_list;
	
	if(m_current_component!=NULL&&m_current_component->composite==true)
	{
	    port->text=string("/*") + port->text + string("*/");
	    CToken* mem_def=CToken::New(port,inport_definition);
	    mem_def->text = port_name;
	    mem_def->line_info=true;
	    token=return_type_begin;
	    while(token!=return_type_end)
	    {
	        mem_def->AppendChild(CToken::Copy(token));
	        token=token->sibling;
	    }
	    token=CToken::New(port,MAINID);
	    token->text=string("::")+port_name;
	    mem_def->AppendChild(token);
	    token=full_list_begin;
	    while(token!=full_list_end)
	    {
	        mem_def->AppendChild(CToken::Copy(token));
	        token=token->sibling;
	    }
        token=CToken::New(token,UNKNOWN);
        token->text="{return ";
	    mem_def->AppendChild(token);
	    token=CToken::New(port,PORTID);
	    token->text=port_name;
	    mem_def->AppendChild(token);
	    CToken* t = CToken::New(port,UNKNOWN);
	    t->text = arg_list;
	    token->AppendChild(t);
	    token=CToken::New(port,UNKNOWN);
	    token->text=";}";
	    mem_def->AppendChild(token);
	    m_current_component->AddToken(mem_def);
	}
	else
	{
	    if(port->type==INPORT)
	    {
	      	port->text=string("/*")+port->text+string("*/");
	    }
	    else
	    {
	      	port->text=string("/*")+port->text;
	      	tend->text=string("*/")+tend->text;
	    }
    }

	if(g_debug)
	{
		if(port->type==INPORT)
			printf("    inport %s %s %s %s\n", return_type.c_str(), port_name.c_str(), full_list.c_str(), arg_list.c_str());
		else
			printf("    outport %s %s %s %s\n", return_type.c_str(), port_name.c_str(), full_list.c_str(), arg_list.c_str());
		
	}
	
    return port;
}

CToken* Parser::parse_arglist(string& full_list, string& arg_list)
{
    CToken *s, *t, *last;
	s=t=Match('(');
	full_list = s->text;
	arg_list = s->text;
	last=s;
    int level=0;
    while(level!=0||NextToken()!=')')
    {
        if(NextToken()=='('||NextToken()=='<')level--;
        if(NextToken()==')'||NextToken()=='>')level++;
        t->sibling=MatchAny();
		t=t->sibling;
		full_list += t->text;
		
		if(level==0&&t->type==','&&last->type==VARID)
		{
		    arg_list += last->text;
		    arg_list += ',';
		}
		last=t;
    }
    
    if(last->type==VARID)
    {
        arg_list += last->text;
    }
    
    t->sibling=MatchAny();
    t=t->sibling;
	full_list += t->text;
	arg_list += t->text;
    return s;

}

CToken* Parser::parse_connect()
{
    string from_c_name, from_p_name, to_c_name, to_p_name, code;
    string from_c_index,from_p_index, to_c_index, to_p_index;
    ComponentInstance *from_c=NULL, *to_c=NULL;
	CToken *con, *token;
    con=Match(CONNECT);
    
    if(NextToken()==VARID)
    {
        token=Match(VARID);
        from_c_name=token->GetID();
        con->AppendChild(token);
    
        if(NextToken()=='[')
            con->AppendChild(MatchPair('[',']',from_c_index));
   	    con->AppendChild(Match('.'));
        token=Match(VARID);
        from_p_name=token->GetID();
        con->AppendChild(token);
        if(NextToken()=='[')
            con->AppendChild(MatchPair('[',']',from_p_index));
    }
    else
    {
        token=Match(PORTID);
        from_p_name=token->GetID();
        if(NextToken()=='[')
            con->AppendChild(MatchPair('[',']',from_p_index));
    }
    
    con->AppendChild(Match(','));
    
    if(NextToken()==VARID)
    {
        token=Match(VARID);
        to_c_name=token->GetID();
        con->AppendChild(token);
        if(NextToken()=='[')
            con->AppendChild(MatchPair('[',']',to_c_index));
    
   	    con->AppendChild(Match('.'));
        token=Match(VARID);
        to_p_name=token->GetID();
        con->AppendChild(token);
        if(NextToken()=='[')
            con->AppendChild(MatchPair('[',']',to_p_index));
    }
    else
    {
        token=Match(PORTID);
        to_p_name=token->GetID();
        if(NextToken()=='[')
            con->AppendChild(MatchPair('[',']',to_p_index));
    }

    token = Match(';');
    con->AppendChild(token);
    
    if(m_component_table==NULL) return con;
    CPort *from_p, *to_p;
    
    if(from_c_name!="")
    {
        from_c=m_component_table->LookupComponent(from_c_name);
        if(from_c==NULL)
        {
            CTranslation::Error(con,10,"component '%s' not declared\n", from_c_name.c_str());
            return con;
        }
        if(from_c->array&&from_c_index=="")
        {
            CTranslation::Error(con,18,"component '%s' was declared as an array\n", from_c_name.c_str());
        }
        if(!from_c->array&&from_c_index!="")
        {
            CTranslation::Error(con,18,"component '%s' was not declared as an array\n", from_c_name.c_str());
        }
        assert(from_c->GetClass()!=NULL);
        from_p=from_c->GetClass()->LookupPort(from_p_name);
        if(from_p==NULL)
        {
            CTranslation::Error(con,11,"port '%s' cannot be found\n", from_p_name.c_str());
            return con;
        }
        if(from_p->type!=OUTPORT)
        {
            CTranslation::Error(con,14,"port '%s' is an inport (outport required)\n", from_p_name.c_str());
            return con;
        }
    }
    else
    {
	    if(m_current_component==NULL)
	    {
            CTranslation::Error(con,12,"component connections out of range\n");
            return con;
	    }
        from_p=m_current_component->LookupPort(from_p_name);
        if(from_p==NULL)
        {
            CTranslation::Error(con,11,"port '%s' cannot be found\n", from_p_name.c_str());
            return con;
        }
        if(from_p->type!=INPORT)
        {   
            CTranslation::Error(con,14,"port '%s' is an outport (inport required)\n", from_p_name.c_str());
            return con;
        }
    }
    
    if(from_p->array&&from_p_index=="")
    {
        CTranslation::Error(con,18,"port '%s' was declared as an array\n", from_p_name.c_str());
    }
    if(!from_p->array&&from_p_index!="")
    {
        CTranslation::Error(con,18,"port '%s' was not declared as an array\n", from_p_name.c_str());
    }

    if(to_c_name!="")
    {
        to_c=m_component_table->LookupComponent(to_c_name);
        if(to_c==NULL)
        {
            CTranslation::Error(con,10,"component '%s' not declared\n", to_c_name.c_str());
            return con;
        }
        assert(to_c->GetClass()!=NULL);
        if(to_c->array&&to_c_index=="")
        {
            CTranslation::Error(con,18,"component '%s' was declared as an array\n", to_c_name.c_str());
        }
        if(!to_c->array&&to_c_index!="")
        {
            CTranslation::Error(con,18,"component '%s' was not declared as an array\n", to_c_name.c_str());
        }

        to_p=to_c->GetClass()->LookupPort(to_p_name);
        if(to_p==NULL)
        {
            CTranslation::Error(con,11,"port '%s' cannot be found in component '%s'\n", to_p_name.c_str(),to_c_name.c_str());
            return con;
        }
	    if (to_p->type!=INPORT)
	    {
            CTranslation::Error(con,14,"port '%s' is an outport (inport required)\n", to_p_name.c_str());
            return con;	        
	    }
    }
    else
    {
	    if(m_current_component==NULL)
	    {
            CTranslation::Error(con,12,"component connections out of range");
            return con;
	    }
        to_p=m_current_component->LookupPort(to_p_name);
        if(to_p==NULL)
        {
            CTranslation::Error(con,11,"port '%s' cannot be found in component '%s'\n", to_p_name.c_str(),to_c_name.c_str());
            return con;
        }
    }
        
    if(!to_p->array&&to_p_index!="")
    {
        CTranslation::Error(con,18,"port '%s' was not declared as an array\n", to_p_name.c_str());
    }

    if(from_c!=NULL&&to_c!=NULL) // primitive outport -> primitive inport
    {
        if(from_c->array||from_p->array)
        {
            code = from_c_name + from_c_index + "." + from_p->functor_name + from_p_index + ".Connect(" +
                    to_c_name + to_c_index + ",(" COMPCXX "component::" + from_p -> functor_type +
                    ")&" + to_c-> class_name + "::" + to_p_name + ")";
            CPort* inst_port = from_c->LookupPort(from_p_name);
            inst_port->connected=true;
        }
        else
        {
            from_c->AddConnection(from_p_name,to_c,to_p_name);
            code = from_c_name + ".p_" + to_c->class_name + "=&" + to_c_name;
        }
    }
    if(from_c!=NULL&&to_c==NULL) // primitive outport -> composite outport/inport
    {
        if(from_c->array||from_p->array)
        {
            code = from_c_name + from_c_index + "." + from_p->functor_name + from_p_index + ".Connect(this,(" 
                   "component::" + from_p -> functor_type +
                   ")&" + to_c-> class_name + "::" + to_p_name + ")";
            CPort* inst_port = from_c->LookupPort(from_p_name);
            inst_port->connected=true;
        }
        else
        {
            from_c->AddConnection(from_p_name,NULL,to_p_name);
            code = from_c_name + ".p_" COMPCXX "parent=this";
        }
    }
    if(from_c==NULL&&to_c!=NULL) // composite inport -> primitive inport
    {
        from_p->bound=true;        
        from_p->connections.push_back(make_pair(to_c,to_p_name));
    }
    if(from_c==NULL&&to_c==NULL) // composite outport -> composite inport
    {
        CTranslation::Error(con,15,"connot connect an inport and an outport of the same component\n");
    }

    con->text = code + " /*" + con->text;
    token->text = string("*/") + token->text;
   
    return con;
}

CToken* Parser::parse_compid()
{
	CToken* comp=Match(COMPID);
	CToken* tmpl=NULL;
	CToken* real_comp=comp;
	string comp_name = comp->GetID();
	CToken *var=NULL;
    string tmpl_args;
    bool array=false;
    unsigned int i;
    
	if(NextToken()=='<')
	{
	    tmpl = MatchPair('<','>');
	    tmpl->text = string("/*") + tmpl->text;
	    comp->AppendChild(tmpl);
	    CToken* t = CToken::New(tmpl,UNKNOWN);
	    t->text = "*/";
	    comp->AppendChild(t);
	}
	switch(NextToken())
	{
	case SCOPE:  // comp::var(...):...{...}

        if(!m_program.empty()&&m_program.back()->type==TEMPLATE)
        {
    	    CToken* tmpl = m_program.back();
    	    m_program.pop_back();
    	    comp->InsertChild(tmpl);
        	tmpl->text = string("/*") + tmpl->text;
        	assert(tmpl->last_child!=NULL);
    	    tmpl->last_child->text += "*/";
        }
        comp->type=MAINID;
		comp->AppendChild(Match(SCOPE));
		m_current_component=ComponentClass::Lookup(comp->GetID());
		assert(m_current_component!=NULL);
		if(NextToken()=='~')
		{
			comp->AppendChild(Match('~'));
			var=Match(COMPID);
		}
		else
			var=Match(VARID);
		if(var->GetID()==comp->GetID())
		    var->type=MAINID;
		comp->AppendChild(var);
		comp->AppendChild(MatchPair('(',')'));
		if(NextToken()==':')
		{
			while(NextToken()!='{')
				comp->AppendChild(MatchAny());
		}
   		m_component_table = m_current_component;
    	comp->AppendChild(parse_funct_body());
		m_current_component->AddToken(comp);
	    m_current_component = NULL;
	    m_component_table = NULL;
	    comp->line_info=true;
        return NULL;
	case '(':   // comp(...):...{...}
	    comp->type=MAINID;
		comp->AppendChild(MatchPair('(',')'));
		if(NextToken()==':')
		{
			while(NextToken()!='{')
				comp->AppendChild(MatchAny());
		}
		if(NextToken()=='{')
		{
    		comp->AppendChild(parse_funct_body());
	    }
	    break;
	default: // component instantiation
	    if(NextToken()==ARRAY)
	    {
	        CToken* t = CToken::New(comp,UNKNOWN);
	        t->text = COMPCXX "array<";
	        t->AppendChild(comp);
	        comp=t;
	        t=Match(ARRAY);
	        t->text = " >";
	        comp->AppendChild(t);
	        array=true;
	    }
	    var=Match(VARID);
	    comp->AppendChild(var);
	    comp->AppendChild(Match(';'));
	    
	    if(m_component_table==NULL)
	    {
            CTranslation::Error(comp,12,"component instantiation out of range");
	    }
	    else
	    {
	        ComponentClass* c = ComponentClass::Lookup(comp_name);
	        assert(c!=NULL);
	        ComponentInstance* inst  = new ComponentInstance (c);
            inst->inst_name = var->GetID();	        
	        m_component_table->AddComponent(var->GetID(),inst);
	        real_comp->text=real_comp->ReplaceID(inst->class_name.c_str()); 
	        inst->array = array;
	        
	        if(tmpl!=NULL)
	        {
	            CToken* t=tmpl;
	            string arg;
	            string id;
	            bool flag=true;
	            int level=0;
	            do
	            {
	                switch(t->type)
	                {
	                case ARGID:
	                    assert(m_current_component!=NULL);
	                    id=t->GetID();
	                    for(i=0;i<m_current_component->tmpl_args.size();i++)
	                        if(id==m_current_component->tmpl_args[i])
	                        {
	                            arg += "${";
	                            arg += m_current_component->tmpl_args[i];
	                            arg += "$}";
	                            break;
	                        }
	                    assert(i<m_current_component->tmpl_args.size());
	                    break;
	                case ',':
	                	if(level==1)
	                	{
	                    	inst->tmpl_values.push_back(arg);
	                    	arg="";
	                    }
	                    else
	                    {
		                    arg+=t->text;
	                    }
	                    break;
	                case '>':
	                	level--;
	                	if(level==0)
	                	{
	                    	inst->tmpl_values.push_back(arg);
	                    	arg="";
	                    	flag=false;
	                    }
	                    else
	                    	arg+=t->text;
	                    break;
	                case '<':
	                	if(level!=0)arg+=t->text;
	                	level++;
	                    break;
	                default:
	                    arg+=t->text;
	                    break;
	                }
	                t=t->sibling;
	            }while(flag==true);

                if(m_current_component!=NULL)
                {
                    if(inst->tmpl_values.size()!=inst->GetClass()->tmpl_args.size())
                        CTranslation::Error(comp,20,"number of template parameters unmatched\n");
                }
	            
	        }
	    }
	    if(m_current_component!=NULL)
	    {
	        m_current_component->composite=true;
	    }
	}
	return comp;
}

CToken* Parser::parse_varid()
{
    CToken* mem_def, *comp, * funct_def;
    CToken* h = Match(VARID), *t=NULL;
    // printf("parse_varid: (%s, %d) %s\n",h->translation->filename.c_str(),h->lineno,h->text.c_str());
    CToken* var = NULL;
    CPort* port;
    while(NextToken()!='('&&NextToken()!=COMPID&&NextToken()!=';'&&NextToken()!=':')
    {
        t=MatchAny();
        if(t->type==VARID)var=t;
        h->AppendChild(t);
    }
    switch(NextToken())
    {
    case '(':  //function
        funct_def = CToken::New(h,function_definition);
        if(!m_program.empty()&&m_program.back()->type==TEMPLATE)
        {
    	    CToken* tmpl = m_program.back();
    	    m_program.pop_back();
    	    funct_def->AppendChild(tmpl);
        }
        funct_def->AppendChild(h);
        t=MatchPair('(',')');
        funct_def->AppendChild(t);
        if(NextToken()==';')
        {
            funct_def->AppendChild(Match(';'));
            return funct_def;
        }
        else
        {
            while(NextToken()!='{'&&NextToken()!=';')
                funct_def->AppendChild(MatchAny());
            if(NextToken()=='{')
                funct_def->AppendChild(parse_funct_body());
            else
                funct_def->AppendChild(Match(';'));
        }
        return funct_def;
    case COMPID: //member function or member variable
        comp=Match(COMPID);
        comp->type=MAINID;
		m_current_component=ComponentClass::Lookup(comp->GetID());
		m_component_table=m_current_component;
        assert(m_current_component!=NULL);
    
        t=h;
        while(t!=NULL)
        {
            for(unsigned int i=0;i<m_current_component->tmpl_args.size();i++)
	        {
	            if( t->GetID() == m_current_component->tmpl_args[i])
	            {
	                t->type=ARGID;
	                break;
	            }
	        }
	        if(t->sibling==NULL)t=t->child;else t=t->sibling;
        }
        
        mem_def = CToken::New(h,member_definition);

        if(!m_program.empty()&&m_program.back()->type==TEMPLATE)
        {
    	    CToken* tmpl = m_program.back();
    	    m_program.pop_back();
    	    mem_def->InsertChild(tmpl);
    	    mem_def->lineno=tmpl->lineno;
        	tmpl->text = string("/*") + tmpl->text;
        	assert(tmpl->last_child!=NULL);
    	    tmpl->last_child->text += "*/";
        }
        mem_def->line_info=true;

        mem_def -> AppendChild(h);
        if(h->child!=NULL) mem_def -> AppendChild(h->child);
        h->child = h->last_child = NULL;

        mem_def->AppendChild(comp);
        if(NextToken()=='<')
        {
    	    CToken* t = MatchPair('<','>');
	        t->text = string("/*") + t->text;
	        mem_def->AppendChild(t);
	        t = CToken::New(t,UNKNOWN);
	        t->text = "*/";
	        mem_def->AppendChild(t);
        }
        mem_def->AppendChild(Match(SCOPE));
        var=Match(VARID);
        port= m_current_component->LookupPort(var->GetID());
        if(port!=NULL)
        {
        	port->bound=true;
        	port->implemented=true;
        }
        mem_def->AppendChild(var);
        while(NextToken()!=';'&&NextToken()!='(')
        	mem_def->AppendChild(MatchAny());
        if(NextToken()=='(')
        {
            mem_def->AppendChild(MatchPair('(',')'));
            while(NextToken()!='{')
                mem_def->AppendChild(MatchAny());
            mem_def->AppendChild(parse_funct_body());
        }
        else
        {
			mem_def->AppendChild(Match(';'));
        }
        m_current_component->AddToken(mem_def);
        m_current_component=NULL;
        m_component_table=NULL;
        return NULL;
    default:
        h->AppendChild(MatchAny());
    	break;
    }
    return h;
}    

CToken* Parser::parse_typename()
{
	CToken* t = Match(TYPENAME);
	t->text = string("/*") + t->text + "*/";
    CToken* mem_def = CToken::New(t,member_definition);

    if(!m_program.empty()&&m_program.back()->type==TEMPLATE)
    {
	    CToken* tmpl = m_program.back();
  	    m_program.pop_back();
   	    mem_def->InsertChild(tmpl);
   	    mem_def->lineno=tmpl->lineno;
       	tmpl->text = string("/*") + tmpl->text;
       	assert(tmpl->last_child!=NULL);
   	    tmpl->last_child->text += "*/";
    }
    
    mem_def->AppendChild(t);
    
    if(NextToken()!=COMPID)
    {
    	while(NextToken()!=';'&&NextToken()!='{')
    		mem_def->AppendChild(MatchAny());
    	if(NextToken()==';')
    		mem_def->AppendChild(MatchAny());
    	else
    		mem_def->AppendChild(MatchPair('{','}'));
    	mem_def->type=UNKNOWN;
    	return mem_def;
    }
    
    t = Match(COMPID);
    t -> type = MAINID;
    string comp_name = t->GetID();
    mem_def->AppendChild( t );
    if(NextToken()=='<');
    {
    	t=MatchPair('<','>');
    	t->text = string("/*") + t->text;
    	mem_def->AppendChild(t);
    	mem_def->last_child->text += "*/";
    }
    mem_def->AppendChild(Match(SCOPE));
    while(NextToken()!=COMPID) mem_def->AppendChild(MatchAny());
    t = Match(COMPID);
	if(t->GetID()!=comp_name)
	{
        CTranslation::Error(t,21,"two component names must match\n");
	}
	t -> type = MAINID;
	mem_def->AppendChild(t);
	
	m_current_component=ComponentClass::Lookup(comp_name);
	m_component_table=m_current_component;
    assert(m_current_component!=NULL);
	
    if(NextToken()=='<');
    {
    	t=MatchPair('<','>');
    	t->text = string("/*") + t->text;
    	mem_def->AppendChild(t);
    	mem_def->last_child->text += "*/";
    }
    mem_def->AppendChild(Match(SCOPE));
    mem_def->AppendChild(Match(VARID));
    while(NextToken()!=';'&&NextToken()!='(')
      	mem_def->AppendChild(MatchAny());
    if(NextToken()=='(')
    {
        mem_def->AppendChild(MatchPair('(',')'));
        while(NextToken()!='{')
           mem_def->AppendChild(MatchAny());
        mem_def->AppendChild(parse_funct_body());
    }
    else
    {
		mem_def->AppendChild(Match(';'));
    }
    
    m_current_component->AddToken(mem_def);
    m_current_component=NULL;
    m_component_table=NULL;
    return NULL;
}

CToken* Parser::parse_funct_body()
{
    ComponentTable* table=NULL;
    if(m_component_table==NULL)
    {
        m_component_table=table=new ComponentTable;
    }
	CToken* funct=Match('{');
	CToken* port;
	int level=0;
	int token;
	while( (token=NextToken())!= '}' || level!=0)
	{
		switch(token)
		{
		case COMPID:
		    funct->AppendChild(parse_compid());
		    break;   
		case CONNECT:
		    funct->AppendChild(parse_connect());
		    break;
		case PORTID:
		    port=Match(PORTID);
		    if(NextToken()=='(')
		    {
		        port -> AppendChild(MatchPair('(',')'));
		    }
		    else
		    {
		        port -> type = UNKNOWN;
		    }
		    funct->AppendChild(port);
		    break;
		default:
		    if(token=='{')level++;
		    if(token=='}')level--;
		    funct->AppendChild(MatchAny());
		}
	}
	funct->AppendChild(Match('}'));
	if(table!=NULL)
	{
	    if(table->Size()!=0&&CTranslation::ErrorCount()==0)
	    {
	        WriteProgram();
            fprintf(m_outfile,"#include \"%s\"\n",m_hfilename.c_str());
	        table->GenerateCode(m_outfile,NULL,true);
	        table->GenerateCode(m_outfile,NULL,false);
	    }
	    delete table;
	    m_component_table=NULL;
	}
	return funct;
}
void Parser::WriteProgram()
{
    for(unsigned int i=0;i<m_program.size();i++)
    {
        m_program[i]->Write(m_outfile);
    }
    m_program.clear();
}
int main(int argc, char* argv[])
{
	g_debug = false;
	g_lineinfo = true;
	
	const char* filename=NULL;
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-verbose")==0)
		{
			g_debug=true;
		}
		else if(strcmp(argv[i],"-noline")==0)
		{
			g_lineinfo=false;
		}
		else if(strlen(argv[i])>3&& argv[i][0]=='-' && (argv[i][1]=='d'||argv[i][1]=='D') )
		{
			char* p = strchr(argv[i],'=');
			if(p!=NULL)
			{
				*p=0;
				string id1 = argv[i]+2;
				string id2 = p+1;
				//printf("#define %s %s \n",id1.c_str(), id2.c_str());
				g_macros[id1]=id2;
			}
		}
		else if(filename==NULL)filename=argv[i];
	}
  	if(filename==NULL)
    {
        printf("error: no input file specfied\n");
        return 1;
    }

  	Parser parser;
  	return parser.parse(filename);

}
