#include "token.h"

CTranslation* CTranslation::m_current = NULL;
std::vector<CTranslation*> CTranslation::m_stack;
std::vector<CTranslation*> CTranslation::m_all;
int CTranslation::m_error_count=0;

CToken* CToken::current=NULL;

void CTranslation::New()
{
  CTranslation* t = new CTranslation;
  t->lineno = 1;
  m_all.push_back(t);
  if(m_current!=NULL)
    m_stack.push_back(m_current); 
  m_current=t;
}

bool CTranslation::Delete()
{
  if(m_stack.empty()) return false;
  m_current=m_stack.back();
  m_stack.pop_back();
  return true;
}

void CTranslation::Clear()
{
  unsigned int i;
  for(i=0;i<m_all.size();i++)
    delete m_all[i];
  m_all.clear();
  m_stack.clear();
  m_current=NULL;
}

void CTranslation::Error(CToken* t, int n, const char* format, ...)
{
    m_error_count++;
    va_list argp;
    va_start(argp,format);
    if(t==NULL)
    	fprintf(stderr,"error %04d (%s,%d):", n,Current()->filename.c_str(),Current()->lineno);
    else
    	fprintf(stderr,"error %04d (%s,%d):", n,t->translation->filename.c_str(),t->lineno);
    vfprintf(stderr,format,argp);
    va_end(argp);
}

CToken* CToken::New(CTranslation* trans, int type)
{
	CToken* t = new CToken(trans, type);
	current = t;
	return t;
}

CToken* CToken::New(CToken* token, int type)
{
	CToken* t = new CToken(token->translation, type);
	t->lineno=token->lineno;
	current = t;
	return t;
}

CToken* CToken::Copy(CToken* token)
{
	CToken* t = new CToken(token->translation, token->type);
	t->lineno=token->lineno;
	t->text = token->text;
	t->line_info = token->line_info;
	return t;
}



CToken* CToken::AppendChild (CToken* token)
{
	CToken* t=child;
	while(t!=NULL&&t->sibling!=NULL)t=t->sibling;
	assert(last_child==t);
	if(child==NULL)
	{
		child=token;
	}
	else
	{
		assert(last_child!=NULL);
		last_child->sibling=token;
	}
	last_child=token;
	while(last_child->sibling!=NULL)
	{
		last_child=last_child->sibling;
	}
	return last_child;
}

CToken* CToken::InsertChild( CToken* token)
{
	CToken* t=child;
	while(t!=NULL&&t->sibling!=NULL)t=t->sibling;
	assert(last_child==t);
	
	t=token;
	while(t!=NULL&&t->sibling!=NULL)t=t->sibling;
	t->sibling=child;
	child=token;
	if(last_child==NULL)last_child=t;
	
	t=child;
	while(t!=NULL&&t->sibling!=NULL)t=t->sibling;
	assert(last_child==t);
	return token;
}
string CToken::GetID()
{
	string id=text;
	unsigned int i=0;
	while( i<id.size()&& (isalnum(id[i]) || id[i]=='_')  ) i++;
	id.erase(i);
	return id;
}
string CToken::ReplaceID(const char* s)
{
    string id=text;
	unsigned int i=0;
	while( i<id.size()&& (isalnum(id[i]) || id[i]=='_')  ) i++;
	id.erase(0,i);
	id=string(s)+id;
	return id;
}

void CToken::Write(FILE* ofile)
{
    if(g_lineinfo&&line_info)
    {
        fprintf(ofile,"\n#line %d \"%s\"\n",lineno,translation->filename.c_str());
    }
    fprintf(ofile,"%s", text.c_str());
    if(child!=NULL) child->Write(ofile);
    if(sibling!=NULL) sibling->Write(ofile);
}
ComponentTable::~ComponentTable()
{
}
ComponentInstance* ComponentTable::LookupComponent(const string& name)
{
	std::map<string, ComponentInstance*>::iterator iter;
	iter=m_instances.find(name);
	if(iter==m_instances.end())
		return NULL;
	else
		return (*iter).second;
}

void ComponentTable::AddComponent(const string& n, ComponentInstance* c)
{
    m_instances[n]=c;
}

void ComponentTable::GenerateCode(FILE* ofile, ComponentInstance * inst, bool decl_only)
{
    std::map<string, ComponentInstance*>::iterator iter;
    for(iter=m_instances.begin();iter!=m_instances.end();iter++)
    {
        (*iter).second->parent=inst;
        (*iter).second->GenerateCode(ofile, decl_only);
    }
}

