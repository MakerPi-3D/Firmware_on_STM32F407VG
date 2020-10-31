#include "globalvariables.h"
#include "user_debug.h"
#include "user_interface.h"
#include "view_commonf.h"
#include "view_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "interface.h"
#include "fatfs.h"
#include "usbd_cdc_core.h"
#include "PrintControl.h"
#include "temperature.h"
#include "sysconfig_data.h"
#include "threed_engine.h"
#include "uart.h"
#include "Alter.h"

#define TransFileBuf (1024*32)  //32K
#define MaxPacketNum (TransFileBuf/APP_RX_DATA_SIZE)  //缓存32K,每接收一个包最大为64字节，则可缓存512个包

  extern __ALIGN_BEGIN_OLD UINT8 USB_Rx_Buffer   [APP_RX_DATA_SIZE] __ALIGN_END ; //USB bulk 包缓冲区
  extern __ALIGN_BEGIN_OLD USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END; //USB bulk设备
//  extern osSemaphoreId ReceiveUartCmdHandle; //信号量
  __align(4) CHAR TextBuf[TransFileBuf]; //32K缓冲区

  ULONG TransFileTimeOut; //传输是否超时
  UINT32 byteswritten;
  FIL MyFile;

  struct
  {
    UINT8 IsM35Order;  //是否是M35获取状态命令
    UINT8 IsM34Order;  //是否是M34机型确认命令
    UINT8 IsM28Order;  //是否是M28开始传输命令
    UINT8 IsM105Order;  //是否是M105获取温度数据
    UINT8 IsG1Order;  //是否是G1控制电机命令
    UINT8 IsG28Order; //G28命令
    UINT8 IsM25Order; //M25暂停打印命令
    UINT8 IsM24Order; //M24继续打印命令
    UINT8 IsM33Order; //M33停止打印命令
    UINT8 IsM700Order; //M700获取USB打印是否完成
    UINT8 IsStartTrans; //是否开始传输
    UINT8 IsEndTrans; //是否传输完毕
    CHAR G1OrderBuf[64]; //G1控制电机命令
    CHAR ModelStr[20]; //机型
    CHAR FileNameStr[40]; //文件名
    ULONG FileSize; //文件大小
    ULONG ReceivedFileSize; //已接收的文件大小
    UINT ReceivedPacketNumInBuf; //在32K的缓冲区中已接收的包个数
    UINT ReceivedDataSizeInBuf; //在32K的缓冲区中已存储的字节数
  } TransFileStatus;


