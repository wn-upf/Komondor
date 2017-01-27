
#line 1 "Komondor.cc"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#line 1 "./COST/cost.h"

























#ifndef queue_t
#define queue_t SimpleQueue
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <deque>
#include <vector>
#include <assert.h>


#line 1 "./COST/priority_q.h"























#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include <stdio.h>
#include <string.h>














template < class ITEM >
class SimpleQueue 
{
 public:
  SimpleQueue() :m_head(NULL) {};
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  void Delete(ITEM*);
  ITEM* NextEvent() const { return m_head; };
  const char* GetName();
 protected:
  ITEM* m_head;
};

template <class ITEM>
const char* SimpleQueue<ITEM>::GetName()
{
  static const char* name = "SimpleQueue";
  return name;
}

template <class ITEM>
void SimpleQueue<ITEM>::EnQueue(ITEM* item)
{
  if( m_head==NULL || item->time < m_head->time )
  {
    if(m_head!=NULL)m_head->prev=item;
    item->next=m_head;
    m_head=item;
    item->prev=NULL;
    return;
  }
    
  ITEM* i=m_head;
  while( i->next!=NULL && item->time > i->next->time)
    i=i->next;
  item->next=i->next;
  if(i->next!=NULL)i->next->prev=item;
  i->next=item;
  item->prev=i;

}

template <class ITEM>
ITEM* SimpleQueue<ITEM> ::DeQueue()
{
  if(m_head==NULL)return NULL;
  ITEM* item = m_head;
  m_head=m_head->next;
  if(m_head!=NULL)m_head->prev=NULL;
  return item;
}

template <class ITEM>
void SimpleQueue<ITEM>::Delete(ITEM* item)
{
  if(item==NULL) return;

  if(item==m_head)
  {
    m_head=m_head->next;
    if(m_head!=NULL)m_head->prev=NULL;
  }
  else
  {
    item->prev->next=item->next;
    if(item->next!=NULL)
      item->next->prev=item->prev;
  }

}

template <class ITEM>
class GuardedQueue : public SimpleQueue<ITEM>
{
 public:
  void Delete(ITEM*);
  void EnQueue(ITEM*);
  bool Validate(const char*);
};
template <class ITEM>
void GuardedQueue<ITEM>::EnQueue(ITEM* item)
{

  ITEM* i=SimpleQueue<ITEM>::m_head;
  while(i!=NULL)
  {
    if(i==item)
    {
      pthread_printf("queue error: item %f(%p) is already in the queue\n",item->time,item);
    }
    i=i->next;
  }
  SimpleQueue<ITEM>::EnQueue(item);
}

template <class ITEM>
void GuardedQueue<ITEM>::Delete(ITEM* item)
{
  ITEM* i=SimpleQueue<ITEM>::m_head;
  while(i!=item&&i!=NULL)
    i=i->next;
  if(i==NULL)
    pthread_printf("error: cannot find the to-be-deleted event %f(%p)\n",item->time,item);
  else
    SimpleQueue<ITEM>::Delete(item);
}

template <class ITEM>
bool GuardedQueue<ITEM>::Validate(const char* s)
{
  char out[1000],buff[100];

  ITEM* i=SimpleQueue<ITEM>::m_head;
  bool qerror=false;

  sprintf(out,"queue error %s : ",s);
  while(i!=NULL)
  {
    sprintf(buff,"%f ",i->time);
    strcat(out,buff);
    if(i->next!=NULL)
      if(i->next->prev!=i)
      {
	qerror=true;
	sprintf(buff," {broken} ");
	strcat(out,buff);
      }
    if(i==i->next)
    {
      qerror=true;
      sprintf(buff,"{loop}");
      strcat(out,buff);
      break;
    }
    i=i->next;
  }
  if(qerror)
    printf("%s\n",out);
  return qerror;
}

template <class ITEM>
class ErrorQueue : public SimpleQueue<ITEM>
{
 public:
  ITEM* DeQueue(double);
  const char* GetName();
};

template <class ITEM>
const char* ErrorQueue<ITEM>::GetName()
{
  static const char* name = "ErrorQueue";
  return name;
}

template <class ITEM>
ITEM* ErrorQueue<ITEM> ::DeQueue(double stoptime)
{
  

  if(drand48()>0.5)
    return SimpleQueue<ITEM>::DeQueue();

  int s=0;
  ITEM* e;
  e=SimpleQueue<ITEM>::m_head;
  while(e!=NULL&&e->time<stoptime)
  {
    s++;
    e=e->next;
  }
  e=SimpleQueue<ITEM>::m_head;
  s=(int)(s*drand48());
  while(s!=0)
  {
    e=e->next;
    s--;
  }
  Delete(e);
  return e;
}

template < class ITEM >
class HeapQueue 
{
 public:
  HeapQueue();
  ~HeapQueue();
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  void Delete(ITEM*);
  const char* GetName();
  ITEM* NextEvent() const { return num_of_elems?elems[0]:NULL; };
 private:
  void SiftDown(int);
  void PercolateUp(int);
  void Validate(const char*);
        
  ITEM** elems;
  int num_of_elems;
  int curr_max;
};

template <class ITEM>
const char* HeapQueue<ITEM>::GetName()
{
  static const char* name = "HeapQueue";
  return name;
}

template <class ITEM>
void HeapQueue<ITEM>::Validate(const char* s)
{
  int i,j;
  char out[1000],buff[100];
  for(i=0;i<num_of_elems;i++)
    if(  ((2*i+1)<num_of_elems&&elems[i]->time>elems[2*i+1]->time) ||
	 ((2*i+2)<num_of_elems&&elems[i]->time>elems[2*i+2]->time) )
    {
      sprintf(out,"queue error %s : ",s);
      for(j=0;j<num_of_elems;j++)
      {
	if(i!=j)
	  sprintf(buff,"%f(%d) ",elems[j]->time,j);
	else
	  sprintf(buff,"{%f(%d)} ",elems[j]->time,j);
	strcat(out,buff);
      }
      printf("%s\n",out);
    }
}
template <class ITEM>
HeapQueue<ITEM>::HeapQueue()
{
  curr_max=16;
  elems=new ITEM*[curr_max];
  num_of_elems=0;
}
template <class ITEM>
HeapQueue<ITEM>::~HeapQueue()
{
  delete [] elems;
}
template <class ITEM>
void HeapQueue<ITEM>::SiftDown(int node)
{
  if(num_of_elems<=1) return;
  int i=node,k,c1,c2;
  ITEM* temp;
        
  do{
    k=i;
    c1=c2=2*i+1;
    c2++;
    if(c1<num_of_elems && elems[c1]->time < elems[i]->time)
      i=c1;
    if(c2<num_of_elems && elems[c2]->time < elems[i]->time)
      i=c2;
    if(k!=i)
    {
      temp=elems[i];
      elems[i]=elems[k];
      elems[k]=temp;
      elems[k]->pos=k;
      elems[i]->pos=i;
    }
  }while(k!=i);
}
template <class ITEM>
void HeapQueue<ITEM>::PercolateUp(int node)
{
  int i=node,k,p;
  ITEM* temp;
        
  do{
    k=i;
    if( (p=(i+1)/2) != 0)
    {
      --p;
      if(elems[i]->time < elems[p]->time)
      {
	i=p;
	temp=elems[i];
	elems[i]=elems[k];
	elems[k]=temp;
	elems[k]->pos=k;
	elems[i]->pos=i;
      }
    }
  }while(k!=i);
}

template <class ITEM>
void HeapQueue<ITEM>::EnQueue(ITEM* item)
{
  if(num_of_elems>=curr_max)
  {
    curr_max*=2;
    ITEM** buffer=new ITEM*[curr_max];
    for(int i=0;i<num_of_elems;i++)
      buffer[i]=elems[i];
    delete[] elems;
    elems=buffer;
  }
        
  elems[num_of_elems]=item;
  elems[num_of_elems]->pos=num_of_elems;
  num_of_elems++;
  PercolateUp(num_of_elems-1);
}

template <class ITEM>
ITEM* HeapQueue<ITEM>::DeQueue()
{
  if(num_of_elems<=0)return NULL;
        
  ITEM* item=elems[0];
  num_of_elems--;
  elems[0]=elems[num_of_elems];
  elems[0]->pos=0;
  SiftDown(0);
  return item;
}

template <class ITEM>
void HeapQueue<ITEM>::Delete(ITEM* item)
{
  int i=item->pos;

  num_of_elems--;
  elems[i]=elems[num_of_elems];
  elems[i]->pos=i;
  SiftDown(i);
  PercolateUp(i);
}



#define CQ_MAX_SAMPLES 25

template <class ITEM>
class CalendarQueue 
{
 public:
  CalendarQueue();
  const char* GetName();
  ~CalendarQueue();
  void enqueue(ITEM*);
  ITEM* dequeue();
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  ITEM* NextEvent() const { return m_head;}
  void Delete(ITEM*);
 private:
  long last_bucket,number_of_buckets;
  double bucket_width;
        
  void ReSize(long);
  double NewWidth();

  ITEM ** buckets;
  long total_number;
  double bucket_top;
  long bottom_threshold;
  long top_threshold;
  double last_priority;
  bool resizable;

  ITEM* m_head;
  char m_name[100];
};


template <class ITEM>
const char* CalendarQueue<ITEM> :: GetName()
{
  sprintf(m_name,"Calendar Queue (bucket width: %.2e, size: %ld) ",
	  bucket_width,number_of_buckets);
  return m_name;
}
template <class ITEM>
CalendarQueue<ITEM>::CalendarQueue()
{
  long i;
        
  number_of_buckets=16;
  bucket_width=1.0;
  bucket_top=bucket_width;
  total_number=0;
  last_bucket=0;
  last_priority=0.0;
  top_threshold=number_of_buckets*2;
  bottom_threshold=number_of_buckets/2-2;
  resizable=true;
        
  buckets= new ITEM*[number_of_buckets];
  for(i=0;i<number_of_buckets;i++)
    buckets[i]=NULL;
  m_head=NULL;

}
template <class ITEM>
CalendarQueue<ITEM>::~CalendarQueue()
{
  delete [] buckets;
}
template <class ITEM>
void CalendarQueue<ITEM>::ReSize(long newsize)
{
  long i;
  ITEM** old_buckets=buckets;
  long old_number=number_of_buckets;
        
  resizable=false;
  bucket_width=NewWidth();
  buckets= new ITEM*[newsize];
  number_of_buckets=newsize;
  for(i=0;i<newsize;i++)
    buckets[i]=NULL;
  last_bucket=0;
  total_number=0;

  
        
  ITEM *item;
  for(i=0;i<old_number;i++)
  {
    while(old_buckets[i]!=NULL)
    {
      item=old_buckets[i];
      old_buckets[i]=item->next;
      enqueue(item);
    }
  }
  resizable=true;
  delete[] old_buckets;
  number_of_buckets=newsize;
  top_threshold=number_of_buckets*2;
  bottom_threshold=number_of_buckets/2-2;
  bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
  last_bucket = long(last_priority/bucket_width) % number_of_buckets;

}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::DeQueue()
{
  ITEM* head=m_head;
  m_head=dequeue();
  return head;
}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::dequeue()
{
  long i;
  for(i=last_bucket;;)
  {
    if(buckets[i]!=NULL&&buckets[i]->time<bucket_top)
    {
      ITEM * item=buckets[i];
      buckets[i]=buckets[i]->next;
      total_number--;
      last_bucket=i;
      last_priority=item->time;
                        
      if(resizable&&total_number<bottom_threshold)
	ReSize(number_of_buckets/2);
      item->next=NULL;
      return item;
    }
    else
    {
      i++;
      if(i==number_of_buckets)i=0;
      bucket_top+=bucket_width;
      if(i==last_bucket)
	break;
    }
  }
        
  
  long smallest;
  for(smallest=0;smallest<number_of_buckets;smallest++)
    if(buckets[smallest]!=NULL)break;

  if(smallest >= number_of_buckets)
  {
    last_priority=bucket_top;
    return NULL;
  }

  for(i=smallest+1;i<number_of_buckets;i++)
  {
    if(buckets[i]==NULL)
      continue;
    else
      if(buckets[i]->time<buckets[smallest]->time)
	smallest=i;
  }
  ITEM * item=buckets[smallest];
  buckets[smallest]=buckets[smallest]->next;
  total_number--;
  last_bucket=smallest;
  last_priority=item->time;
  bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
  item->next=NULL;
  return item;
}
template <class ITEM>
void CalendarQueue<ITEM>::EnQueue(ITEM* item)
{
  
  if(m_head==NULL)
  {
    m_head=item;
    return;
  }
  if(m_head->time>item->time)
  {
    enqueue(m_head);
    m_head=item;
  }
  else
    enqueue(item);
}
template <class ITEM>
void CalendarQueue<ITEM>::enqueue(ITEM* item)
{
  long i;
  if(item->time<last_priority)
  {
    i=(long)(item->time/bucket_width);
    last_priority=item->time;
    bucket_top=bucket_width*(i+1)+bucket_width*0.5;
    i=i%number_of_buckets;
    last_bucket=i;
  }
  else
  {
    i=(long)(item->time/bucket_width);
    i=i%number_of_buckets;
  }

        
  

  if(buckets[i]==NULL||item->time<buckets[i]->time)
  {
    item->next=buckets[i];
    buckets[i]=item;
  }
  else
  {

    ITEM* pos=buckets[i];
    while(pos->next!=NULL&&item->time>pos->next->time)
    {
      pos=pos->next;
    }
    item->next=pos->next;
    pos->next=item;
  }
  total_number++;
  if(resizable&&total_number>top_threshold)
    ReSize(number_of_buckets*2);
}
template <class ITEM>
void CalendarQueue<ITEM>::Delete(ITEM* item)
{
  if(item==m_head)
  {
    m_head=dequeue();
    return;
  }
  long j;
  j=(long)(item->time/bucket_width);
  j=j%number_of_buckets;
        
  

  
  

  ITEM** p = &buckets[j];
  
  ITEM* i=buckets[j];
    
  while(i!=NULL)
  {
    if(i==item)
    { 
      (*p)=item->next;
      total_number--;
      if(resizable&&total_number<bottom_threshold)
	ReSize(number_of_buckets/2);
      return;
    }
    p=&(i->next);
    i=i->next;
  }   
}
template <class ITEM>
double CalendarQueue<ITEM>::NewWidth()
{
  long i, nsamples;
        
  if(total_number<2) return 1.0;
  if(total_number<=5)
    nsamples=total_number;
  else
    nsamples=5+total_number/10;
  if(nsamples>CQ_MAX_SAMPLES) nsamples=CQ_MAX_SAMPLES;
        
  long _last_bucket=last_bucket;
  double _bucket_top=bucket_top;
  double _last_priority=last_priority;
        
  double AVG[CQ_MAX_SAMPLES],avg1=0,avg2=0;
  ITEM* list,*next,*item;
        
  list=dequeue(); 
  long real_samples=0;
  while(real_samples<nsamples)
  {
    item=dequeue();
    if(item==NULL)
    {
      item=list;
      while(item!=NULL)
      {
	next=item->next;
	enqueue(item);
	item=next;      
      }

      last_bucket=_last_bucket;
      bucket_top=_bucket_top;
      last_priority=_last_priority;

                        
      return 1.0;
    }
    AVG[real_samples]=item->time-list->time;
    avg1+=AVG[real_samples];
    if(AVG[real_samples]!=0.0)
      real_samples++;
    item->next=list;
    list=item;
  }
  item=list;
  while(item!=NULL)
  {
    next=item->next;
    enqueue(item);
    item=next;      
  }
        
  last_bucket=_last_bucket;
  bucket_top=_bucket_top;
  last_priority=_last_priority;
        
  avg1=avg1/(double)(real_samples-1);
  avg1=avg1*2.0;
        
  
  long count=0;
  for(i=0;i<real_samples-1;i++)
  {
    if(AVG[i]<avg1&&AVG[i]!=0)
    {
      avg2+=AVG[i];
      count++;
    }
  }
  if(count==0||avg2==0)   return 1.0;
        
  avg2 /= (double) count;
  avg2 *= 3.0;
        
  return avg2;
}

