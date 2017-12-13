/************************************************************************
 * $CVSHeader: sense/code/common/sense.h,v 1.6 2007/03/03 17:51:23 mlisee Exp $
 *
 * @<title> SENSE Library </title>@ 
 *
 * @<h1> sense.h </h1>@
 * This header file provides
 * necessary functionalities for network simulation. All SENSE
 * components must include this header file.
 *
 * @<!--Copyright 2003 Gilbert (Gang) Chen, Boleslaw K. Szymanski and
 * Rensselaer Polytechnic Institute. All worldwide rights reserved.  A
 * license to use, copy, modify and distribute this software for
 * non-commercial research purposes only is hereby granted, provided
 * that this copyright notice and accompanying disclaimer is not
 * modified or removed from the software.
 *
 * DISCLAIMER: The software is distributed "AS IS" without any express
 * or implied warranty, including but not limited to, any implied
 * warranties of merchantability or fitness for a particular purpose
 * or any warranty of non-infringement of any current or pending
 * patent rights. The authors of the software make no representations
 * about the suitability of this software for any particular
 * purpose. The entire risk as to the quality and performance of the
 * software is with the user. Should the software prove defective, the
 * user assumes the cost of all necessary servicing, repair or
 * correction. In particular, neither Rensselaer Polytechnic
 * Institute, nor the authors of the software are liable for any
 * indirect, special, consequential, or incidental damages related to
 * the software, to the maximum extent the law permits. -->@
 *
 ************************************************************************/

#ifndef SENSE_H
#define SENSE_H

#include "config.h"
const double speed_of_light=299792458.0;

#include <string>
using std::string;

/************************************************************************
 * SENSE is built on top of COST, and makes use of @CorsaAllocator@ defined
 * in corsa_alloc.h.
 ************************************************************************/

#include "cost.h"
#include "corsa_alloc.h"

/************************************************************************
 * @<h2> Packet Management in SENSE </h2>@
 *
 * There are two main design goals on the packet management in SENSE: to
 * allow components to pass packets in two forms, either as a packet
 * structure or as a pointer to a packet structure, and to share a
 * packet among all receiving components if the packet is passed by a
 * pointer.
 *
 * The first goal is achieved by using a modern C++ technique,
 * referred to as traits. According to Bjarne Stroustrup, a trait is
 * "a small object whose main purpose is to carry information used by
 * another object or algorithm to determine policy or implementation
 * details."  In SENSE, a special trait class is declared which can be
 * used to tell if a certain template parameter is a packet structure
 * or a packet pointer.

 * The second goal of sharing a packet through the pointer is done by
 * the use of a popular programming technique, reference counting.
 * When a component is about to own a packet, it must increase the reference
 * count of the packet to indicate the newly created ownership. When a component
 * is about to release a packet, it needs to decrease the reference
 * count. Only when the reference count goes to zero will the packet be
 * actually deleted.
 *
 * @<h2>ether_addr_t</h2>@
 ************************************************************************/

#include "ether_addr.h"

/************************************************************************
 * This is the packet trait class that takes one template parameter,
 * @T@.  It serves dual purposes. First, if @T@ is a pointer, then
 * @nonpointer_t@ gives the corresponding structure. Second, it
 * defines three member functions, which are to be called when
 * respective operations need to be performed.
 *
 * In the general declaration, @nonpointer_t@ is just @T@, and in all
 * three member function no action is taken. This is for packets passed
 * as plain structures.
 ************************************************************************/

template <class T>
class packet_trait
{
 public:
  typedef T nonpointer_t;
  static void free(const T&) { }
  static void free(T&t) { t.~T(); }
  static void init(T&t) { new (&t) T; }
  static void init(const T&) { }
  static void inc_ref(const T&) { }
  static T& access_hdr(T&t) { return t; }
  static void dump(std::string& str, const T&p) { p.dump(str) ;}  
  static void check_ref(const T&, int) { }
};

/*
** nextStatsId can't be a static member since there can be different packet
** definitions for different layers.
*/
unsigned int	nextStatsId = 0;

/************************************************************************
 * A smart packet is designed for layered network architecture. It has
 * two important fields, @hdr@ for the header and @pld@ for the
 * payload (which is usually the encapsulated higher layer
 * packet). There are two ways to access the header or payload,
 * working for two different cases.
 *
 * Let us use a simple example to illustrate the usage of this class
 * in two cases. Suppose we have a declaration @P@ @p@ (most likely
 * this is an argument). If we know for sure that @p@ is a pointer,
 * then we can simply use @p->hdr@ to access the header, @p->pld@ to
 * access the payload.
 *
 * However, if we don't know whether @p@ is a pointer or a structure
 * (which happens when @P@ is a template parameter), we cannot access
 * the field directly, and we have to use the function @access_hdr@.
 * We cannot do @P::access_hdr(p)@, because @P@ may be
 * a pointer. However, we can always rely on @packet_trait@, so we have
 * @packet_trait<P>::access_hdr(p)@.  
 ************************************************************************/

