#ifndef _ARM2AVR_H_
#define _ARM2AVR_H_

enum {
  NOS_OF_AVR_OUTPUTS  = 4,
  NOS_OF_AVR_BTNS     = 4,
  NOS_OF_AVR_INPUTS   = 4
};

enum {
  BRAKE = 1,
  FLOAT = 0
};


typedef   struct
{
  UWORD   AdValue[NOS_OF_AVR_INPUTS];
  UWORD   Buttons;
  UWORD   Battery;
} IOFROMAVR;

typedef   struct
{
  UBYTE   Power;
  UBYTE   PwmFreq;
  SBYTE   PwmValue[NOS_OF_AVR_OUTPUTS];
  UBYTE   OutputMode;
  UBYTE   InputPower;
} IOTOAVR;

extern IOTOAVR volatile IoToAvr;
extern IOFROMAVR volatile IoFromAvr;


#endif
