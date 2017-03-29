#include <assert.h> 
 #include<vector> 
template <class T> class compcxx_array { public: 
virtual ~compcxx_array() { for (typename std::vector<T*>::iterator i=m_elements.begin();i!=m_elements.end();i++) delete (*i); } 
void SetSize(unsigned int n) { for(unsigned int i=0;i<n;i++)m_elements.push_back(new T); } 
T& operator [] (unsigned int i) { assert(i<m_elements.size()); return(*m_elements[i]); } 
unsigned int size() { return m_elements.size();} 
private: std::vector<T*> m_elements; }; 
class compcxx_component; 
template <class T> class compcxx_functor {public: 
void Connect(compcxx_component&_c, T _f){ c.push_back(&_c); f.push_back(_f); } 
protected: std::vector<compcxx_component*>c; std::vector<T> f; }; 
class compcxx_component { public: 
typedef void  (compcxx_component::*Node_outportSelfStartTX_f_t)(Notification &notification);
typedef void  (compcxx_component::*Node_outportSelfFinishTX_f_t)(Notification &notification);
typedef void  (compcxx_component::*Node_outportSendLogicalNack_f_t)(LogicalNack &logical_nack);
typedef void  (compcxx_component::*Node_outportAskForTxModulation_f_t)(Notification &notification);
typedef void  (compcxx_component::*Node_outportAnswerTxModulation_f_t)(Notification &notification);
};
