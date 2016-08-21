/*License LGPL v3. guo8113@126.com*/
#include	"math.h"
#include	"stdio.h"
#include  "string.h"
#include	"calendar.h"

#include  "FreeRTOS.h"  		//OS系统函数头文件
#include  "Semphr.h"
#include   "task.h"

#include	"GUI.h"
#include 	"WM.h"
#include  "DIALOG.h"
#include  "BUTTON.h"
#include  "MENU.h"

#include	"rtc.h"
#include	"lcd.h"
#include "alarm.h"

/*
***************************************************************
						宏定义区
****************************************************************
*/
#define ID_MENU_DATE       (GUI_ID_USER +  0)
#define ID_MENU_TIME       (GUI_ID_USER +  1)
#define ID_MENU_EXIT       (GUI_ID_USER +  2)


#define GUI_ID_BUT0				 (GUI_ID_USER +  3)
#define GUI_ID_BUT1				 (GUI_ID_USER +  4)
#define GUI_ID_BUT2				 (GUI_ID_USER +  5)
#define GUI_ID_BUT3				 (GUI_ID_USER +  6)
#define GUI_ID_BUT4				 (GUI_ID_USER +  7)
#define GUI_ID_BUT5				 (GUI_ID_USER +  8)
#define GUI_ID_BUT6				 (GUI_ID_USER +  9)
#define GUI_ID_BUT7				 (GUI_ID_USER +  10)

#define GUI_ID_T0						(GUI_ID_USER +  11)
#define GUI_ID_T1						(GUI_ID_USER +  12)
#define GUI_ID_T2						(GUI_ID_USER +  13)
#define GUI_ID_T3						(GUI_ID_USER +  14)
#define GUI_ID_T4						(GUI_ID_USER +  15)

#define GUI_ID_BUT8         (GUI_ID_USER +  30)
#define GUI_ID_BUT9					(GUI_ID_USER +  31)
#define ID_MENU_SHOW_ALARM	(GUI_ID_USER +  17)


#define PI 3.1415926
#define SLEEP 1000     //空闲时关闭屏幕的时间SLEEP/100 s
#define ACLOCKX 150  //模拟时钟圆心x坐标
#define ACLOCKY 180			//模拟时钟圆心y坐标
#define ACLOCKR	60			//模拟时钟半径
#define DCLOCKY 100		//数字时钟起始y坐标

/*********************************************************************
*
*       		全局变量
*
**********************************************************************
*/
u8 IsCloseCalendar=0;			//控制万年历界面的开关
u8 IsOpenSetDate=0;
u8 IsOpenSetTime=0;
u8 IsOpenShowAlarm=0;

static u32 lcdIdleCount = 0;

static u8 isAlarm=0;   //控制设置时间/闹钟
static u8 isPeriod=0;  //是否是重复闹钟
static action_t isOn=kOff;
static u8 slotSel=kSlot1; //dest_t slot;
static s8 alarmSelected=-1; // Which alarm is selected
u8 listBoxMap[ALARM_COUNT]={0}; // Map between alarm index and listbox index, [1...8]

_calendar_obj TimerSet;						//用来保存临时的设置信息
u8 ReDrawDesktop = 0;

u8 *week_str[7]=
{
	"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
};
extern const u8 mon_table[];
extern SemaphoreHandle_t xSemAlarm;
/*********************************************************************
*
*       Dialog resource
*
* This table conatins the info required to create the dialog.
* It has been created by ucGUIbuilder.
*/
//struct GUI_WIDGET_CREATE_INFO_struct {
//  GUI_WIDGET_CREATE_FUNC* pfCreateIndirect;
//  const char* pName;                     /* Text ... Not used on all widgets */
//  I16 Id;                                /* ID ... should be unique in a dialog */
//  I16 x0, y0, xSize, ySize;              /* Define position and size */
//  U16 Flags;                             /* Widget specific create flags (opt.) */
//  I32 Para;                              /* Widget specific parameter (opt.) */ 
//};
static const GUI_WIDGET_CREATE_INFO _SetDateDialogCreate[] = {
    { FRAMEWIN_CreateIndirect,  "Set date",          0,                       20,  45,  200,150,FRAMEWIN_CF_ACTIVE,0},
    { BUTTON_CreateIndirect,     "+",               GUI_ID_BUTTON0,          29, 3,  24, 20, 0,0},
    { BUTTON_CreateIndirect,     "-",               GUI_ID_BUTTON1,          29, 62, 24, 20, 0,0},
    { BUTTON_CreateIndirect,     "+",               GUI_ID_BUTTON2,          87, 3,  24, 20, 0,0},
    { BUTTON_CreateIndirect,     "-",               GUI_ID_BUTTON3,          85, 62, 24, 20, 0,0},
    { BUTTON_CreateIndirect,     "+",               GUI_ID_BUTTON4,          151,3,  24, 20, 0,0},
    { BUTTON_CreateIndirect,     "-",               GUI_ID_BUTTON5,          151,62, 24, 20, 0,0},
    { TEXT_CreateIndirect,       "",                 GUI_ID_TEXT0,            29, 33, 40, 16, 0,0},
    { TEXT_CreateIndirect,       "",                GUI_ID_TEXT1,            87, 33, 22, 16, 0,0},
    { TEXT_CreateIndirect,       "",                GUI_ID_TEXT2,            151,33, 22, 16, 0,0},
    { BUTTON_CreateIndirect,    "Cancel",            GUI_ID_BUTTON6,          0,  95, 95, 30, 0,0},
    { BUTTON_CreateIndirect,    "Set",               GUI_ID_BUTTON7,          99, 95, 95, 30, 0,0}
};

