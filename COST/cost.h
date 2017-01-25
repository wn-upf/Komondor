/* $CVSHeader: sense/code/common/cost.h,v 1.3 2007/01/28 02:36:05 mlisee Exp $ */

/*************************************************************************
*   Copyright 2003, 2007 Mark Lisee, Gilbert (Gang) Chen, Boleslaw K.
*   Szymanski and Rensselaer Polytechnic Institute. All worldwide rights
*   reserved. A license to use, copy, modify and distribute this software for
*   non-commercial research purposes only is hereby granted, provided that
*   this copyright notice and accompanying disclaimer is not modified or
*   removed from the software.
*
*   DISCLAIMER: The software is distributed "AS IS" without any
*   express or implied warranty, including but not limited to, any
*   implied warranties of merchantability or fitness for a particular
*   purpose or any warranty of non-infringement of any current or
*   pending patent rights. The authors of the software make no
*   representations about the suitability of this software for any
*   particular purpose. The entire risk as to the quality and
*   performance of the software is with the user. Should the software
*   prove defective, the user assumes the cost of all necessary
*   servicing, repair or correction. In particular, neither Rensselaer
*   Polytechnic Institute, nor the authors of the software are liable
*   for any indirect, special, consequential, or incidental damages
*   related to the software, to the maximum extent the law permits.
*************************************************************************/

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

#include "priority_q.h"
#include "corsa_alloc.h"

class trigger_t {};
typedef double simtime_t;

#ifdef COST_DEBUG
#define Printf(x) Print x
#else
#define Printf(x)
#endif

/* the base class of all timer classes 
 */
class TimerBase;

/* the base class of all event classes */

struct CostEvent
{
  double time;
  CostEvent* next;
  union {
    CostEvent* prev;
    int pos;  // exclusively for heap queue.
  };
  TimerBase* object;
  int index;
  unsigned char active;
};

/* this virtual function is called by the simulation engine */

class TimerBase
{
 public:
  virtual void activate(CostEvent*) = 0;
  inline virtual ~TimerBase() {}	//mwl required by gcc 4.0
};

class TypeII;

/* the COST simulation engine */

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
        //printf("scheduled event-> time: %f, object: %p\n",e->time,e->object);
        m_queue.EnQueue(e);
      }
  void		CancelEvent(CostEvent*e)
      {
        //printf("cancel event-> time: %f, object: %p\n",e->time,e->object);
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
  double	clearStatsTime;	// time to zero stats
  double	eventRate;
  double	runningTime;
  long		eventsProcessed;
  double	m_clock;
  queue_t<CostEvent>	m_queue;
  std::vector<TypeII*>	m_components;
  static CostSimEng	*m_instance;
  std::vector<CorsaAllocator*>	m_allocators;
};

/* the base class of all component classes */
/* */

class TypeII
{
 public: 
  virtual void Start() {};
  virtual void Stop() {};
  inline virtual ~TypeII() {}		//mwl required by gcc 4.0
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
      // otherwise, nextTime == clearStatsTime
      printf( "Clearing statistics @ %f\n", nextTime);
      nextTime = stopTime;
      ClearStats();
    }
    //printf("time: %f, event: %p\n", e->time, e); 
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
  
  //#ifndef VIZ
  printf("# -------------------------------------------------------------------------\n");	
  printf("# CostSimEng with %s, stopped at %f\n", m_queue.GetName(), stopTime);	
  printf("# %ld events processed in %.3f seconds, event processing rate: %.0f\n",	
  eventsProcessed, runningTime, eventRate);
  //#endif //VIZ
}




/* timer is defined as a special component */

template <class T> component Timer : public TimerBase
{
 public:
  struct event_t : public CostEvent { T data; };
  /* the pointer to the simulation engine is passed
     in the configuration function */
  Timer() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(T const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline T& GetData() { return m_event.data; }
  inline void SetData(T const &d) { m_event.data = d; }
  void Cancel();
  outport void to_component(T&);
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
};

template <class T>
void Timer<T>::Set(T const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}

template <class T>
void Timer<T>::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}

template <class T>
void Timer<T>::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}

template <class T>
void Timer<T>::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  to_component(m_event.data);
}

/* another, more complicated timer */

template <class T> component MultiTimer : public TimerBase
{
 public:
  struct event_t : public CostEvent { T data; };
  MultiTimer();
  virtual ~MultiTimer();
	
  outport void to_component(T&, unsigned int i);
 
  inline void Set(double t, unsigned int index =0);
  inline void Set( T const & data, double t,unsigned int index=0);
  void Cancel (unsigned int index=0);
  void activate(CostEvent*event);

  inline bool Active(unsigned int index=0) { return GetEvent(index)->active; }
  inline double GetTime(unsigned int index=0) { return GetEvent(index)->time; }
  inline T& GetData(unsigned int index=0) { return GetEvent(index)->data; }
  inline void SetData(T const &d, unsigned int index) { GetEvent(index)->data = d; }

  event_t* GetEvent(unsigned int index);
				 
 private:
  std::vector<event_t*> m_events;
  CostSimEng* m_simeng;
};

