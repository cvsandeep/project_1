/*
 * functional_interface.h
 *
 *  Created on: Apr 9, 2019
 *      Author: venksand
 */

#ifndef SRC_FUNCTIONAL_INTERFACE_H_
#define SRC_FUNCTIONAL_INTERFACE_H_

u32 laststate;

void UpdateRGBled(u8 hue, u8 sat, u8 val);
u8 GetHue(void);
u8 GetSat(void);
u8 GetVal(void);
bool GetDetectType(void);
bool IsExit(void);

void RunTest1(void);
void RunTest2(void);
void RunTest3(void);
void RunTest4(void);
void RunTest5(void);

#endif /* SRC_FUNCTIONAL_INTERFACE_H_ */