///////////////上传文件--中断回调函数usbd_cdc_DataOut 中的处理///////////////////

  void ReceiveFileContent(UINT16 PackDataSize) //接收文件内容
  {
    (void)memmove(&TextBuf[TransFileStatus.ReceivedDataSizeInBuf],USB_Rx_Buffer,PackDataSize);
    TransFileStatus.ReceivedDataSizeInBuf=TransFileStatus.ReceivedDataSizeInBuf+PackDataSize;
    ++TransFileStatus.ReceivedPacketNumInBuf;

    TransFileStatus.ReceivedFileSize=TransFileStatus.ReceivedFileSize+PackDataSize;
    if(TransFileStatus.ReceivedFileSize>=TransFileStatus.FileSize)  //文件是否接收完成
    {
      TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum;
      TransFileStatus.IsEndTrans=1;
      TransFileStatus.IsStartTrans=0;
      TransFileStatus.ReceivedFileSize=0;
      TransFileStatus.FileSize=0;
    }
  }

  void AnalyzeM35Order(UINT16 PackDataSize) //M35
  {
    //字符串结尾
    if(PackDataSize<64)
    {
      USB_Rx_Buffer[PackDataSize]=0;
    }
    else
    {
      USB_Rx_Buffer[63]=0; //指令长度不能超63字节
    }

    TransFileStatus.ReceivedDataSizeInBuf=PackDataSize; //缓冲区中的数据大小
    TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum; //让缓冲区满，从而去处理数据
    TransFileStatus.IsM35Order=1; //M35命令
  }

  void AnalyzeM34Order(UINT16 PackDataSize) //M34 M14 //M14为机型命令
  {
    //字符串结尾
    if(PackDataSize<64)
    {
      USB_Rx_Buffer[PackDataSize]=0;
    }
    else
    {
      USB_Rx_Buffer[63]=0; //指令长度不能超63字节
    }

    //获取机型
    (void)strcpy(TransFileStatus.ModelStr,(PCHAR)&USB_Rx_Buffer[4]);//获取机型:M14

    TransFileStatus.ReceivedDataSizeInBuf=PackDataSize; //缓冲区中的数据大小
    TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum; //让缓冲区满，从而去处理数据
    TransFileStatus.IsM34Order=1; //M34命令
  }

  void AnalyzeM28Order(UINT16 PackDataSize) //M28 test.gcode S1024
  {
    PCHAR StrHeadPos;
    PCHAR StrEndPos;
    //字符串结尾
    if(PackDataSize<64)
    {
      USB_Rx_Buffer[PackDataSize]=0;
    }
    else
    {
      USB_Rx_Buffer[63]=0; //指令长度不能超63字节
    }
    //获取文件名
    StrHeadPos=(PCHAR)&USB_Rx_Buffer[4];
    StrEndPos=strchr(StrHeadPos,' ');
    *StrEndPos=0;
    (void)snprintf(TransFileStatus.FileNameStr, sizeof(TransFileStatus.FileNameStr), "0:/%s",StrHeadPos);  //获取文件名，并添加上SD卡路径  CHAR FileNameStr[40]//长度不能超
    //获取文件大小
    StrHeadPos=StrEndPos+2;  //指向文件大小数值的开头
    StrEndPos=strchr(StrHeadPos,' ');
    *StrEndPos=0;
    TransFileStatus.FileSize=(ULONG)strtol(StrHeadPos, NULL, 10);//获取文件大小  ULONG FileSize//大小不能超 32位

    TransFileStatus.ReceivedDataSizeInBuf=PackDataSize; //缓冲区中的数据大小
    TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum; //让缓冲区满，从而去处理数据
    TransFileStatus.IsStartTrans=1; //开始传输
    TransFileStatus.IsM28Order=1;	//是M28命令
  }

  void AnalyzeM105Order(UINT16 PackDataSize) //M105
  {
    TransFileStatus.ReceivedDataSizeInBuf=PackDataSize; //缓冲区中的数据大小
    TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum; //让缓冲区满，从而去处理数据
    TransFileStatus.IsM105Order=1; //M105命令
  }

  void AnalyzeG1Order(UINT16 PackDataSize) //G1 F150 X100 Y10 Z15----就是GCODE命令
  {
    //字符串结尾
    if(PackDataSize<64)
    {
      USB_Rx_Buffer[PackDataSize]=0;
    }
    else
    {
      USB_Rx_Buffer[63]=0; //指令长度不能超63字节
    }
    (void)strcpy(TransFileStatus.G1OrderBuf,(PCHAR)USB_Rx_Buffer);
    TransFileStatus.ReceivedDataSizeInBuf=PackDataSize; //缓冲区中的数据大小
    TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum; //让缓冲区满，从而去处理数据
    TransFileStatus.IsG1Order=1; //G1命令
  }

  void AnalyzeG28Order(UINT16 PackDataSize) //G28命令归零
  {
    TransFileStatus.ReceivedDataSizeInBuf=PackDataSize; //缓冲区中的数据大小
    TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum; //让缓冲区满，从而去处理数据
    TransFileStatus.IsG28Order=1; //G28命令
  }

  void AnalyzeM25Order(UINT16 PackDataSize) //M25暂停打印命令
  {
    TransFileStatus.ReceivedDataSizeInBuf=PackDataSize; //缓冲区中的数据大小
    TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum; //让缓冲区满，从而去处理数据
    TransFileStatus.IsM25Order=1; //M25命令
  }

  void AnalyzeM24Order(UINT16 PackDataSize) //M24继续打印命令
  {
    TransFileStatus.ReceivedDataSizeInBuf=PackDataSize; //缓冲区中的数据大小
    TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum; //让缓冲区满，从而去处理数据
    TransFileStatus.IsM24Order=1; //M24命令
  }

  void AnalyzeM33Order(UINT16 PackDataSize) //M33停止打印命令
  {
    TransFileStatus.ReceivedDataSizeInBuf=PackDataSize; //缓冲区中的数据大小
    TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum; //让缓冲区满，从而去处理数据
    TransFileStatus.IsM33Order=1; //M33命令
  }

  void AnalyzeM700Order(UINT16 PackDataSize) //M700获取USB联是否已完成打印
  {
    TransFileStatus.ReceivedDataSizeInBuf=PackDataSize; //缓冲区中的数据大小
    TransFileStatus.ReceivedPacketNumInBuf=MaxPacketNum; //让缓冲区满，从而去处理数据
    TransFileStatus.IsM700Order=1; //M700命令
  }

  void USBBulkReceiveData(UINT16 ReceivedCountInOnePack)
  {
    if(1==TransFileStatus.IsStartTrans) //是否已开始传输文件内容
    {
      ReceiveFileContent(ReceivedCountInOnePack);
    }
    else if((USB_Rx_Buffer[0]=='M') &&(USB_Rx_Buffer[1]=='3') && (USB_Rx_Buffer[2]=='5'))
    {
      AnalyzeM35Order(ReceivedCountInOnePack);//命令格式：M35
    }
    else if((USB_Rx_Buffer[0]=='M') &&(USB_Rx_Buffer[1]=='3') && (USB_Rx_Buffer[2]=='4'))
    {
      AnalyzeM34Order(ReceivedCountInOnePack);//命令格式：M34 M14  //M14为机型名称
    }
    else if((USB_Rx_Buffer[0]=='M') &&(USB_Rx_Buffer[1]=='2') && (USB_Rx_Buffer[2]=='8')) //没有传输文件内容时，检测是否接收到M28命令
    {
      AnalyzeM28Order(ReceivedCountInOnePack);//命令格式：M28 test.gcode S1024 M14
    }
    else if((USB_Rx_Buffer[0]=='M') &&(USB_Rx_Buffer[1]=='1') && (USB_Rx_Buffer[2]=='0') && (USB_Rx_Buffer[3]=='5'))  //M105
    {
      AnalyzeM105Order(ReceivedCountInOnePack);//命令格式：M105
    }
    else if((USB_Rx_Buffer[0]=='G') &&(USB_Rx_Buffer[1]=='1'))  //G1移动命令
    {
      AnalyzeG1Order(ReceivedCountInOnePack);//命令格式：G1 F150 X100 Y10 Z15----就是GCODE命令
    }
    else if((USB_Rx_Buffer[0]=='G') &&(USB_Rx_Buffer[1]=='2')&& (USB_Rx_Buffer[2]=='8'))  //G28归零命令
    {
      AnalyzeG28Order(ReceivedCountInOnePack);//命令格式：G28
    }
    else if((USB_Rx_Buffer[0]=='M') &&(USB_Rx_Buffer[1]=='2')&& (USB_Rx_Buffer[2]=='5'))  //M25暂停打印
    {
      AnalyzeM25Order(ReceivedCountInOnePack);//命令格式：M25
    }
    else if((USB_Rx_Buffer[0]=='M') &&(USB_Rx_Buffer[1]=='2')&& (USB_Rx_Buffer[2]=='4'))  //M24继续打印
    {
      AnalyzeM24Order(ReceivedCountInOnePack);//命令格式：M24
    }
    else if((USB_Rx_Buffer[0]=='M') &&(USB_Rx_Buffer[1]=='3')&& (USB_Rx_Buffer[2]=='3'))  //M33停止打印
    {
      AnalyzeM33Order(ReceivedCountInOnePack);//命令格式：M33
    }
    else if((USB_Rx_Buffer[0]=='M') &&(USB_Rx_Buffer[1]=='7')&& (USB_Rx_Buffer[2]=='0')&& (USB_Rx_Buffer[3]=='0'))  //M700获取USB联机是否已完成打印
    {
      AnalyzeM700Order(ReceivedCountInOnePack);//命令格式：M700
    }

    if(MaxPacketNum==TransFileStatus.ReceivedPacketNumInBuf) //缓冲区满
    {
      TransFileStatus.ReceivedPacketNumInBuf=0;
      task_receive_uart_release();
//      (void)osSemaphoreRelease(ReceiveUartCmdHandle);	 //跳转到任务处理
    }
    else //缓冲区没有满，接收下一个包
    {
      (void)DCD_EP_PrepareRx (&USB_OTG_dev,CDC_OUT_EP,(UINT8 *)USB_Rx_Buffer,sizeof(USB_Rx_Buffer));
    }
    serial_print[0] = false;
		serial_print[1] = false;
  }