template <class H, class P>
class smart_packet_t
{
public:
  inline static smart_packet_t<H,P>	*alloc();
  inline void	free();
  inline void	destroy();
  inline void	inc_ref();
  inline smart_packet_t<H,P>	*copy();
  inline void	inc_pld_ref() const;
  inline bool	check_ref(int r)	{ return r<=refcount; }
  P		&access_pld()	{ return packet_trait<P>::access_hdr(pld); }
  static void	free_pld(P const &p)	{ packet_trait<P>::free(p); }
  H		*operator ->()		{ return &hdr; }
  std::string	dump() const;
  H		hdr;
  P		pld;
private:
  static CorsaAllocator	m_allocator;
  unsigned int		statsId;
public:
  int		refcount;
  unsigned int	getStatsId() const	{ return statsId; }
  void		setStatsId( unsigned int id) { statsId = id; }
  void		newStatsId()		{ statsId = nextStatsId++; }
};

/************************************************************************
 * We use our own memory allocator, @CorsaAllocator@, which is more
 * efficient than the default memory allocator, since for each packet
 * type we only need to deal with memory blocks of the same size.
 ************************************************************************/

template <class H, class P> CorsaAllocator	smart_packet_t<H,P>::m_allocator (sizeof(smart_packet_t<H,P>));

/************************************************************************
 * This function creates a new packet obtained from the allocator. It then
 * sets @p->pld@ to null if @p->pld@ is a pointer, and the reference count to 1.
 ************************************************************************/

template <class H, class P>
smart_packet_t<H,P>* smart_packet_t<H,P>::alloc()
{
  smart_packet_t<H,P>* p=static_cast<smart_packet_t<H,P>*>(m_allocator.alloc());
  addToPacketList( p);
  packet_trait<H>::init(p->hdr);
  packet_trait<P>::init(p->pld);
  p->refcount=1;
  p->statsId = 0xdeadbeef;
  return p;
}

/************************************************************************
 * This function creates a new copy from an existing packet. 
 ************************************************************************/

template <class H, class P>
smart_packet_t<H,P>* smart_packet_t<H,P>::copy()
{
  smart_packet_t<H,P>* p=static_cast<smart_packet_t<H,P>*>(m_allocator.alloc());
  addToPacketList( p);
  packet_trait<H>::init(p->hdr);
  packet_trait<P>::init(p->pld);
  p->hdr=hdr;
  p->pld=pld;
  inc_pld_ref();
  p->refcount=1;
  p->statsId = statsId;
  return p;
}


/************************************************************************
 * This function first creates a new packet obtained from the allocator. It then
 * sets @p->pld@ to null if @p->pld@ is a pointer, and the reference count to 1.
 ************************************************************************/

template <class H, class P>
void smart_packet_t<H,P>::free()
{
#ifdef SENSE_DEBUG
  packet_trait<P>::check_ref(pld,refcount);
#endif
  removeFromPacketList( this);
  packet_trait<P>::free(pld);
  refcount--;
  if(refcount==0)
  {
    packet_trait<H>::free(hdr);
    m_allocator.free(this);
  }
}

/************************************************************************
 * This function destroys the packet without looking at its reference count.
 ************************************************************************/

template <class H, class P>
void smart_packet_t<H,P>::destroy()
{
  removeFromPacketList( this);
  packet_trait<H>::free(hdr);
  packet_trait<P>::free(pld);
  m_allocator.free(this);
}

/************************************************************************
 * This function increases the reference count. 
 * It must also increase the reference count of the payload packet.
 ************************************************************************/

template <class H, class P>
void smart_packet_t<H,P>::inc_ref()
{
  packet_trait<P>::inc_ref(pld);
  refcount++;
  return;
}

/************************************************************************
 * This function is called before forwarding the  
 * encapsulated payload packet to a higher layer. The reference count in @pld@
 * is increased if @pld@ is a packet pointer, otherwise nothing
 * happens.
 ************************************************************************/

template <class H, class P>
void smart_packet_t<H,P>::inc_pld_ref() const
{
  packet_trait<P>::inc_ref(pld);
}

/************************************************************************
 * This function is called to show the content of the packet. It calls
 * the @dump()@ function of the header, then, depending on the return value,
 * it may or may not call the @dump()@ function of the payload packet.
 ************************************************************************/

template <class H, class P>
std::string smart_packet_t<H,P>::dump() const
{
  std::string str="[";
  std::string h;
  char		mwlString[20];
  sprintf( mwlString, " rc %d", refcount);
  if( hdr.dump(h) )
  {
    std::string m;
    packet_trait<P>::dump(m,pld);
    str = str + h + ", " + m + mwlString + "]";
  }
  else
  {
    str = str + h + ", NULL "+ mwlString + "]";
  }
  return str;
}

