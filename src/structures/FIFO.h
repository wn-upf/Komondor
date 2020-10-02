#include "notification.h"
#include <deque>

/*
	FIFO Class
*/

struct FIFO
{	
		std::deque <Notification> m_queue;
		
		int queue_size;

		Notification GetFirstPacket();
		Notification GetPacketAt(int n);
		void DelFirstPacket();		
		void DeletePacketIn(int i);
		void PutPacket(Notification &packet);
		void PutPacketFront(Notification &packet);
		void PutPacketIn(Notification &packet, int);
		int QueueSize();
};

Notification FIFO :: GetFirstPacket()
{
	return(m_queue.front());	
}; 

Notification FIFO :: GetPacketAt(int n)
{
	return(m_queue.at(n));	
}; 


void FIFO :: DelFirstPacket()
{
	m_queue.pop_front();
}; 

void FIFO :: PutPacket(Notification &packet)
{	
	m_queue.push_back(packet);
}; 

void FIFO :: PutPacketFront(Notification &packet)
{	
	m_queue.push_front(packet);
}; 

int FIFO :: QueueSize()
{
	return(m_queue.size());
}; 

void FIFO :: PutPacketIn(Notification & packet,int i)
{
	m_queue.insert(m_queue.begin()+i,packet);
}; 

void FIFO :: DeletePacketIn(int i)
{
	m_queue.erase(m_queue.begin()+i);
};


