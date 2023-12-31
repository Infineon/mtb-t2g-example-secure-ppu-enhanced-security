/**********************************************************************************************************************
 * \file secure_config.h
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
#ifndef SECURE_CONFIG_H
#define SECURE_CONFIG_H

#if defined(__cplusplus)
extern "C" {
#endif

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
/* Secure Image Version: Macros to define the secure image version and ID */
#define SI_APP_VERSION(major, minor)      (((major) << 24u) | ((minor) << 16u))

#define SI_SECURE_DIGSIG_SIZE               (256u)          /* Size (in Bytes) of the digital signature */

/* Flash Boot clock selection:
 * Clock selection for Flash boot execution
 */
#define SI_FLASHBOOT_CLK_8MHZ               (0UL)           /* 8MHz clock selection for Flashboot */
#define SI_FLASHBOOT_CLK_25MHZ              (1UL)           /* 25MHz clock selection for Flashboot */
#define SI_FLASHBOOT_CLK_50MHZ              (2UL)           /* 50MHz clock selection for Flashboot */
#define SI_FLASHBOOT_CLK_100MHZ             (3UL)           /* USER Configuration */

/* Flash Boot wait window:
 * Debugger wait window selection for Flash boot execution
 */
#define SI_FLASHBOOT_WAIT_20MS              (0UL)           /* 20ms debugger wait window for Flashboot */
#define SI_FLASHBOOT_WAIT_10MS              (1UL)           /* 10ms debugger wait window for Flashboot */
#define SI_FLASHBOOT_WAIT_1MS               (2UL)           /* 1ms debugger wait window for Flashboot */
#define SI_FLASHBOOT_WAIT_0MS               (3UL)           /* 0ms debugger wait window for Flashboot */
#define SI_FLASHBOOT_WAIT_100MS             (4UL)           /* 100ms debugger wait window for Flashboot */

/* Debugger pin configuration */
#define SI_FLASHBOOT_SWJ_DISABLE            (1UL)           /* Do not enable SWJ pins in Flash boot. Listen window is skipped */
#define SI_FLASHBOOT_SWJ_ENABLE             (2UL)           /* Enable SWJ pins in Flash boot */

/* Flash Boot validation:
 * Flash boot validation selection in chip NORMAL mode
 */
#define SI_FLASHBOOT_VALIDATE_DISABLE       (1UL)           /* Do not validate app1 in NORMAL mode */
#define SI_FLASHBOOT_VALIDATE_ENABLE        (2UL)           /* Validate app1 in NORMAL mode */

/* Flash boot loader configuration */
#define SI_FLASHBOOT_FBLOADER_ENABLE        (1UL)           /* Internal bootloader is launched if the other bootloader conditions are met */
#define SI_FLASHBOOT_FBLOADER_DISABLE       (2UL)           /* Internal bootloader is disabled */

/* Application format:
 * Application format selection for secure boot
 */
#define SI_APP_FORMAT_BASIC                 (0UL)           /* Basic application format (no header) */
#define SI_APP_FORMAT_CYPRESS               (1UL)           /* Cypress application format (Cypress header) */

/* Security enhancement configuration:
 * Security enhancement configuration for secure boot
 */
#define SI_SECURITY_NOT_ENHANCED            (0x00000000)    /* No Security Enhanced */
#define SI_SECURITY_ENHANCED                (0xFEDEEDDF)    /* Security Enhanced */

/* Application type:
 * Application type selection for secure boot
 */
#define SI_APP_ID_FLASHBOOT                 (0x8001UL)      /* Flash boot ID Type */
#define SI_APP_ID_SECUREIMG                 (0x8002UL)      /* Secure image ID Type */
#define SI_APP_ID_BOOTLOADER                (0x8003UL)      /* Bootloader ID Type */

/***************************************
 *            Constants
 ***************************************
 */
#define SI_TOC_FLAGS_CLOCKS_MASK            (0x00000003UL)  /* Mask for Flashboot clock selection */
#define SI_TOC_FLAGS_CLOCKS_POS             (0UL)           /* Bit position of Flashboot clock selection */
#define SI_TOC_FLAGS_DELAY_MASK             (0x0000001CUL)  /* Mask for Flashboot wait window selection */
#define SI_TOC_FLAGS_DELAY_POS              (2UL)           /* Bit position of Flashboot wait window selection */
#define SI_TOC_FLAGS_SWJEN_POS              (5UL)           /* Bit position of SWJ pin configuration */
#define SI_TOC_FLAGS_APP_VERIFY_MASK        (0x80000000UL)  /* Mask for Flashboot NORMAL mode app1 validation */
#define SI_TOC_FLAGS_APP_VERIFY_POS         (7UL)           /* Bit position of Flashboot NORMAL mode app1 validation */

