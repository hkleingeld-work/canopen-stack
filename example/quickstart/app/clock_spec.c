/******************************************************************************
   Copyright 2020 Embedded Office GmbH & Co. KG

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
******************************************************************************/

/******************************************************************************
* INCLUDES
******************************************************************************/

#include "co_core.h"
                                              /* select application drivers: */
#include "co_can_dummy.h"                     /* CAN driver                  */
#include "co_timer_dummy.h"                   /* Timer driver                */
#include "co_nvm_dummy.h"                     /* NVM driver                  */

/******************************************************************************
* PUBLIC DEFINES
******************************************************************************/

/* Define some default values for our CANopen node: */
#define APP_NODE_ID       1u                  /* CANopen node ID             */
#define APP_BAUDRATE      250000u             /* CAN baudrate                */
#define APP_CAN_BUS_ID    0u                  /* Bus ID (driver specific)    */
#define APP_TMR_N         16u                 /* Number of software timers   */
#define APP_TICKS_PER_SEC 1000u               /* Timer clock frequency in Hz */
#define APP_OBJ_N         128u                /* Object dictionary max size  */

/******************************************************************************
* PUBLIC VARIABLES
******************************************************************************/

/* allocate global variables for runtime value of objects */
uint8_t  Obj1001_00_08 = 0;

uint32_t Obj2100_01_20 = 0;
uint8_t  Obj2100_02_08 = 0;
uint8_t  Obj2100_03_08 = 0;

/* define the static object dictionary */
const CO_OBJ ClockOD[APP_OBJ_N] = {
    {CO_KEY(0x1000, 0, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0},
    {CO_KEY(0x1001, 0, CO_UNSIGNED8 |CO_OBJ____R_), 0, (uintptr_t)&Obj1001_00_08},
    {CO_KEY(0x1005, 0, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0x80},
    {CO_KEY(0x1017, 0, CO_UNSIGNED16|CO_OBJ_D__R_), 0, (uintptr_t)0},

    {CO_KEY(0x1018, 0, CO_UNSIGNED8 |CO_OBJ_D__R_), 0, (uintptr_t)4},
    {CO_KEY(0x1018, 1, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0},
    {CO_KEY(0x1018, 2, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0},
    {CO_KEY(0x1018, 3, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0},
    {CO_KEY(0x1018, 4, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0},

    {CO_KEY(0x1200, 0, CO_UNSIGNED8 |CO_OBJ_D__R_), 0, (uintptr_t)2},
    {CO_KEY(0x1200, 1, CO_UNSIGNED32|CO_OBJ_DN_R_), 0, (uintptr_t)0x600},
    {CO_KEY(0x1200, 2, CO_UNSIGNED32|CO_OBJ_DN_R_), 0, (uintptr_t)0x580},

    {CO_KEY(0x1800, 0, CO_UNSIGNED8 |CO_OBJ_D__R_), 0, (uintptr_t)2},
    {CO_KEY(0x1800, 1, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0x40000180},
    {CO_KEY(0x1800, 2, CO_UNSIGNED8 |CO_OBJ_D__R_), 0, (uintptr_t)254},

    {CO_KEY(0x1A00, 0, CO_UNSIGNED8 |CO_OBJ_D__R_), 0, (uintptr_t)3},
    {CO_KEY(0x1A00, 1, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0x21000120},
    {CO_KEY(0x1A00, 2, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0x21000208},
    {CO_KEY(0x1A00, 3, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0x21000308},

    {CO_KEY(0x2100, 0, CO_UNSIGNED8 |CO_OBJ_D__R_), 0, (uintptr_t)3},
    {CO_KEY(0x2100, 1, CO_UNSIGNED32|CO_OBJ___PR_), 0, (uintptr_t)&Obj2100_01_20},
    {CO_KEY(0x2100, 2, CO_UNSIGNED8 |CO_OBJ___PR_), 0, (uintptr_t)&Obj2100_02_08},
    {CO_KEY(0x2100, 3, CO_UNSIGNED8 |CO_OBJ___PR_), 0, (uintptr_t)&Obj2100_03_08},

    CO_OBJ_DIR_ENDMARK  /* mark end of used objects */
};

/* Each software timer needs some memory for managing
 * the lists and states of the timed action events.
 */
CO_TMR_MEM TmrMem[APP_TMR_N];

/* Each SDO server needs memory for the segmented or
 * block transfer requests.
 */
uint8_t SdoSrvMem[CO_SDOS_N * CO_SDO_BUF_BYTE];

/* Select the drivers for your application. For possible
 * selections, see the directory /drivers. In this example
 * we select the driver templates. You may fill them with
 * your specific hardware functionality.
 */
const CO_IF_DRV AppDriver = {
    &DummyCanDriver,
    &DummyTimerDriver,
    &DummyNvmDriver
};

/* Collect all node specification settings in a single
 * structure for initializing the node easily.
 */
CO_NODE_SPEC AppSpec = {
    APP_NODE_ID,             /* default Node-Id                */
    APP_BAUDRATE,            /* default Baudrate               */
    (CO_OBJ *)&ClockOD[0],   /* pointer to object dictionary   */
    APP_OBJ_N,               /* object dictionary max length   */
    NULL,                    /* no EMCY info fields (unused)   */
    &TmrMem[0],              /* pointer to timer memory blocks */
    APP_TMR_N,               /* number of timer memory blocks  */
    APP_TICKS_PER_SEC,       /* timer clock frequency in Hz    */
    (CO_IF_DRV *)&AppDriver, /* select drivers for application */
    &SdoSrvMem[0]            /* SDO Transfer Buffer Memory     */
};