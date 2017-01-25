#ifndef	_ether_addr_h_
#define	_ether_addr_h_

/************************************************************************
 * This class defines the addresses used in Ethernet. It is in fact a
 * wrapper class over @int@. The @compare@ struct is an function
 * object that can be used by STL containers.
 ************************************************************************/

class ether_addr_t
{
 public:
  enum { LENGTH = 6, BROADCAST = -1 };

  ether_addr_t () : addr (0) {} ;
  ether_addr_t (int a) : addr (a) {};

  bool operator == (const ether_addr_t& another) const { return addr==another.addr; }
  bool operator == (const int& another) const { return addr==another; }
  bool operator < (const ether_addr_t& another) const { return addr<another.addr; }
  bool operator > (const ether_addr_t& another) const { return addr>another.addr; }

  operator const int& () const { return addr; };
  struct compare 
  {
    bool operator() ( const ether_addr_t & e1, const ether_addr_t & e2)
      {
	return e1.addr > e2.addr;
      }
  };
 private:
  int addr;
};

#endif	// _ether_addr_h_