//////////////////////////////上传文件--OS任务中的处理//////////////////////////////////

  void ResertVariable(void) //变量重置
  {
    TransFileStatus.IsStartTrans=0;
    TransFileStatus.FileSize=0;
    TransFileStatus.ReceivedFileSize=0;
    TransFileStatus.ReceivedPacketNumInBuf=0;
    TransFileStatus.ReceivedDataSizeInBuf=0;
  }

  void FinishTransFile(void) //完成接收
  {
    if(TransFileStatus.ReceivedDataSizeInBuf>0)
    {
      (void)f_write(&MyFile, TextBuf, TransFileStatus.ReceivedDataSizeInBuf, (UINT *)&byteswritten);  //写入最后的数据
    }
    (void)f_close(&MyFile); //关闭文件
    t_gui_p.IsTransFile=0;
    if ((strcmp(TransFileStatus.FileNameStr, "0:/APP2.bin") == 0) || (strcmp(TransFileStatus.FileNameStr, "0:/APP2.BIN") == 0))
    {
      osDelay(1000);
      sys_task_enter_critical();
      NVIC_SystemReset(); // 软复位
      __set_FAULTMASK(1);
      sys_task_exit_critical();
    }
    else if ((strcmp(TransFileStatus.FileNameStr, "0:/SysConfig1.txt") == 0) || (strcmp(TransFileStatus.FileNameStr, "0:/SYSCONFIG1.TXT") == 0))
    {

    }
    else
    {
      //让GUI跳转界面并开始打印
      (void)strcpy(SettingInfoToSYS.PrintFileName,&TransFileStatus.FileNameStr[3]);
      (void)strcpy(SDFileName,&TransFileStatus.FileNameStr[3]);//复制文件名给GUI
      printFileControl.setSDMedium();//设置从SD卡读取文件
      pauseprint=0;
      print_flage = 1;
      gui_set_curr_display(maindisplayF);
      osDelay(1000);
      strcpy(printname,SDFileName);
      respond_gui_send_sem(FilePrintValue);
    }
  }

  UINT8 IsModelRight(void) //机型是否正确
  {
    if (0 == strcmp(t_sys.model_str, TransFileStatus.ModelStr))
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }


  void CompareModel(void)
  {
    ResertVariable(); //变量重置
    if(!IsModelRight()) //机型不正确，不上传文件
    {
      //(void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"Err:ModelNotRight",strlen("Err:ModelNotRight")); //返回Err给电脑端
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"Err:ModelNotRight",strlen("Err:ModelNotRight")); //返回Err给电脑端
    }
    else
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"ok",strlen("ok")); //返回ok给电脑端
    }
  }

  void TransFileReady(void);

  void M28ReadyToTransFile(void) //M28 命令
  {
    if(IsPrint())//正在打印，不上传文件
    {
      ResertVariable(); //变量重置
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"no",strlen("no")); //返回no给电脑端
    }
    else
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"ok",strlen("ok"));	//返回ok给电脑端
      t_gui_p.IsTransFile=1;//置位GUI上传文件标志
      gui_set_curr_display(transfileF);
      TransFileReady();  //传输相关的一些设置
      (void)sys_os_delay(1000);  //延时以让GUI任务切换相应界面
      (void)f_open(&MyFile, TransFileStatus.FileNameStr, FA_CREATE_ALWAYS | FA_WRITE); //创建文件
    }
  }

  void BackMachineStatusToComputer(void)
  {
    ResertVariable(); //变量重置
    if(t_gui_p.IsWarning)
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"SysErr",strlen("SysErr")); //液晶屏正在显示警告界面或错误界面
    }
    else if(IsPausePrint())
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"Pausing",strlen("Pausing")); //正在暂停状态
    }
    else if(IsPrint())
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"Printing",strlen("Printing")); //正在打印状态
    }
    else
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"NotPrinting",strlen("NotPrinting")); //没有在打印
    }
  }

  void BackPrintDataToComputer(void)
  {
    INT NozzleTemp;
    INT HotbedTemp;
    INT Percent;
    CHAR BackDataBuf[64];
    ResertVariable(); //变量重置
    NozzleTemp=(INT)sg_grbl::temperature_get_extruder_current(0); //读取喷嘴温度
    HotbedTemp=(INT)sg_grbl::temperature_get_bed_current(); //读取热床温度
    Percent=t_gui.print_percent; //读取打印进度

    if(IsPrint())
    {
      (void)snprintf(BackDataBuf, sizeof(BackDataBuf), "S:Printing T:%d B:%d P:%d",NozzleTemp,HotbedTemp,Percent); //合成字符串
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)BackDataBuf,strlen(BackDataBuf)); //返回温度数据
    }
    else
    {
      (void)snprintf(BackDataBuf, sizeof(BackDataBuf), "S:NotPrinting T:%d B:%d",NozzleTemp,HotbedTemp); //合成字符串
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)BackDataBuf,strlen(BackDataBuf)); //返回温度数据
    }
  }


  void G1OrderToSys(void)
  {
    ResertVariable(); //变量重置
    if(IsPrint())
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"Printing",strlen("Printing")); //正在打印，不执行操作，返回对应信息
    }
    else
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"ok",strlen("ok")); //没有打印，去执行命令

      if(0U == t_gui_p.G28_ENDSTOPS_COMPLETE)
      {
        sys_send_gcode_cmd("G28");//没有归零过，先归零
      }
      sys_send_gcode_cmd(TransFileStatus.G1OrderBuf); //用消息队列将Gcode命令传输到Gcode处理任务去处理
    }
  }

  void G28OrderToSys(void)
  {
    ResertVariable(); //变量重置
    if(IsPrint())
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"Printing",strlen("Printing")); //正在打印，不执行操作，返回对应信息
    }
    else
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"ok",strlen("ok")); //没有打印，去执行命令
      sys_send_gcode_cmd("G28");//用消息队列将Gcode命令传输到Gcode处理任务去处理
    }
  }

  void M25OrderToSys(void)
  {
    ResertVariable(); //变量重置
    if(IsPrint())
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"ok",strlen("ok")); //正在打印，去执行命令
      //暂停打印命令相当于在GUI界面点击了暂停打印按钮
      t_gui_p.IsComputerControlToPausePrint=1;
    }
    else
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"NotPrinting",strlen("NotPrinting")); //没有在打印状态，不执行操作，返回对应信息
    }
  }

  void M24OrderToSys(void)
  {
    ResertVariable(); //变量重置
    if(IsPausePrint())
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"ok",strlen("ok")); //正在暂停状态，去执行命令
      //继续打印命令相当于在GUI界面点击了继续打印按钮
      t_gui_p.IsComputerControlToResumePrint=1;
    }
    else
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"NotPausing",strlen("NotPausing")); //不是在暂停状态，不执行操作，返回对应信息
    }
  }

  void M33OrderToSys(void)
  {
    ResertVariable(); //变量重置
    if(IsPrint() || IsPausePrint())  //在打印状态或在暂停状态
    {

      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"ok",strlen("ok")); //正在打印或在暂停状态，去执行命令
      //停止打印命令相当于在GUI界面点击了停止打印按钮
      t_gui_p.IsComputerControlToStopPrint=1;
    }
    else
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"NotPrinting",strlen("NotPrinting")); //没在打印或在暂停状态，不执行操作，返回对应信息
    }
  }


  static bool IsUSBPrintFinished=false;  //联机打印是否完成
  static bool IsUSBPrintStop=false; //打印中途是否停止了打印
  static bool IsUSBPrintPause=false; //打印中途是否暂停了打印
  void SetIsUSBPrintFinished(bool Value)
  {
    IsUSBPrintFinished=Value;
  }

  void SetIsUSBPrintStop(bool Value)
  {
    IsUSBPrintStop=Value;
  }

  void SetIsUSBPrintPause(bool Value)
  {
    IsUSBPrintPause=Value;
  }

  void M700OrderToSys(void)
  {
    if(IsUSBPrintFinished)
    {
      (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"PrintFinished",strlen("PrintFinished")); //已完成打印
    }
    else
    {
      if(IsUSBPrintPause)
      {
        (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"PrintPause",strlen("PrintPause")); //暂停了打印
      }
      else if(IsUSBPrintStop)
      {
        (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"PrintStop",strlen("PrintStop")); //停止了打印
      }
      else
      {
        (void)DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(UINT8*)"NotPrintFinished",strlen("NotPrintFinished")); //正在打印中
      }
    }
  }


  void WriteFileDataToSD(void) //将接收到的数据写入SD卡文件中
  {
    (void)f_write(&MyFile, TextBuf,TransFileStatus.ReceivedDataSizeInBuf, (UINT *)&byteswritten);
  }

  void SaveUSBFile(void)
  {
    TransFileTimeOut=sys_task_get_tick_count()+2000;  //传输超时时间设置
    if(TransFileStatus.IsEndTrans) //传输是否结束
    {
      TransFileStatus.IsEndTrans=0; //重置标志
      FinishTransFile();
    }
    else
    {
      if(TransFileStatus.ReceivedDataSizeInBuf>0) //有接收到数据
      {
        if(1==t_gui_p.IsTransFile) //写文件内容到SD
        {
          WriteFileDataToSD(); //将接收到的文件内容保存到SD卡中
        }
        else if(TransFileStatus.IsM35Order) //M35命令，获取状态命令
        {
          TransFileStatus.IsM35Order=0; //重置标志
          BackMachineStatusToComputer(); //返回状态数据
        }
        else if(TransFileStatus.IsM34Order) //M34命令，机型确认命令
        {
          TransFileStatus.IsM34Order=0; //重置标志
          CompareModel(); //对比机型
        }
        else if(TransFileStatus.IsM28Order) //M28命令，开始文件传输
        {
          TransFileStatus.IsM28Order=0; //重置标志
          M28ReadyToTransFile(); //准备去传输文件
        }
        else if(TransFileStatus.IsM105Order) //M105命令，返回打印数据
        {
          TransFileStatus.IsM105Order=0; //重置标志
          BackPrintDataToComputer(); //返回温度数据
        }
        else if(TransFileStatus.IsG1Order) //G1控制电机命令，控制电机移动
        {
          TransFileStatus.IsG1Order=0; //重置标志
          G1OrderToSys(); //将接受到的G1命令传给系统处理
        }
        else if(TransFileStatus.IsG28Order) //G28归零命令
        {
          TransFileStatus.IsG28Order=0; //重置标志
          G28OrderToSys(); //将接受到的G28命令传给系统处理
        }
        else if(TransFileStatus.IsM25Order) //M25暂停打印命令
        {
          TransFileStatus.IsM25Order=0; //重置标志
          M25OrderToSys(); //将接受到的M25命令传给系统处理
        }
        else if(TransFileStatus.IsM24Order) //M24继续打印命令
        {
          TransFileStatus.IsM24Order=0; //重置标志
          M24OrderToSys(); //将接受到的M24命令传给系统处理
        }
        else if(TransFileStatus.IsM33Order) //M33停止打印命令
        {
          TransFileStatus.IsM33Order=0; //重置标志
          M33OrderToSys(); //将接受到的M33命令传给系统处理
        }
        else if(TransFileStatus.IsM700Order) //M33停止打印命令
        {
          TransFileStatus.IsM700Order=0; //重置标志
          M700OrderToSys(); //将接受到的M700命令传给系统处理
        }
      }
    }
    TransFileStatus.ReceivedDataSizeInBuf=0; //缓冲区数据清零
    (void)DCD_EP_PrepareRx (&USB_OTG_dev,CDC_OUT_EP,(UINT8 *)USB_Rx_Buffer,sizeof(USB_Rx_Buffer)); //重新启动接收
  }

  void IsTranFileErr(void)
  {
    if(1==t_gui_p.IsTransFile) //正在传输文件
    {
      if(TransFileTimeOut<sys_task_get_tick_count()) //若传输中途超时，则主动中断此次传输
      {
        t_gui_p.IsTransFile=0; //GUI标志位重置
        ResertVariable(); //变量重置

        (void)f_close(&MyFile); //关闭文件
        (void)f_unlink(TransFileStatus.FileNameStr); //删除文件
      }
    }
  }

#ifdef __cplusplus
} // extern "C" {
#endif

