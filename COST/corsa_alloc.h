
/************************************************************************
 * CorsaAllocator is used to allocate memory for events. Since a
 * simple memory management scheme is adopted where each memory block
 * allocated is assumed to have the same size, events of the same size,
 * even if they are of different types, share the same CorsaAllocator.
 * It is the simulation platform that manages a number of CorsaAllocators,
 * each with a different event size.
 *
 *   Copyright 2003 Gilbert (Gang) Chen, Boleslaw K. Szymanski and
 *   Rensselaer Polytechnic Institute. All worldwide rights reserved.
 *   A license to use, copy, modify and distribute this software for
 *   non-commercial research purposes only is hereby granted, provided
 *   that this copyright notice and accompanying disclaimer is not
 *   modified or removed from the software.
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
 *
*************************************************************************/

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
    CorsaAllocator(unsigned int );         // default segment size is 256
    CorsaAllocator(unsigned int, int);     // set segement size
    ~CorsaAllocator();		// deallocate memory
    void *alloc();		// STL container operation
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
    CorsaAllocator(const CorsaAllocator& ) {}  // uncopyable
    void Setup(unsigned int,int); // called by two constructors
    void InitSegment(int);
  
    unsigned int m_datasize;
    char** m_segments;	          // an array of pointers
    int m_segment_number;         // number of segements allocated
    int m_segment_max;      
    int m_segment_size;	          // size of segment. m_data also increases
				  // by this number every time
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
    Setup(datasize,256);	  // default is 256
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
	delete[] m_segments[i];	   // delete all segements
    delete[] m_segments;			// delete pointer array
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
    if(seg_size<16)seg_size=16;    // minimum is 16
    m_segment_size=seg_size;			
    m_segment_number=1;		   // only one segment will be allocated
    m_segment_max=seg_size;	   // can have this many segment at most
    m_segments= new char* [ m_segment_max ] ;   // allocate memory for pointer array
    m_segments[0]= new char [m_segment_size*m_datasize];  // allocate first segment

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
    // no more pre-allocated memory avaiable.  Must add another segment.
    {
	int i;
	if(m_segment_number==m_segment_max)	
	// exceeds the maximum segment number.  Must extend the
	// segment pointer array
	{
	    m_segment_max*=2;		// double it
	    char** buff;
	    buff=new char* [m_segment_max];   // new buffer
#ifdef CORSA_DEBUG
	    if(buff==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	    for(i=0;i<m_segment_number;i++)
		buff[i]=m_segments[i];	// copy
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

    DT* item=m_free_list;		// m_free_list is always valid here
    m_free_list=m_free_list->next;
    m_size++;

#ifdef CORSA_DEBUG
    item->self=item;
    char* p=reinterpret_cast<char*>(item);
    p+=sizeof(DT*);
    //printf("alloc: %p %p %p\n",p,item,item->self);
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
    //printf("release: %p %p %p\n",p,item,item->self);
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