ComponentClass::~ComponentClass()
{
}
void ComponentClass::GenerateCode(FILE* ofile, ComponentInstance * inst, bool decl_only)
{
    std::map<string, ComponentInstance*>::iterator iter;
    ComponentInstance* child;
    string s;
    unsigned int i,j;
    string::size_type pos;
    for(iter=m_instances.begin();iter!=m_instances.end();iter++)
    {
        child=(*iter).second;
        for(i=0;i<tmpl_args.size();i++)
        {
            s=string("${")+tmpl_args[i]+"$}";
            for(j=0;j<child->tmpl_values.size();j++)
            {
                pos=child->tmpl_values[j].find(s);
                while(pos!=string::npos)
                {
                    child->tmpl_values[j] = child->tmpl_values[j].substr(0,pos) + inst->tmpl_values[i] + child->tmpl_values[j].substr(pos+s.size());
                    pos=child->tmpl_values[j].find(s);
                }  
            }
        }
        /*printf("args: ");
	    for(j=0;j<child->tmpl_values.size();j++)
	    {
	    	if(j!=0)printf(",");
	    	printf("%s",child->tmpl_values[j].c_str());
	    }
	    printf("\n");*/
        
        
    }
    
    ComponentTable::GenerateCode(ofile,inst,decl_only);
}


std::map<string, ComponentClass*> ComponentClass::m_classes;

ComponentClass* ComponentClass::New(CToken* t, const string& n)
{
	ComponentClass* comp_class = new ComponentClass (n);
	m_classes[n]=comp_class;
	return comp_class;
}
ComponentClass* ComponentClass::Lookup(const string& name)
{
	std::map<string, ComponentClass*>::iterator iter;
	iter=m_classes.find(name);
	if(iter==m_classes.end())
		return NULL;
	else
		return (*iter).second;
}

CPort* ComponentClass::AddPort(CToken* t, const string& name, int type)
{
    //printf("add port %s\n",name.c_str());
	CPort* port = new CPort(name,type);
	ports[name]=port;
	port->token=t;
	return port;
}

CPort* ComponentClass::LookupPort(const string& name)
{
	std::map<string, CPort*>::iterator iter;
	iter=ports.find(name);
	if(iter==ports.end())
		return NULL;
	else
		return (*iter).second;
}

void ComponentClass::AnalyzeTemplate(CToken* tmpl)
{
    CToken* t = tmpl->child;
    while(t!=NULL)
    {
        if(t->type==CLASS && t->sibling!=NULL && t->sibling->type == VARID)
        {
            tmpl_args.push_back(t->sibling->GetID());
        }
        t=t->sibling;
    }
}

int ComponentInstance::tmp_number=1;

ComponentInstance::ComponentInstance(ComponentClass* c)
{
	m_class = c;
	parent = NULL;
	array = false;
	char buffer[MAX_STRING_LENGTH];
	sprintf(buffer,COMPCXX "%s_%d",c->class_name.c_str(),++tmp_number);
	class_name  = buffer;
    std::map<string, CPort*>::iterator iter;
	for(iter=c->ports.begin();iter!=c->ports.end();iter++)
	{
	    CPort* port = new CPort( *((*iter).second) );
	    m_ports[(*iter).second->name]= port;
	}
}

CPort* ComponentInstance::LookupPort(const string& name)
{
	std::map<string, CPort*>::iterator iter;
	iter=m_ports.find(name);
	if(iter==m_ports.end())
		return NULL;
	else
		return (*iter).second;
}

bool ComponentInstance::AddConnection(const string& out, ComponentInstance* inst, const string& in)
{
    unsigned int i;
    for(i=0;i<m_peers.size();i++)
    {
        if(m_peers[i]==inst)break;
    }
    if(i==m_peers.size())
        m_peers.push_back(inst);
    
	std::map<string, CPort*>::iterator iter;
	iter=m_ports.find(out);
	if(iter==m_ports.end())
		return false;
	(*iter).second->connections.push_back(make_pair(inst,in));
	(*iter).second->connected = true;
	return true;
}

void ComponentInstance::GenerateCode(FILE* ofile, bool decl_only)
{
    unsigned int i;
    std::map<string, CPort*>::iterator iter;
    if(decl_only)
    {
        for(iter=m_ports.begin();iter!=m_ports.end();iter++)
        {
            if((*iter).second->type==OUTPORT&&(*iter).second->connected==false)
            {
                CTranslation::Error((*iter).second->token,13,"outport '%s' of component '%s' not connected\n",(*iter).second->name.c_str(),inst_name.c_str());
            }
            if((*iter).second->type==INPORT&&(*iter).second->bound==false)
            {
                CTranslation::Error((*iter).second->token,16,"inport '%s' of component '%s' not bound\n",(*iter).second->name.c_str(),inst_name.c_str());
            }
        }
    }
    m_class->GenerateCode(ofile, this, decl_only);
    for(i=0;i<m_class->source.size();i++)
    {
        if( (m_class->source[i]->type==component_declaration) == decl_only )
            Convert(ofile, m_class->source[i]);
    }
}

