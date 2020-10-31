#include "user_debug.h"
#include <ctype.h>
#include "user_interface.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "globalvariables.h"
#include <string.h>
#include "view_common.h"
#include "fatfs.h"
#include "interface.h"


  static CHAR Path[MK_MAX_LFN];
  static INT GUIpage;
  static INT PageItemNum;
  static INT CurrentPos;
  static INT IsCurrentPageFileScanFinished;

  static FILINFO fileinfo;
  static FILINFO sdfileinfo;

  INT CheckIsGcodeFile(CONST PCHAR p)
  {
    UINT i;
    CHAR a[10];
    i = strlen(p);
    if(i > 0 && p[0] == '.')
    {
      return 0;
    }
    while (p[i] != '.')
    {
      --i;
    }
    (void)strcpy(a, &p[i + 1]);

    // 文件后缀转为小写
    for(i=0; i<strlen(a); ++i)
    {
      a[i] = tolower(a[i]);
    }

    if ((0 == strcmp(a, "gcode"))  || (0 == strcmp(a, "sgcode")) ||
        (0 == strcmp(a, "sgc")) || (0 == strcmp(a, "gco"))) // 长文件太长，文件名会变为8.3的方式
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }

  void DelectSDFiles(void)
  {
    if(t_power_off.is_file_from_sd)
    {
      return;//断电续打，但文件在U盘，删除SD卡里的文件
    }
    CHAR lfnamebuffer[MK_MAX_LFN];
    CHAR PathAndFilename[MK_MAX_LFN];
    PCHAR sdfname;
    DIR dirs;
    sdfileinfo.fsize = MK_MAX_LFN;
//    sdfileinfo.lfname = lfnamebuffer;
    if (f_opendir(&dirs, SDPath) == FR_OK)
    {
      while ((f_readdir(&dirs, &sdfileinfo) == FR_OK) && sdfileinfo.fname[0] )   //依次读取文件名
      {
        if ((sdfileinfo.fattrib & AM_DIR) ==  0)   //不是目录
        {
#if _USE_LFN
          sdfname = *sdfileinfo.fname ? sdfileinfo.fname : sdfileinfo.fname;
#else
          sdfname = fileinfo.fname;
#endif
          if (CheckIsGcodeFile(sdfname))
          {
            (void)snprintf(PathAndFilename, sizeof(PathAndFilename), "%s", SDPath);
            (void)strcat(PathAndFilename, sdfname);
            (void)f_unlink(PathAndFilename);
          }
        }
      }
      (void)f_closedir(&dirs);
    }
  }

  void FindFolder(void)
  {
    DIR dirs_files;
    INT dirSkip;
    CHAR lfnamebuf[MK_MAX_LFN];
    PCHAR fname;
    fileinfo.fsize = MK_MAX_LFN;
//    fileinfo.lfname = lfnamebuf;

    if(IsCurrentPageFileScanFinished)  //若当前页已扫描完，不再扫描，直接返回
    {
      return;
    }

    if(f_opendir(&dirs_files,Path) == FR_OK)
    {
      USER_EchoLog("f_opendir DIR ok!\r\n");
      while((f_readdir(&dirs_files,&fileinfo) == FR_OK)&& fileinfo.fname[0] )  //依次读取文件名
      {
        if (fileinfo.fattrib & AM_DIR)  //文件夹
        {
#if _USE_LFN
          fname = *fileinfo.fname ? fileinfo.fname : fileinfo.fname;
#else
          fname = fileinfo.fname;
#endif

          dirSkip=strcmp("System Volume Information",fname);
          dirSkip= (dirSkip && strcmp("$RECYCLE.BIN",fname));
          dirSkip= (dirSkip && strcmp("LOST.DIR",fname));
          dirSkip= (dirSkip && strcmp("RECYCLER",fname));
          dirSkip = (dirSkip && ('.' != fname[0]));
          if(dirSkip) //过滤掉系统文件夹System Volume Information和$RECYCLE.BIN，以防由此引发的死机
          {
            if(CurrentPos>=(OnePageNum * GUIpage))
            {
              IsCurrentPageFileScanFinished=1;
              t_gui_p.IsHaveNextPage = 1;
              ++GUIpage;
              (void)f_closedir(&dirs_files);
              return;
            }
            if(CurrentPos>=(OnePageNum * (GUIpage-1)))
            {
              (void)strcpy(DisplayFileName[PageItemNum],fname);
              t_gui_p.IsHaveFile[PageItemNum] = 1;
              t_gui_p.IsDir[PageItemNum] = 1;
              ++PageItemNum;
            }
            ++CurrentPos;
          }
        }
      }
      (void)f_closedir(&dirs_files);
    }
    else
    {
      USER_ErrLog("f_opendir DIR not ok!\r\n");
    }
  }

  void FindGcodeFile(void)
  {
    DIR dirs_files;
    CHAR lfnamebuf[MK_MAX_LFN];
    PCHAR fname;
    fileinfo.fsize = MK_MAX_LFN;
//    fileinfo.lfname = lfnamebuf;

    if(IsCurrentPageFileScanFinished)  //若当前页已扫描完，不再扫描，直接返回
    {
      return;
    }

    if(f_opendir(&dirs_files,Path) == FR_OK)
    {
      while((f_readdir(&dirs_files,&fileinfo) == FR_OK)&& fileinfo.fname[0] )  //依次读取文件名
      {
        if ((fileinfo.fattrib & AM_DIR) ==  0)  //文件
        {
#if _USE_LFN
          fname = *fileinfo.fname ? fileinfo.fname : fileinfo.fname;
#else
          fname = fileinfo.fname;
#endif

          if(CheckIsGcodeFile(fname))
          {
            if(CurrentPos>=(OnePageNum * GUIpage))
            {
              IsCurrentPageFileScanFinished=1;
              t_gui_p.IsHaveNextPage = 1;
              ++GUIpage;
              (void)f_closedir(&dirs_files);
              return;
            }
            if(CurrentPos>=(OnePageNum * (GUIpage-1)))
            {
              (void)strcpy(DisplayFileName[PageItemNum],fname);
              t_gui_p.IsHaveFile[PageItemNum] = 1;
              t_gui_p.IsDir[PageItemNum] = 0;
              ++PageItemNum;
            }
            ++CurrentPos;
          }
        }
      }
      (void)f_closedir(&dirs_files);
    }
  }


  void FileScan(void)  //在GUI界面显示的时候，文件夹显示在前面，Gcode文件显示在后面
  {
    PageItemNum=0;
    CurrentPos=0;
    FindFolder();//寻找文件夹
    FindGcodeFile();//寻找Gcode文件
  }

  void CleanFilNameBuf(void)
  {
    INT i;
    for(i=0; i<OnePageNum; ++i)
    {
      DisplayFileName[i][0]=0;
      t_gui_p.IsHaveFile[i]=0;
      t_gui_p.IsDir[i]=0;
    }
    t_gui_p.IsHaveNextPage = 0;
    IsCurrentPageFileScanFinished=0;
  }