#endif /*PRIORITY_QUEUE_H*/

#line 38 "./COST/cost.h"


#line 1 "./COST/corsa_alloc.h"
































#ifndef corsa_allocator_h
#define corsa_allocator_h

#include <typeinfo>
#include <string>

class CorsaAllocator
{
private:
    struct DT{
#ifdef CORSA_DEBUG
	DT* self;
#endif
	DT* next;
    };
public:
    CorsaAllocator(unsigned int );         
    CorsaAllocator(unsigned int, int);     
    ~CorsaAllocator();		
    void *alloc();		
    void free(void*);
    unsigned int datasize() 
    {
#ifdef CORSA_DEBUG
	return m_datasize-sizeof(DT*);
#else
	return m_datasize; 
#endif
    }
    int size() { return m_size; }
    int capacity() { return m_capacity; }			
    
    const char* GetName() { return m_name.c_str(); }
    void SetName( const char* name) { m_name=name; } 

private:
    CorsaAllocator(const CorsaAllocator& ) {}  
    void Setup(unsigned int,int); 
    void InitSegment(int);
  
    unsigned int m_datasize;
    char** m_segments;	          
    int m_segment_number;         
    int m_segment_max;      
    int m_segment_size;	          
				  
    DT* m_free_list; 
    int m_size;
    int m_capacity;

    int m_free_times,m_alloc_times;
    int m_max_allocs;

    std::string m_name;
};
#ifndef CORSA_NODEF
CorsaAllocator::CorsaAllocator(unsigned int datasize)
{
    Setup(datasize,256);	  
}

CorsaAllocator::CorsaAllocator(unsigned int datasize, int segsize)
{
    Setup(datasize,segsize);
}

CorsaAllocator::~CorsaAllocator()
{
    #ifdef CORSA_DEBUG
    printf("%s -- alloc: %d, free: %d, max: %d\n",GetName(),
	   m_alloc_times,m_free_times,m_max_allocs);
    #endif

    for(int i=0;i<m_segment_number;i++)
	delete[] m_segments[i];	   
    delete[] m_segments;			
}

void CorsaAllocator::Setup(unsigned int datasize,int seg_size)
{

    char buffer[50];
    sprintf(buffer,"%s[%d]",typeid(*this).name(),datasize);
    m_name = buffer;

#ifdef CORSA_DEBUG
    datasize+=sizeof(DT*);  
#endif

    if(datasize<sizeof(DT))datasize=sizeof(DT);
    m_datasize=datasize;
    if(seg_size<16)seg_size=16;    
    m_segment_size=seg_size;			
    m_segment_number=1;		   
    m_segment_max=seg_size;	   
    m_segments= new char* [ m_segment_max ] ;   
    m_segments[0]= new char [m_segment_size*m_datasize];  

    m_size=0;
    m_capacity=0;
    InitSegment(0);

    m_free_times=m_alloc_times=m_max_allocs=00;
}

void CorsaAllocator::InitSegment(int s)
{
    char* p=m_segments[s];
    m_free_list=reinterpret_cast<DT*>(p);
    for(int i=0;i<m_segment_size-1;i++,p+=m_datasize)
    {
	reinterpret_cast<DT*>(p)->next=
	    reinterpret_cast<DT*>(p+m_datasize);
    }
    reinterpret_cast<DT*>(p)->next=NULL;
    m_capacity+=m_segment_size;
}

