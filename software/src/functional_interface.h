/*
 * functional_interface.h
 *
 *  Created on: Apr 9, 2019
 *      Author: venksand
 */

#ifndef SRC_FUNCTIONAL_INTERFACE_H_
#define SRC_FUNCTIONAL_INTERFACE_H_

void UpdateRGBled(u16 hue, u8 sat, u8 val, bool display);
u16 GetHue(void);
u8 GetSat(void);
u8 GetVal(void);
bool GetDetectType(void);
bool IsExit(void);
void DisplayDutycycle(u8 r_duty, u8 g_duty, u8 b_duty);
void OLEDrgb_PutStringXY(u8 x, u8 y, char* s);
void OLEDrgb_PutIntigerXY(u8 x, u8 y, int32_t num, int32_t radix);
void UpdateDispaly(u16 hue, u8 sat, u8 val);
u8 calc_duty(u32 high, u32 low);

void RunTest1(void);
void RunTest2(void);
void RunTest3(void);
void RunTest4(void);
void RunTest5(void);

#endif /* SRC_FUNCTIONAL_INTERFACE_H_ */
