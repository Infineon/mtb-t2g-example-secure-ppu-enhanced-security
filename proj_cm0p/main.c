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
#include "cy_retarget_io.h"
#include "secure_config.h"
#include "shared.h"

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
/* location info provided from linker */
extern const char __si_addr_linker_symbol[];
/* becomes true when rx data comes */
volatile bool g_uartReadFlag = false;
/* testing target */
int8_t g_target = (-1);

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define UART_IRQ_PRIORITY   (3)
#define SI_ADDR             ((uint32_t)__si_addr_linker_symbol)

/* Flashboot parameters */
#define SI_FLASHBOOT_FLAGS  ((SI_FLASHBOOT_CLK_50MHZ << SI_TOC_FLAGS_CLOCKS_POS) \
                                | (SI_FLASHBOOT_WAIT_20MS << SI_TOC_FLAGS_DELAY_POS) \
                                | (SI_FLASHBOOT_SWJ_ENABLE << SI_TOC_FLAGS_SWJEN_POS) \
                                | (SI_FLASHBOOT_VALIDATE_DISABLE << SI_TOC_FLAGS_APP_VERIFY_POS) \
                                | (SI_FLASHBOOT_FBLOADER_DISABLE << SI_TOC_FLAGS_FBLOADER_ENABLE_POS))

#define TARGET_CM70         (0)
#define TARGET_CM71         (1)
#define TARGET_CM0P         (2)

#define PCMASK_3_4_5_6_7    (CY_PROT_PCMASK3 | CY_PROT_PCMASK4 | CY_PROT_PCMASK5 | CY_PROT_PCMASK6 | CY_PROT_PCMASK7)
#define PCMASK_3_4_5        (CY_PROT_PCMASK3 | CY_PROT_PCMASK4 | CY_PROT_PCMASK5)
#define PCMASK_2_6_7        (CY_PROT_PCMASK2 | CY_PROT_PCMASK6 | CY_PROT_PCMASK7)
#define PCMASK_2_3_4_5_7    (CY_PROT_PCMASK2 | CY_PROT_PCMASK3 | CY_PROT_PCMASK4 | CY_PROT_PCMASK5 | CY_PROT_PCMASK7)

/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
/*********************************************************************************************************************/
CY_SECTION(".cy_toc_part2") __USED static const SFLASH_TOC2_Type SFLASH_TOC2 =
{
    .TOC2_OBJECT_SIZE                   = SI_TOC2_OBJECTSIZE,
    .TOC2_MAGIC_NUMBER                  = SI_TOC2_MAGICNUMBER,
    .TOC2_SMIF_CFG_STRUCT_ADDR          = 0UL,
    .TOC2_FIRST_USER_APP_ADDR           = SI_ADDR,                          /* top address of CM0+ app */
    .TOC2_FIRST_USER_APP_FORMAT         = SI_APP_FORMAT_BASIC,              /* format of CM0+ app */
    .TOC2_SECOND_USER_APP_ADDR          = 0UL,                              /* not used */
    .TOC2_SECOND_USER_APP_FORMAT        = 0UL,                              /* basic */
    .TOC2_FIRST_CMX_1_USER_APP_ADDR     = 0UL,                              /* not verified by Flash Boot*/
    .TOC2_SECOND_CMX_1_USER_APP_ADDR    = 0UL,                              /* nused */
    .TOC2_FIRST_CMX_2_USER_APP_ADDR     = 0UL,                              /* not verified by Flash Boot*/
    .TOC2_SECOND_CMX_2_USER_APP_ADDR    = 0UL,                              /* not used */
    .RESERVED14                         = {0UL},
    .TOC2_SECURITY_UPDATES_MARKER       = SI_SECURITY_ENHANCED,             /* enable PPUs configuration */
    .TOC2_SHASH_OBJECTS                 = 3UL,                              /* KEY + APP_PROTECTION + TOC2 */
    .TOC2_SIGNATURE_VERIF_KEY           = 0UL,
    .TOC2_APP_PROTECTION_ADDR           = SI_SWPU_BEGIN,                    /* this area shall not be modified */
    .RESERVED15                         = {0UL},
    .TOC2_REVISION                      = 0UL,                              /* not used */
    .TOC2_FLAGS                         = SI_FLASHBOOT_FLAGS,
};

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
    printf("CM0+: Access violation has been detected, process terminated...\r\n");

    while (1)
    {
        ;
    }
}

