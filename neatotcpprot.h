#ifndef NEATOTCPPROT_H
#define NEATOTCPPROT_H

#include "stdint.h"

#define MAX_PROT_SIZE		255

// type
#define PROT_LEFT_WHEEL_MOVE	0x55
#define PROT_RIGHT_WHEEL_MOVE	0x66
#define PROT_AUDIO_PLAY		0x77

#define PROT_FINISHED_0		0x00
#define PROT_FINISHED_1		0x55

typedef struct
{
    uint16_t protLen;
    uint8_t sendBuf[MAX_PROT_SIZE];
} neatoTcpProt_t;

extern neatoTcpProt_t ntp_str;

/* PROT MOVE L / R  WHEEL*/
/*
  [0] -> type
  [1] -> lByte arg1
  [2] -> hByte arg1
  [3] -> lByte arg1
  [4] -> hByte arg1
  [5] -> protFin 1
  [6] -> protFin 2
  [7] -> chkSum
*/



#endif // NEATOTCPPROT_H
