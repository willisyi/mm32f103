/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI界面主函数
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : 本实验主要演示游标，一共有12中游标。
*			  	GUI_CursorArrowS 	小箭头。
*				GUI_CursorArrowM 	中箭头。
*				GUI_CursorArrowL 	大箭头。
*				GUI_CursorArrowSI   小反箭头。
*				GUI_CursorArrowMI   中反箭头。
*				GUI_CursorArrowLI   大反箭头。
*				GUI_CursorCrossS    小十字。
*				GUI_CursorCrossM    中十字。
*				GUI_CursorCrossL    大十字。
*				GUI_CursorCrossSI   小反十字。
*				GUI_CursorCrossMI   中反十字。
*				GUI_CursorCrossLI   大反十字。
*	修改记录 :
*		版本号    日期          作者                 说明
*		v1.0    2013-04-17    Eric2013      ST固件库版本 V1.0.2版本。
*
*********************************************************************************************************
*/
#include "lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "led.h"

#include "GUI.h"
#include "DIALOG.h"
#include "WM.h"
#include "BUTTON.h"
#include "CHECKBOX.h"
#include "DROPDOWN.h"
#include "EDIT.h"
#include "FRAMEWIN.h"
#include "LISTBOX.h"
#include "MULTIEDIT.h"
#include "RADIO.h"
#include "SLIDER.h"
#include "TEXT.h"
#include "PROGBAR.h"
#include "SCROLLBAR.h"
#include "LISTVIEW.h"
#include "GRAPH.h"
#include "MENU.h"
#include "MULTIPAGE.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
/*********************************************************************
*
*       Dialog resource
*
* This table contAins the info required to create the dialog.
* It has been created manually, but could also be created by a GUI-builder.
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Dialog", 0,                 0,   0, 240, 320, FRAMEWIN_CF_MOVEABLE, 0  },
  { BUTTON_CreateIndirect,   "OK",     GUI_ID_OK,        100,   5,  60,  20 },
  { BUTTON_CreateIndirect,   "Cancel", GUI_ID_CANCEL,    100,  30,  60,  20 },
  { TEXT_CreateIndirect,     "LText",  0,                 10,  55,  48,  15, TEXT_CF_LEFT  },
  { TEXT_CreateIndirect,     "RText",  0,                 10,  80,  48,  15, TEXT_CF_RIGHT },
  { EDIT_CreateIndirect,     NULL,     GUI_ID_EDIT0,      60,  55, 100,  15, 0, 50 },
  { EDIT_CreateIndirect,     NULL,     GUI_ID_EDIT1,      60,  80, 100,  15, 0, 50 },
  { TEXT_CreateIndirect,     "Hex",    0,                 10, 100,  48,  15, TEXT_CF_RIGHT },
  { EDIT_CreateIndirect,     NULL,     GUI_ID_EDIT2,      60, 100, 100,  15, 0, 6 },
  { TEXT_CreateIndirect,     "Bin",    0,                 10, 120,  48,  15, TEXT_CF_RIGHT },
  { EDIT_CreateIndirect,     NULL,     GUI_ID_EDIT3,      60, 120, 100,  15 },
  { LISTBOX_CreateIndirect,  NULL,     GUI_ID_LISTBOX0,   10,  10,  60,  40 },
  { CHECKBOX_CreateIndirect, NULL,     GUI_ID_CHECK0,     10, 140,   0,   0 },
  { CHECKBOX_CreateIndirect, NULL,     GUI_ID_CHECK1,     30, 140,   0,   0 },
  { SLIDER_CreateIndirect,   NULL,     GUI_ID_SLIDER0,    60, 140, 180,  20 },
  { SLIDER_CreateIndirect,   NULL,     GUI_ID_SLIDER1,    10, 170, 230,  30 },
  { DROPDOWN_CreateIndirect,  NULL,    GUI_ID_DROPDOWN0, 170,  10,  80,  60, 0, 3  },
  { DROPDOWN_CreateIndirect,  NULL,    GUI_ID_DROPDOWN1, 170,  60,  80,  60, 0, 3  }
};


/*********************************************************************
*
*              Initializers for listbox
*/
static const GUI_ConstString _apListBox[] = {
  "English", "Deutsch", "Fran鏰is", "Japanese", "Italiano", NULL
};

/*******************************************************************
*
*       static code
*
********************************************************************
*/