static const GUI_WIDGET_CREATE_INFO _SetTimeDialogCreate[] = {
    { FRAMEWIN_CreateIndirect,  "Set time",          0,                       20,  45,  200,200,FRAMEWIN_CF_ACTIVE,0},//x0, y0, xSize, ySize;
    { BUTTON_CreateIndirect,     "+",               GUI_ID_BUT0,          5, 3,  24, 20, 0,0},
    { BUTTON_CreateIndirect,     "-",               GUI_ID_BUT1,          5, 62, 24, 20, 0,0},
    { BUTTON_CreateIndirect,     "+",               GUI_ID_BUT2,          45, 3,  24, 20, 0,0},
    { BUTTON_CreateIndirect,     "-",               GUI_ID_BUT3,          45, 62, 24, 20, 0,0},
    { BUTTON_CreateIndirect,     "+",               GUI_ID_BUT4,          90,3,  24, 20, 0,0},
    { BUTTON_CreateIndirect,     "-",               GUI_ID_BUT5,          90,62, 24, 20, 0,0},
    { TEXT_CreateIndirect,      "",                  GUI_ID_T0,            10, 33, 32, 16, 0,0},
    { TEXT_CreateIndirect,      "",                  GUI_ID_T1,            55, 33, 32, 16, 0,0},
    { TEXT_CreateIndirect,      "",                  GUI_ID_T2,            88,33, 32, 16, 0,0},
		{ TEXT_CreateIndirect,      ":",                 GUI_ID_T3,            40, 33, 10, 16, 0,0},
		{ CHECKBOX_CreateIndirect,  0,									GUI_ID_CHECK0,		120, 3, 68, 20, 0, 0},		// alarm
		{ CHECKBOX_CreateIndirect,  0,									GUI_ID_CHECK1,		120, 33, 68, 20, 0, 0},		// repeat
		{	CHECKBOX_CreateIndirect,  0,									GUI_ID_CHECK2, 		120, 62,68, 20, 0, 0}, 		//off/on
		{ TEXT_CreateIndirect,      "Slot:",             GUI_ID_T4,            5, 95, 30, 16, 0,0},
		{ CHECKBOX_CreateIndirect,  0,									GUI_ID_CHECK3,		40, 95, 50, 20, 0, 0},    //Slot1
	  { CHECKBOX_CreateIndirect,  0,									GUI_ID_CHECK4,		100, 95, 50, 20, 0, 0},		//Slot2
		{ BUTTON_CreateIndirect,    "Cancel",            GUI_ID_BUT6,          0,  145, 95, 30, 0,0},
    { BUTTON_CreateIndirect,    "Set",               GUI_ID_BUT7,          99, 145, 95, 30, 0,0}
};
GUI_WIDGET_CREATE_INFO _ShowAlarmDialogCreate[] = {
    { FRAMEWIN_CreateIndirect,  "Alarm Settings",    0,                       20,  45,  200,210,FRAMEWIN_CF_ACTIVE,0},
    { TEXT_CreateIndirect,      "Time",              GUI_ID_TEXT3,            8,  2,  28, 16, 0,0},
	  { TEXT_CreateIndirect,      "Rpt",               GUI_ID_TEXT4,            87, 2,  20, 16, 0,0},
    { TEXT_CreateIndirect,      "En",                GUI_ID_TEXT5,            117,2,  30, 16, 0,0},
		{ TEXT_CreateIndirect,      "Off/On",            GUI_ID_TEXT6,            147,2,  40, 16, 0,0},
    { CHECKBOX_CreateIndirect,   NULL,               GUI_ID_CHECK7,           147,24, 20, 20, 0,0}, // OFF/ON
    { CHECKBOX_CreateIndirect,   NULL,               GUI_ID_CHECK6,           117,24, 20, 20, 0,0}, // Enable
    { CHECKBOX_CreateIndirect,   NULL,               GUI_ID_CHECK5,           87, 24, 19, 15, 0,0}, // Repeat
    { LISTBOX_CreateIndirect,    NULL,               GUI_ID_LISTBOX0,         8,  50, 159,85, 0,0},
		{ BUTTON_CreateIndirect,    "Close",             GUI_ID_BUT8,          8,  164,75, 25, 0,0},
		{ BUTTON_CreateIndirect,    "Del Sel",           GUI_ID_BUT9,          113,164,74, 25, 0,0}
};



/*
***************************************************************
						函数声明区
****************************************************************
*/
static void AddMenuItem(MENU_Handle hMenu, MENU_Handle hSubmenu, char* pText, U16 Id, U16 Flags);
static void CreateMenu(WM_HWIN hWin);
static void ShowSolarCalendar(void);
//static void ShowLunarCalendar(void);
static void InitAnalogClock(void);
static void _SetDateCallback(WM_MESSAGE * pMsg);
static void InitSetDateDialog(WM_MESSAGE * pMsg);
static void InitSetTimeDialog(WM_MESSAGE * pMsg);
//static void InitShowAlarmDialog(WM_MESSAGE * pMsg);
static void _SetTimeCallback(WM_MESSAGE * pMsg) ;


/*****************************************************************
**      FunctionName:void InitDialog(WM_MESSAGE * pMsg)
**      Function: to initialize the Dialog items
**                                                      
**      call this function in _cbCallback --> WM_INIT_DIALOG
*****************************************************************/

static void InitSetDateDialog(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin = pMsg->hWin;
	  char str[5];
	
    //
    //FRAMEWIN
    //
    FRAMEWIN_SetClientColor(hWin,0x00ff00);
    FRAMEWIN_SetFont(hWin,&GUI_Font16_ASCII);
    FRAMEWIN_SetTextAlign(hWin,GUI_TA_VCENTER|GUI_TA_CENTER);
    FRAMEWIN_SetTitleHeight(hWin,18);
    //
    //GUI_ID_BUTTON0
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUTTON0),BUTTON_CI_UNPRESSED,0x00ffff);
    //
    //GUI_ID_BUTTON1
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUTTON1),BUTTON_CI_UNPRESSED,0x00ffff);
    //
    //GUI_ID_BUTTON2
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUTTON2),BUTTON_CI_UNPRESSED,0x00ffff);
    //
    //GUI_ID_BUTTON3
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUTTON3),BUTTON_CI_UNPRESSED,0x00ffff);
    //
    //GUI_ID_BUTTON4
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUTTON4),BUTTON_CI_UNPRESSED,0x00ffff);
    //
    //GUI_ID_BUTTON5
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUTTON5),BUTTON_CI_UNPRESSED,0x00ffff);
		
