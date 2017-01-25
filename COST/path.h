/*************************************************************************
 *   @<title> path.h </title>@
 *
 *   @<!-- Copyright 2006 Mark Lisee, Gang Chen, Boleslaw K. Szymanski and
 *   Rensselaer Polytechnic Institute. All worldwide rights reserved.  A
 *   license to use, copy, modify and distribute this software for
 *   non-commercial research purposes only is hereby granted, provided that
 *   this copyright notice and accompanying disclaimer is not modified or
 *   removed from the software.
 *
 *   DISCLAIMER: The software is distributed "AS IS" without any express or
 *   implied warranty, including but not limited to, any implied warranties of
 *   merchantability or fitness for a particular purpose or any warranty of
 *   non-infringement of any current or pending patent rights. The authors of
 *   the software make no representations about the suitability of this
 *   software for any particular purpose. The entire risk as to the quality
 *   and performance of the software is with the user. Should the software
 *   prove defective, the user assumes the cost of all necessary servicing,
 *   repair or correction. In particular, neither Rensselaer Polytechnic
 *   Institute, nor the authors of the software are liable for any indirect,
 *   special, consequential, or incidental damages related to the software,
 *   to the maximum extent the law permits.-->@
 *
 *************************************************************************/

#ifndef	_path_h_
#define	_path_h_

#include <string>
using std::string;

template <int size>
class path_t
{
 public:
  path_t(): length(0), index( 0), overFlow( false) {}
  void AddNode(ether_addr_t& n)
    {
      if( length == 0)
      {
	nodes[0] = n;
	length++;
      }
      else if( length < size)
      {
	if( nodes[ length-1] != n)	// node can't transmit to itself
	{
	  nodes[ length] = n;
	  length++;
	}
      }
      else
	overFlow = true;
      return;
    }
  void Clear() { length=0; }
  const char* ToString()
      {
	static string str;
	str = "";
	char buff[10];
	int	L2=length > 15 ? 15 : length;
	
	sprintf( buff, " {%2d}", length);
	str += buff;
	for(int i=0;i<L2;i++)
	{
	  sprintf(buff," %3d", (int) nodes[i] );
	  str += buff;
	}
	return str.c_str();
      }
  bool	firstNode( int &node)
    {
      index = 0;
      return nextNode( node);
    }
  bool	nextNode( int &node)
    {
      if( index < length)
      {
	node = (int) nodes[ index++];
	return true;
      }
      else
	return false;
    }
  bool	getOverFlow() const	{ return overFlow; }
  int	getLength() const	{ return length; }
 private:
  ether_addr_t	nodes[size];
  int		length;
  int		index;
  bool		overFlow;
};

#ifndef	VR_SIZE
#define VR_SIZE 20
#endif	//VR_SIZE

typedef path_t<VR_SIZE>	Path;

#endif	// _path_h_
