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
typedef void  (compcxx_component::*Agent_outportRequestInformationToAp_f_t)();
typedef void  (compcxx_component::*Agent_outportSendConfigurationToAp_f_t)(Configuration &new_configuration);
typedef void  (compcxx_component::*Agent_outportAnswerToController_f_t)(int agent_id, Configuration &configuration,
			Performance &performance, Action *actions, int processed_configuration);
typedef void  (compcxx_component::*CentralController_outportSendCommandToAgent_f_t)(int destination_agent_id, int command_id,
			Configuration &new_configuration, double shared_performance, int type_of_reward);
typedef void  (compcxx_component::*Node_outportSelfStartTX_f_t)(Notification &notification);
typedef void  (compcxx_component::*Node_outportSelfFinishTX_f_t)(Notification &notification);
typedef void  (compcxx_component::*Node_outportSendLogicalNack_f_t)(LogicalNack &logical_nack_info);
typedef void  (compcxx_component::*Node_outportAskForTxModulation_f_t)(Notification &notification);
typedef void  (compcxx_component::*Node_outportAnswerTxModulation_f_t)(Notification &notification);
typedef void  (compcxx_component::*Node_outportAnswerToAgent_f_t)(Configuration &configuration, Performance &performance);
typedef void  (compcxx_component::*Node_outportSetNewWlanConfiguration_f_t)(Configuration &new_configuration);
typedef void  (compcxx_component::*Node_outportRequestSpatialReuseConfiguration_f_t)();
typedef void  (compcxx_component::*Node_outportNewSpatialReuseConfiguration_f_t)(Configuration &new_configuration);
typedef void  (compcxx_component::*TrafficGenerator_outportNewPacketGenerated_f_t)();
};
