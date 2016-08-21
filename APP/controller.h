#ifndef	__CONTROLLER_H__
#define __CONTROLLER_H__

// Show calendar task
#define SLOT0 PAout(2)  //PA4
#define SLOT1 PAout(4)	//PC4
#define SLOT_ON (0)
#define SLOT_OFF (1)

void Controller_Init(void);
void Controller_RelayAction(u8 ids, u8 op);

#endif