//		BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUTTON0),0, &bmButtonBmp1_1bpp, 4, 7);
//		BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUTTON2),0, &bmButtonBmp1_1bpp, 4, 7);
//		BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUTTON4),0, &bmButtonBmp1_1bpp, 4, 7);
//		
//		BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUTTON1),0, &bmButtonBmp2_1bpp, 4, 7);
//		BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUTTON3),0, &bmButtonBmp2_1bpp, 4, 7);
//    BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUTTON5),0, &bmButtonBmp2_1bpp, 4, 7);			
    //
    //GUI_ID_TEXT0
    //
    TEXT_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_TEXT0),0x00ff00);
    TEXT_SetTextColor(WM_GetDialogItem(hWin,GUI_ID_TEXT0),0xffffff);
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_TEXT0),&GUI_Font16_ASCII);
		TimerSet.w_year = calendar.w_year;
		sprintf(str,"%4d",TimerSet.w_year);
		TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT0),str);
    //
    //GUI_ID_TEXT1
    //
    TEXT_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_TEXT1),0x00ff00);
    TEXT_SetTextColor(WM_GetDialogItem(hWin,GUI_ID_TEXT1),0xffffff);
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_TEXT1),&GUI_Font16_ASCII);
		
		TimerSet.w_month = calendar.w_month;
		sprintf(str,"%02d",TimerSet.w_month);
		TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT1),str);
    //
    //GUI_ID_TEXT2
    //
    TEXT_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_TEXT2),0x00ff00);
    TEXT_SetTextColor(WM_GetDialogItem(hWin,GUI_ID_TEXT2),0xffffff);
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_TEXT2),&GUI_Font16_ASCII);
		
		TimerSet.w_date = calendar.w_date;
		sprintf(str,"%02d",TimerSet.w_date);
		TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT2),str);
    //
    //GUI_ID_BUTTON6
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUTTON6),BUTTON_CI_UNPRESSED,0xffff00);
    BUTTON_SetFont(WM_GetDialogItem(hWin,GUI_ID_BUTTON6),&GUI_Font16_ASCII);
    //
    //GUI_ID_BUTTON7
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUTTON7),BUTTON_CI_UNPRESSED,0xffff00);
    BUTTON_SetFont(WM_GetDialogItem(hWin,GUI_ID_BUTTON7),&GUI_Font16_ASCII);

}


static void InitSetTimeDialog(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin = pMsg->hWin;
		char str[5];
	
		isOn = kOff;
		isPeriod =0;
		isAlarm =0;
	  slotSel = kSlot1;
		
    //
    //FRAMEWIN
    //
    FRAMEWIN_SetClientColor(hWin,0x00ff00);
    FRAMEWIN_SetFont(hWin,&GUI_Font16_ASCII);
    FRAMEWIN_SetTextAlign(hWin,GUI_TA_VCENTER|GUI_TA_CENTER);
    FRAMEWIN_SetTitleHeight(hWin,18);
    //
    //GUI_ID_BUTTON0
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUT0),BUTTON_CI_UNPRESSED,0x00ffff);
    //
    //GUI_ID_BUTTON1
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUT1),BUTTON_CI_UNPRESSED,0x00ffff);
    //
    //GUI_ID_BUTTON2
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUT2),BUTTON_CI_UNPRESSED,0x00ffff);
    //
    //GUI_ID_BUTTON3
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUT3),BUTTON_CI_UNPRESSED,0x00ffff);
    //
    //GUI_ID_BUTTON4
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUT4),BUTTON_CI_UNPRESSED,0x00ffff);
    //
    //GUI_ID_BUTTON5
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUT5),BUTTON_CI_UNPRESSED,0x00ffff);
		
//		BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUT0),0, &bmButtonBmp1_1bpp, 4, 7);
//		BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUT2),0, &bmButtonBmp1_1bpp, 4, 7);
//		BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUT4),0, &bmButtonBmp1_1bpp, 4, 7);
//		
//		BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUT1),0, &bmButtonBmp2_1bpp, 4, 7);
//		BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUT3),0, &bmButtonBmp2_1bpp, 4, 7);
//    BUTTON_SetBitmapEx (WM_GetDialogItem(hWin,GUI_ID_BUT5),0, &bmButtonBmp2_1bpp, 4, 7);				
		
    //
    //GUI_ID_TEXT0
    //
    TEXT_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_T0),0x00ff00);
    TEXT_SetTextColor(WM_GetDialogItem(hWin,GUI_ID_T0),0xffffff);
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_T0),&GUI_Font16_ASCII);
		
		TimerSet.hour = calendar.hour;
		sprintf(str,"%02d",TimerSet.hour);
		TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_T0),str);		
    //
    //GUI_ID_TEXT1
    //
    TEXT_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_T1),0x00ff00);
    TEXT_SetTextColor(WM_GetDialogItem(hWin,GUI_ID_T1),0xffffff);
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_T1),&GUI_Font16_ASCII);
		
		TimerSet.min = calendar.min;
		sprintf(str,"%02d",TimerSet.min);
		TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_T1),str);			
    //
    //GUI_ID_TEXT2
    //
    TEXT_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_T2),0x00ff00);
    TEXT_SetTextColor(WM_GetDialogItem(hWin,GUI_ID_T2),0xffffff);
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_T2),&GUI_Font16_ASCII);
		
		TimerSet.sec = calendar.sec;
		sprintf(str,"%02d",TimerSet.sec);
		TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_T2),str);				
		
    //
    //GUI_ID_BUTTON6
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUT6),BUTTON_CI_UNPRESSED,0xffff00);
    BUTTON_SetFont(WM_GetDialogItem(hWin,GUI_ID_BUT6),&GUI_Font16_ASCII);
    //
    //GUI_ID_BUTTON7
    //
    BUTTON_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_BUT7),BUTTON_CI_UNPRESSED,0xffff00);
    BUTTON_SetFont(WM_GetDialogItem(hWin,GUI_ID_BUT7),&GUI_Font16_ASCII);
    //
    //GUI_ID_TEXT3
    //
    TEXT_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_T3),0x00ff00);
    TEXT_SetTextColor(WM_GetDialogItem(hWin,GUI_ID_T3),0xffffff);
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_T3),&GUI_Font16_ASCII);
    //
    //GUI_ID_TEXT4
    //
    TEXT_SetBkColor(WM_GetDialogItem(hWin,GUI_ID_T4),0x00ff00);
    TEXT_SetTextColor(WM_GetDialogItem(hWin,GUI_ID_T4),0xffffff);
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_T4),&GUI_Font16_ASCII);
		
		//Checkbox
		CHECKBOX_SetFont(WM_GetDialogItem(hWin,GUI_ID_CHECK0), &GUI_Font16_ASCII);
		CHECKBOX_SetFont(WM_GetDialogItem(hWin,GUI_ID_CHECK1), &GUI_Font16_ASCII);
		CHECKBOX_SetText(WM_GetDialogItem(hWin,GUI_ID_CHECK0), "Alarm");
		CHECKBOX_SetText(WM_GetDialogItem(hWin,GUI_ID_CHECK1), "Repeat");
		CHECKBOX_SetText(WM_GetDialogItem(hWin,GUI_ID_CHECK2), "Off/On");
	  CHECKBOX_SetText(WM_GetDialogItem(hWin,GUI_ID_CHECK3), "Slot1");
		CHECKBOX_SetState(WM_GetDialogItem(hWin,GUI_ID_CHECK3), 1);  //Note slot1 was selected by default
		CHECKBOX_SetText(WM_GetDialogItem(hWin,GUI_ID_CHECK4), "Slot2");

}

