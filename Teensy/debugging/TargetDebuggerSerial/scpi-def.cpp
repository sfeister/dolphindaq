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

/// EXTERNAL VARIABLES (from TargetDebuggerSerial.ino)
extern uint32_t analog_write_hz;
extern uint16_t electron_power_debug;
extern uint16_t proton_power_debug;
extern int electron_pin;
extern int proton_pin;
extern uint16_t laser_raw;
extern uint16_t system_mode;

static scpi_result_t OutputFrequencyQ(scpi_t * context) {
    SCPI_ResultUInt32(context, analog_write_hz);

    return SCPI_RES_OK;
}

static scpi_result_t OutputFrequency(scpi_t * context) {
    uint32_t param1;

    /* read first parameter if present */
    if (!SCPI_ParamUInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    analog_write_hz = param1;
    analogWriteFrequency(proton_pin, analog_write_hz);
    analogWriteFrequency(electron_pin, analog_write_hz);

    return SCPI_RES_OK;
}

static scpi_result_t ElectronPowerQ(scpi_t * context) {
    SCPI_ResultUInt32(context, electron_power_debug);

    return SCPI_RES_OK;
}

static scpi_result_t ElectronPower(scpi_t * context) {
    uint32_t param1;

    /* read first parameter if present */
    if (!SCPI_ParamUInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    electron_power_debug = param1;

    return SCPI_RES_OK;
}

static scpi_result_t ProtonPowerQ(scpi_t * context) {
    SCPI_ResultUInt32(context, electron_power_debug);

    return SCPI_RES_OK;
}

static scpi_result_t ProtonPower(scpi_t * context) {
    uint32_t param1;

    /* read first parameter if present */
    if (!SCPI_ParamUInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    proton_power_debug = param1;

    return SCPI_RES_OK;
}

static scpi_result_t PhotodiodeValueQ(scpi_t * context) {
    SCPI_ResultUInt32(context, laser_raw);

    return SCPI_RES_OK;
}

static scpi_result_t SystemModeQ(scpi_t * context) {
    SCPI_ResultUInt32(context, system_mode);

    return SCPI_RES_OK;
}

static scpi_result_t SystemMode(scpi_t * context) {
    uint32_t param1;

    /* read first parameter if present */
    if (!SCPI_ParamUInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    system_mode = param1;

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
    {.pattern = "OUTPut:FREQuency?", .callback = OutputFrequencyQ,},
    {.pattern = "OUTPut:FREQuency", .callback = OutputFrequency,},
    {.pattern = "ELECtron:POWer?", .callback = ElectronPowerQ,},
    {.pattern = "ELECtron:POWer", .callback = ElectronPower,},
    {.pattern = "PROTon:POWer?", .callback = ProtonPowerQ,},
    {.pattern = "PROTon:POWer", .callback = ProtonPower,},
    {.pattern = "PHOTodiode:VALue?", .callback = PhotodiodeValueQ,},
    {.pattern = "SYSTem:MODE?", .callback = SystemModeQ,},
    {.pattern = "SYSTem:MODE", .callback = SystemMode,},

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