/**********************************************************************************************************************
 * Function Name: handleUartEvent
 * Summary:
 *  UART event handler callback function. Sets the read flag to true upon successful reception of data.
 * Parameters:
 *  handlerArg - argument for the handler provided during callback registration
 *  event - interrupt cause flags
 * Return:
 *  void
 **********************************************************************************************************************
 */
void handleUartEvent(void *handlerArg, cyhal_uart_event_t event)
{
    (void)handlerArg;

    if (CYHAL_UART_IRQ_RX_DONE == (event & CYHAL_UART_IRQ_RX_DONE))
    {
        /* Set read flag */
        g_uartReadFlag = true;
    }
    else
    {
        CY_ASSERT(0);
    }
}

/**********************************************************************************************************************
 * Function Name: printTargets
 * Summary:
 *  Prints set of targets.
 * Parameters:
 *  void
 * Return:
 *  void
 **********************************************************************************************************************
 */
void printTargets(void)
{
    printf("====================================================\r\n"
           "Targets:\r\n"
           "====================================================\r\n"
           "Press '0' : To select the CM7_0\r\n"
           "Press '1' : To select the CM7_1\r\n"
           "Press '2' : To select the CM0+\r\n"
           "====================================================\r\n");
}

/**********************************************************************************************************************
 * Function Name: printInstructions
 * Summary:
 *  Prints set of instructions.
 * Parameters:
 *  void
 * Return:
 *  void
 **********************************************************************************************************************
 */
void printInstructions(void)
{
    printf("====================================================\r\n"
           "Instructions for %s:\r\n"
           "====================================================\r\n"
           "Press 'a' : To reads the address 0x40004090\r\n"
           "Press 'b' : To writes the address 0x40004090\r\n"
           "Press 'c' : To reads the address 0x40201004\r\n"
           "Press 'd' : To writes the address 0x40201004\r\n"
           "Press 'e' : To reads the address 0x40201020\r\n"
           "Press 'f' : To writes the address 0x40201020\r\n"
           "Press 'g' : To reads the address 0x40201300\r\n"
           "Press 'h' : To writes the address 0x40201300\r\n"
           "Press 'i' : To reads the address 0x402013C8\r\n"
           "Press 'j' : To writes the address 0x402013C8\r\n"
           "Press 'k' : To reads the address 0x402013CC\r\n"
           "Press 'l' : To writes the address 0x402013CC\r\n"
           "Press 'm' : To reads the address 0x40201400\r\n"
           "Press 'n' : To writes the address 0x40201400\r\n",
           g_target == TARGET_CM0P ? "CM0+" : g_target == TARGET_CM70 ? "CM7_0" : "CM7_1");
    if (g_target == TARGET_CM70)
    {
        printf("Press 'o' : To make selected target executes ECC check\r\n");
    }
    printf("====================================================\r\n");
}

/**********************************************************************************************************************
 * Function Name: processKeyPress
 * Summary:
 *  Function to process the key pressed.
 *  Depending on the command passed as parameter, the process to be performed is selected.
 * Parameters:
 *  keyPressed - command read through terminal
 * Return:
 *  The core number to wait for response at caller (-1 means no need to wait)
 **********************************************************************************************************************
 */
int8_t processKeyPress(char keyPressed)
{
    int8_t tobeWait = -1;

    printf("Pressed key: %c\r\n", keyPressed);

    if (g_target == (-1))
    {
        switch (keyPressed)
        {
        /* Press '0' : To set CM7_0 as the testing target */
        case '0':
            g_target = TARGET_CM70;
            break;
        /* Press '1' : To set CM7_1 as the testing target */
        case '1':
            g_target = TARGET_CM71;
            break;
        /* Press '2' : To set CM0+ as the testing target */
        case '2':
            g_target = TARGET_CM0P;
            break;
        default:
            printf("Wrong key pressed !! See below instructions:\r\n");
            printTargets();
            break;
        }
        if (g_target != (-1))
        {
            printInstructions();
        }
    }
    else
    {
        uint8_t req;
        if ((keyPressed >= 'a') && ((req = (keyPressed - 'a')) < REQ_NUM))
        {
            if (g_target == TARGET_CM0P)
            {
                if (req < RW_NUM)
                {
                    printf("%s\r\n", doRequestedAccess(req));
                }
                printTargets();
            }
            else
            {
                if (req < (g_target == TARGET_CM70 ? REQ_NUM : RW_NUM))
                {
                    g_shared[g_target] = req;
                    tobeWait = g_target;
                }
            }
            g_target = (-1);
        }
        else
        {
            printf("Wrong key pressed !! See below instructions:\r\n");
            printInstructions();
        }
    }

    return tobeWait;
}