static void PaintShowAlarmDialog(WM_MESSAGE * pMsg)
{
    //WM_HWIN hWin = pMsg->hWin;
    GUI_SetColor(0x000000);
    GUI_DrawLine(8,20,183,20);
}
static void InitShowAlarmDialog(WM_MESSAGE * pMsg)
{
	  int i,j,cur;
	  _calendar_obj tempCalendar;
	  char alarm[9]={0};
	  WM_HWIN hWin = pMsg->hWin;

	 alarmSelected = -1;
	 memset(listBoxMap, 0, sizeof(listBoxMap));
    //
    //FRAMEWIN
    //
    FRAMEWIN_AddCloseButton(hWin, FRAMEWIN_BUTTON_RIGHT, 0);
    //
    //GUI_ID_BUTTON0
    //
    BUTTON_SetFont(WM_GetDialogItem(hWin,GUI_ID_BUT8),&GUI_Font16_ASCII);
    //
    //GUI_ID_TEXT0
    //
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_TEXT3),&GUI_Font16_ASCII);
    //
    //GUI_ID_BUTTON1
    //
    BUTTON_SetFont(WM_GetDialogItem(hWin,GUI_ID_BUT9),&GUI_Font16_ASCII);
    //
    //GUI_ID_TEXT2
    //
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_TEXT4),&GUI_Font16_ASCII);
    //
    //GUI_ID_TEXT3
    //
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_TEXT5),&GUI_Font16_ASCII);
    //
    //GUI_ID_TEXT3
    //
    TEXT_SetFont(WM_GetDialogItem(hWin,GUI_ID_TEXT6),&GUI_Font16_ASCII);
    //
    //GUI_ID_LISTBOX0
    //
    LISTBOX_SetFont(WM_GetDialogItem(hWin,GUI_ID_LISTBOX0),&GUI_Font16_ASCII);
    LISTBOX_SetAutoScrollV(WM_GetDialogItem(hWin,GUI_ID_LISTBOX0),1);
    LISTBOX_SetAutoScrollH(WM_GetDialogItem(hWin,GUI_ID_LISTBOX0),1);

   
		for(i=0, j=0;i<ALARM_COUNT;i++)
		{
			 if((g_alarmObj.seccount[i] >0) && (g_alarmObj.seccount[i]<0xffffffff))
			 {
				  Utilites_Count2Calendar(&tempCalendar, g_alarmObj.seccount[i]);
				  sprintf(alarm,"%2d:%02d:%02d",tempCalendar.hour, tempCalendar.min, tempCalendar.sec);
				 
					LISTBOX_AddString(WM_GetDialogItem(hWin,GUI_ID_LISTBOX0), alarm);
					listBoxMap[j++] = i+1; //0表示没有map，所以 +  1 
			 }
		}
	if(listBoxMap[0]) // if listbox not none
	{
		 cur = listBoxMap[0]-1;
	   LISTBOX_SetSel(WM_GetDialogItem(hWin,GUI_ID_LISTBOX0),0);
	   CHECKBOX_SetState(WM_GetDialogItem(hWin,GUI_ID_CHECK5),g_alarmObj.repeat[cur]);
		 if(g_alarmObj.hasSet[cur])
				CHECKBOX_SetState(WM_GetDialogItem(hWin,GUI_ID_CHECK6),0); //Enable
		 else
				CHECKBOX_SetState(WM_GetDialogItem(hWin,GUI_ID_CHECK6),1); //Enable
		 CHECKBOX_SetState(WM_GetDialogItem(hWin,GUI_ID_CHECK7),g_alarmObj.action[cur]); //Off/On
		 alarmSelected = cur;
	} 
}

/*********************************************************************
*
*       Dialog callback routine
*/
static void _SetDateCallback(WM_MESSAGE * pMsg) 
{
    int NCode, Id;
		char str[5];
    WM_HWIN hWin = pMsg->hWin;
    switch (pMsg->MsgId) 
    {
				case WM_DELETE:
						IsOpenSetDate=0;
						break;
        case WM_INIT_DIALOG:
            InitSetDateDialog(pMsg);
            break;
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ESCAPE:
                    GUI_EndDialog(hWin, 1);
                    break;
                case GUI_KEY_ENTER:
                    GUI_EndDialog(hWin, 0);
                    break;
            }
            break;
        case WM_NOTIFY_PARENT:
            Id = WM_GetId(pMsg->hWinSrc); 
            NCode = pMsg->Data.v;        
            switch (Id) 
            {
                case GUI_ID_BUTTON7:		//设置
                    if(NCode==WM_NOTIFICATION_RELEASED){
                        GUI_EndDialog(hWin, 0);
											  RTC_Set(TimerSet.w_year,TimerSet.w_month,TimerSet.w_date,calendar.hour,calendar.min,calendar.sec);//设置时间						
										}
                    break;
                case GUI_ID_BUTTON6:		//取消
                    if(NCode==WM_NOTIFICATION_RELEASED)
                        GUI_EndDialog(hWin, 0);
                    break;
								case GUI_ID_BUTTON0:
										if(NCode==WM_NOTIFICATION_RELEASED){
											  TimerSet.w_year++;
												if(TimerSet.w_year>=2099){
														TimerSet.w_year=2099;
												}
												sprintf(str,"%04d",TimerSet.w_year);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT0),str);
										}
										break;
								case GUI_ID_BUTTON1:
										if(NCode==WM_NOTIFICATION_RELEASED){
												TimerSet.w_year--;
												if(TimerSet.w_year<=2000){
													TimerSet.w_year=2000;
												}
												sprintf(str,"%04d",TimerSet.w_year);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT0),str);										
										}
										break;										
								case GUI_ID_BUTTON2:
										if(NCode==WM_NOTIFICATION_RELEASED){
												TimerSet.w_month++;
												if(TimerSet.w_month>=12){
														TimerSet.w_month=12;
												}
												sprintf(str,"%02d",TimerSet.w_month);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT1),str);
												
												if(TimerSet.w_date>=(mon_table[TimerSet.w_month])){
														TimerSet.w_date=mon_table[TimerSet.w_month];
												}
												sprintf(str,"%02d",TimerSet.w_date);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT2),str);														
												
										}
										break;
								case GUI_ID_BUTTON3:
										if(NCode==WM_NOTIFICATION_RELEASED){
												TimerSet.w_month--;
												if(TimerSet.w_month<=1){
														TimerSet.w_month=1;
												}
												sprintf(str,"%02d",TimerSet.w_month);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT1),str);			

												if(TimerSet.w_date>=(mon_table[TimerSet.w_month])){
														TimerSet.w_date=mon_table[TimerSet.w_month];
												}
												sprintf(str,"%02d",TimerSet.w_date);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT2),str);														
										}
										break;										
								case GUI_ID_BUTTON4:
										if(NCode==WM_NOTIFICATION_RELEASED){
												TimerSet.w_date++;
												if(TimerSet.w_date>=(mon_table[TimerSet.w_month])){
														TimerSet.w_date=mon_table[TimerSet.w_month];
												}
												sprintf(str,"%02d",TimerSet.w_date);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT2),str);														
										}
										break;				
								case GUI_ID_BUTTON5:
										if(NCode==WM_NOTIFICATION_RELEASED){
												TimerSet.w_date--;
												if(TimerSet.w_date<=1){
														TimerSet.w_date=1;
												}
												sprintf(str,"%02d",TimerSet.w_date);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_TEXT2),str);											
										}
										break;										
            }
            break;
        default:
            WM_DefaultProc(pMsg);
    }
}