template <class T>
MultiTimer<T>::MultiTimer()
{
  m_simeng = CostSimEng::Instance(); 
  GetEvent(0);
}

template <class T>
MultiTimer<T>::~MultiTimer()
{
  for(unsigned int i=0;i<m_events.size();i++)
    delete m_events[i];
}

template <class T>
typename MultiTimer<T>::event_t* MultiTimer<T>::GetEvent(unsigned int index)
{
  if (index>=m_events.size())
  {
    for (unsigned int i=m_events.size();i<=index;i++)
    {
      m_events.push_back(new event_t);
      m_events[i]->active=false;
      m_events[i]->index=i;
    }
  }
  return m_events[index];
}

template <class T>
void MultiTimer<T>::Set(T const & data, double time, unsigned int index)
{
  event_t * e = GetEvent(index);
  if(e->active)m_simeng->CancelEvent(e);
  e->time = time;
  e->data = data;
  e->object = this;
  e->active = true;
  m_simeng->ScheduleEvent(e);
}

template <class T>
void MultiTimer<T>::Set(double time, unsigned int index)
{
  event_t * e = GetEvent(index);
  if(e->active)m_simeng->CancelEvent(e);
  e->time = time;
  e->object = this;
  e->active = true;
  m_simeng->ScheduleEvent(e);
}

template <class T>
void MultiTimer<T>::Cancel(unsigned int index)
{
  event_t * e = GetEvent(index);
  if(e->active)
    m_simeng->CancelEvent(e);
  e->active = false;
}

template <class T>
void MultiTimer<T>::activate(CostEvent*e)
{
  event_t * event = (event_t*)e;
  event->active = false;
  to_component(event->data,event->index);
}

/* yet another timer */


template <class T> component InfiTimer : public TimerBase
{
 public:
  struct event_t : public CostEvent { T data; };
  InfiTimer();
  virtual ~InfiTimer();
	
  outport void to_component(T&, unsigned int i);
 
  void activate(CostEvent*event);

  inline unsigned int Set(double t);
  inline unsigned int Set( T const & data, double t);
  inline void Cancel (unsigned int index);
  inline event_t* GetEvent(unsigned int index);

  inline bool Active(unsigned int index) { return GetEvent(index)->active; }
  inline double GetTime(unsigned int index) { return GetEvent(index)->time; }
  inline T& GetData(unsigned int index) { return GetEvent(index)->data; }
  inline void SetData(T const &d, unsigned int index) { GetEvent(index)->data = d; }

 private:
  inline void ReleaseSlot(unsigned int i) { m_free_slots.push_back(i); }
  inline unsigned int GetSlot();
					 
  std::vector<event_t*> m_events;
  std::vector<unsigned int> m_free_slots;
  CostSimEng* m_simeng;

  CorsaAllocator* m_allocator;
};

template <class T>
InfiTimer<T>::InfiTimer()
{
  m_simeng = CostSimEng::Instance();
  m_allocator = m_simeng->GetAllocator(sizeof(event_t));
  GetEvent(0);
}

template <class T>
InfiTimer<T>::~InfiTimer()
{
  for(unsigned int i=0;i<m_events.size();i++)
    m_allocator->free(m_events[i]);
}

template <class T>
typename InfiTimer<T>::event_t* InfiTimer<T>::GetEvent(unsigned int index)
{
  if (index>=m_events.size())
  {
    for (unsigned int i=m_events.size();i<=index;i++)
    {
      m_events.push_back( (event_t*) m_allocator->alloc() );
      m_events[i]->active=false;
      m_events[i]->index=i;
      m_free_slots.push_back(i);
    }
  }
  return m_events[index];
}

template <class T>
unsigned int InfiTimer<T>::Set(T const & data, double time)
{
  int index=GetSlot();
  event_t * e = GetEvent(index);
  assert(e->active==false);
  e->time = time;
  e->data = data;
  e->object = this;
  e->active = true;
  m_simeng->ScheduleEvent(e);
  return index;
}

template <class T>
unsigned int InfiTimer<T>::Set(double time)
{
  int index=GetSlot();
  event_t * e = GetEvent(index);
  assert(e->active==false);
  e->time = time;
  e->object = this;
  e->active = true;
  m_simeng->ScheduleEvent(e);
  return index;
}

template <class T>
void InfiTimer<T>::Cancel(unsigned int index)
{
  event_t * e = GetEvent(index);
  assert(e->active);
  m_simeng->CancelEvent(e);
  ReleaseSlot(index);
  e->active = false;
}

template <class T>
void InfiTimer<T>::activate(CostEvent*e)
{
  event_t * event = (event_t*)e;
  event->active = false;
  ReleaseSlot(event->index);
  to_component(event->data,event->index);
}

template <class T>
unsigned int InfiTimer<T>::GetSlot()
{
  if(m_free_slots.empty())
    GetEvent(m_events.size()*2-1);
  int i=m_free_slots.back();
  m_free_slots.pop_back();
  return i;
}