void* CorsaAllocator::alloc()
{
    #ifdef CORSA_DEBUG
    m_alloc_times++;
    if(m_alloc_times-m_free_times>m_max_allocs)
	m_max_allocs=m_alloc_times-m_free_times;
    #endif
    if(m_free_list==NULL)	
    
    {
	int i;
	if(m_segment_number==m_segment_max)	
	
	
	{
	    m_segment_max*=2;		
	    char** buff;
	    buff=new char* [m_segment_max];   
#ifdef CORSA_DEBUG
	    if(buff==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	    for(i=0;i<m_segment_number;i++)
		buff[i]=m_segments[i];	
	    delete [] m_segments;		
	    m_segments=buff;
	}
	m_segment_size*=2;
	m_segments[m_segment_number]=new char[m_segment_size*m_datasize];
#ifdef CORSA_DEBUG
	    if(m_segments[m_segment_number]==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	InitSegment(m_segment_number);
	m_segment_number++;
    }

    DT* item=m_free_list;		
    m_free_list=m_free_list->next;
    m_size++;

#ifdef CORSA_DEBUG
    item->self=item;
    char* p=reinterpret_cast<char*>(item);
    p+=sizeof(DT*);
    
    return static_cast<void*>(p);
#else
    return static_cast<void*>(item);
#endif
}

void CorsaAllocator::free(void* data)
{
#ifdef CORSA_DEBUG
    m_free_times++;
    char* p=static_cast<char*>(data);
    p-=sizeof(DT*);
    DT* item=reinterpret_cast<DT*>(p);
    
    if(item!=item->self)
    {
	if(item->self==(DT*)0xabcd1234)
	    printf("%s: packet at %p has already been released\n",GetName(),p+sizeof(DT*)); 
	else
	    printf("%s: %p is probably not a pointer to a packet\n",GetName(),p+sizeof(DT*));
    }
    assert(item==item->self);
    item->self=(DT*)0xabcd1234;
#else
    DT* item=static_cast<DT*>(data);
#endif

    item->next=m_free_list;
    m_free_list=item;
    m_size--;
}
#endif /* CORSA_NODEF */

#endif /* corsa_allocator_h */

#line 39 "./COST/cost.h"


class trigger_t {};
typedef double simtime_t;

#ifdef COST_DEBUG
#define Printf(x) Print x
#else
#define Printf(x)
#endif



class TimerBase;



struct CostEvent
{
  double time;
  CostEvent* next;
  union {
    CostEvent* prev;
    int pos;  
  };
  TimerBase* object;
  int index;
  unsigned char active;
};



class TimerBase
{
 public:
  virtual void activate(CostEvent*) = 0;
  inline virtual ~TimerBase() {}	
};

class TypeII;



class CostSimEng
{
 public:

  class seed_t
      {
       public:
	void operator = (long seed) { srand48(seed); };
      };
  seed_t		Seed;
  CostSimEng()
      : stopTime( 0), clearStatsTime( 0), m_clock( 0.0)
      {
        if( m_instance == NULL)
	  m_instance = this;
        else
	  printf("Error: only one simulation engine can be created\n");
      }
  virtual		~CostSimEng()	{ }
  static CostSimEng	*Instance()
      {
        if(m_instance==NULL)
        {
	  printf("Error: a simulation engine has not been initialized\n");
	  m_instance = new CostSimEng;
        }
        return m_instance;
      }
  CorsaAllocator	*GetAllocator(unsigned int datasize)
      {
    	for(unsigned int i=0;i<m_allocators.size();i++)
    	{
	  if(m_allocators[i]->datasize()==datasize)return m_allocators[i];
    	} 
    	CorsaAllocator* allocator=new CorsaAllocator(datasize);
    	char buffer[25];
    	sprintf(buffer,"EventAllocator[%d]",datasize);
    	allocator->SetName(buffer);
    	m_allocators.push_back(allocator);
    	return allocator;
      }
  void		AddComponent(TypeII*c)
      {
        m_components.push_back(c);
      }
  void		ScheduleEvent(CostEvent*e)
      {
	if( e->time < m_clock)
	  assert(e->time>=m_clock);
        
        m_queue.EnQueue(e);
      }
  void		CancelEvent(CostEvent*e)
      {
        
        m_queue.Delete(e);
      }
  double	Random( double v=1.0)	{ return v*drand48();}
  int		Random( int v)		{ return (int)(v*drand48()); }
  double	Exponential(double mean)	{ return -mean*log(Random());}
  virtual void	Start()		{}
  virtual void	Stop()		{}
  void		Run();
  double	SimTime()	{ return m_clock; } 
  void		StopTime( double t)	{ stopTime = t; }
  double	StopTime() const	{ return stopTime; }
  void		ClearStatsTime( double t)	{ clearStatsTime = t; }
  double	ClearStatsTime() const	{ return clearStatsTime; }
  virtual void	ClearStats()	{}
 private:
  double	stopTime;
  double	clearStatsTime;	
  double	eventRate;
  double	runningTime;
  long		eventsProcessed;
  double	m_clock;
  queue_t<CostEvent>	m_queue;
  std::vector<TypeII*>	m_components;
  static CostSimEng	*m_instance;
  std::vector<CorsaAllocator*>	m_allocators;
};




class TypeII
{
 public: 
  virtual void Start() {};
  virtual void Stop() {};
  inline virtual ~TypeII() {}		
  TypeII()
      {
        m_simeng=CostSimEng::Instance();
        m_simeng->AddComponent(this);
      }

#ifdef COST_DEBUG
  void Print(const bool, const char*, ...);
#endif
    
  double Random(double v=1.0) { return v*drand48();}
  int Random(int v) { return (int)(v*drand48());}
  double Exponential(double mean) { return -mean*log(Random());}
  inline double SimTime() const { return m_simeng->SimTime(); }
  inline double StopTime() const { return m_simeng->StopTime(); }
 private:
  CostSimEng* m_simeng;
}; 

#ifdef COST_DEBUG
void TypeII::Print(const bool flag, const char* format, ...)
{
  if(flag==false) return;
  va_list ap;
  va_start(ap, format);
  printf("[%.10f] ",SimTime());
  vprintf(format,ap);
  va_end(ap);
}
#endif

CostSimEng* CostSimEng::m_instance = NULL;

void CostSimEng::Run()
{
  double	nextTime = (clearStatsTime != 0.0 && clearStatsTime < stopTime) ? clearStatsTime : stopTime;

  m_clock = 0.0;
  eventsProcessed = 0l;
  std::vector<TypeII*>::iterator iter;
      
  struct timeval start_time;    
  gettimeofday( &start_time, NULL);

  Start();

  for( iter = m_components.begin(); iter != m_components.end(); iter++)
    (*iter)->Start();

  CostEvent* e=m_queue.DeQueue();
  while( e != NULL)
  {
    if( e->time >= nextTime)
    {
      if( nextTime == stopTime)
	break;
      
      printf( "Clearing statistics @ %f\n", nextTime);
      nextTime = stopTime;
      ClearStats();
    }
    
    assert( e->time >= m_clock);
    m_clock = e->time;
    e->object->activate( e);
    eventsProcessed++;
    e = m_queue.DeQueue();
  }
  m_clock = stopTime;
  for(iter = m_components.begin(); iter != m_components.end(); iter++)
    (*iter)->Stop();
	    
  Stop();

  struct timeval stop_time;    
  gettimeofday(&stop_time,NULL);

  runningTime = stop_time.tv_sec - start_time.tv_sec +
      (stop_time.tv_usec - start_time.tv_usec) / 1000000.0;
  eventRate = eventsProcessed/runningTime;
  
  
  printf("# -------------------------------------------------------------------------\n");	
  printf("# CostSimEng with %s, stopped at %f\n", m_queue.GetName(), stopTime);	
  printf("# %ld events processed in %.3f seconds, event processing rate: %.0f\n",	
  eventsProcessed, runningTime, eventRate);
  
}







#line 6 "Komondor.cc"



#line 1 "Node.h"
#include <math.h>
#include <algorithm>


#line 1 "structures/Notification.h"
#ifndef _AUX_
#define _AUX_


struct TxInfo
{
	int packet_id;			
	int destination_id;		
	double tx_duration;		
	double tx_power;		
	int x;					
	int y;					
	int z;					

	void printTxInfo(void){
		printf("packet_id = %d - destination_id = %d - tx_duration = %f - tx_power = %f - position = (%d, %d, %d)\n",
				packet_id, destination_id, tx_duration, tx_power, x, y, z);
	}
};


struct Notification
{
	
	int source_id;		
	int left_channel;	
	int right_channel;	
	int packet_length;	

	
	TxInfo tx_info;

	void printNotification(void){
		printf("source_id = %d - left_channel = %d - right_channel = %d - packet_length = %d -",
				source_id, left_channel, right_channel, packet_length);
		printf("tx_info: ");
		tx_info.printTxInfo();
	}
};

#endif

#line 4 "Node.h"


#line 1 "structures/NACK.h"
#ifndef _AUX1_
#define _AUX1_


struct NackInfo
{
	int source_id;			
	int packet_id;			
	int loss_reason;			
	






	int node_id_a;			
	int node_id_b;			

	void printNackInfo(void){
		printf("source_id = %d - packet_id = %d - loss_reason = %d - node_id_a = %d - node_id_b = %d\n",
				source_id, packet_id, loss_reason, node_id_a, node_id_b);
	}
};

#endif

#line 5 "Node.h"


#line 1 "list_of_defines.h"

#define T_SLOT 			0.000009



#define STATE_UNKNOWN	-1	// Unkwnown state (e.g. at the beginning of the simulation)
#define STATE_SENSING	0	// Sensing the channel and decreasing backoff counter when possible according CCA requirements
#define STATE_TX		1	// Transmitting. Transmission are finished after the transmission duration established.
#define STATE_RX		2	// Receiving a packet from other node.


#define PRINT_LOG		0	// Print logs per console
#define WRITE_LOG		1	// Write log in file


#define TX_INITIATED	0	// Transmission is initiated ('inportSomeNodeStartTX()')
#define TX_FINISHED		1	// Transmission is finished ('inportSomeNodeFinishTX()')


#define PACKET_NOT_LOST					-1	// Packet is not lost
#define PACKET_LOST_DESTINATION_TX		0	// Destination was already transmitting when the transmission was attempted
#define PACKET_LOST_LOW_SIGNAL			1	// Signal strenght is not enough (< CCA) to be decoded
#define PACKET_LOST_INTERFERENCE		2	// There are interference signals greater than cca (collision)
#define PACKET_LOST_PURE_COLLISION		3	// Two nodes transmitting to same destination with signal strengths enough to be decoded
#define PACKET_LOST_LOW_SIGNAL_AND_RX	4	// Destination already receiving and new signal strength was not enough to be decoded


#define NO_NODE_ID	-1


#define NO_PACKET_ID	-1
#define MAX_PACKET_ID	645210	// Maybe some day it is needed :D


#define PAUSE_TIMER		0	// Try to pause a timer (e.g. backoff)
#define RESUME_TIMER	1	// Try to resume timer


#define CCA_NOT_EXCEEDED	0	// CCA is not exceeded (primary channel is free)
#define CCA_EXCEEDED		1	// CCA is exceeded


#define PICO_TO_DBM		0	// Convert pW to dBm
#define DBM_TO_PICO		1	// Convert dBm to pW
#define MILLI_TO_DBM	2	// Convert mW to dBm
#define DBM_TO_MILLI	3	// Convert dBm to mW






#define LOG_B00 "B00" 	// other option: #define LOG_B00 "B00: Start() starts"
#define LOG_B01 "B01"	// Start() ends
#define LOG_B02 "B02"
#define LOG_B03 "B03"
#define LOG_B04 "B04"
#define LOG_B05 "B05"
#define LOG_B06 "B06"
#define LOG_B07 "B07"
#define LOG_B08 "B08"
#define LOG_B09 "B09"
#define LOG_B10 "B10"
#define LOG_B11 "B11"
#define LOG_B12 "B12"
#define LOG_B13 "B13"
#define LOG_B14 "B14"
#define LOG_B15 "B15"
#define LOG_B16 "B16"


#define LOG_C00 "C00"	// Stop() starts
#define LOG_C01 "C01"	// Stop() ends
#define LOG_C02 "C02"	// Statistics: Time...
#define LOG_C03 "C03"
#define LOG_C04 "C04"
#define LOG_C05 "C05"
#define LOG_C06 "C06"
#define LOG_C07 "C07"
#define LOG_C08 "C08"
#define LOG_C09 "C09"
#define LOG_C10 "C10"
#define LOG_C11 "C11"
#define LOG_C12 "C12"
#define LOG_C13 "C13"
#define LOG_C14 "C14"
#define LOG_C15 "C15"
#define LOG_C16 "C16"


#define LOG_D00 "D00"
#define LOG_D01 "D01"
#define LOG_D02 "D02"
#define LOG_D03 "D03"
#define LOG_D04 "D04"
#define LOG_D05 "D05"
#define LOG_D06 "D06"
#define LOG_D07 "D07"
#define LOG_D08 "D08"
#define LOG_D09 "D09"
#define LOG_D10 "D10"
#define LOG_D11 "D11"
#define LOG_D12 "D12"
#define LOG_D13 "D13"
#define LOG_D14 "D14"
#define LOG_D15 "D15"
#define LOG_D16 "D16"


#define LOG_E00 "E00"
#define LOG_E01 "E01"
#define LOG_E02 "E02"
#define LOG_E03 "E03"
#define LOG_E04 "E04"
#define LOG_E05 "E05"
#define LOG_E06 "E06"
#define LOG_E07 "E07"
#define LOG_E08 "E08"
#define LOG_E09 "E09"
#define LOG_E10 "E10"
#define LOG_E11 "E11"
#define LOG_E12 "E12"
#define LOG_E13 "E13"
#define LOG_E14 "E14"
#define LOG_E15 "E15"
#define LOG_E16 "E16"


#define LOG_F00 "F00"
#define LOG_F01 "F01"
#define LOG_F02 "F02"
#define LOG_F03 "F03"
#define LOG_F04 "F04"
#define LOG_F05 "F05"
#define LOG_F06 "F06"
#define LOG_F07 "F07"
#define LOG_F08 "F08"
#define LOG_F09 "F09"
#define LOG_F10 "F10"
#define LOG_F11 "F11"
#define LOG_F12 "F12"
#define LOG_F13 "F13"
#define LOG_F14 "F14"
#define LOG_F15 "F15"
#define LOG_F16 "F16"


#define LOG_G00 "G00"
#define LOG_G01 "G01"
#define LOG_G02 "G02"
#define LOG_G03 "G03"
#define LOG_G04 "G04"
#define LOG_G05 "G05"
#define LOG_G06 "G06"
#define LOG_G07 "G07"
#define LOG_G08 "G08"
#define LOG_G09 "G09"
#define LOG_G10 "G10"
#define LOG_G11 "G11"
#define LOG_G12 "G12"
#define LOG_G13 "G13"
#define LOG_G14 "G14"
#define LOG_G15 "G15"
#define LOG_G16 "G16"


#define LOG_H00 "H00"
#define LOG_H01 "H01"
#define LOG_H02 "H02"
#define LOG_H03 "H03"
#define LOG_H04 "H04"
#define LOG_H05 "H05"
#define LOG_H06 "H06"
#define LOG_H07 "H07"
#define LOG_H08 "H08"
#define LOG_H09 "H09"
#define LOG_H10 "H10"
#define LOG_H11 "H11"
#define LOG_H12 "H12"
#define LOG_H13 "H13"
#define LOG_H14 "H14"
#define LOG_H15 "H15"
#define LOG_H16 "H16"


#define LOG_I00 "I00"
#define LOG_I01 "I01"
#define LOG_I02 "I02"
#define LOG_I03 "I03"
#define LOG_I04 "I04"
#define LOG_I05 "I05"
#define LOG_I06 "I06"
#define LOG_I07 "I07"
#define LOG_I08 "I08"
#define LOG_I09 "I09"
#define LOG_I10 "I10"
#define LOG_I11 "I11"
#define LOG_I12 "I12"
#define LOG_I13 "I13"
#define LOG_I14 "I14"
#define LOG_I15 "I15"
#define LOG_I16 "I16"


#define LOG_Z00 "Z00"	// Node info
#define LOG_Z01 "Z01"
#define LOG_Z02 "Z02"
#define LOG_Z03 "Z03"
#define LOG_Z04 "Z04"
#define LOG_Z05 "Z05"
#define LOG_Z06 "Z06"
#define LOG_Z07 "Z07"
#define LOG_Z08 "Z08"
#define LOG_Z09 "Z09"
#define LOG_Z10 "Z10"
#define LOG_Z11 "Z11"
#define LOG_Z12 "Z12"
#define LOG_Z13 "Z13"
#define LOG_Z14 "Z14"
#define LOG_Z15 "Z15"
#define LOG_Z16 "Z16"


#line 6 "Node.h"




#line 148 "Node.h"
;


#line 173 "Node.h"
;


#line 185 "Node.h"
;








#line 307 "Node.h"
;








#line 365 "Node.h"
;









#line 431 "Node.h"
;








#line 448 "Node.h"
;
















double computePowerReceived(int path_loss, int distance, double wavelength, double tx_power, int tx_gain, int rx_gain){
	double pw_received = 0;
	switch(path_loss){
		
		case 0:{
			pw_received = tx_power + tx_gain + rx_gain + 20 * log10(wavelength/(4*M_PI*distance));
			break;
		}
		
		case 1:{
			double tx_heigth = 10;		
			double rx_heigth = 10;		
			double path_loss_A = 69.55 + 26.16 * log10((3*pow(10,8))/wavelength) - 13.82 * log10(tx_heigth);
			double path_loss_B = 44.9 - 6.55 * log10(tx_heigth);
			double path_loss_E = 3.2 * pow(log10(11.7554 * rx_heigth),2) - 4.97;
			double path_loss = path_loss_A + path_loss_B * log10(distance/1000) - path_loss_E;
			pw_received = tx_power + tx_gain + rx_gain - path_loss;
			break;
		}
		
		case 2: {
			double path_loss_factor = 5;
			double shadowing = 9.5;
			double obstacles = 30;
			double walls_frequency = 5; 
			double shadowing_at_wlan = (((double) rand())/RAND_MAX)*shadowing;
			double obstacles_at_wlan = (((double) rand())/RAND_MAX)*obstacles;
			double alpha = 4.4; 
			double path_loss = path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan + (distance/walls_frequency)*obstacles_at_wlan;
			pw_received = tx_power + tx_gain - path_loss; 
			break;
		}
		
		case 3: {
			double path_loss_factor = 5;
			double shadowing = 9.5;
			double obstacles = 30;
			double walls_frequency = 5; 
			double shadowing_at_wlan = 1/2*shadowing;
			double obstacles_at_wlan = 1/2*obstacles;
			double alpha = 4.4; 
			double path_loss = path_loss_factor + 10*alpha*log10(distance) + shadowing_at_wlan + (distance/walls_frequency)*obstacles_at_wlan;
			pw_received = tx_power + tx_gain - path_loss; 
			break;
		}
		default:{
			printf("Path loss model not found!\n");
			break;
		}
	}
	return pw_received;
}







double convertPower(int conversion_type, double power){
	double converted_power;
	switch(conversion_type){

		case PICO_TO_DBM:{
			
			converted_power = 10 * log10(power * pow(10,-9));
			break;
		}

		case DBM_TO_PICO:{
			
			converted_power = pow(10,(power + 90)/10) ;
			break;
		}
		default:{
			printf("Power conversion type not found!\n");
			break;
		}
	}
	return converted_power;
}







double computeDistance(int x1, int y1, int z1, int x2, int y2, int z2){
	double distance = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
	return distance;
}








int exceededCCA(double* channel_power, int primary_channel, int cca){
	
	if(channel_power[primary_channel] > convertPower(DBM_TO_PICO, cca)){
		return 1;
	} else {
		return 0;
	}
}













#line 620 "Node.h"
int getBoundaryChannel(int position, int *channels_available, int total_channels_number){
	int left_tx_ch = 0;
	int left_tx_ch_is_set = 0;
	int right_tx_ch = 0;
	for(int c = 0; c < total_channels_number; c++){
		if(channels_available[c]){
			if(!left_tx_ch_is_set){
				left_tx_ch = c;
				left_tx_ch_is_set = 1;
			}
			if(right_tx_ch < c){
				right_tx_ch = c;
			}
		}
	}
	if(position) {
		return right_tx_ch; 
	} else {
		return left_tx_ch; 
	}
}












#line 1415 "Node.h"
void printChannelForTx(int save_node_logs, int print_location, int *channels_for_tx, int num_channels_komondor, FILE *own_log_file){
	if(print_location == 1){
		for(int c = 0; c < num_channels_komondor; c++){
			 if(save_node_logs)  fprintf(own_log_file, "%d  ", channels_for_tx[c]);
			
		}
		 if(save_node_logs)  fprintf(own_log_file, "\n");
	} else {
		printf("channels_for_tx: ");
		for(int c = 0; c < num_channels_komondor; c++){
			printf("%d  ", channels_for_tx[c]);
			
		}
		printf("\n");
	}
}





#line 8 "Komondor.cc"


#line 1 "structures/Notification.h"
#ifndef _AUX_
#define _AUX_


struct TxInfo
{
	int packet_id;			
	int destination_id;		
	double tx_duration;		
	double tx_power;		
	int x;					
	int y;					
	int z;					

	void printTxInfo(void){
		printf("packet_id = %d - destination_id = %d - tx_duration = %f - tx_power = %f - position = (%d, %d, %d)\n",
				packet_id, destination_id, tx_duration, tx_power, x, y, z);
	}
};


struct Notification
{
	
	int source_id;		
	int left_channel;	
	int right_channel;	
	int packet_length;	

	
	TxInfo tx_info;

	void printNotification(void){
		printf("source_id = %d - left_channel = %d - right_channel = %d - packet_length = %d -",
				source_id, left_channel, right_channel, packet_length);
		printf("tx_info: ");
		tx_info.printTxInfo();
	}
};

#endif

#line 9 "Komondor.cc"


#line 1 "structures/NACK.h"
#ifndef _AUX1_
#define _AUX1_


struct NackInfo
{
	int source_id;			
	int packet_id;			
	int loss_reason;			
	






	int node_id_a;			
	int node_id_b;			

	void printNackInfo(void){
		printf("source_id = %d - packet_id = %d - loss_reason = %d - node_id_a = %d - node_id_b = %d\n",
				source_id, packet_id, loss_reason, node_id_a, node_id_b);
	}
};

#endif

#line 10 "Komondor.cc"


#line 1 "list_of_defines.h"

#define T_SLOT 			0.000009



#define STATE_UNKNOWN	-1	// Unkwnown state (e.g. at the beginning of the simulation)
#define STATE_SENSING	0	// Sensing the channel and decreasing backoff counter when possible according CCA requirements
#define STATE_TX		1	// Transmitting. Transmission are finished after the transmission duration established.
#define STATE_RX		2	// Receiving a packet from other node.


#define PRINT_LOG		0	// Print logs per console
#define WRITE_LOG		1	// Write log in file


#define TX_INITIATED	0	// Transmission is initiated ('inportSomeNodeStartTX()')
#define TX_FINISHED		1	// Transmission is finished ('inportSomeNodeFinishTX()')


#define PACKET_NOT_LOST					-1	// Packet is not lost
#define PACKET_LOST_DESTINATION_TX		0	// Destination was already transmitting when the transmission was attempted
#define PACKET_LOST_LOW_SIGNAL			1	// Signal strenght is not enough (< CCA) to be decoded
#define PACKET_LOST_INTERFERENCE		2	// There are interference signals greater than cca (collision)
#define PACKET_LOST_PURE_COLLISION		3	// Two nodes transmitting to same destination with signal strengths enough to be decoded
#define PACKET_LOST_LOW_SIGNAL_AND_RX	4	// Destination already receiving and new signal strength was not enough to be decoded


#define NO_NODE_ID	-1


#define NO_PACKET_ID	-1
#define MAX_PACKET_ID	645210	// Maybe some day it is needed :D


#define PAUSE_TIMER		0	// Try to pause a timer (e.g. backoff)
#define RESUME_TIMER	1	// Try to resume timer


#define CCA_NOT_EXCEEDED	0	// CCA is not exceeded (primary channel is free)
#define CCA_EXCEEDED		1	// CCA is exceeded


#define PICO_TO_DBM		0	// Convert pW to dBm
#define DBM_TO_PICO		1	// Convert dBm to pW
#define MILLI_TO_DBM	2	// Convert mW to dBm
#define DBM_TO_MILLI	3	// Convert dBm to mW






#define LOG_B00 "B00" 	// other option: #define LOG_B00 "B00: Start() starts"
#define LOG_B01 "B01"	// Start() ends
#define LOG_B02 "B02"
#define LOG_B03 "B03"
#define LOG_B04 "B04"
#define LOG_B05 "B05"
#define LOG_B06 "B06"
#define LOG_B07 "B07"
#define LOG_B08 "B08"
#define LOG_B09 "B09"
#define LOG_B10 "B10"
#define LOG_B11 "B11"
#define LOG_B12 "B12"
#define LOG_B13 "B13"
#define LOG_B14 "B14"
#define LOG_B15 "B15"
#define LOG_B16 "B16"


#define LOG_C00 "C00"	// Stop() starts
#define LOG_C01 "C01"	// Stop() ends
#define LOG_C02 "C02"	// Statistics: Time...
#define LOG_C03 "C03"
#define LOG_C04 "C04"
#define LOG_C05 "C05"
#define LOG_C06 "C06"
#define LOG_C07 "C07"
#define LOG_C08 "C08"
#define LOG_C09 "C09"
#define LOG_C10 "C10"
#define LOG_C11 "C11"
#define LOG_C12 "C12"
#define LOG_C13 "C13"
#define LOG_C14 "C14"
#define LOG_C15 "C15"
#define LOG_C16 "C16"


#define LOG_D00 "D00"
#define LOG_D01 "D01"
#define LOG_D02 "D02"
#define LOG_D03 "D03"
#define LOG_D04 "D04"
#define LOG_D05 "D05"
#define LOG_D06 "D06"
#define LOG_D07 "D07"
#define LOG_D08 "D08"
#define LOG_D09 "D09"
#define LOG_D10 "D10"
#define LOG_D11 "D11"
#define LOG_D12 "D12"
#define LOG_D13 "D13"
#define LOG_D14 "D14"
#define LOG_D15 "D15"
#define LOG_D16 "D16"


#define LOG_E00 "E00"
#define LOG_E01 "E01"
#define LOG_E02 "E02"
#define LOG_E03 "E03"
#define LOG_E04 "E04"
#define LOG_E05 "E05"
#define LOG_E06 "E06"
#define LOG_E07 "E07"
#define LOG_E08 "E08"
#define LOG_E09 "E09"
#define LOG_E10 "E10"
#define LOG_E11 "E11"
#define LOG_E12 "E12"
#define LOG_E13 "E13"
#define LOG_E14 "E14"
#define LOG_E15 "E15"
#define LOG_E16 "E16"


#define LOG_F00 "F00"
#define LOG_F01 "F01"
#define LOG_F02 "F02"
#define LOG_F03 "F03"
#define LOG_F04 "F04"
#define LOG_F05 "F05"
#define LOG_F06 "F06"
#define LOG_F07 "F07"
#define LOG_F08 "F08"
#define LOG_F09 "F09"
#define LOG_F10 "F10"
#define LOG_F11 "F11"
#define LOG_F12 "F12"
#define LOG_F13 "F13"
#define LOG_F14 "F14"
#define LOG_F15 "F15"
#define LOG_F16 "F16"


#define LOG_G00 "G00"
#define LOG_G01 "G01"
#define LOG_G02 "G02"
#define LOG_G03 "G03"
#define LOG_G04 "G04"
#define LOG_G05 "G05"
#define LOG_G06 "G06"
#define LOG_G07 "G07"
#define LOG_G08 "G08"
#define LOG_G09 "G09"
#define LOG_G10 "G10"
#define LOG_G11 "G11"
#define LOG_G12 "G12"
#define LOG_G13 "G13"
#define LOG_G14 "G14"
#define LOG_G15 "G15"
#define LOG_G16 "G16"


#define LOG_H00 "H00"
#define LOG_H01 "H01"
#define LOG_H02 "H02"
#define LOG_H03 "H03"
#define LOG_H04 "H04"
#define LOG_H05 "H05"
#define LOG_H06 "H06"
#define LOG_H07 "H07"
#define LOG_H08 "H08"
#define LOG_H09 "H09"
#define LOG_H10 "H10"
#define LOG_H11 "H11"
#define LOG_H12 "H12"
#define LOG_H13 "H13"
#define LOG_H14 "H14"
#define LOG_H15 "H15"
#define LOG_H16 "H16"


#define LOG_I00 "I00"
#define LOG_I01 "I01"
#define LOG_I02 "I02"
#define LOG_I03 "I03"
#define LOG_I04 "I04"
#define LOG_I05 "I05"
#define LOG_I06 "I06"
#define LOG_I07 "I07"
#define LOG_I08 "I08"
#define LOG_I09 "I09"
#define LOG_I10 "I10"
#define LOG_I11 "I11"
#define LOG_I12 "I12"
#define LOG_I13 "I13"
#define LOG_I14 "I14"
#define LOG_I15 "I15"
#define LOG_I16 "I16"


#define LOG_Z00 "Z00"	// Node info
#define LOG_Z01 "Z01"
#define LOG_Z02 "Z02"
#define LOG_Z03 "Z03"
#define LOG_Z04 "Z04"
#define LOG_Z05 "Z05"
#define LOG_Z06 "Z06"
#define LOG_Z07 "Z07"
#define LOG_Z08 "Z08"
#define LOG_Z09 "Z09"
#define LOG_Z10 "Z10"
#define LOG_Z11 "Z11"
#define LOG_Z12 "Z12"
#define LOG_Z13 "Z13"
#define LOG_Z14 "Z14"
#define LOG_Z15 "Z15"
#define LOG_Z16 "Z16"


#line 11 "Komondor.cc"





#line 84 "Komondor.cc"
;


#line 88 "Komondor.cc"
;








#line 118 "Komondor.cc"
;





#line 458 "Komondor.cc"
const char* getfield(char* line, int num){
    const char* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}




int getNumOfLines(char *nodes_filename){
	int num_lines = 0;
	
	FILE* stream = fopen(nodes_filename, "r");
	if (!stream){
		printf("Nodes configuration file %s not found!\n", nodes_filename);
		exit(-1);
	}
	char line[1024];
	while (fgets(line, 1024, stream))
	{
		num_lines++;
	}
	num_lines--;
	fclose(stream);
	return num_lines;
}







#include "compcxx_Komondor.h"
class compcxx_Node_5;/*template <class T> */
#line 267 "./COST/cost.h"
class compcxx_Timer_2 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_2() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Node_5* p_compcxx_parent;};

class compcxx_Node_5;/*template <class T> */
#line 267 "./COST/cost.h"
class compcxx_Timer_4 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_4() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Node_5* p_compcxx_parent;};