void ComponentInstance::Convert(FILE* ofile, CToken* t)
{
    if(g_lineinfo&&t->line_info)
    {
        fprintf(ofile,"\n#line %d \"%s\"\n",t->lineno,t->translation->filename.c_str());
    }

    unsigned int i;
    CPort* port;
    std::map<string, CPort*>::iterator iter;
    switch(t->type)
    {
    case MAINID:
        fprintf(ofile,"%s", t->ReplaceID(class_name.c_str()).c_str());
        break;
    case end_of_component_declaration:
        for(i=0;i<m_peers.size();i++)
        {
            if(m_peers[i]!=NULL)
                fprintf(ofile, "public:%s* p_%s;",m_peers[i]->class_name.c_str(),m_peers[i]->class_name.c_str());
            else
                fprintf(ofile,"public:%s* p_" COMPCXX "parent;",parent->class_name.c_str());
            
        }
        break;
    case PORTID:
        iter=m_ports.find(t->GetID());
        assert(iter!=m_ports.end());
        port=(*iter).second;
        fprintf(ofile,"(");
        if(port->type==OUTPORT)
        {
            if(array)
            {
                fprintf(ofile,"%s",port->functor_name.c_str());
                if(t->child!=NULL) Convert(ofile,t->child);
            }
            else
            {
                for(i=0;i<port->connections.size();i++)
                {
                    if(i!=0)fprintf(ofile,",");
                    if(port->connections[i].first==NULL)
                    {   
                        fprintf(ofile,"p_" COMPCXX "parent->%s",port->connections[i].second.c_str());
                    }
                    else
                    {
                     fprintf(ofile,"p_%s->%s", port->connections[i].first->class_name.c_str(),port->connections[i].second.c_str());
                    }
                    if(t->child!=NULL) Convert(ofile,t->child);
                }
            }
        }
        else
        {
            for(i=0;i<port->connections.size();i++)
            {
                if(i!=0)fprintf(ofile,",");
                fprintf(ofile,"%s.%s", port->connections[i].first->inst_name.c_str(),port->connections[i].second.c_str());
                if(t->child!=NULL) Convert(ofile,t->child);
            }
        }
        fprintf(ofile,")");
        if(t->sibling!=NULL) Convert(ofile,t->sibling);
        return;
    case inport_definition:
    	port=m_class->LookupPort(t->GetID());
    	assert(port!=NULL);
    	if(!port->implemented)
    	{
    		if(t->child!=NULL) Convert(ofile,t->child);
	        if(t->sibling!=NULL) Convert(ofile,t->sibling);
    	}
    	return;
    case outport_declaration:
        iter=m_ports.find(t->GetID());
        assert(iter!=m_ports.end());
        port=(*iter).second;
        if(array||port->array)
        {
            string return_type;
            string full_list;
            
            CToken* token;
            token=port->return_type_begin;
            while(token!=port->return_type_end)
            {
                if(token->type==ARGID)
                {
                    return_type += ReplaceID(token->GetID());
                    return_type += " ";
                }
                else
                    return_type += token->text;
                token=token->sibling;
            }

            token=port->full_list_begin;
            while(token!=port->full_list_end)
            {
                if(token->type==ARGID)
                {
                    full_list += ReplaceID(token->GetID());
                    full_list += " ";
                }
                else
                    full_list += token->text;
                token=token->sibling;
            }

            fprintf(ofile, "class my_%s:public compcxx_functor<%s>{ public:",port->functor_type.c_str(),port->functor_type.c_str());
            fprintf(ofile, "%s operator() %s { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)",return_type.c_str(),full_list.c_str());
            fprintf(ofile, "(c[compcxx_i]->*f[compcxx_i])%s; return (c[0]->*f[0])%s;};};", port->arg_list.c_str(), port->arg_list.c_str());
            if(port->array)
                fprintf(ofile, "compcxx_array<my_%s > %s;", port->functor_type.c_str(), port->functor_name.c_str());
            else
                fprintf(ofile, "my_%s %s;", port->functor_type.c_str(), port->functor_name.c_str());

            g_header += string("typedef ") + return_type + " (compcxx_component::*" + port->functor_type;
            g_header += string(")") + full_list + ";\n";
        }
        break;
    case component_declaration:
        for(i=0;i<m_peers.size();i++)
        {
            if(m_peers[i]!=NULL)
                fprintf(ofile, "class %s;",m_peers[i]->class_name.c_str());
            else
                fprintf(ofile, "class %s;",parent->class_name.c_str());
        }
        break;
    case ARGID:
        fprintf(ofile,"%s ",ReplaceID(t->GetID()).c_str());
        //printf("convert %s to %s\n",t->GetID().c_str(), ReplaceID(t->GetID()).c_str());
        break;
    default:
        fprintf(ofile,"%s", t->text.c_str());
        break;
    }
    if(t->child!=NULL) Convert(ofile,t->child);
    if(t->sibling!=NULL) Convert(ofile,t->sibling);
}

string ComponentInstance::ReplaceID(const string& id)
{
    unsigned int i;
    for(i=0;i<m_class->tmpl_args.size();i++)
    {
        if(id==m_class->tmpl_args[i])break;
    }
    assert(i<m_class->tmpl_args.size());
    return tmpl_values[i];
}
