/*License LGPL v3. guo8113@126.com*/
#include "alarm.h"
#include "rtc.h"
#include "24cxx.h"
#include "string.h"

alarm_control_t g_alarmObj;
alarm_control_t tempObj;

//找到距离当前时间最近的需要设置的alarm，返回index
static u8 Utilites_FindAlarm2Set(void)
{
	u8 i;
	u8 index=255;
	u32 min = 0xffffffff;
	for(i=0;i<ALARM_COUNT;i++)
	{
		if (g_alarmObj.seccount[i]!=0&& (g_alarmObj.hasSet[i]==0))
		{
			//g_alarmObj.seccount[i] = Alarm_CheckValidAlarm(g_alarmObj.seccount[i]);
			if((g_alarmObj.seccount[i] < min) )
			{
					index=i;
					min = g_alarmObj.seccount[i];
			}
		}
	}  
	return index;
}

static u32 Alarm_CheckValidAlarm(u32 count)
{
	u32 cur;
	u32 temp;
	cur = Utilites_Calendar2Count(calendar.w_year, calendar.w_month, calendar.w_date,calendar.hour,calendar.min, calendar.sec);
	if ((count <= cur) && (count !=0))
	{
		temp = (cur -count)/ONEDAY;
		return count+(temp+1)*ONEDAY;
	}
	else
	{
		return count;
	}
}
/* This funtion will set the latest alarm to RTC ALARM register, before setting, validate all alarm*/
void Alarm_SetOneAlarm(u8 flag)
{
	 u8 index,i;
	
	 /* Adjust all alarm to the future time*/
	 for(i=0;i<ALARM_COUNT;i++)
	 {
		  g_alarmObj.seccount[i] = Alarm_CheckValidAlarm(g_alarmObj.seccount[i]);
	 }
	 
	 /* Find the alarm to set */
	 index = Utilites_FindAlarm2Set();
	if(index< ALARM_COUNT)
	{
		 if(flag) // 闹钟触发进入的配置
		 {
			   RTC_SetAlarm(g_alarmObj.seccount[index]);
			   g_alarmObj.current = index;
				 g_alarmObj.hasSet[index] = 1;// Indicate we have set this alarm
		 }
		 else if(g_alarmObj.current < ALARM_COUNT) // 新添闹钟设置进入的配置
		 {
			   if(g_alarmObj.seccount[index]<g_alarmObj.seccount[g_alarmObj.current])
				 {
					  g_alarmObj.hasSet[g_alarmObj.current] = 0;
			 
						 /* Adjust all alarm to the future time*/
						for(i=0;i<ALARM_COUNT;i++)
						{
								g_alarmObj.seccount[i] = Alarm_CheckValidAlarm(g_alarmObj.seccount[i]);
						}
						index = Utilites_FindAlarm2Set(); // re-find the alarm to set
						
						RTC_SetAlarm(g_alarmObj.seccount[index]);
						g_alarmObj.current = index;
						
						if(g_alarmObj.repeat[index] ==0)
						{
								g_alarmObj.hasSet[index] = 1;// Indicate we have set this alarm		
						}							
				 }
				 else
					 return;
		 }
		 else
		 {
			   RTC_SetAlarm(g_alarmObj.seccount[index]);
			   g_alarmObj.current = index;
				 g_alarmObj.hasSet[index] = 1;// Indicate we have set this alarm
		 }
		 
		 if (g_alarmObj.repeat[index]!=0)// alarm should repeat, so alway enable it.
		 {
				g_alarmObj.hasSet[index] = 0;
		 }
  }
	else
	{
		 g_alarmObj.current = 0xff; // No alarm set.
	}
}

//读取保存的alarm值，调用前需初始化AT24C08
void Alarm_LoadAlarmSetting(void)
{
	 AT24CXX_Read(ALARM_SAVE_ADDR, (void*)&tempObj,sizeof(g_alarmObj));
	 memcpy((void*)&g_alarmObj, (void *)&tempObj, sizeof(g_alarmObj));
}

void  Alarm_StoreAlarmSetting(void)
{
	AT24CXX_Write(ALARM_SAVE_ADDR, (u8*)&g_alarmObj, sizeof(g_alarmObj));
}


void Alarm_UpdateAlarmObj(u32 count, u8 dest, u8 isPeriod, action_t isOn)
{
	g_alarmObj.seccount[g_alarmObj.next] = Alarm_CheckValidAlarm(count); // check valiade then write.
	g_alarmObj.repeat[g_alarmObj.next] = isPeriod;
	g_alarmObj.action[g_alarmObj.next] = isOn;
	g_alarmObj.dest[g_alarmObj.next] = dest;
	g_alarmObj.hasSet[g_alarmObj.next] = 0;
  g_alarmObj.next ++;
  if(g_alarmObj.next>=ALARM_COUNT)
	{
	  g_alarmObj.next = 0;
	}
}
void Alarm_DelOneAlarm(u8 index) // Only set to 0
{
	g_alarmObj.seccount[index] = 0;
	g_alarmObj.repeat[index] = 0;
	g_alarmObj.action[index] = kOff;
	g_alarmObj.dest[index] = kSlotNone;
	g_alarmObj.hasSet[index] = 1;
}