static void _SetTimeCallback(WM_MESSAGE * pMsg) 
{
    int NCode, Id;
		u8 temp;
		unsigned int alarm;
		char str[5];
    WM_HWIN hWin = pMsg->hWin;
    switch (pMsg->MsgId) 
    {
				case	WM_DELETE:
						IsOpenSetTime=0;
						break;
        case WM_INIT_DIALOG:
            InitSetTimeDialog(pMsg);
            break;
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ESCAPE:
                    GUI_EndDialog(hWin, 1);
                    break;
                case GUI_KEY_ENTER:
                    GUI_EndDialog(hWin, 0);
                    break;
            }
            break;
        case WM_NOTIFY_PARENT:
            Id = WM_GetId(pMsg->hWinSrc); 
            NCode = pMsg->Data.v;        
            switch (Id) 
            {
                case GUI_ID_BUT7:
                    if(NCode==WM_NOTIFICATION_RELEASED)	//设置
										{
                        GUI_EndDialog(hWin, 0);
												if(isAlarm)//设置闹钟/时间
												{
														if (slotSel ==0)
														{
															GUI_MessageBox("Please select slot!", "Warning", 0);
															break;
														}
														
													  alarm = Utilites_Calendar2Count(calendar.w_year,calendar.w_month,calendar.w_date,TimerSet.hour,TimerSet.min,TimerSet.sec);
														//RTC_SetAlarm(alarm);
														Alarm_UpdateAlarmObj(alarm, slotSel, isPeriod, isOn);
														xSemaphoreGive(xSemAlarm);	// 设置闹钟		
																												
												}else{
													RTC_Set(calendar.w_year,calendar.w_month,calendar.w_date,TimerSet.hour,TimerSet.min,TimerSet.sec);//设置时间											
												}
                    }
										break;
                case GUI_ID_BUT6:
                    if(NCode==WM_NOTIFICATION_RELEASED)//取消
                        GUI_EndDialog(hWin, 0);
                    break;
								case GUI_ID_BUT0:
										if(NCode==WM_NOTIFICATION_RELEASED){
											  TimerSet.hour++;
												if(TimerSet.hour>=23){
														TimerSet.hour=0;
												}
												sprintf(str,"%02d",TimerSet.hour);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_T0),str);											
										}
										break;
								case GUI_ID_BUT1:
										if(NCode==WM_NOTIFICATION_RELEASED){
												TimerSet.hour--;
												if(TimerSet.hour>23){
														TimerSet.hour=23;
												}
												sprintf(str,"%02d",TimerSet.hour);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_T0),str);												
										}
										break;									
								case GUI_ID_BUT2:
										if(NCode==WM_NOTIFICATION_RELEASED){
											  TimerSet.min++;
												if(TimerSet.min>=59){
														TimerSet.min=0;
												}
												sprintf(str,"%02d",TimerSet.min);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_T1),str);													
										}
										break;									
								case GUI_ID_BUT3:
										if(NCode==WM_NOTIFICATION_RELEASED){
											  TimerSet.min--;
												if(TimerSet.min>59){
														TimerSet.min=59;
												}
												sprintf(str,"%02d",TimerSet.min);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_T1),str);													
										}
										break;																	
								case GUI_ID_BUT4:
										if(NCode==WM_NOTIFICATION_RELEASED){
												TimerSet.sec++;
												if(TimerSet.sec>=59){
														TimerSet.sec=0;
												}
												sprintf(str,"%02d",TimerSet.sec);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_T2),str);											
										}
										break;										
								case GUI_ID_BUT5:
										if(NCode==WM_NOTIFICATION_RELEASED){
											  TimerSet.sec--;
												if(TimerSet.sec>59){
														TimerSet.sec=59;
												}
												sprintf(str,"%02d",TimerSet.sec);
												TEXT_SetText(WM_GetDialogItem(hWin,GUI_ID_T2),str);												
										}
										break;	
								case GUI_ID_CHECK0:
										if(NCode==WM_NOTIFICATION_VALUE_CHANGED){
											isAlarm=CHECKBOX_IsChecked(WM_GetDialogItem(hWin,GUI_ID_CHECK0));
										}
										break;
								case GUI_ID_CHECK1:
										if(NCode==WM_NOTIFICATION_VALUE_CHANGED){
											isPeriod=CHECKBOX_IsChecked(WM_GetDialogItem(hWin,GUI_ID_CHECK1));
										}
										break;
							  case GUI_ID_CHECK2:
										if(NCode==WM_NOTIFICATION_VALUE_CHANGED){
											if(CHECKBOX_IsChecked(WM_GetDialogItem(hWin,GUI_ID_CHECK2))==1)
												isOn = kOn;
											else
												isOn = kOff;
										}
										break;
								case GUI_ID_CHECK3:
										if(NCode==WM_NOTIFICATION_VALUE_CHANGED){
											temp = CHECKBOX_IsChecked(WM_GetDialogItem(hWin,GUI_ID_CHECK3));
											if (temp !=0)
												slotSel |= kSlot1;
											else
												slotSel &=  ~MASK_SLOT1;
										}
										break;
							  case GUI_ID_CHECK4:
										if(NCode==WM_NOTIFICATION_VALUE_CHANGED){
											temp = CHECKBOX_IsChecked(WM_GetDialogItem(hWin,GUI_ID_CHECK4));
											if (temp !=0)
												slotSel |= kSlot2;
											else
												slotSel &=  ~MASK_SLOT2;
										}
										break;
            }
            break;
        default:
            WM_DefaultProc(pMsg);
    }
}