//打开SD卡
  void GUIOpenSDCard(void)
  {
    CleanFilNameBuf();
    (void)strcpy(Path,"1:/");
    GUIpage=1;
    (void)strcpy(CurrentPath,Path);
    t_gui_p.IsRootDir = 1;
    t_gui_p.CurrentPage = GUIpage;
    FileScan();
  }

//打开目录
  void GUIOpenSDDir(void)
  {
    INT LastPathLength;
    LastPathLength=(INT)strlen(Path);
    if(t_gui_p.IsRootDir)
    {
      (void)sprintf(&Path[LastPathLength], "%s", SettingInfoToSYS.DirName);
    }
    else
    {
      (void)sprintf(&Path[LastPathLength], "/%s", SettingInfoToSYS.DirName);
    }
    GUIpage=1;
    CleanFilNameBuf();
    (void)strcpy(CurrentPath,Path);
    t_gui_p.IsRootDir = 0;
    t_gui_p.CurrentPage = GUIpage;
    FileScan();
  }

//返回上层目录
  void GUIBackSDLastDir(void)
  {
    CleanFilNameBuf();
    PCHAR LastDirEndPos;
    INT LastDirEndPosition;
    PCHAR DiskPath;

    DiskPath=USBHPath;
    if(0 == strcmp(Path,DiskPath))
    {
      (void)strcpy(Path,DiskPath);
      t_gui_p.IsRootDir = 1;
    }
    else
    {
      LastDirEndPos=strrchr(Path,'/');
      LastDirEndPosition=LastDirEndPos-Path;
      Path[LastDirEndPosition]=0;
      if(0 == strcmp(Path,"1:"))
      {
        t_gui_p.IsRootDir = 1;
        (void)strcpy(Path,DiskPath);
      }
      else
      {
        t_gui_p.IsRootDir = 0;
      }
    }
    GUIpage=1;
    (void)strcpy(CurrentPath,Path);
    t_gui_p.CurrentPage = GUIpage;
    FileScan();
  }

//点击下一页
  void GUINextPage(void)
  {
    CleanFilNameBuf();
    t_gui_p.CurrentPage = GUIpage;
    FileScan();
  }

//点击上一页
  void GUILastPage(void)
  {
    if((t_gui_p.IsHaveNextPage) == 1)
    {
      GUIpage=GUIpage-2;
    }
    else
    {
      GUIpage=GUIpage-1;
    }
    CleanFilNameBuf();
    t_gui_p.CurrentPage = GUIpage;
    FileScan();
  }

#ifdef __cplusplus
} // extern "C" {
#endif