/************************************************************************
 * This is the partial specialization of @packet_trait@ for smart
 * packets.  Notice that there is no specialization for general packet
 * pointers. The design decision here is, the specialization for
 * pointers other than that of smart packets should be left to
 * users. When a user wants to pass a pointer of a certain packet, he
 * must implements required member functions, such as alloc(), free(),
 * as well as a partial or complete specialization of @packet_trait@
 * for that pointer
 ************************************************************************/

template <class H, class P>
class packet_trait< smart_packet_t<H,P>* >
{
 public:
  typedef smart_packet_t<H,P> nonpointer_t;
  static void free(nonpointer_t* const &p) { if(p!=NULL) p->free(); };
  static void inc_ref(nonpointer_t* const &p) { if(p!=NULL) p->inc_ref(); };
  static H& access_hdr( nonpointer_t * p) { return p->hdr; }
  static void init(nonpointer_t* &p) { p=NULL; }
  static void dump(std::string& str,nonpointer_t* const &p)
      { if(p!=NULL) str= p->dump(); else str="NULL"; }
  static void check_ref(nonpointer_t* const &p, int ref)
      {
	if(p!=NULL&&!p->check_ref(ref))
	  printf("Packet %p: Payload refcount is smaller than the current refcount\n", p);
      }

};

/************************************************************************
 * @<h2>coordinate_t</h2>@
 * 
 * This class defines the coordination of a position in a two dimensional space.
 ************************************************************************/

class coordinate_t
{
 public:
  coordinate_t () : x(0.0), y(0.0) { }
  coordinate_t ( double _x, double _y ) : x(_x), y(_y) { }
  double x,y;
};

/************************************************************************
 * In COST, timers can only pass one argument due to the
 * limitation of C++ template (we cannot declare two template classes
 * with the same name but with different numbers of template
 * parameters). Therefore, in case we need two arguments, we have to
 * use @pair@ in STL.
 ************************************************************************/

#include <utility>
using std::make_pair;
using std::pair;


/************************************************************************
 * @<h2>triple</h2>@
 * 
 * This class is similar to @pair@ in STL, except that it takes three
 * template parameters.
 ************************************************************************/

template <class T1, class T2, class T3>
class triple
{
 public:
  typedef T1 first_type;
  typedef T2 second_type;
  typedef T3 third_type;

  T1 first;
  T2 second;
  T3 third;

  triple() : first(T1()), second(T2()), third(T3()) {}
  triple(const T1& a, const T2& b, const T3& c) : first(a), second(b), third(c) {}

  template <class U1, class U2, class U3>
      triple(const triple<U1, U2, U3>& t) : first(t.first), second(t.second), third(t.third) {}
};

template <class T1, class T2, class T3>
inline bool operator == (const triple<T1, T2, T3>& x, const triple<T1, T2, T3>& y)
{
  return x.first == y.first && x.second == y.second && x.third == y.third;
}

template <class T1, class T2, class T3>
inline triple<T1, T2, T3> make_triple(const T1& x, const T2& y, const T3& z)
{
  return triple<T1, T2, T3>(x, y, z);
}

/************************************************************************
 * @<h2>Quadruple</h2>@
 * We also need a template class to combine four variables, but since the
 * name 'quadruple' may already be defined as a long double, use the name
 * 'Quadruple' instead.
 ************************************************************************/

template <class T1, class T2, class T3, class T4>
class Quadruple
{
 public:
    typedef T1 first_type;
    typedef T2 second_type;
    typedef T3 third_type;
    typedef T4 fourth_type;

    T1 first;
    T2 second;
    T3 third;
    T4 fourth;

    Quadruple() : first(T1()), second(T2()), third(T3()), fourth(T4()) {}
    Quadruple(const T1& a, const T2& b, const T3& c, const T4& d) 
	: first(a), second(b), third(c), fourth(d) {}

    template <class U1, class U2, class U3, class U4>
	Quadruple(const Quadruple<U1, U2, U3, U4>& t)
	: first(t.first), second(t.second), third(t.third), fourth(t.fourth) {}
};

template <class T1, class T2, class T3, class T4>
inline bool operator == (const Quadruple<T1, T2, T3, T4>& x,
			 const Quadruple<T1, T2, T3, T4>& y)
{
    return x.first == y.first && x.second == y.second && 
	x.third == y.third && x.fourth == y.fourth; 
}

template <class T1, class T2, class T3, class T4>
inline Quadruple<T1, T2, T3, T4> make_Quadruple(const T1& a, const T2& b, const T3& c, const T4& d)
{
  return Quadruple<T1, T2, T3, T4>(a, b, c, d);
}

#include "path.h"

#endif /* SENSE_H */