static void _ShowAlarmCallback(WM_MESSAGE * pMsg) 
{
    int NCode, Id;
		u8 temp;
    WM_HWIN hWin = pMsg->hWin;
    switch (pMsg->MsgId) 
    {
        case WM_PAINT:
            PaintShowAlarmDialog(pMsg);
            break;
		    case WM_DELETE:
            IsOpenShowAlarm=0;
						Alarm_StoreAlarmSetting(); // Store Alarm setting
            break;
        case WM_INIT_DIALOG:
            InitShowAlarmDialog(pMsg);
            break;
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ESCAPE:
                    GUI_EndDialog(hWin, 1);
                    break;
                case GUI_KEY_ENTER:
                    GUI_EndDialog(hWin, 0);
                    break;
            }
            break;
        case WM_NOTIFY_PARENT:
            Id = WM_GetId(pMsg->hWinSrc); 
            NCode = pMsg->Data.v;        
            switch (Id) 
            {
                case GUI_ID_BUT8: // cancel button
                    if(NCode==WM_NOTIFICATION_RELEASED)
                        GUI_EndDialog(hWin, 0);
                    break;
                case GUI_ID_BUT9: // delete selected button
                    if(NCode==WM_NOTIFICATION_RELEASED)
										{
                        GUI_EndDialog(hWin, 0);
												Alarm_DelOneAlarm(alarmSelected); // Delete the selected alarm
										}		
                    break;
								case GUI_ID_CHECK5: //Rept
									 if(NCode==WM_NOTIFICATION_VALUE_CHANGED)
									 {
										  temp = CHECKBOX_IsChecked(WM_GetDialogItem(hWin,GUI_ID_CHECK5));
										  g_alarmObj.repeat[alarmSelected] = temp;
									 }
										break;
								case GUI_ID_CHECK6: //Enable
									if(NCode==WM_NOTIFICATION_VALUE_CHANGED)
									{
										  temp = CHECKBOX_IsChecked(WM_GetDialogItem(hWin,GUI_ID_CHECK6));
										 if (temp==0)
										 {
										  g_alarmObj.hasSet[alarmSelected] = 1;
										 }else{
											 g_alarmObj.hasSet[alarmSelected] = 0;
										 }
									}
										break;
								case GUI_ID_CHECK7: //OFF/ON
									if(NCode==WM_NOTIFICATION_VALUE_CHANGED)
									{
										  temp = CHECKBOX_IsChecked(WM_GetDialogItem(hWin,GUI_ID_CHECK7));
										 if (temp==0)
										 {
										  g_alarmObj.action[alarmSelected] = kOff;
										 }else{
											 g_alarmObj.action[alarmSelected] = kOn;
										 } 
									}
										break;	
								case GUI_ID_LISTBOX0:
									 if(NCode==WM_NOTIFICATION_SEL_CHANGED)
									 {
										  alarmSelected = LISTBOX_GetSel(WM_GetDialogItem(hWin,GUI_ID_LISTBOX0));
										  alarmSelected = listBoxMap[alarmSelected]-1;
										 	CHECKBOX_SetState(WM_GetDialogItem(hWin,GUI_ID_CHECK5),g_alarmObj.repeat[alarmSelected]);
											CHECKBOX_SetState(WM_GetDialogItem(hWin,GUI_ID_CHECK6),g_alarmObj.hasSet[alarmSelected] ? 0 : 1); //Enable
											CHECKBOX_SetState(WM_GetDialogItem(hWin,GUI_ID_CHECK7),g_alarmObj.action[alarmSelected]); //Off/On
									 }
									 break;
            }
            break;
        default:
            WM_DefaultProc(pMsg);
    }
}


/*
***************************************************************
*函数名：AddMenuItem
*功能：	 添加菜单及属性
*参数：  hMenu：菜单句柄   hSubmenu：子菜单句柄 pText：菜单文本指针
         Id：菜单标示符 	  Flags：MENU_IF_DISABLED ，MENU_IF_SEPARATOR	 
*返回值：无
****************************************************************
*/
static void AddMenuItem(MENU_Handle hMenu, MENU_Handle hSubmenu, char* pText, U16 Id, U16 Flags) {
  MENU_ITEM_DATA Item;
  Item.pText    = pText;
  Item.hSubmenu = hSubmenu;
  Item.Flags    = Flags;
  Item.Id       = Id;
  MENU_AddItem(hMenu, &Item);
}

/*
***************************************************************
*函数名：CreateMenu
*功能：	 创建菜单
*参数：  hWin：窗口句柄(菜单父窗口) 
*返回值：无
****************************************************************
*/
 void CreateMenu(WM_HWIN hWin) {
  MENU_Handle hMenu, hMenuAlarm, hMenuStart;
  MENU_SetDefaultFont(&GUI_Font16_ASCII);

  /* Create menu 'Game' */
  hMenuStart = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
  AddMenuItem(hMenuStart, 0, "Date",   ID_MENU_DATE,  0);
  AddMenuItem(hMenuStart, 0, "Time", ID_MENU_TIME, 0);
  AddMenuItem(hMenuStart, 0, "Exit", ID_MENU_EXIT, 0);
	 
	hMenuAlarm = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	AddMenuItem(hMenuAlarm, 0, "Show",   ID_MENU_SHOW_ALARM,  0);


  /* Create main menu */
  hMenu = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_HORIZONTAL, 0);
  AddMenuItem(hMenu, hMenuStart,    "Setting",    0, 0);
	AddMenuItem(hMenu, hMenuAlarm, "Alram",1,0);
  /* Attach menu to framewin */
  //FRAMEWIN_AddMenu(hWin, hMenu);
  MENU_SetTextColor(hMenu,MENU_CI_ENABLED,GUI_WHITE);
  MENU_SetTextColor(hMenuStart,MENU_CI_ENABLED,GUI_WHITE);

  MENU_SetTextColor(hMenu,MENU_CI_ACTIVE_SUBMENU,GUI_WHITE);
  MENU_SetTextColor(hMenuStart,MENU_CI_SELECTED,GUI_YELLOW);
	MENU_SetTextColor(hMenuAlarm,MENU_CI_SELECTED,GUI_BLACK);

  MENU_SetBkColor(hMenu,MENU_CI_ACTIVE_SUBMENU,GUI_RED);
  MENU_SetBkColor(hMenuStart,MENU_CI_SELECTED,GUI_RED);
	MENU_SetBkColor(hMenuAlarm,MENU_CI_SELECTED,GUI_RED);

  MENU_SetBkColor(hMenu,MENU_CI_ENABLED,GUI_BLUE);
  MENU_SetBkColor(hMenuStart,MENU_CI_ENABLED,GUI_GREEN);
	MENU_SetBkColor(hMenuAlarm,MENU_CI_ENABLED,GUI_GREEN);

  MENU_Attach(hMenu,hWin,0,0,239,0,0);
}