#define SI_TOC_FLAGS_FBLOADER_ENABLE_POS    (9UL)           /* Bit position of Flashboot Loader Enable */

#define SI_TOC2_MAGICNUMBER                 (0x01211220UL)  /* TOC2 identifier */

#define SI_TOC2_OBJECTSIZE                  (0x000001FCUL)  /* Number of TOC2 object */
#define SI_SWPU_BEGIN                       (0x17007600UL)  /* Address of SWPU configuration */
#define SI_TOC2_BEGIN                       (0x17007C00UL)  /* Address of TOC2 */

#define SI_PUBLIC_KEY                       (0x17006400UL)  /* PUBLIC KEY address in SFlash */

/*********************************************************************************************************************/
/*-------------------------------------------------Data Structures---------------------------------------------------*/
/*********************************************************************************************************************/
/* TOC2 structure
 * note: this structure is subset of SFLASH_Type which includes whole SFLASH contents defined in "cyip_sflash_v2.h"
 */
typedef struct
{
    /* Object size in bytes for CRC calculation starting from offset 0x00 */
    __IOM uint32_t TOC2_OBJECT_SIZE;

    /* Magic number(0x01211220) */
    __IOM uint32_t TOC2_MAGIC_NUMBER;

    /* Null terminated table of pointers representing the SMIF configuration structure */
    __IOM uint32_t TOC2_SMIF_CFG_STRUCT_ADDR;

    /* Address of First User Application Object */
    __IOM uint32_t TOC2_FIRST_USER_APP_ADDR;

    /* Format of First User Application Object.
     *  0 - Basic
     *  1 - Cypress standard
     *  2 - Simplified
     */
    __IOM uint32_t TOC2_FIRST_USER_APP_FORMAT;

    /* Address of Second User Application Object */
    __IOM uint32_t TOC2_SECOND_USER_APP_ADDR;

    /* Format of Second User Application Object.
     *  0 - Basic
     *  1 - Cypress standard
     *  2 - Simplified
     */
    __IOM uint32_t TOC2_SECOND_USER_APP_FORMAT;

    /* Address of First CM4 or CM7 core1 User Application Object */
    __IOM uint32_t TOC2_FIRST_CMX_1_USER_APP_ADDR;

    /* Address of Second CM4 or CM7 core1 User Application Object */
    __IOM uint32_t TOC2_SECOND_CMX_1_USER_APP_ADDR;

    /* Address of First CM4 or CM7 core2 User Application Object */
    __IOM uint32_t TOC2_FIRST_CMX_2_USER_APP_ADDR;

    /* Address of Second CM4 or CM7 core2 User Application Object */
    __IOM uint32_t TOC2_SECOND_CMX_2_USER_APP_ADDR;

    __IM uint32_t RESERVED14[52];

    /* Marker for Security Updates */
    __IOM uint32_t TOC2_SECURITY_UPDATES_MARKER;

    /* Number of additional objects to be verified for SECURE_HASH */
    __IOM uint32_t TOC2_SHASH_OBJECTS;

    /* Address of signature verification key (0 if none).The object is
     * signature specific key. It is the public key in case of RSA
     */
    __IOM uint32_t TOC2_SIGNATURE_VERIF_KEY;

    /* Address of  Application Protection */
    __IOM uint32_t TOC2_APP_PROTECTION_ADDR;

    __IM uint32_t RESERVED15[58];

    /* Indicates TOC2 Revision. It is not used now. */
    __IOM uint32_t TOC2_REVISION;

    /* Controls default configuration */
    __IOM uint32_t TOC2_FLAGS;

} SFLASH_TOC2_Type;

/* Secure image application header in Cypress format */
typedef struct
{
    /* Object size (Bytes) */
    volatile uint32_t objSize;

    /* Application ID/version */
    volatile uint32_t appId;

    /* Attributes (reserved for future use) */
    volatile uint32_t appAttributes;

    /* Number of cores */
    volatile uint32_t numCores;

    /* VT offset - offset to the vector table from that entry */
    volatile uint32_t core0Vt;

    /* core ID */
    volatile uint32_t core0Id;

} CYSAF_APPHEADER_Type;

#if defined(__cplusplus)
}
#endif

#endif /* SECURE_CONFIG_H */

/* [] END OF FILE */