class compcxx_Node_5;/*template <class T> */
#line 267 "./COST/cost.h"
class compcxx_Timer_3 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_3() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Node_5* p_compcxx_parent;};


#line 9 "Node.h"
class compcxx_Node_5 : public compcxx_component, public TypeII{
	public:
		
		void Setup();
		void Start();
		void Stop();
		void initializeVariables();
		double computeTxTime(int num_channels_used);
		void printNodeStatistics(int write_or_print);
		int isPacketLost(Notification notification);
		void sendNack(int packet_id, int node_id_a, int node_id_b, int loss_reason);
		void processNack(NackInfo nack_info);
		void cleanNack();
		void updateChannelsPower(Notification notification, int update_type);
		Notification generateNotification(int destination_id, double tx_duration);
		void getTxChannelsByChannelBonding(int channel_bonding_model, int *channels_free);
		void getChannelOccupancyByCCA();
		double computeBackoff(int pdf, double lambda);
		void handlePacketLoss();
		void computeMaxInterference(Notification notification);
		void handleBackoff(int pause_or_resume, Notification notification);
		void pauseBackoff();
		void resumeBackoff();
		void restartNode();
		void printNodeInfo(int write_or_print);
		void printChannelPower(int write_or_print);

	public:
		int node_id; 				
		int destination_id;			
		double lambda;				
		double mu;					
		int primary_channel;		
		int min_channel_allowed;	
		int max_channel_allowed;	
		double tpc_default;			
		double cca_default;			
		int x;						
		int y;						
		int z;						
		double tx_gain;				
		double rx_gain;				
		int channel_bonding_model;	
		int cochannel_model;		
		int collisions_model;		

		
		double sim_time;				
		int total_nodes_number;			
		double wavelength;				
		int num_channels_komondor;		
		int CW;							
		int pdf_backoff;				
		int pdf_tx_time;				
		int packet_length;				
		int path_loss_model;			
		int num_packets_aggregated;		
		double noise_level;				
		int save_node_logs;				
		int basic_channel_bandwidth;	
		int data_rate_array[4];			

		
		int packets_sent;
		double *total_time_transmitting_per_channel;		
		double *total_time_transmitting_in_num_channels;	
		double *total_time_lost_per_channel;				
		double *total_time_lost_in_num_channels;			
		double throughput;									
		int packets_lost;									
		int *nacks_received;								

	private:
		
		double *channel_power;				
		int *channels_free;					
		int *channels_for_tx;				

		
		FILE *own_log_file;					
		char own_file_path[32];				

		
		int node_state;						
		double remaining_backoff;			
		int progress_bar_delta;				
		int progress_bar_counter;			

		
		int current_left_channel;			
		int current_right_channel;			
		int current_tpc;					
		int current_cca;					
		int current_destination_id;			
		int num_channels_allowed;			
		double current_tx_duration;			
		int packet_id;						

		
		NackInfo nack;						
		double max_pw_interference;			
		int channel_max_interference;		
		double *power_received_per_node;	
		double pw_received_interest;		
		int receiving_from_node_id;			
		int receiving_packet_id;			
		int *hidden_nodes_list;				

	public:
		
		/*inport */void inline inportSomeNodeStartTX(Notification &notification);
		/*inport */void inline inportSomeNodeFinishTX(Notification &notification);
		/*inport */void inline inportNackReceived(NackInfo &nack_info);

		
		class my_Node_outportSelfStartTX_f_t:public compcxx_functor<Node_outportSelfStartTX_f_t>{ public:void  operator() (Notification &notification) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(notification); return (c[0]->*f[0])(notification);};};my_Node_outportSelfStartTX_f_t outportSelfStartTX_f;/*outport void outportSelfStartTX(Notification &notification)*/;
		class my_Node_outportSelfFinishTX_f_t:public compcxx_functor<Node_outportSelfFinishTX_f_t>{ public:void  operator() (Notification &notification) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(notification); return (c[0]->*f[0])(notification);};};my_Node_outportSelfFinishTX_f_t outportSelfFinishTX_f;/*outport void outportSelfFinishTX(Notification &notification)*/;
		class my_Node_outportSendNack_f_t:public compcxx_functor<Node_outportSendNack_f_t>{ public:void  operator() (NackInfo &nack_info) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(nack_info); return (c[0]->*f[0])(nack_info);};};my_Node_outportSendNack_f_t outportSendNack_f;/*outport void outportSendNack(NackInfo &nack_info)*/;

		
		compcxx_Timer_2 /*<trigger_t> */trigger_backoff; 		
		compcxx_Timer_3 /*<trigger_t> */trigger_toFinishTX; 	
		compcxx_Timer_4 /*<trigger_t> */trigger_sim_time;		

		
		/*inport */inline void endBackoff(trigger_t& t1);
		/*inport */inline void myTXFinished(trigger_t& t1);
		/*inport */inline void printProgressBar(trigger_t& t1);

		
		compcxx_Node_5 () {
			trigger_backoff.p_compcxx_parent=this /*connect trigger_backoff.to_component,*/;
			trigger_toFinishTX.p_compcxx_parent=this /*connect trigger_toFinishTX.to_component,*/;
			trigger_sim_time.p_compcxx_parent=this /*connect trigger_sim_time.to_component,*/;
		}
};


#line 15 "Komondor.cc"
class compcxx_KomondorEnvironment_6 : public compcxx_component, public CostSimEng
{
	public:
		void Setup(double sim_time, int save_node_logs, char *system_filename, char *nodes_filename, char *script_filename);
		void Stop();
		void Start();
		void inputChecker();
		void setupEnvironmentByReadingInputFile(char *system_filename);
		void printSystemInfo(int write_or_print);
		void generateNodesByReadingInputFile(char *nodes_filename, double sim_time);
		void printNodesInfo(int write_or_print);
		void readNodesFile(char *nodes_filename, double sim_time);

