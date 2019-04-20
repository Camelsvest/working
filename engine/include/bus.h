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
	char*	getDesc() { return m_Desc; }
	BusModule* getFrom() { return m_From; }
	BusModule* getDest() { return m_Dest; }
	void	setDest(BusModule *module) { m_Dest = module; }

protected:
	virtual ~BusEvent();
	
	uint32_t generateSeqNo();

private:
	int32_t		m_Id;
	void *		m_Data;
	char *		m_Desc;
	int32_t		m_RefCount;

	uint32_t	m_SeqNo;	
	BusModule *	m_From;
	BusModule * m_Dest;

	static pthread_mutex_t m_GlobalMutex;
	static uint32_t m_GlobalUniqueSeqNo;
};

class BusModule : public Object {
public:
	BusModule();
	virtual ~BusModule();

	void	setBusModuleId(int32_t id) { m_Id = id; }
	int32_t getBusModuleId() { return m_Id; }
	int32_t setBusModuleDesc(const char *desc);
	int32_t setBus(Bus *bus);

	int postEvent(BusEvent *event);
	int dispatchEvent(BusEvent *event);
	int32_t subscribeEvent(event_id id);
	int32_t unsubscribeEvent(event_id id);

protected:
	virtual int activateEvent(BusEvent *event) = 0;

private:
	std::list<event_id> m_SubscribedId;
	Bus*				m_Bus;

	int32_t	m_Id;
	char *	m_Desc;
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
	int32_t dispatchEventToModule(BusModule *module, BusEvent *event);

protected:
	int32_t detachModule(uint32_t moduleId);

	virtual void runOnce();

private:
	uint32_t		m_ModuleIndex;
	std::array<BusModule *, MODULE_MAX_AMOUNT>  m_ModuleArray;
	std::list<BusEvent *>   m_EventList;
};

#endif
