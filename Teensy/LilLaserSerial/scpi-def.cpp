/*-
 * BSD 2-Clause License
 *
 * Copyright (c) 2012-2018, Jan Breuer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 This code is an adaptation of the "common-cxx" example folder from [scpi-parser].

 GitHub page for [scpi-parser]: https://github.com/j123b567/scpi-parser
 Git checkout was on March 4, 2023. Revision 4e87990.

 The following files were modified for Arduino:
   [scpi-parser]/examples/common-cxx/scpi-def.cpp
   [scpi-parser]/examples/common-cxx/scpi-def.h

 The original [scpi-parser] code is an open project by Jan Breuer.
    Modified for Arduino by Scott Feister on March 4, 2023
    BSD license is above for the [scpi-parser] project.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Arduino.h>
#include <SCPI_Parser.h>
#include "scpi-def.h"

/// EXTERNAL VARIABLES (from LilLaserSerial.ino)
extern uint64_t trigcnt;
extern bool await_update;

extern bool out_enabled_pr;
extern uint32_t prepulse_us_pr;
extern uint32_t mainpulse_us_pr;
extern double prepulse_power_pr;
extern double mainpulse_power_pr;
extern uint32_t refreprate_hz_pr;

extern const bool out_enabled;
extern const uint32_t prepulse_us;
extern const uint32_t mainpulse_us;
extern const double prepulse_power;
extern const double mainpulse_power;
extern const uint32_t refreprate_hz;


static scpi_result_t DAQ_TriggerCountQ(scpi_t * context) {
    SCPI_ResultUInt64(context, trigcnt);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_TriggerCount(scpi_t * context) {
    uint64_t param1;

    /* read first parameter if present */
    if (!SCPI_ParamUInt64(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    trigcnt = param1;

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_TriggerInternalReprateQ(scpi_t * context) {
    SCPI_ResultDouble(context, refreprate_hz);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_AwaitQ(scpi_t * context) {
    SCPI_ResultBool(context, await_update);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_TriggerInternalReprate(scpi_t * context) {
    double param1;

    /* read first parameter if present */
    if (!SCPI_ParamDouble(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    refreprate_hz_pr = constrain(param1, 0.10, 5000.0); // Constrain reference rep rate to between 0.1 Hz and 5000 Hz, somewhat arbitrarily, to avoid lockouts
    await_update = true;

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_OutputEnabled(scpi_t * context) {
    bool param1;

    /* read first parameter if present */
    if (!SCPI_ParamBool(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    out_enabled_pr = param1;
    await_update = true;

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_OutputEnabledQ(scpi_t * context) {
    SCPI_ResultBool(context, out_enabled);

    return SCPI_RES_OK;
}

// LILLASER Specific

static scpi_result_t DAQ_LaserPrePowerQ(scpi_t * context) {
    SCPI_ResultDouble(context, prepulse_power);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_LaserPrePower(scpi_t * context) {
    double param1;

    /* read first parameter if present */
    if (!SCPI_ParamDouble(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    prepulse_power_pr = param1;
    await_update = true;

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_LaserMainPowerQ(scpi_t * context) {
    SCPI_ResultDouble(context, mainpulse_power);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_LaserMainPower(scpi_t * context) {
    double param1;

    /* read first parameter if present */
    if (!SCPI_ParamDouble(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    mainpulse_power_pr = param1;
    await_update = true;

    return SCPI_RES_OK;
}


static scpi_result_t DAQ_LaserPreDurationQ(scpi_t * context) {
    SCPI_ResultUInt32(context, prepulse_us);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_LaserPreDuration(scpi_t * context) {
    uint32_t param1;

    /* read first parameter if present */
    if (!SCPI_ParamUInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    prepulse_us_pr = param1;
    await_update = true;

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_LaserMainDurationQ(scpi_t * context) {
    SCPI_ResultUInt32(context, mainpulse_us);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_LaserMainDuration(scpi_t * context) {
    uint32_t param1;

    /* read first parameter if present */
    if (!SCPI_ParamUInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    mainpulse_us_pr = param1;
    await_update = true;

    return SCPI_RES_OK;
}



const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    {"*CLS", SCPI_CoreCls, 0},
    {"*ESE", SCPI_CoreEse, 0},
    {"*ESE?", SCPI_CoreEseQ, 0},
    {"*ESR?", SCPI_CoreEsrQ, 0},
    {"*IDN?", SCPI_CoreIdnQ, 0},
    {"*OPC", SCPI_CoreOpc, 0},
    {"*OPC?", SCPI_CoreOpcQ, 0},
    {"*RST", SCPI_CoreRst, 0},
    {"*SRE", SCPI_CoreSre, 0},
    {"*SRE?", SCPI_CoreSreQ, 0},
    {"*STB?", SCPI_CoreStbQ, 0},
    {"*WAI", SCPI_CoreWai, 0},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {"SYSTem:ERRor[:NEXT]?", SCPI_SystemErrorNextQ, 0},
    {"SYSTem:ERRor:COUNt?", SCPI_SystemErrorCountQ, 0},
    {"SYSTem:VERSion?", SCPI_SystemVersionQ, 0},

    {"STATus:QUEStionable[:EVENt]?", SCPI_StatusQuestionableEventQ, 0},
    {"STATus:QUEStionable:ENABle", SCPI_StatusQuestionableEnable, 0},
    {"STATus:QUEStionable:ENABle?", SCPI_StatusQuestionableEnableQ, 0},

    {"STATus:PRESet", SCPI_StatusPreset, 0},

    /* My instrument */
    /* DAQ */
    {.pattern = "TRIGger:COUNt?", .callback = DAQ_TriggerCountQ,},
    {.pattern = "TRIGger:COUNt", .callback = DAQ_TriggerCount,},
    {.pattern = "AWAIT?", .callback = DAQ_AwaitQ,},
    {.pattern = "TRIGger:INTernal:REPrate?", .callback = DAQ_TriggerInternalReprateQ,},
    {.pattern = "TRIGger:INTernal:REPrate", .callback = DAQ_TriggerInternalReprate,},
    {.pattern = "OUTPut:ENABled?", .callback = DAQ_OutputEnabledQ,},
    {.pattern = "OUTPut:ENABled", .callback = DAQ_OutputEnabled,},
    {.pattern = "LASer:PRE:POWer?", .callback = DAQ_LaserPrePowerQ,},
    {.pattern = "LASer:PRE:POWer", .callback = DAQ_LaserPrePower,},
    {.pattern = "LASer:MAIN:POWer?", .callback = DAQ_LaserMainPowerQ,},
    {.pattern = "LASer:MAIN:POWer", .callback = DAQ_LaserMainPower,},
    {.pattern = "LASer:PRE:DURation?", .callback = DAQ_LaserPreDurationQ,},
    {.pattern = "LASer:PRE:DURation", .callback = DAQ_LaserPreDuration,},
    {.pattern = "LASer:MAIN:DURation?", .callback = DAQ_LaserMainDurationQ,},
    {.pattern = "LASer:MAIN:DURation", .callback = DAQ_LaserMainDuration,},

    {"SYSTem:COMMunication:TCPIP:CONTROL?", SCPI_SystemCommTcpipControlQ, 0},

    SCPI_CMD_LIST_END
};

scpi_interface_t scpi_interface = {
    /*.error = */ SCPI_Error,
    /*.write = */ SCPI_Write,
    /*.control = */ SCPI_Control,
    /*.flush = */ SCPI_Flush,
    /*.reset = */ SCPI_Reset,
};

char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

scpi_t scpi_context;