	public:
		compcxx_array<compcxx_Node_5 >node_container;

		double lambda;				
		double wavelength;			
		int num_channels_komondor;	
		int basic_channel_bandwidth;	
		int CW;						
		int pdf_backoff;			
		int pdf_tx_time;			
		int packet_length;				
		int path_loss_model;		
		int num_packets_aggregated;	
		double noise_level;			
		int cochannel_model;		
		int collisions_model;		
		int logs_flag;				
		int save_node_logs;			

	private:
		
		int total_nodes_number;
		FILE *logs_output_file;		
		FILE *script_file;			
};


#line 288 "./COST/cost.h"

#line 288 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 300 "./COST/cost.h"

#line 300 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 311 "./COST/cost.h"

#line 311 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 319 "./COST/cost.h"

#line 319 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->endBackoff(m_event.data));
}




#line 288 "./COST/cost.h"

#line 288 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 300 "./COST/cost.h"

#line 300 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 311 "./COST/cost.h"

#line 311 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 319 "./COST/cost.h"

#line 319 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->printProgressBar(m_event.data));
}




#line 288 "./COST/cost.h"

#line 288 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 300 "./COST/cost.h"

#line 300 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 311 "./COST/cost.h"

#line 311 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 319 "./COST/cost.h"

#line 319 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->myTXFinished(m_event.data));
}




#line 134 "Node.h"

#line 135 "Node.h"

#line 136 "Node.h"

#line 146 "Node.h"
void compcxx_Node_5 :: Setup(){
	
}
#line 150 "Node.h"
void compcxx_Node_5 :: Start(){

	
	if(save_node_logs) sprintf(own_file_path,"%s%d.txt","./output/logs_output_node_", node_id);
	if(save_node_logs) remove(own_file_path);
	if(save_node_logs) own_log_file = fopen(own_file_path, "at");

	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; Start()\n", SimTime(), node_id, STATE_UNKNOWN, LOG_B00);

	printNodeInfo(WRITE_LOG);

	initializeVariables();

	remaining_backoff = computeBackoff(pdf_backoff, lambda);
	resumeBackoff();

	
	if(node_id == 0){
		printf("PROGRESS BAR:\n");
		trigger_sim_time.Set(SimTime() + 0.000001);
	}

	
}
#line 175 "Node.h"
void compcxx_Node_5 :: Stop(){

	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    - Node Stop()\n", SimTime(), node_id, node_state, LOG_C00);

	printNodeStatistics(PRINT_LOG);
	printNodeStatistics(WRITE_LOG);

	if(save_node_logs) fclose(own_log_file);

	
}
#line 193 "Node.h"
void compcxx_Node_5 :: inportSomeNodeStartTX(Notification &notification){

	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; inportSomeNodeStartTX(): source = %d - destination = %d\n",
			SimTime(), node_id, node_state, LOG_D00, notification.source_id, notification.tx_info.destination_id);

	if(notification.source_id == node_id){ 

		if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - I have started a TX in channels: %d - %d to N%d\n",
				SimTime(), node_id, node_state, LOG_D02, notification.left_channel, notification.right_channel, notification.tx_info.destination_id);

	} else {	

		if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - N%d has started a TX in channels: %d - %d to N%d\n",
				SimTime(), node_id, node_state, LOG_D02, notification.source_id, notification.left_channel, notification.right_channel, notification.tx_info.destination_id);

		updateChannelsPower(notification, TX_INITIATED);	
		pw_received_interest = power_received_per_node[notification.source_id];
		computeMaxInterference(notification);

		
		int loss_reason;	
		switch(node_state){

			
			case 0:{

				if(notification.tx_info.destination_id == node_id){	

					if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    + I am the TX destination (N%d)\n",
							SimTime(), node_id, node_state, LOG_D07, notification.tx_info.destination_id);

					
					loss_reason = isPacketLost(notification);
					if(loss_reason != PACKET_NOT_LOST) {	

						if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;       - Reception of packet %d from %d CANNOT be started because of reason %d\n",
								SimTime(), node_id, node_state, LOG_D14, notification.tx_info.packet_id, notification.source_id, loss_reason);
						sendNack(notification.tx_info.packet_id, notification.source_id, NO_NODE_ID, loss_reason);

					} else {	

						if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;       - Reception of packet %d from %d CAN be started\n",
								SimTime(), node_id, node_state, LOG_D15, notification.tx_info.packet_id, notification.source_id);
						node_state = STATE_RX;
						receiving_from_node_id = notification.source_id;
						receiving_packet_id = notification.tx_info.packet_id;
						pauseBackoff();

					}
				} else {	

					if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    + I am NOT the TX destination (N%d)\n",
								SimTime(), node_id, node_state, LOG_D07, notification.tx_info.destination_id);
					handleBackoff(PAUSE_TIMER, notification);	
				}
				break;
			}

			
			case 1:{

				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D07; - I am in TRANSMITTING state\n",SimTime(), node_id);

				if(notification.tx_info.destination_id == node_id){ 

					if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    + I am the TX destination (N%d)\n",
												SimTime(), node_id, node_state, LOG_D07, notification.tx_info.destination_id);
					
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;D14;    + I am transmitting, packet cannot be received\n",SimTime(), node_id);
					loss_reason = PACKET_LOST_DESTINATION_TX;
					sendNack(notification.tx_info.packet_id, notification.source_id, -1, loss_reason);

				} else {	
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;D07; - I am NOT the TX destination (N%d)\n",SimTime(), node_id, notification.tx_info.destination_id);
				}
				break;
			}

			
			case 2:{

				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D07; - I am in RECEIVING state\n",SimTime(), node_id);

				if(notification.tx_info.destination_id == node_id){	
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s;    + I am the TX destination (N%d)\n",
												SimTime(), node_id, node_state, LOG_D07, notification.tx_info.destination_id);
					if(convertPower(PICO_TO_DBM,pw_received_interest) >= current_cca){
						
						if(save_node_logs) fprintf(own_log_file, "%f;N%d;D07; - I am the TX destination (N%d)\n",SimTime(),
								node_id, notification.tx_info.destination_id);
						
						loss_reason = 3;
						sendNack(notification.tx_info.packet_id, receiving_from_node_id, notification.source_id, loss_reason);
					} else {
						
						if(save_node_logs) fprintf(own_log_file, "%f;N%d;D07; - I am NOT the TX destination (N%d)\n",SimTime(),
								node_id, notification.tx_info.destination_id);
						
						loss_reason = 4;
						sendNack(notification.tx_info.packet_id, receiving_from_node_id, notification.source_id, loss_reason);
					}
				} else if(convertPower(PICO_TO_DBM,max_pw_interference) >= current_cca){
					loss_reason = 2;
					sendNack(notification.tx_info.packet_id, notification.source_id, -1, loss_reason);
				}
				break;
			}
			default:
				printf("ERROR: %d is not a correct state\n", node_state);
				break;
		}
	}

	
}
#line 315 "Node.h"
void compcxx_Node_5 :: inportSomeNodeFinishTX(Notification &notification){

	if(save_node_logs) fprintf(own_log_file, "%f;N%d;E00; inportSomeNodeFinishTX(): source = %d - destination = %d\n",
			SimTime(), node_id, notification.source_id, notification.tx_info.destination_id);

	if(notification.source_id == node_id){
		
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;E02; - I have finished a TX in channel range: %d - %d\n",
				SimTime(), node_id, notification.left_channel, notification.right_channel);
	} else {
		
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;E02; - N%d has finished a TX in channel range: %d - %d\n",
				SimTime(), node_id, notification.source_id, notification.left_channel, notification.right_channel);
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;E03; - Initial power of transmitter N%d = %f pW\n",
				SimTime(), node_id, notification.source_id, power_received_per_node[notification.source_id]);
		updateChannelsPower(notification, 0);

		
		switch(node_state){
			
			case 0:{
				handleBackoff(1, notification);	
				break;
			}
			
			case 1:{
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;E09; - I am TRANSMITTING ---> do nothing\n", SimTime(), node_id);
				break;
			}
			
			case 2:{
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;E09; - I am RECEIVING\n", SimTime(), node_id);
				
				if(notification.tx_info.destination_id == node_id){
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;E06; - Packet %d transmission from %d is finished successfully\n",
							SimTime(), node_id, notification.tx_info.packet_id, notification.source_id);
					node_state = 0;
					receiving_from_node_id = -1;
					receiving_packet_id = -1;
					
					handleBackoff(1, notification);
				} else {
					
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;E09; - I am not the destination --> do nothing\n", SimTime(), node_id);
				}
				break;
			}
		}
	}
	
}
#line 374 "Node.h"
void compcxx_Node_5 :: inportNackReceived(NackInfo &nack_info){
	if(nack_info.source_id != node_id){
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;G00; inportNackReceived() from %d\n", SimTime(), node_id, nack_info.source_id);
		processNack(nack_info);
	} else {
		
	}
	
}









#line 391 "Node.h"
void compcxx_Node_5 :: endBackoff(trigger_t &){
	
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;F00; endBackoff()\n", SimTime(), node_id);

	node_state = 1;	

	getChannelOccupancyByCCA();
	getTxChannelsByChannelBonding(channel_bonding_model, channels_free);

	if(save_node_logs) fprintf(own_log_file, "%f:N%d;F02 - Channels for transmitting: ",SimTime(), node_id);
	printChannelForTx(save_node_logs, 1, channels_for_tx, num_channels_komondor, own_log_file);

	
	if(channels_for_tx[0] != -1){
		
		if(save_node_logs) fprintf(own_log_file, "%f:N%d;F03 - Transmission is possible\n",SimTime(), node_id);
		current_left_channel = getBoundaryChannel(0, channels_for_tx,  num_channels_komondor);
		current_right_channel = getBoundaryChannel(1, channels_for_tx, num_channels_komondor);
		int num_channels_tx = current_right_channel - current_left_channel + 1;
		current_tx_duration = computeTxTime(num_channels_tx-1);
		if(save_node_logs) fprintf(own_log_file, "%f:N%d;F04    + Selected transmission range: %d - %d\n",
				SimTime(), node_id, current_left_channel, current_right_channel);
		
		Notification notification = generateNotification(current_destination_id, current_tx_duration);
		(outportSelfStartTX_f(notification));
		trigger_toFinishTX.Set(SimTime()+current_tx_duration);
		packets_sent++;
		packet_id++;
		for(int c = current_left_channel; c <= current_right_channel; c++){
			total_time_transmitting_per_channel[c] += current_tx_duration;
		}
		total_time_transmitting_in_num_channels[current_right_channel - current_left_channel] += current_tx_duration;
	} else {
		
		if(save_node_logs) fprintf(own_log_file, "%f:N%d;F03 - Transmission is NOT possible\n",SimTime(), node_id);
		remaining_backoff = computeBackoff(pdf_backoff, lambda);
		resumeBackoff();
		if(save_node_logs) fprintf(own_log_file, "%f:N%d;F05    + New backoff generated of %f s\n",SimTime(), node_id, remaining_backoff);
	}
	
}
#line 439 "Node.h"
void compcxx_Node_5 :: myTXFinished(trigger_t &){
	
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;G00; myTXFinished()\n", SimTime(), node_id);
	node_state = 0;	
	Notification notification = generateNotification(current_destination_id, 0);
	(outportSelfFinishTX_f(notification));
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;G02; - New BO generated of %f s\n", SimTime(), node_id, remaining_backoff);
	restartNode();
	
}
#line 585 "Node.h"
double compcxx_Node_5 :: computeTxTime(int ix_num_channels_used){

	double tx_time;
	double data_rate = data_rate_array[ix_num_channels_used];
	switch(pdf_tx_time){
		
		case 0:{
			tx_time = Exponential((packet_length * num_packets_aggregated)/data_rate);
			break;
		}
		
		case 1:{
			tx_time = (packet_length * num_packets_aggregated)/data_rate;
			break;
		}
		default:{
			printf("TX time model not found!\n");
			break;
		}
	}
	return tx_time;
}














#line 652 "Node.h"
void compcxx_Node_5 :: getChannelOccupancyByCCA(){
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		if(channel_power[c] < convertPower(DBM_TO_PICO, current_cca)){
			channels_free[c] = 1;
		} else {
			channels_free[c] = 0;
		}
	}
}