/*
***************************************************************
*函数名：OnMenu
*功能：  响应菜单
*参数：  pMsg：消息指针 
*返回值：无
****************************************************************
*/

static void OnMenu(WM_MESSAGE* pMsg) {
  MENU_MSG_DATA* pData = (MENU_MSG_DATA*)pMsg->Data.p;//将窗口消息类型的指针转换为菜单消息类型的指针
  MENU_Handle    hMenu = pMsg->hWinSrc;
  WM_HWIN         hWin = pMsg->hWin;
  switch (pData->MsgType) 
  {
  case MENU_ON_INITMENU:
      
	 break;
  case MENU_ON_ITEMSELECT:
    switch (pData->ItemId)
	{
    case  ID_MENU_DATE:
					IsOpenSetDate=1;
					GUI_CreateDialogBox(_SetDateDialogCreate, GUI_COUNTOF(_SetDateDialogCreate), &_SetDateCallback, hWin, 0, 0);
					WM_Exec();
            break;

    case ID_MENU_TIME:
			    IsOpenSetTime=1;
					GUI_CreateDialogBox(_SetTimeDialogCreate, GUI_COUNTOF(_SetTimeDialogCreate), &_SetTimeCallback, hWin, 0, 0);
					WM_Exec();
					break;
		
		case ID_MENU_SHOW_ALARM:
				 IsOpenShowAlarm=1;
			   GUI_CreateDialogBox(_ShowAlarmDialogCreate, GUI_COUNTOF(_ShowAlarmDialogCreate), &_ShowAlarmCallback, hWin, 0, 0);
		     WM_Exec();
         break;
		
    case ID_MENU_EXIT:
		  IsCloseCalendar = 1;		//关闭万年历
		  WM_DeleteWindow(hMenu);		//删除菜单	

    default:
			break;

    }
    break;

    default:
    WM_DefaultProc(pMsg);
  }
}


/*
***************************************************************
*函数名：DeskTop_cbCallback
*功能：	 桌面回调函数，响应桌面按钮
*参数：  pMsg：消息指针 
*返回值：无
****************************************************************
*/
static void _cbCallback(WM_MESSAGE* pMsg)  
{ 
//	WM_HWIN hWin = pMsg->hWin;
	switch (pMsg->MsgId) 
	{ 
		case WM_PAINT: 
				    GUI_Clear();
						InitAnalogClock();		//初始化模拟时钟		
						//ShowLunarCalendar();				//更新农历
		
						POINT_COLOR=GREEN;

						LCD_ShowNum(10,DCLOCKY,calendar.w_year,4,16);
						LCD_ShowChar(10+32,DCLOCKY,'-',16,0);
						LCD_ShowNum(10+40,DCLOCKY,calendar.w_month,2,16);
						LCD_ShowChar(10+56,DCLOCKY,'-',16,0);
						LCD_ShowNum(10+64,DCLOCKY,calendar.w_date,2,16);
						
						LCD_ShowNum(10,DCLOCKY+20,calendar.hour,2,16);
						LCD_ShowChar(10+16,DCLOCKY+20,':',16,0);
						LCD_ShowNum(10+24,DCLOCKY+20,calendar.min,2,16);
						LCD_ShowChar(10+40,DCLOCKY+20,':',16,0);
						LCD_ShowNum(10+48,DCLOCKY+20,calendar.sec,2,16);
					
						POINT_COLOR=YELLOW;
						LCD_ShowString(10,DCLOCKY+40,strlen(week_str[calendar.week])*16,16,strlen(week_str[calendar.week]),week_str[calendar.week]);
					  break;
        case WM_MENU:
			       
					  OnMenu(pMsg);
					  break;

		default: 
				  WM_DefaultProc(pMsg); 

	} 
}

/*
***************************************************************
*函数名：ShowLunarCalendar
*功能：	 显示农历日期
*参数：  无
*返回值：无
****************************************************************
*/

//const u8 *month_str[12]={"一月","二月","三月","四月","五月","六月","七月","八月","九月","十月","冬月","腊月",};//农历月份
//const u8 *lunar_day_str[30]=											  //农历日 
//{
//"初一","初二","初三","初四","初五","初六","初七","初八","初九","初十",
//"十一","十二","十三","十四","十五","十六","十七","十八","十九","廿十",
//"廿一","廿二","廿三","廿四","廿五","廿六","廿七","廿八","廿九","三十",	
//}; 

//const u8 *shengxiao_str[12]=			 //十二生肖
//{
//"申猴","酉鸡","戌狗","亥猪","子鼠","丑牛","寅虎","卯兔","辰龙","巳蛇","午马","未羊",	
//};

//static void ShowLunarCalendar(void){
//		GetLunarDate();		//得到农历日期
//		Show_Str(60,80,BLUE,(u8*)month_str[LunarCalender.month-1],16,0);
//		Show_Str(60+32,80,BLUE,(u8*)lunar_day_str[LunarCalender.day-1],16,0);
//	  Show_Str(60+80,80,BLUE,(u8*)shengxiao_str[LunarCalender.year%12],16,0);
//}
/*
***************************************************************
*函数名：ShowSolarCalendar
*功能：	 显示阳历
*参数：  无
*返回值：无
****************************************************************
*/

