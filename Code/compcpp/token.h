#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <vector>
#include <map>
#include <string>
using std::string;

#define NONE
#define OUTPORT 301
#define INPORT 302
#define COMPONENT 303
#define CONNECT 304
#define TEMPLATE 305
#define VARID 306
#define UNKNOWN 307
#define COMPID 308
#define PORTID 309
#define MAINID 310
#define SCOPE 311
#define ARROW 312
#define ARROWSTAR 313
#define DOTSTAR 314
#define CLASS 315
#define ARRAY 316
#define ARGID 317
#define FSPECIFIER 318
#define TYPENAME 319

#define component_declaration 401
#define end_of_component_declaration 402
#define member_definition 403
#define function_definition 404
#define outport_declaration 405
#define inport_definition 406
#define MIN_TOKEN 300

#define COMPCXX "compcxx_"
#define MAX_STRING_LENGTH 256

extern bool g_lineinfo;
extern bool g_debug;
extern string g_header;
extern std::map<string,string> g_macros;

class CToken;
class CTranslation
{
 public:
  static CTranslation* Current() { return m_current; }
  static int ErrorCount() { return m_error_count; }
 private:
  static CTranslation* m_current;
  static std::vector<CTranslation*> m_stack;
  static std::vector<CTranslation*> m_all;
  static int m_error_count;

 public:
  void* include_stack;
  int lineno;
  string filename;
  static void New();
  static bool Delete();
  static void Clear ();
  static void Error (CToken*, int, const char*, ... );
};

class CToken
{
 public:
  CTranslation* translation;
  int type;
  int lineno;
  bool line_info;
  string text;

  CToken(CTranslation* _translation, int _type = UNKNOWN)
    : translation(_translation), type(_type), line_info(false),child(NULL), sibling(NULL), last_child(NULL)
    {
      lineno=translation->lineno;
    }

  static CToken* New(CTranslation*, int = UNKNOWN);
  static CToken* New(CToken*, int );
  static CToken* Copy(CToken*);
  string GetID();
  string ReplaceID(const char* );
  CToken* AppendChild(CToken*);
  CToken* InsertChild(CToken*);
  static CToken* Current() { return current; }
  void Push(const char*s) { text += s; }
  void Push(const char c) { text += c; }
  void Dump() { printf("#%d#%s",lineno,text.c_str()); }
  void Write(FILE*f);

  CToken *child, *sibling, *last_child;
  private:
  static CToken* current;
};

class ComponentInstance;

class CPort
{
public:
	CPort(): type(UNKNOWN), token(NULL), connected(false), 
	        bound (false), implemented (false), array(false) {}
	CPort(const string& n, int t): name(n),type(t),token(NULL), 
	        connected(false), bound (false), implemented (false), array(false) {}
	/*CPort(const CPort& port): name(port.name),type(port.type),token(port.token), 
	        connected(port.connected), bound (port.bound), array(port.array)
	{
	    printf("copy %s(%d)\n",name.c_str(),port.connections.size());
	    for(unsigned int i=0;i<port.connections.size();i++)
	    {
	        connections.push_back(port.connections[i]);
	    }
	}*/
	string name;
	int type;
	CToken* token;
	bool connected;
	bool bound;
	bool implemented;
	bool array;
	
	string functor_type;
	string functor_name;
	
	string arg_list;
	CToken *full_list_begin,*full_list_end;
	CToken *return_type_begin,*return_type_end;
	
	std::vector<std::pair<ComponentInstance*,string> > connections;
};

class ComponentInstance;

class ComponentTable
{
public:
    virtual ~ComponentTable();
	ComponentInstance* LookupComponent(const string&);
	void AddComponent(const string&, ComponentInstance*);
	virtual void GenerateCode(FILE*, ComponentInstance*, bool);
	unsigned int Size() { return m_instances.size(); }
protected:
	std::map<string, ComponentInstance*> m_instances;
};

class ComponentClass : public ComponentTable
{
 public:
 	ComponentClass(const string& n): class_name(n), composite (false) { }
 	virtual ~ComponentClass();
	static ComponentClass* New(CToken*, const string&);
	static ComponentClass* Lookup(const string& cname);
	
	CPort* AddPort(CToken*, const string& name, int t);
	CPort* LookupPort(const string& name);
	void AddToken(CToken* t) { source.push_back(t); }
	void AnalyzeTemplate(CToken*t);
	virtual void GenerateCode(FILE*, ComponentInstance*, bool);
	
	string class_name;
	std::vector<string> tmpl_args;
 	std::map<string, CPort*> ports;
 	std::vector<CToken*> source;
 	bool composite;
private:
	static std::map<string, ComponentClass*> m_classes;
};

class ComponentInstance
{
 public:
 	ComponentInstance(ComponentClass* c);
 	bool AddConnection(const string&, ComponentInstance*, const string&);
 	ComponentClass* GetClass() { return m_class; }
 	CPort* LookupPort(const string& name);
 	void GenerateCode(FILE*, bool);
    void Convert(FILE*, CToken*);
    string ReplaceID(const string& id);
    
    string parameter;
 	string class_name;
 	string inst_name;
 	std::vector<string> tmpl_values;
 	bool array;

    ComponentInstance* parent;
 private:
 	std::map<string, CPort*> m_ports;
 	std::vector<ComponentInstance*> m_peers;
 	ComponentClass* m_class;
 	static int tmp_number;
};