#line 669 "Node.h"
void compcxx_Node_5 :: updateChannelsPower(Notification notification, int update_type){

	double pw_received_pico;

	if(update_type == TX_FINISHED) {

		pw_received_pico = power_received_per_node[notification.source_id];
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - Pre update channel state [dBm]: ", SimTime(), node_id, node_state, LOG_D03);

	} else if(update_type == TX_INITIATED) {

		double distance = computeDistance(x, y, z, notification.tx_info.x, notification.tx_info.y, notification.tx_info.z);
		double pw_received = computePowerReceived(path_loss_model, distance, wavelength, notification.tx_info.tx_power, tx_gain, rx_gain);
		pw_received_pico = convertPower(DBM_TO_PICO, pw_received);	
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - Pre update channel state [dBm]: ", SimTime(), node_id, node_state, LOG_E04);

	} else {
		printf("ERROR: update_type %d does not exist!!!", update_type);
	}
	printChannelPower(WRITE_LOG);

	double distance = computeDistance(x, y, z, notification.tx_info.x, notification.tx_info.y, notification.tx_info.z);
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - Distance to N%d = %f m\n",
			SimTime(), node_id, node_state, LOG_D04, notification.source_id, distance);
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - Power received from transmitter N%d = %f dBm (%f pW)\n",
			SimTime(), node_id, node_state, LOG_D05, notification.source_id, convertPower(PICO_TO_DBM, pw_received_pico), pw_received_pico);
	power_received_per_node[notification.source_id] = pw_received_pico;

	
	double total_power[num_channels_komondor];
	for(int i = 0; i < num_channels_komondor; i++) {
		total_power[i] = 0;
	}

	
	for(int i = notification.left_channel; i <= notification.right_channel; i++){
		total_power[i] = pw_received_pico;
	}

	
	switch(cochannel_model){
		
		case 0:{
			
			break;
		}
		
		case 1:{
			for(int c = 0; c < num_channels_komondor; c++) {
				if(c < notification.left_channel || c > notification.right_channel){
					if(c < notification.left_channel) {
						total_power[c] += convertPower(DBM_TO_PICO, convertPower(PICO_TO_DBM, pw_received_pico) - 20*abs(c-notification.left_channel));
					} else if(c > notification.right_channel) {
						total_power[c] += convertPower(DBM_TO_PICO, convertPower(PICO_TO_DBM, pw_received_pico) - 20*abs(c-notification.right_channel));
					}
					if(total_power[c] < 0.000001){
						total_power[c] = 0;
					}
				} else {
					
				}
			}
			break;
		}
		
		case 2:{
			for(int i = 0; i < num_channels_komondor; i++) {
				for(int j = notification.left_channel; j <= notification.right_channel; j++){
					if(i != j) {
						total_power[i] += convertPower(PICO_TO_DBM, convertPower(DBM_TO_PICO, pw_received_pico) - 20*abs(i-j));
						if(total_power[i] < 0.00001){
							total_power[i] = 0;
						}
					}
				}
			}
			break;
		}
		default:{
			break;
		}
	}

	
	for(int c = 0; c < num_channels_komondor; c++){
		if(update_type == 0) {
			channel_power[c] -= total_power[c];
		} else {
			channel_power[c] += total_power[c];
		}
	}

	if(save_node_logs){
		if(update_type == TX_INITIATED)	fprintf(own_log_file, "%f;N%d;S%d;%s; - Post update channel state [dBm]: ",
				SimTime(), node_id, node_state, LOG_D06);
		if(update_type == TX_FINISHED) fprintf(own_log_file, "%f;N%d;S%d;%s; - Post update channel state [dBm]: ",
				SimTime(), node_id, node_state, LOG_E05);
	}
	printChannelPower(WRITE_LOG);
}





#line 773 "Node.h"
void compcxx_Node_5 :: getTxChannelsByChannelBonding(int channel_bonding_model, int *channels_free){
	

	
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		channels_for_tx[c] = 0;
	}

	
	int left_free_ch = 0;
	int left_free_ch_is_set = 0;
	int right_free_ch = 0;
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		if(channels_free[c]){
			if(!left_free_ch_is_set){
				left_free_ch = c;
				left_free_ch_is_set = 1;
			}
			if(right_free_ch < c){
				right_free_ch = c;
			}
		}
	}
	
	
	
	int num_free_ch = right_free_ch - left_free_ch + 1;
	int num_available_ch = max_channel_allowed - min_channel_allowed + 1;


	int m;			
	int left_tx_ch;	
	int right_tx_ch;	


	
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		
	}

	
	switch(channel_bonding_model){

		
		case 0:{
			if(primary_channel >= left_free_ch && primary_channel <= right_free_ch){
				channels_for_tx[primary_channel] = 1;
			}
			break;
		}

		
		case 1:{
			int tx_possible = 1;
			
			for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
				if(!channels_free[c]){
					tx_possible = 0;
				}
			}
			if(tx_possible){
				left_tx_ch = left_free_ch;
				right_tx_ch = right_free_ch;
				for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
					channels_for_tx[c] = 1;
				}
			} else {
				
				channels_for_tx[0] = -1;
			}
			break;
		}

		
		case 2:{
			while(1){
				
				
				if(fmod(log10(num_available_ch)/log10(2), 1) == 0){
					
					m = primary_channel % num_available_ch;
					left_tx_ch = primary_channel - m;
					right_tx_ch = primary_channel + num_available_ch - m - 1;
					
					if((left_tx_ch >= min_channel_allowed) && (right_tx_ch <= max_channel_allowed)){
						
						
						break;
					} else {
						
						num_available_ch --;
					}
				} else{
					
					num_available_ch --;
				}
			}

			
			int tx_possible = 1;
			for(int c = left_tx_ch; c <= right_tx_ch; c++){
				if(!channels_free[c]){
					tx_possible = 0;
				}
			}
			if(tx_possible){
				for(int c = left_tx_ch; c <= right_tx_ch; c++){
					channels_for_tx[c] = 1;
				}
			} else {
				
				channels_for_tx[0] = -1;
			}
			break;
		}

		
		case 3:{
			for(int c = left_free_ch; c <= right_free_ch; c++){
				channels_for_tx[c] = 1;
			}
			break;
		}

		
		case 4:{
			while(1){
				
				
				if(fmod(log10(num_free_ch)/log10(2), 1) == 0){
					
					m = primary_channel % num_free_ch;
					left_tx_ch = primary_channel - m;
					right_tx_ch = primary_channel + num_free_ch - m - 1;
					
					if((left_tx_ch >= min_channel_allowed) && (right_tx_ch <= max_channel_allowed)){
						
						
						for(int c = left_tx_ch; c <= right_tx_ch; c++){
							channels_for_tx[c] = 1;
						}
						break;
					} else {
						
						num_free_ch --;
					}
				} else{
					
					num_free_ch --;
				}
			}
			break;
		}
		default:{
			
			break;
		}
	}


	
	for(int c = min_channel_allowed; c <= max_channel_allowed; c++){
		
	}

	

}







#line 947 "Node.h"
void compcxx_Node_5 :: computeMaxInterference(Notification notification) {
	max_pw_interference = 0;
	for(int c = current_left_channel; c <= current_right_channel; c++){
		if(max_pw_interference <= (channel_power[c] - power_received_per_node[notification.source_id])){
			max_pw_interference = channel_power[c] - power_received_per_node[notification.source_id];
			channel_max_interference = c;
		}
	}
}















#line 970 "Node.h"
Notification compcxx_Node_5 :: generateNotification(int destination_id, double tx_duration){
	Notification notification;
	TxInfo tx_info;
	tx_info.destination_id = destination_id;
	tx_info.tx_duration = tx_duration;
	tx_info.tx_power = current_tpc;
	tx_info.x = x;
	tx_info.y = y;
	tx_info.z = z;
	tx_info.packet_id = packet_id;
	notification.tx_info = tx_info;
	notification.source_id = node_id;
	notification.left_channel = current_left_channel;
	notification.right_channel = current_right_channel;
	notification.packet_length = packet_length;

	return notification;
}











#line 998 "Node.h"
void compcxx_Node_5 :: sendNack(int packet_id, int node_id_a, int node_id_b, int loss_reason){
	NackInfo nack_info;
	nack_info.source_id = node_id;
	nack_info.packet_id = packet_id;
	nack_info.loss_reason = loss_reason;
	nack_info.node_id_a = node_id_a;
	nack_info.node_id_b = node_id_b;
	(outportSendNack_f(nack_info));
}





#line 1011 "Node.h"
void compcxx_Node_5 :: cleanNack(){
	nack.source_id = NO_NODE_ID;
	nack.packet_id = NO_PACKET_ID;
	nack.loss_reason = PACKET_NOT_LOST;
	nack.node_id_a = NO_NODE_ID;
	nack.node_id_b = NO_NODE_ID;
}







#line 1024 "Node.h"
void compcxx_Node_5 :: processNack(NackInfo nack_info) {

	if(save_node_logs) fprintf(own_log_file, "%f;N%d;S%d;%s; - processNack()\n",
							SimTime(), node_id, node_state, LOG_D02);

	if(nack_info.node_id_a == node_id || nack_info.node_id_b == node_id){

		
		nacks_received[nack_info.loss_reason] ++;

		switch(nack_info.loss_reason){

			case PACKET_LOST_DESTINATION_TX:{
				
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Destination %d was transmitting!s\n",
						SimTime(), node_id, nack_info.source_id);
				
				hidden_nodes_list[nack_info.source_id] = 1;
				handlePacketLoss();
				break;
			}

			case PACKET_LOST_LOW_SIGNAL:{
				
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Power received in destination %d is less than its CCA!\n",
						SimTime(), node_id, nack_info.source_id);
				handlePacketLoss();
				break;
			}

			case PACKET_LOST_INTERFERENCE:{
				
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Interference sensed in destination %d is greater than its CCA!\n",
						SimTime(), node_id, nack_info.source_id);
				handlePacketLoss();
				break;
			}

			case PACKET_LOST_PURE_COLLISION:{
				
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Pure collision detected at destination %d! %d was transmitting and %d appeared\n",
										SimTime(), node_id, nack_info.source_id, nack_info.node_id_a, nack_info.node_id_b);
				
				if(nack_info.node_id_a != node_id) {
					hidden_nodes_list[nack_info.node_id_a] = 1;
				} else if (nack_info.node_id_b != node_id) {
					hidden_nodes_list[nack_info.node_id_b] = 1;
				}
				handlePacketLoss();
				break;
			}
			case PACKET_LOST_LOW_SIGNAL_AND_RX:{
				
				
				if(nack_info.node_id_a == node_id) {
					if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Collision detected at destination %d! %d appeared when %d was transmitting\n",
							SimTime(), node_id, nack_info.source_id, nack_info.node_id_b, nack_info.node_id_a);
					hidden_nodes_list[nack_info.node_id_b] = 1;
					handlePacketLoss();
				}
				break;
			}
			default:{
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;G03;    + Unknown reason for packet loss\n", SimTime(), node_id);
				break;
			}
		}
	} else {
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;G02; I am NOT implied in the NACK\n", SimTime(), node_id);
	}
}











#line 1105 "Node.h"
void compcxx_Node_5 :: handlePacketLoss(){
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;G02; Packet %d has been LOST!\n", SimTime(), node_id, packet_id);
	for(int c = current_left_channel; c <= current_right_channel; c++){
		total_time_lost_per_channel[c] += current_tx_duration;
	}
	total_time_lost_in_num_channels[current_right_channel - current_left_channel] += current_tx_duration;
	packets_lost ++;
}











#line 1123 "Node.h"
int compcxx_Node_5 :: isPacketLost(Notification notification){

	
	int loss_reason = PACKET_NOT_LOST;
	if(convertPower(DBM_TO_PICO, max_pw_interference) >= current_cca){
		
		loss_reason = PACKET_LOST_INTERFERENCE;
	}
	if (convertPower(DBM_TO_PICO, pw_received_interest) < current_cca) {
		
		loss_reason = PACKET_LOST_LOW_SIGNAL;
	}
	return loss_reason;
}
















#line 1152 "Node.h"
double compcxx_Node_5 :: computeBackoff(int pdf, double lambda){
	double backoff;
	switch(pdf){
		
		case 0:{
			backoff = Exponential(1/lambda);
			break;
		}
		
		case 1:{
			backoff = 1/lambda;
			break;
		}
		default:{
			printf("Backoff model not found!\n");
			break;
		}
	}
	return backoff;
}








#line 1179 "Node.h"
void compcxx_Node_5 :: handleBackoff(int pause_or_resume, Notification notification){

	switch(pause_or_resume){

		case PAUSE_TIMER:{

			if(save_node_logs) fprintf(own_log_file, "%f;N%d;D10; - primary_channel (#%d) affected\n", SimTime(), node_id, primary_channel);
			if(save_node_logs) fprintf(own_log_file, "%f;N%d;D11;    + Power sensed in primary channel:  %f dBm (%f pW)\n", SimTime(), node_id, convertPower(DBM_TO_PICO, channel_power[primary_channel]), channel_power[primary_channel]);

			if(exceededCCA(channel_power, primary_channel, current_cca) == CCA_EXCEEDED){ 

				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D12;    + CCA (%d dBm) exceeded\n", SimTime(), node_id, current_cca);
				pauseBackoff();

			} else {	

				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D12;    + CCA (%d dBm) NOT exceeded\n", SimTime(), node_id, current_cca);
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D10; - primary_channel (#%d) NOT affected\n", SimTime(), node_id, primary_channel);

			}
			break;
		}

		case RESUME_TIMER:{

			if(save_node_logs) fprintf(own_log_file, "%f;N%d;E09; - I am SENSING\n", SimTime(), node_id);
			if(save_node_logs) fprintf(own_log_file, "%f;N%d;E06; - Primary_channel (#%d) affected\n",
					SimTime(), node_id, primary_channel);
			if(save_node_logs) fprintf(own_log_file, "%f;N%d;E07; - Power sensed in primary channel:  %f dBm (%f pW)\n",
					SimTime(), node_id, convertPower(DBM_TO_PICO, channel_power[primary_channel]), channel_power[primary_channel]);

			if(exceededCCA(channel_power, primary_channel, current_cca) == CCA_NOT_EXCEEDED){ 
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;E08;    + CCA (%d dBm) NOT exceeded\n",
						SimTime(), node_id, current_cca);
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;D10; - primary_channel (#%d) NOT affected\n", SimTime(), node_id, primary_channel);
				resumeBackoff();

			} else {	
				if(save_node_logs) fprintf(own_log_file, "%f;N%d;E08;    + CCA (%d dBm) exceeded\n", SimTime(), node_id, current_cca);
			}
			break;
		}
		default:{
			printf("pause_or_resume mode unknown!");
			break;
		}
	}
}





#line 1231 "Node.h"
void compcxx_Node_5 :: pauseBackoff(){
	
	if(trigger_backoff.Active()){
		remaining_backoff = trigger_backoff.GetTime()-SimTime();
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;D13;       - Backoff is active --> freeze it at %f s\n", SimTime(), node_id, remaining_backoff);
		trigger_backoff.Cancel();
		
	} else {
		
		if(save_node_logs) fprintf(own_log_file, "%f;N%d;D13;       - Backoff is NOT active (frozen at %f s)\n", SimTime(), node_id, trigger_backoff.GetTime());
	}
}





#line 1247 "Node.h"
void compcxx_Node_5 :: resumeBackoff(){
	if(save_node_logs) fprintf(own_log_file, "%f;N%d;E09;       - resuming backoff at %f\n",
			SimTime(), node_id, remaining_backoff);
	
	
	trigger_backoff.Set(SimTime() + remaining_backoff);
}











#line 1264 "Node.h"
void compcxx_Node_5 :: restartNode(){
	total_time_transmitting_in_num_channels[current_right_channel - current_left_channel] += current_tx_duration;
	
	current_tx_duration = 0;
	node_state = 0;
	receiving_from_node_id = -1;
	receiving_packet_id = -1;
	remaining_backoff = computeBackoff(pdf_backoff, lambda);
	resumeBackoff();
	cleanNack();
}


#line 1276 "Node.h"
void compcxx_Node_5 :: printNodeInfo(int write_or_print){
	switch(write_or_print){
		case PRINT_LOG:{
			printf("    - Node %d info:\n", node_id);
			printf("       · position = (%d, %d, %d)\n", x, y, z);
			printf("       · primary_channel = %d\n", primary_channel);
			printf("       · min_channel_allowed = %d\n", min_channel_allowed);
			printf("       · max_channel_allowed = %d\n", max_channel_allowed);
			printf("       · lambda = %f packets/s\n", lambda);
			printf("       · channel_bonding_model = %d\n", channel_bonding_model);
			printf("       · destination_id = %d\n", destination_id);
			printf("       · tpc_default = %f dBm\n", tpc_default);
			printf("       · cca_default = %f dBm\n", cca_default);
			break;
		}
		case WRITE_LOG:{
			if(save_node_logs) {
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Node info:\n", SimTime(), node_id, node_state, LOG_Z00);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · position = (%d, %d, %d)\n", SimTime(), node_id, node_state, LOG_Z00, x, y, z);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · primary_channel = %d\n", SimTime(), node_id, node_state, LOG_Z00, primary_channel);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · min_channel_allowed = %d\n", SimTime(), node_id, node_state, LOG_Z00, min_channel_allowed);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · max_channel_allowed = %d\n", SimTime(), node_id, node_state, LOG_Z00, max_channel_allowed);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · lambda = %f packets/s\n", SimTime(), node_id, node_state, LOG_Z00, lambda);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · channel_bonding_model = %d\n", SimTime(), node_id, node_state, LOG_Z00, channel_bonding_model);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · destination_id = %d\n", SimTime(), node_id, node_state, LOG_Z00, destination_id);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · tpc_default = %f dBm\n", SimTime(), node_id, node_state, LOG_Z00, tpc_default);
				fprintf(own_log_file, "%f;N%d;S%d;%s;       · cca_default = %f dBm\n", SimTime(), node_id, node_state, LOG_Z00, cca_default);
			}
			break;
		}
	}


}





