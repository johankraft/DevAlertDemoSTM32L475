/*******************************************************************************
 * DFM (DevAlert Firmware Monitor) Library v2023.61.8.16
 * Percepio AB, www.percepio.com
 *
 * dfmCodes.h
 *
 * Contains definitions for all Alerts and Symptoms.
 *
 * Terms of Use
 * This file is part of the DevAlert Firmware Monitor library (SOFTWARE), which
 * is the intellectual property of Percepio AB (PERCEPIO) and provided under a
 * license as follows.
 * The SOFTWARE may be used free of charge for the purpose of collecting and
 * transferring data to the DevAlert cloud service. It may not be used or modified
 * for other purposes without explicit permission from PERCEPIO.
 * You may distribute the SOFTWARE in its original source code form, assuming
 * this text (terms of use, disclaimer, copyright notice) is unchanged. You are
 * allowed to distribute the SOFTWARE with minor modifications intended for
 * configuration or porting of the SOFTWARE, e.g., to allow using it on a
 * specific processor, processor family or with a specific communication
 * interface. Any such modifications should be documented directly below
 * this comment block.
 * The DevAlert cloud service and the Tracealyzer desktop application are
 * provided under separate terms.
 *
 * Disclaimer
 * The SOFTWARE is being delivered to you AS IS and PERCEPIO makes no warranty
 * as to its use or performance. PERCEPIO does not and cannot warrant the
 * performance or results you may obtain by using the SOFTWARE or documentation.
 * PERCEPIO make no warranties, express or implied, as to noninfringement of
 * third party rights, merchantability, or fitness for any particular purpose.
 * In no event will PERCEPIO, its technology partners, or distributors be liable
 * to you for any consequential, incidental or special damages, including any
 * lost profits or lost savings, even if a representative of PERCEPIO has been
 * advised of the possibility of such damages, or for any claim by any third
 * party. Some jurisdictions do not allow the exclusion or limitation of
 * incidental, consequential or special damages, or the exclusion of implied
 * warranties or limitations on how long an implied warranty may last, so the
 * above limitations may not apply to you.
 *
 * Tabs are used for indent in this file (1 tab = 4 spaces)
 *
 * Copyright Percepio AB, 2023.
 * www.percepio.com
******************************************************************************/


#ifndef __DFM_CODES_H__
#define __DFM_CODES_H__

/* Alert Types */
/* The following Alert Types are published and will not change. */
#define DFM_TYPE_HEARTBEAT (7) /* Heartbeat failure */
#define DFM_TYPE_BAD_MESSAGE (6) /* Invalid/bad message received */
#define DFM_TYPE_OVERLOAD (5) /* CPU Overload */
#define DFM_TYPE_MANUAL_TRACE (4) /* User invoked alert */
#define DFM_TYPE_HARDFAULT (3) /* Hard Fault */
#define DFM_TYPE_MALLOC_FAILED (2) /* Malloc Failed */
#define DFM_TYPE_ASSERT_FAILED (1) /* Assert Failed */


/* Symptoms */
/* The following Symptoms are published and will not change. */
#define DFM_SYMPTOM_ARM_SCB_FCSR (7) /* ARM_SCB_FCSR */
#define DFM_SYMPTOM_STACKPTR (6) /* Stack Pointer */
#define DFM_SYMPTOM_PC (5) /* PC */
#define DFM_SYMPTOM_LINE (4) /* Line */
#define DFM_SYMPTOM_FUNCTION (3) /* Function */
#define DFM_SYMPTOM_FILE (2) /* File */
#define DFM_SYMPTOM_CURRENT_TASK (1) /* Current Task */


#endif /* __DFM_CODES_H__ */
