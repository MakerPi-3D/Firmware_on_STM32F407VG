#ifndef USBFILETRANSFER_H
#define USBFILETRANSFER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  void SaveUSBFile(void);
  void IsTranFileErr(void);
  void SetIsUSBPrintFinished(bool Value);
  void SetIsUSBPrintStop(bool Value);
  void SetIsUSBPrintPause(bool Value);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif //USBFILETRANSFER_H