#line 1314 "Node.h"
void compcxx_Node_5 :: printProgressBar(trigger_t &){
	printf("* %d %% *\n", progress_bar_counter * progress_bar_delta);
	trigger_sim_time.Set(SimTime()+sim_time/(100/progress_bar_delta));

	
	if(node_id == 0 && progress_bar_counter == (100/progress_bar_delta)-1){
		trigger_sim_time.Set(SimTime()+sim_time/(100/progress_bar_delta)-0.0001);
	}
	progress_bar_counter ++;
}





#line 1328 "Node.h"
void compcxx_Node_5 :: initializeVariables() {

	channel_power = (double *) malloc(num_channels_komondor * sizeof(*channel_power));
	num_channels_allowed = (max_channel_allowed - min_channel_allowed + 1);
	total_time_transmitting_per_channel = (double *) malloc(num_channels_komondor * sizeof(*total_time_transmitting_per_channel));
	channels_free = (int *) malloc(num_channels_komondor * sizeof(*channels_free));
	channels_for_tx = (int *) malloc(num_channels_komondor * sizeof(*channels_for_tx));
	total_time_lost_per_channel = (double *) malloc(num_channels_komondor * sizeof(*total_time_lost_per_channel));
	for(int i = 0; i < num_channels_komondor; i++){
		channel_power[i] = 0;
		total_time_transmitting_per_channel[i] = 0;
		channels_free[i] = 0;
		channels_for_tx[i] = 0;
		total_time_lost_per_channel[i] = 0;
	}

	total_time_transmitting_in_num_channels = (double *) malloc(num_channels_allowed * sizeof(*total_time_transmitting_in_num_channels));
	total_time_lost_in_num_channels = (double *) malloc(num_channels_allowed * sizeof(*total_time_lost_in_num_channels));
	for(int i = 0; i < num_channels_allowed; i++){
		total_time_transmitting_in_num_channels[i] = 0;
		total_time_lost_in_num_channels[i] = 0;
	}
	
	power_received_per_node = (double *) malloc(total_nodes_number * sizeof(*power_received_per_node));
	
	hidden_nodes_list = (int *) malloc(total_nodes_number * sizeof(*hidden_nodes_list));
	for(int i = 0; i < total_nodes_number; i++){
		power_received_per_node[i] = 0;
		hidden_nodes_list[i] = 0;
	}

	
	nacks_received = (int *) malloc(5 * sizeof(*nacks_received));
	for(int i = 0; i < 5; i++){
		nacks_received[i] = 0;
	}

	current_destination_id = destination_id;
	pw_received_interest = 0; 	
	progress_bar_delta = 5;	
	progress_bar_counter = 0;
	packets_sent = 0;
	node_state = 0; 
	throughput = 0;
	current_left_channel =  min_channel_allowed;
	current_right_channel = max_channel_allowed;
	current_tpc = tpc_default;
	current_cca = cca_default;

	node_state = 0;

	data_rate_array[0] = 81.5727 * packet_length * num_packets_aggregated;	
	data_rate_array[1] = 150.8068 * packet_length * num_packets_aggregated; 
	data_rate_array[2] = 215.7497 * packet_length * num_packets_aggregated;	
	data_rate_array[3] = 284.1716 * packet_length * num_packets_aggregated; 

	packets_lost = 0;
}






#line 1391 "Node.h"
void compcxx_Node_5 :: printChannelPower(int write_or_print){

	switch(write_or_print){
		case PRINT_LOG:{
			printf("channel_power [dBm]: ");
			for(int c = 0; c < num_channels_komondor; c++){
				printf("%f  ", convertPower(PICO_TO_DBM, channel_power[c]));
			}
			printf("\n");
			break;
		}
		case WRITE_LOG:{
			for(int c = 0; c < num_channels_komondor; c++){
				 if(save_node_logs) fprintf(own_log_file, "%f  ", convertPower(PICO_TO_DBM, channel_power[c]));
			}
			if(save_node_logs)  fprintf(own_log_file, "\n");
			break;
		}
	}
}





#line 1435 "Node.h"
void compcxx_Node_5 :: printNodeStatistics(int write_or_print){

	
	double packets_lost_percentage = 0;
	if (packets_sent > 0) {
		packets_lost_percentage = double(packets_lost * 100)/double(packets_sent);
	}
	throughput = (((double)(packets_sent-packets_lost) * packet_length * num_packets_aggregated / 1000000))/SimTime();
	int hidden_nodes_number = 0;
	for(int i = 0; i < total_nodes_number; i++){
		if(hidden_nodes_list[i] == 1) hidden_nodes_number++;
	}

	switch(write_or_print){

		case PRINT_LOG:{

			printf("------- N%d ------\n", node_id);

			
			printf(" - Throughput = %f Mbps\n", throughput);

			
			printf(" - Packets sent = %d - Packets lost = %d  (%f %% lost)\n",
							packets_sent, packets_lost, packets_lost_percentage);

			
			printf("    · Time EFFECTIVELY transmitting in N channels: ");
			for(int n = 0; n < num_channels_allowed; n++){
				printf("(%d) %f  ", n+1, total_time_transmitting_in_num_channels[n] - total_time_lost_in_num_channels[n]);
			}
			printf("\n");

			
			printf("    · Time EFFECTIVELY transmitting in each channel: ");
			for(int c = 0; c < num_channels_komondor; c++){
				printf("(#%d) %f ", c, total_time_transmitting_per_channel[c] - total_time_lost_per_channel[c]);
			}
			printf("\n");

			
			printf("    · Time LOST transmitting in N channels: ");
			for(int n = 0; n < num_channels_allowed; n++){
				printf("(%d) %f  ", n+1, total_time_lost_in_num_channels[n]);
			}
			printf("\n");

			
			printf("    · Time LOST transmitting in each channel: ");
			for(int c = 0; c < num_channels_komondor; c++){
				printf("(#%d) %f ", c, total_time_lost_per_channel[c]);
			}
			printf("\n");

			
			printf(" - Total hidden nodes: %d\n", hidden_nodes_number);
			printf(" - Hidden nodes list: ");
			for(int i = 0; i < total_nodes_number; i++){
				printf("%d  ", hidden_nodes_list[i]);
			}
			printf("\n\n");
			break;
		}

		case WRITE_LOG:{

			if (save_node_logs){

				
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Throughput = %f Mbps\n",
						SimTime(), node_id, node_state, LOG_Z00, throughput);

				
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Packets sent: %d\n", SimTime(), node_id, node_state, LOG_Z00, packets_sent);
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Packets lost: %d\n", SimTime(), node_id, node_state, LOG_Z00, packets_lost);
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Loss ratio: %f\n", SimTime(), node_id, node_state, LOG_Z00, packets_lost_percentage);

				
				fprintf(own_log_file, "%f;N%d;S%d;%s;        · Time EFFECTIVELY transmitting in N channels: ",
						SimTime(), node_id, node_state, LOG_Z00);
				for(int n = 0; n < num_channels_allowed; n++){
					fprintf(own_log_file, "(%d) %f  ", n+1, total_time_transmitting_in_num_channels[n] - total_time_lost_in_num_channels[n]);
				}
				fprintf(own_log_file, "\n");

				
				fprintf(own_log_file, "%f;N%d;S%d;%s;        · Time EFFECTIVELY transmitting in each channel: ",
						SimTime(), node_id, node_state, LOG_Z00);
				for(int c = 0; c < num_channels_komondor; c++){
					fprintf(own_log_file, "(#%d) %f ", c, total_time_transmitting_per_channel[c] - total_time_lost_per_channel[c]);
				}
				fprintf(own_log_file, "\n");

				
				fprintf(own_log_file, "%f;N%d;S%d;%s;        · Time LOST transmitting in N channels: ",
						SimTime(), node_id, node_state, LOG_Z00);
				for(int n = 0; n < num_channels_allowed; n++){
					fprintf(own_log_file, "(%d) %f  ", n+1, total_time_lost_in_num_channels[n]);
				}
				fprintf(own_log_file, "\n");

				
				fprintf(own_log_file, "%f;N%d;S%d;%s;        · Time LOST transmitting in each channel: ",
						SimTime(), node_id, node_state, LOG_Z00);
				for(int c = 0; c < num_channels_komondor; c++){
					fprintf(own_log_file, "(#%d) %f ", c, total_time_lost_per_channel[c]);
				}
				fprintf(own_log_file, "\n");

				
				int hidden_nodes_number = 0;
				for(int n = 0; n < total_nodes_number; n++){
					if(hidden_nodes_list[n] == 1) hidden_nodes_number++;
				}
				fprintf(own_log_file, "%f;N%d;S%d;%s;    - Total hidden nodes: %d\n",
						SimTime(), node_id, node_state, LOG_Z00, hidden_nodes_number);

				fprintf(own_log_file, "%f;N%d;S%d;%s;- Hidden nodes list: ",
						SimTime(), node_id, node_state, LOG_Z00);
				for(int i = 0; i < total_nodes_number; i++){
					fprintf(own_log_file, "%d  ", hidden_nodes_list[i]);
				}
			}
			break;
		}
	}
}

#line 54 "Komondor.cc"
void compcxx_KomondorEnvironment_6 :: Setup(double sim_time, int save_node_logs_console, char *system_filename, char *nodes_filename, char *script_filename){
	printf("-------------- Setup() --------------\n");

	save_node_logs = save_node_logs_console;

	remove("/output/logs_output.txt");
	logs_output_file = fopen("./output/logs_output.txt","at");

	script_file = fopen(script_filename,"at");
	fprintf(script_file, "------------------------------------\n");

	printf("Reading input configuration files...\n");

	
	setupEnvironmentByReadingInputFile(system_filename);

	
	generateNodesByReadingInputFile(nodes_filename, sim_time);

	
	inputChecker();

	
	for(int n = 0; n < total_nodes_number; n++){
		for(int m=0; m < total_nodes_number; m++) {
			node_container[n].outportSelfStartTX_f.Connect(node_container[m],(compcxx_component::Node_outportSelfStartTX_f_t)&compcxx_Node_5::inportSomeNodeStartTX) /*connect node_container[n].outportSelfStartTX,node_container[m].inportSomeNodeStartTX*/;
			node_container[n].outportSelfFinishTX_f.Connect(node_container[m],(compcxx_component::Node_outportSelfFinishTX_f_t)&compcxx_Node_5::inportSomeNodeFinishTX) /*connect node_container[n].outportSelfFinishTX,node_container[m].inportSomeNodeFinishTX*/;
			node_container[n].outportSendNack_f.Connect(node_container[m],(compcxx_component::Node_outportSendNack_f_t)&compcxx_Node_5::inportNackReceived) /*connect node_container[n].outportSendNack,node_container[m].inportNackReceived*/;
		}
	}
}
#line 86 "Komondor.cc"
void compcxx_KomondorEnvironment_6 :: Start(){
	printf("\n--------------  Start() --------------\n");
}
#line 96 "Komondor.cc"
void compcxx_KomondorEnvironment_6 :: Stop(){
	printf("-------------- MAIN Stop() --------------\n");
	int total_packets_sent = 0;
	double total_throughput = 0;
	for(int m=0;m<total_nodes_number;m++){
		total_packets_sent += node_container[m].packets_sent;
		total_throughput += node_container[m].throughput;
	}
	
	printf("[STATS] Total number of packets sent = %d\n", total_packets_sent);
	printf("[STATS] Total throughput = %f\n", total_throughput);
	printf("[STATS] Average number of packets sent = %d\n", (total_packets_sent/total_nodes_number));
	printf("[STATS] Average throughput = %f\n", (total_throughput/total_nodes_number));
	printf("\n\n");


	fprintf(logs_output_file, "STATISTICS:\n");
	fprintf(logs_output_file, "- Total number of packets sent = %d\n", total_packets_sent);
	fclose(logs_output_file);

	fprintf(script_file, "- Total number of packets sent = %d\n", total_packets_sent);
	fclose(script_file);
}
#line 123 "Komondor.cc"
void compcxx_KomondorEnvironment_6 :: inputChecker(){
	printf(" - Validating input files...\n");

	int nodes_ids[total_nodes_number];
	double nodes_x[total_nodes_number];
	double nodes_y[total_nodes_number];
	double nodes_z[total_nodes_number];
	for(int i = 0; i<total_nodes_number;i++){
		nodes_ids[i] = 0;
		nodes_x[i] = 0;
		nodes_y[i] = 0;
		nodes_z[i] = 0;
	}

	for (int i = 0; i < total_nodes_number; i++) {
		nodes_ids[i] = node_container[i].node_id;
		nodes_x[i] = node_container[i].x;
		nodes_y[i] = node_container[i].y;
		nodes_z[i] = node_container[i].z;

		
		if (node_container[i].primary_channel > node_container[i].max_channel_allowed
				|| node_container[i].primary_channel < node_container[i].min_channel_allowed
				|| node_container[i].min_channel_allowed > node_container[i].max_channel_allowed
				|| node_container[i].primary_channel > num_channels_komondor
				|| node_container[i].min_channel_allowed > num_channels_komondor
				|| node_container[i].max_channel_allowed > num_channels_komondor) {
			printf("\nERROR: Channels are not properly configured at node in line %d\n",i+2);
			printf("   · primary_channel = %d, range: %d - %d  \n",
					node_container[i].primary_channel, node_container[i].min_channel_allowed, node_container[i].max_channel_allowed);
			exit(-1);
		}
	}

	for (int i = 0; i < total_nodes_number; i++) {
		for (int j = 0; j < total_nodes_number; j++) {
			
			if(i!=j && nodes_ids[i] == nodes_ids[j] && i < j) {
				printf("\nERROR: Nodes in lines %d and %d have the same ID\n\n",i+2,j+2);
				exit(-1);
			}
			
			if(i!=j && nodes_x[i] == nodes_x[j] && nodes_y[i] == nodes_y[j] && nodes_z[i] == nodes_z[j] && i < j) {
				printf("\nWARNING: Nodes in lines %d and %d are exactly at the same position\n\n",i+2,j+2);
			}
		}
	}
	printf("    · Input files validated!\n");
}