/**********************************************************************************************************************
 * Function Name: initPPU1st
 * Summary:
 *  Function to initialize PPU (1st step).
 * Parameters:
 *  none
 * Return:
 *  none
 **********************************************************************************************************************
 */
void initPPU1st(void)
{
    /* set PC for CM0+ (PC2) */
    if (Cy_Prot_ConfigBusMaster(CPUSS_MS_ID_CM0, true, true, CY_PROT_PCMASK2) != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_SetActivePC(CPUSS_MS_ID_CM0, CY_PROT_PC2) != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* set PC for CM7_0/1 (CM7_0 = PC6, CM7_1 = PC7) */
    if (Cy_Prot_ConfigBusMaster(CPUSS_MS_ID_CM7_0, true, true, PCMASK_3_4_5_6_7) != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_SetActivePC(CPUSS_MS_ID_CM7_0, CY_PROT_PC6) != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_ConfigBusMaster(CPUSS_MS_ID_CM7_1, true, true, PCMASK_3_4_5_6_7) != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_SetActivePC(CPUSS_MS_ID_CM7_1, CY_PROT_PC7) != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Programmable PPU11 Slave structure(PC2-7) Setting */
    if (Cy_Prot_ConfigPpuProgSlaveAtt(PERI_MS_PPU_PR11, PCMASK_3_4_5, CY_PROT_PERM_R, CY_PROT_PERM_R, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_ConfigPpuProgSlaveAtt(PERI_MS_PPU_PR11, PCMASK_2_6_7, CY_PROT_PERM_RW, CY_PROT_PERM_RW, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Programmable PPU11 Master structure(PC2-7) Setting */
    if (Cy_Prot_ConfigPpuProgMasterAtt(PERI_MS_PPU_PR11, PCMASK_3_4_5_6_7, CY_PROT_PERM_R, CY_PROT_PERM_R, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_ConfigPpuProgMasterAtt(PERI_MS_PPU_PR11, CY_PROT_PCMASK2, CY_PROT_PERM_RW, CY_PROT_PERM_RW, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Programmable PPU12 Slave structure(PC2-7) Setting. This PPU need to reconfigurs after ECC checking is completed */
    if (Cy_Prot_ConfigPpuProgSlaveAtt(PERI_MS_PPU_PR12, PCMASK_2_3_4_5_7, CY_PROT_PERM_R, CY_PROT_PERM_R, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_ConfigPpuProgSlaveAtt(PERI_MS_PPU_PR12, CY_PROT_PCMASK6, CY_PROT_PERM_RW, CY_PROT_PERM_RW, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Programmable PPU12 Master structure(PC2-7) Setting. This PPU need to reconfigurs after ECC checking is completed */
    if (Cy_Prot_ConfigPpuProgMasterAtt(PERI_MS_PPU_PR12, PCMASK_3_4_5_6_7, CY_PROT_PERM_R, CY_PROT_PERM_R, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_ConfigPpuProgMasterAtt(PERI_MS_PPU_PR12, CY_PROT_PCMASK2, CY_PROT_PERM_RW, CY_PROT_PERM_RW, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Programmable PPU13 Slave structure(PC2-7) Setting */
    if (Cy_Prot_ConfigPpuProgSlaveAtt(PERI_MS_PPU_PR13, PCMASK_3_4_5, CY_PROT_PERM_R, CY_PROT_PERM_R, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_ConfigPpuProgSlaveAtt(PERI_MS_PPU_PR13, PCMASK_2_6_7, CY_PROT_PERM_RW, CY_PROT_PERM_RW, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Programmable PPU13 Master structure(PC2-7) Setting */
    if (Cy_Prot_ConfigPpuProgMasterAtt(PERI_MS_PPU_PR13, PCMASK_3_4_5_6_7, CY_PROT_PERM_R, CY_PROT_PERM_R, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_ConfigPpuProgMasterAtt(PERI_MS_PPU_PR13, CY_PROT_PCMASK2, CY_PROT_PERM_RW, CY_PROT_PERM_RW, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Fixed PPU17 Slave structure(PC2-7) Setting */
    if (Cy_Prot_ConfigPpuFixedSlaveAtt(PERI_MS_PPU_FX_CPUSS_CM0, PCMASK_3_4_5_6_7, CY_PROT_PERM_R, CY_PROT_PERM_R, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_ConfigPpuFixedSlaveAtt(PERI_MS_PPU_FX_CPUSS_CM0, CY_PROT_PCMASK2, CY_PROT_PERM_RW, CY_PROT_PERM_RW, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Fixed PPU17 Master structure(PC2-7) Setting */
    if (Cy_Prot_ConfigPpuFixedMasterAtt(PERI_MS_PPU_FX_CPUSS_CM0, PCMASK_3_4_5_6_7, CY_PROT_PERM_R, CY_PROT_PERM_R, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    if (Cy_Prot_ConfigPpuFixedMasterAtt(PERI_MS_PPU_FX_CPUSS_CM0, CY_PROT_PCMASK2, CY_PROT_PERM_RW, CY_PROT_PERM_RW, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    printf("PPU configured (1st)\r\n");
}

/**********************************************************************************************************************
 * Function Name: initPPU2nd
 * Summary:
 *  Function to initialize PPU (2nd step).
 *  This reconfigures PPU12 so that CM7_0 cannot access the ECC check function.
 * Parameters:
 *  none
 * Return:
 *  none
 **********************************************************************************************************************
 */
void initPPU2nd(void)
{
    if (Cy_Prot_ConfigPpuProgSlaveAtt(PERI_MS_PPU_PR12, CY_PROT_PCMASK6, CY_PROT_PERM_R, CY_PROT_PERM_R, false)
                                                                                                    != CY_PROT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    printf("PPU configured (2nd)\r\n");
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
    char uartReadValue;

    /* Initialize the device and board peripherals */
    if (cybsp_init() != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to use the debug UART port */
    if (cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE) != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* The UART callback handler registration */
    cyhal_uart_register_callback(&cy_retarget_io_uart_obj, handleUartEvent, NULL);

    /* Enable UART events to get notified on receiving
     * RX data and on RX errors */
    cyhal_uart_enable_event(&cy_retarget_io_uart_obj,
                            (cyhal_uart_event_t)(CYHAL_UART_IRQ_RX_ERROR | CYHAL_UART_IRQ_RX_DONE),
                            UART_IRQ_PRIORITY, true);

    /* enable interrupts */
    __enable_irq();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("****************** "
           "code example of PPU Enhanced Security "
           "****************** \r\n\n");

    /* Disable the write buffer to make the peripheral registers can be directly accessed */
    CPUSS->BUFF_CTL = 0;

    /* configure PPU setting */
    initPPU1st();

    /* Clear shared data */
    memset(g_shared, 0xFF, sizeof(g_shared));

    /* Enable CM7_0/1. CY_CORTEX_M7_APPL_ADDR is calculated in linker script, check it in case of problems. */
    Cy_SysEnableCM7(CORE_CM7_0, CY_CORTEX_M7_0_APPL_ADDR);
    Cy_SysEnableCM7(CORE_CM7_1, CY_CORTEX_M7_1_APPL_ADDR);

    /* Display target selection */
    printTargets();

    for (;;)
    {
        /* Begin asynchronous RX read */
        cyhal_uart_read_async(&cy_retarget_io_uart_obj, (void*) &uartReadValue, sizeof(uartReadValue));

        /* Check if the read flag has been set by the callback */
        if (g_uartReadFlag)
        {
            /* Clear read flag */
            g_uartReadFlag = false;

            /* Clear shared data */
            memset(g_shared, 0xFF, sizeof(g_shared));

            /* Process the command */
            int8_t tobeWait = processKeyPress(uartReadValue);

            /* Waiting for the response from CM7_0/1 */
            if (tobeWait != (-1))
            {
                uint32_t *pRsp = ((tobeWait == TARGET_CM70) ? &g_shared[FROM_CM7_0_TO_CM0] : &g_shared[FROM_CM7_1_TO_CM0]);
                while (*pRsp == 0xFFFFFFFF)
                {
                    cyhal_system_delay_ms(DELAY_BETWEEN_READ_MS);
                }
                /* Display the string from CM7_0/1 */
                printf("CM7_%d reports: %s\r\n\r\n", tobeWait, (char *)*pRsp);
                if (strcmp((char *)*pRsp, MSG_ECCCHECK_DONE) == 0)
                {
                    /* reconfigure PPU setting */
                    initPPU2nd();
                }
                printTargets();
            }
        }

        /* Delay between next read */
        cyhal_system_delay_ms(DELAY_BETWEEN_READ_MS);
    }

    return 0;
}

/* [] END OF FILE */