/*********************************************************************
*
*       _cbBkWindow	  桌面窗口的回调函数，
* 在这里主要用于清屏和显示本次试验的title，根据EMWIN的执行机制，它会自动剪切
* 得到需要清屏的地方，不会全部的清
*/
static void _cbBkWindow(WM_MESSAGE* pMsg) {
  switch (pMsg->MsgId) {
  case WM_PAINT:
    GUI_SetBkColor(GUI_RED);
    GUI_Clear();
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font24_ASCII);
    GUI_DispStringHCenterAt("DIALOG_All - Sample", 100, 5);
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       _cbCallback	 创建的阻塞对话框的回调函数
*/
static void _cbCallback(WM_MESSAGE * pMsg) {
  int NCode, Id;
  WM_HWIN hEdit0, hEdit1, hEdit2, hEdit3, hListBox, hDropd0, hDropd1;
  WM_HWIN hWin = pMsg->hWin;
  switch (pMsg->MsgId) {
    case WM_INIT_DIALOG:
      /* Get window handles for all widgets */
      hEdit0   = WM_GetDialogItem(hWin, GUI_ID_EDIT0);
      hEdit1   = WM_GetDialogItem(hWin, GUI_ID_EDIT1);
      hEdit2   = WM_GetDialogItem(hWin, GUI_ID_EDIT2);
      hEdit3   = WM_GetDialogItem(hWin, GUI_ID_EDIT3);
      hListBox = WM_GetDialogItem(hWin, GUI_ID_LISTBOX0);
      hDropd0  = WM_GetDialogItem(hWin, GUI_ID_DROPDOWN0);
      hDropd1  = WM_GetDialogItem(hWin, GUI_ID_DROPDOWN1);
      /* Initialize all widgets */
      EDIT_SetText(hEdit0, "EDIT widget 0");
      EDIT_SetText(hEdit1, "EDIT widget 1");
      EDIT_SetTextAlign(hEdit1, GUI_TA_LEFT);
      EDIT_SetHexMode(hEdit2, 0x1234, 0, 0xffffff);
      EDIT_SetBinMode(hEdit3, 0x1234, 0, 0xffff);
      LISTBOX_SetText(hListBox, _apListBox);
      WM_DisableWindow (WM_GetDialogItem(hWin, GUI_ID_CHECK1));
      CHECKBOX_Check(  WM_GetDialogItem(hWin, GUI_ID_CHECK0));
      CHECKBOX_Check(  WM_GetDialogItem(hWin, GUI_ID_CHECK1));
      SLIDER_SetWidth( WM_GetDialogItem(hWin, GUI_ID_SLIDER0), 5);
      SLIDER_SetValue( WM_GetDialogItem(hWin, GUI_ID_SLIDER1), 50);
      SCROLLBAR_CreateAttached(hListBox, SCROLLBAR_CF_VERTICAL);
      DROPDOWN_AddString(hDropd0, "Item 0");
      DROPDOWN_AddString(hDropd0, "Item 1");
      DROPDOWN_AddString(hDropd0, "Item 2");
      DROPDOWN_AddString(hDropd1, "Item 0");
      DROPDOWN_AddString(hDropd1, "Item 1");
      DROPDOWN_AddString(hDropd1, "Item 2");
      DROPDOWN_AddString(hDropd1, "Item 3");
      DROPDOWN_AddString(hDropd1, "Item 4");
      break;
    case WM_KEY:
      switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) {
      case GUI_KEY_ESCAPE:
        GUI_EndDialog(hWin, 1);
        break;
      case GUI_KEY_ENTER:
        GUI_EndDialog(hWin, 0);
        break;
      }
      break;
    case WM_NOTIFY_PARENT:
      Id    = WM_GetId(pMsg->hWinSrc);    /* Id of widget */
      NCode = pMsg->Data.v;               /* Notification code */
      switch (NCode) {
        case WM_NOTIFICATION_RELEASED:    /* React only if released */
          if (Id == GUI_ID_OK) {          /* OK Button */
            GUI_EndDialog(hWin, 0);
          }
          if (Id == GUI_ID_CANCEL) {      /* Cancel Button */
            GUI_EndDialog(hWin, 1);
          }
          break;
      }
      break;
    default:
      WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
void MainTaskWM(void *pvParameters) {

  GUI_Init();
  GUI_CURSOR_Show();
  //设置桌面窗口的回调函数
  WM_SetCallback(WM_HBKWIN, &_cbBkWindow);

  //标记  WM_CF_MEMDEV 频繁用于启用所有窗口的存储设备。请注意，在调用GUI_Init() 之前允许设
  //置创建标记，这将导致背景窗口也受到创建标记的影响。
  
//  WM_SetCreateFlags(WM_CF_MEMDEV);  /* Use memory devices on all windows to avoid flicker */
  while (1) {
    //创建阻塞式对话框
    GUI_ExecDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, WM_HBKWIN, 0, 0);
    GUI_Delay(1000);
  }
}

/*************************** End of file ****************************/