#line 178 "Komondor.cc"
void compcxx_KomondorEnvironment_6 :: setupEnvironmentByReadingInputFile(char *system_filename) {
	printf("- Reading system configuration file '%s'...\n", system_filename);
	fprintf(logs_output_file, "- Reading system configuration file '%s'...\n", system_filename);

	FILE* stream_system = fopen(system_filename, "r");
	if (!stream_system){
		printf("   · Komondor system file '%s' not found! ---> Exiting...\n", system_filename);
		fprintf(logs_output_file, "   · Komondor system file '%s' not found! ---> Exiting...\n", system_filename);
		exit(-1);
	} else {
		printf("   · System configuration: \n");
		fprintf(logs_output_file, "   · System configuration: \n");
	}
	char line_system[1024];
	int first_line_skiped_flag = 0;
	int field_ix;
	while (fgets(line_system, 1024, stream_system))	{
		if(!first_line_skiped_flag){
			first_line_skiped_flag = 1;
		} else{

			field_ix = 1;
			char* tmp = strdup(line_system);

			tmp = strdup(line_system);
			const char* num_channels_komondor_char = getfield(tmp, field_ix);
			num_channels_komondor = atoi(num_channels_komondor_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* basic_channel_bandwidth_char = getfield(tmp, field_ix);
			basic_channel_bandwidth = atoi(basic_channel_bandwidth_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* cw_char = getfield(tmp, field_ix);
			CW = atoi(cw_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* pdf_backoff_char = getfield(tmp, field_ix);
			pdf_backoff = atoi(pdf_backoff_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* pdf_tx_time_char = getfield(tmp, field_ix);
			pdf_tx_time = atoi(pdf_tx_time_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* pkt_length_char = getfield(tmp, field_ix);
			packet_length = atoi(pkt_length_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* logs_flag_char = getfield(tmp, field_ix);
			logs_flag = atoi(logs_flag_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* path_loss_model_char = getfield(tmp, field_ix);
			path_loss_model = atoi(path_loss_model_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* num_packets_aggregated_char = getfield(tmp, field_ix);
			num_packets_aggregated = atoi(num_packets_aggregated_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* wavelength_char = getfield(tmp, field_ix);
			wavelength = atof(wavelength_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* noise_level_char = getfield(tmp, field_ix);
			noise_level = atof(noise_level_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* cochannel_model_char = getfield(tmp, field_ix);
			cochannel_model = atof(cochannel_model_char);
			field_ix++;

			tmp = strdup(line_system);
			const char* collisions_model_char = getfield(tmp, field_ix);
			collisions_model = atof(collisions_model_char);
			field_ix++;

			free(tmp);
		}
	}
	fclose(stream_system);
	printSystemInfo(PRINT_LOG);
	printSystemInfo(WRITE_LOG);
}





#line 278 "Komondor.cc"
void compcxx_KomondorEnvironment_6 :: printSystemInfo(int write_or_print){
	switch(write_or_print){
		case PRINT_LOG:{
			printf("      - num_channels_komondor = %d\n", num_channels_komondor);
			printf("      - basic_channel_bandwidth = %d\n", basic_channel_bandwidth);
			printf("      - CW = %d\n", CW);
			printf("      - pdf_backoff = %d\n", pdf_backoff);
			printf("      - pdf_tx_time = %d\n", pdf_tx_time);
			printf("      - packet_length = %d\n", packet_length);
			printf("      - logs_flag = %d\n", logs_flag);
			printf("      - path_loss_model = %d\n", path_loss_model);
			printf("      - num_packets_aggregated = %d\n", num_packets_aggregated);
			printf("      - wavelength = %f m\n", wavelength);
			printf("      - noise_level = %f dBm\n", noise_level);
			printf("      - cochannel_model = %d\n", cochannel_model);
			printf("      - collisions_model = %d\n", collisions_model);
			printf("      - save_node_logs = %d\n", save_node_logs);
			break;
		}
		case WRITE_LOG:{
			fprintf(logs_output_file, "      - num_channels_komondor = %d\n", num_channels_komondor);
			fprintf(logs_output_file, "      - basic_channel_bandwidth = %d\n", basic_channel_bandwidth);
			fprintf(logs_output_file, "      - CW = %d\n", CW);
			fprintf(logs_output_file, "      - pdf_backoff = %d\n", pdf_backoff);
			fprintf(logs_output_file, "      - pdf_tx_time = %d\n", pdf_tx_time);
			fprintf(logs_output_file, "      - packet_length = %d\n", packet_length);
			fprintf(logs_output_file, "      - logs_flag = %d\n", logs_flag);
			fprintf(logs_output_file, "      - path_loss_model = %d\n", path_loss_model);
			fprintf(logs_output_file, "      - num_packets_aggregated = %d\n", num_packets_aggregated);
			fprintf(logs_output_file, "      - wavelength = %f m\n", wavelength);
			fprintf(logs_output_file, "      - noise_level = %f dBm\n", noise_level);
			fprintf(logs_output_file, "      - cochannel_model = %d\n", cochannel_model);
			fprintf(logs_output_file, "      - collisions_model = %d\n", collisions_model);
			break;
		}
	}
}








#line 322 "Komondor.cc"
void compcxx_KomondorEnvironment_6 :: generateNodesByReadingInputFile(char *nodes_filename, double sim_time) {
	printf("- Reading nodes configuration...\n");
	fprintf(logs_output_file, "- Reading nodes configuration...\n");

	total_nodes_number = getNumOfLines(nodes_filename);
	printf("   · Number of nodes: %d\n", total_nodes_number);
	fprintf(logs_output_file, "   · Number of nodes: %d\n", total_nodes_number);

	node_container.SetSize(total_nodes_number);
	FILE* stream_nodes = fopen(nodes_filename, "r");
	char line_nodes[1024];
	int first_line_skiped_flag = 0;
	int node_ix = 0;
	int node_id_aux = 0;
	int field_ix;
	while (fgets(line_nodes, 1024, stream_nodes)){
		field_ix = 1;
		if(!first_line_skiped_flag){
			first_line_skiped_flag = 1;
		} else{
			char* tmp_nodes = strdup(line_nodes);
			fprintf(logs_output_file, " ----------------------------- \n");

			
			node_container[node_ix].total_nodes_number = total_nodes_number;
			tmp_nodes = strdup(line_nodes);
			node_id_aux = atoi(getfield(tmp_nodes, field_ix));
			node_container[node_ix].node_id = node_id_aux;
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].primary_channel = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].min_channel_allowed = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].max_channel_allowed = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].tpc_default = atof(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].cca_default = atof(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].x = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].y = atof(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].z = atof(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].tx_gain = atof(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].rx_gain = atof(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].channel_bonding_model = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			tmp_nodes = strdup(line_nodes);
			node_container[node_ix].destination_id = atoi(getfield(tmp_nodes, field_ix));
			field_ix++;
			double EB = (double) (CW-1)/2;
			lambda = 1/(EB * T_SLOT);
			node_container[node_ix].lambda = lambda;
			node_container[node_ix].wavelength = wavelength;
			node_container[node_ix].path_loss_model = path_loss_model;
			node_container[node_ix].pdf_backoff = pdf_backoff;
			node_container[node_ix].pdf_tx_time = pdf_tx_time;
			node_container[node_ix].packet_length = packet_length;
			node_container[node_ix].num_packets_aggregated = num_packets_aggregated;
			node_container[node_ix].num_channels_komondor = num_channels_komondor;
			node_container[node_ix].noise_level = noise_level;
			node_container[node_ix].sim_time = sim_time;
			node_container[node_ix].cochannel_model = cochannel_model;
			node_container[node_ix].save_node_logs = save_node_logs;
			node_container[node_ix].collisions_model = collisions_model;
			node_container[node_ix].basic_channel_bandwidth = basic_channel_bandwidth;
			free(tmp_nodes);
			node_ix++;
		}
	}
	printNodesInfo(PRINT_LOG);
	printNodesInfo(WRITE_LOG);
}





#line 414 "Komondor.cc"
void compcxx_KomondorEnvironment_6 :: printNodesInfo(int write_or_print){
	switch(write_or_print){
		case PRINT_LOG:{
			for(int n = 0; n < total_nodes_number; n++){
				printf("     + Node %d:\n", node_container[n].node_id);
				printf("      - node_id = %d\n", node_container[n].node_id);
				printf("      - position = (%d, %d, %d) m\n", node_container[n].x, node_container[n].y, node_container[n].z);
				printf("      - primary_channel = %d\n", node_container[n].primary_channel);
				printf("      - min_channel_allowed = %d\n", node_container[n].min_channel_allowed);
				printf("      - max_channel_allowed = %d\n", node_container[n].max_channel_allowed);
				printf("      - tpc_default = %f dBm\n", node_container[n].tpc_default);
				printf("      - cca_default = %f dBm\n", node_container[n].cca_default);
				printf("      - tx_gain = %f dB\n", node_container[n].tx_gain);
				printf("      - rx_gain = %f dB\n", node_container[n].rx_gain);
				printf("      - channel_bonding_model = %d\n", node_container[n].channel_bonding_model);
				printf("      - destination_id = %d\n", node_container[n].destination_id);
				printf("      - lambda = %f pkts/s\n", node_container[n].lambda);
			}
			break;
		}
		case WRITE_LOG:{
			for(int n = 0; n < total_nodes_number; n++){
				fprintf(logs_output_file, "     + Node %d:\n", node_container[n].node_id);
				fprintf(logs_output_file, "      - node_id = %d\n", node_container[n].node_id);
				fprintf(logs_output_file, "      - position = (%d, %d, %d) m\n", node_container[n].x, node_container[n].y, node_container[n].z);
				fprintf(logs_output_file, "      - primary_channel = %d\n", node_container[n].primary_channel);
				fprintf(logs_output_file, "      - min_channel_allowed = %d\n", node_container[n].min_channel_allowed);
				fprintf(logs_output_file, "      - max_channel_allowed = %d\n", node_container[n].max_channel_allowed);
				fprintf(logs_output_file, "      - tpc_default = %f dBm\n", node_container[n].tpc_default);
				fprintf(logs_output_file, "      - cca_default = %f dBm\n", node_container[n].cca_default);
				fprintf(logs_output_file, "      - tx_gain = %f dB\n", node_container[n].tx_gain);
				fprintf(logs_output_file, "      - rx_gain = %f dB\n", node_container[n].rx_gain);
				fprintf(logs_output_file, "      - channel_bonding_model = %d\n", node_container[n].channel_bonding_model);
				fprintf(logs_output_file, "      - destination_id = %d\n", node_container[n].destination_id);
				fprintf(logs_output_file, "      - lambda = %f pkts/s\n", node_container[n].lambda);
			}
			break;
		}
	}
}




int main(int argc, char *argv[]){

	printf("\n\n\n------------------------------------------\n");
	printf("*******************************\n");
	printf("***** KOMONDOR SIMULATION *****\n");
	printf("*******************************\n");

	printf("\n");
	printf("System configuration:\n");
	printf("- System input file := %s\n", argv[1]);
	printf("- Nodes input file := %s\n", argv[2]);
	printf("- Script output file := %s\n", argv[3]);
	printf("- Observation time := %s s\n", argv[4]);
	printf("- Seed := %s\n", argv[5]);
	printf("- Logs on := %s\n", argv[6]);
	printf("\n");

	if(argc != 7) {
		printf("error: Arguments where not set properly!\n  Execute './KomondorSimulation -stop_time [s] -seed -logs_on -system_filename -nodes_filename'.\n\n");
		return(-1);
	}

	char *system_filename = argv[1];
	char *nodes_filename = argv[2];
	char *script_filename = argv[3];
	double Sim_time = atof(argv[4]);
	int seed = atoi(argv[5]);
	int save_node_logs = atoi(argv[6]);

	
	compcxx_KomondorEnvironment_6 test;
	test.Seed = seed;
	test.StopTime(Sim_time);
	test.Setup(Sim_time, save_node_logs,system_filename, nodes_filename, script_filename);
	test.Run();
	return(0);
};
