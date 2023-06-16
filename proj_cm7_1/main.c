/**********************************************************************************************************************
 * \file main.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 * 
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of 
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 * 
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and 
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all 
 * derivative works of the Software, unless such copies or derivative works are solely in the form of 
 * machine-executable object code generated by a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 *********************************************************************************************************************/

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "secure_config.h"
#include "shared.h"

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
/**********************************************************************************************************************
 * Function Name: HardFault_Handler
 * Summary:
 *  This is the handler function for HardFault exception.
 *  This function overrides default HardFault_Handler, directly registered to __Vectors.
 * Parameters:
 *  none
 * Return:
 *  none
 **********************************************************************************************************************
 */
void HardFault_Handler(void)
{
    g_shared[FROM_CM7_1_TO_CM0] = (uint32_t)"CM7_1: Access violation has been detected, process terminated...\r\n";

    while (1)
    {
        ;
    }
}

/**********************************************************************************************************************
 * Function Name: main
 * Summary:
 *  This is the main function.
 * Parameters:
 *  none
 * Return:
 *  int
 **********************************************************************************************************************
 */
int main(void)
{
    /* Enable global interrupts */
    __enable_irq();

    for (;;)
    {
        /* Check request and perform it if there */
        if (g_shared[FROM_CM0_TO_CM7_1] < RW_NUM)
        {
            g_shared[FROM_CM7_1_TO_CM0] = (uint32_t)doRequestedAccess((uint8_t)g_shared[FROM_CM0_TO_CM7_1]);
            g_shared[FROM_CM0_TO_CM7_1] = 0xFFFFFFFF;
        }
        else
        {
            /* Delay between next read */
            cyhal_system_delay_ms(DELAY_BETWEEN_READ_MS);
        }
    }
}

/* [] END OF FILE */
