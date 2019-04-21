#ifndef _BUS_H_
#define _BUS_H_

#include <stdint.h>
#include <list>
#include <array>

#include "object.h"
#include "thread.h"

#include "def.h"
#include "events.h"

class Bus;
class BusModule;

class BusEvent : public Object {
	friend class Bus;
public:
	BusEvent();
	int32_t init(BusModule *from, int32_t id, const char *desc, void *data);

	int32_t addRef();
	int32_t release();

	int32_t	getEventId() { return m_Id; }
	void*	getData() { return m_Data; }
	const char*	getDesc() { return getObjectName(); }
	BusModule*  getFrom() { return m_From; }
    uint32_t    getSeqNo() { return m_SeqNo; }

    uint32_t    getRespSeqNo() { return m_RespSeqNo; }
    void        setRespSeqNo(uint32_t seqNo) { m_RespSeqNo = seqNo; }

protected:
	virtual ~BusEvent();
	
	uint32_t generateSeqNo();

private:
	int32_t		m_Id;
	void *		m_Data;

	int32_t		m_RefCount;

	uint32_t	m_SeqNo;
	BusModule *	m_From;

    uint32_t    m_RespSeqNo;

	static pthread_mutex_t m_GlobalMutex;
	static uint32_t m_GlobalUniqueSeqNo;
};

class BusModule : public Object {
public:
	BusModule();
	virtual ~BusModule();

	void	        setBusModuleId(int32_t id) { m_Id = id; }
	int32_t         getBusModuleId() { return m_Id; }
    
	bool            setBusModuleDesc(const char *desc);
    const char *    getBusModuleDesc()  { return getObjectName(); }
    
	int32_t         setBus(Bus *bus);

	int             postEvent(BusEvent *event);
    int32_t         postEventSychronous(BusEvent *event);
	int             dispatchEvent(BusEvent *event);
	int32_t         subscribeEvent(event_id id);
	int32_t         unsubscribeEvent(event_id id);

    virtual void    onAttach();
    virtual void    onDetach();
    
protected:
	virtual int activateEvent(BusEvent *event) = 0;

private:
	std::list<event_id> m_SubscribedId;
	Bus*				m_Bus;

	int32_t	m_Id;
};

#define MODULE_MAX_AMOUNT	32

class Bus : public Thread {
public:
	Bus();
	virtual ~Bus();

	int32_t allocModuleId();

	int32_t attachModule(BusModule *module);
	int32_t detachModule(BusModule *module);

	int32_t dispatchEvent(BusEvent *event);
    int32_t dispatchEventSynchronous(BusEvent *event);

protected:
	int32_t detachModule(uint32_t moduleId);

	virtual void runOnce();

private:
	uint32_t		m_ModuleIndex;
	std::array<BusModule *, MODULE_MAX_AMOUNT>  m_ModuleArray;
	std::list<BusEvent *>   m_EventList;
    std::list<BusEvent *>   m_SyncEventList;
};

#endif