static void ShowSolarCalendar(void){
		static u8 temp0=0;
	  static u8 temp1=0;
	  static u8 temp2=0;	
		static int xsec=ACLOCKX;
		static int ysec=ACLOCKY;
		static int xmin=ACLOCKX;
		static int ymin=ACLOCKY;
		static int xhour=ACLOCKX;
		static int yhour=ACLOCKY;	
		if(temp0!=calendar.sec){	   //1秒到了则更新
			temp0 = calendar.sec;
			
			GUI_SetColor(GUI_BLACK);				
			GUI_DrawLine(ACLOCKX,ACLOCKY,xsec,ysec);			//秒针
			GUI_DrawLine(ACLOCKX,ACLOCKY,xmin,ymin);			//分针
			GUI_DrawLine(ACLOCKX,ACLOCKY,xhour,yhour);			//时针
			xsec = (int)(60*cos(PI*((int)calendar.sec-15)*6/180.0))+ACLOCKX;
			ysec = (int)(60*sin(PI*((int)calendar.sec-15)*6/180.0))+ACLOCKY;	
			GUI_SetColor(GUI_RED);
			GUI_DrawLine(ACLOCKX,ACLOCKY,xsec,ysec);			//秒针
			GUI_DrawLine(ACLOCKX,ACLOCKY,xmin,ymin);	
			GUI_DrawLine(ACLOCKX,ACLOCKY,xhour,yhour);	
			if(temp1!=calendar.min){
				temp1 = calendar.min;
				GUI_SetColor(GUI_BLACK);				
				GUI_DrawLine(ACLOCKX,ACLOCKY,xmin,ymin);					
				xmin = (int)(45*cos(PI*((int)calendar.min-15)*6/180.0))+ACLOCKX;
				ymin = (int)(45*sin(PI*((int)calendar.min-15)*6/180.0))+ACLOCKY;		
				GUI_SetColor(GUI_RED);
				GUI_DrawLine(ACLOCKX,ACLOCKY,xmin,ymin);			

			}
			if(temp2!=calendar.hour){
				temp2 = calendar.hour;		
				GUI_SetColor(GUI_BLACK);				
				GUI_DrawLine(ACLOCKX,ACLOCKY,xhour,yhour);					
				xhour = (int)(20*cos(PI*((int)calendar.hour%12-3)*30/180.0))+ACLOCKX;
				yhour = (int)(20*sin(PI*((int)calendar.hour%12-3)*30/180.0))+ACLOCKY;		
				GUI_SetColor(GUI_RED);
				GUI_DrawLine(ACLOCKX,ACLOCKY,xhour,yhour);			

			}
			GUI_FillCircle(ACLOCKX, ACLOCKY, 3);
			
			//ShowLunarCalendar();				//更新农历

			POINT_COLOR=GREEN;
			LCD_ShowNum(10,DCLOCKY,calendar.w_year,4,16);
			LCD_ShowChar(10+32,DCLOCKY,'-',16,0);
			LCD_ShowNum(10+40,DCLOCKY,calendar.w_month,2,16);
			LCD_ShowChar(10+56,DCLOCKY,'-',16,0);
			LCD_ShowNum(10+64,DCLOCKY,calendar.w_date,2,16);
			
			LCD_ShowNum(10,DCLOCKY+20,calendar.hour,2,16);
			LCD_ShowChar(10+16,DCLOCKY+20,':',16,0);
			LCD_ShowNum(10+24,DCLOCKY+20,calendar.min,2,16);
			LCD_ShowChar(10+40,DCLOCKY+20,':',16,0);
			LCD_ShowNum(10+48,DCLOCKY+20,calendar.sec,2,16);
		
			POINT_COLOR=YELLOW;
			LCD_ShowString(10,DCLOCKY+40,100,16,16,week_str[calendar.week]);
		}	
}

/*
***************************************************************
*函数名：InitAnalogClock
*功能：	 初始化模拟时钟
*参数：  无
*返回值：无
****************************************************************
*/

static void InitAnalogClock(void){
	int i=0;
	int x0;
	int y0;
	int x1;
	int y1;
	GUI_SetColor(GUI_RED);
	GUI_DrawCircle(ACLOCKX, ACLOCKY, ACLOCKR);// r=60, x = 160
	GUI_FillCircle(ACLOCKX, ACLOCKY, 3);
	for(i=0; i<12; i++){
		x0 = (int)(65*cos(PI*i*30/180.0))+ACLOCKX;
		y0 = (int)(65*sin(PI*i*30/180.0))+ACLOCKY;
		x1 = (int)(80*cos(PI*i*30/180.0))+ACLOCKX;
		y1 = (int)(80*sin(PI*i*30/180.0))+ACLOCKY;		
		GUI_DrawLine(x0,y0,x1,y1);	
	}
	for(i=0; i<60; i++){
		x0 = (int)(75*cos(PI*i*6/180.0))+ACLOCKX;
		y0 = (int)(75*sin(PI*i*6/180.0))+ACLOCKY;
		x1 = (int)(80*cos(PI*i*6/180.0))+ACLOCKX;
		y1 = (int)(80*sin(PI*i*6/180.0))+ACLOCKY;		
		GUI_DrawLine(x0,y0,x1,y1);	
	}	
}

/*
***************************************************************
*函数名：ShowCalendar
*功能：	 桌面界面
*参数：  无
*返回值：无
****************************************************************
*/
void ShowCalendar(void)
{	
	WM_SetDesktopColor(GUI_BLACK);      /* Automacally update desktop window */
  WM_SetCreateFlags(WM_CF_MEMDEV);  /* Use memory devices on all windows to avoid flicker */
  CreateMenu(WM_HBKWIN);			//在桌面窗口创建菜单
  WM_SetCallback(WM_HBKWIN,_cbCallback);  //设置桌面的回调函数
  GUI_Exec();
	BACK_COLOR = BLACK;

  InitAnalogClock();		//初始化模拟时钟
	while(1)
	{
		if(IsCloseCalendar){				//关闭万年历
				IsCloseCalendar=0;		//清除标记
			  IsOpenSetDate=0;
		    IsOpenSetTime=0;
			  IsOpenShowAlarm=0;
				return ; //结束此功能
		}
		
		if((!IsOpenSetDate)&&(!IsOpenSetTime)&&(!IsOpenShowAlarm)){
			ShowSolarCalendar();			//显示阳历
			lcdIdleCount++;
			if (lcdIdleCount > SLEEP)
			{
				 LCD_Off();   // Close LCD display
				 LCD_LED = 0; // close LCD light
				 vTaskSuspend(NULL);
				 lcdIdleCount = 0;
				 LCD_LED = 1;
				 LCD_On();
			}
		}
		else
		{
			 lcdIdleCount=0; //  Re-count on the sleep time.
		}
		GUI_Delay(10);
	}
		
}



