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
#include <diode.pb.h> // custom protobuffer

/// EXTERNAL VARIABLES (from DiodeSerial.ino)
extern uint64_t trigcnt;
extern dolphindaq_diode_Settings settings;
extern uint16_t imgbuf_trace[];
extern bool await_update;

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

// DIODE Specific
static scpi_result_t DAQ_DiodeTraceDtQ(scpi_t * context) {
    SCPI_ResultDouble(context, settings.trace_dt);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_DiodeDt(scpi_t * context) {
    double param1;

    /* read first parameter if present */
    if (!SCPI_ParamDouble(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    settings.dt = param1;
    await_update = true;

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_DiodeDtQ(scpi_t * context) {
    SCPI_ResultDouble(context, settings.dt);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_DiodeTraceNtQ(scpi_t * context) {
    SCPI_ResultUInt32(context, settings.trace_nt);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_DiodeTraceYminQ(scpi_t * context) {
    SCPI_ResultDouble(context, settings.trace_ymin);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_DiodeTraceYmaxQ(scpi_t * context) {
    SCPI_ResultDouble(context, settings.trace_ymax);

    return SCPI_RES_OK;
}

static scpi_result_t DAQ_DiodeTraceYarrayQ(scpi_t * context) {
    SCPI_ResultArrayUInt16(context, &imgbuf_trace[0], settings.trace_nt, SCPI_FORMAT_ASCII);

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
    {.pattern = "DIODe:DT", .callback = DAQ_DiodeDt,},
    {.pattern = "DIODe:DT?", .callback = DAQ_DiodeDtQ,},
    {.pattern = "DIODe:TRACe:DT?", .callback = DAQ_DiodeTraceDtQ,},
    {.pattern = "DIODe:TRACe:NT?", .callback = DAQ_DiodeTraceNtQ,},
    {.pattern = "DIODe:TRACe:YMIN?", .callback = DAQ_DiodeTraceYminQ,},
    {.pattern = "DIODe:TRACe:YMAX?", .callback = DAQ_DiodeTraceYmaxQ,},
    {.pattern = "DIODe:TRACe:YARRay?", .callback = DAQ_DiodeTraceYarrayQ,},

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
