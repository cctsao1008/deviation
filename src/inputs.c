/*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Deviation is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Deviation.  If not, see <http://www.gnu.org/licenses/>.

 Most of this code is based on the mixer from er9x developed by
 Erez Raviv <erezraviv@gmail.com>
 http://code.google.com/p/er9x/
 and the th9x project
 http://code.google.com/p/th9x/
 */

#include "common.h"
#include "mixer.h"
#include "config/model.h"

#define INPNAME_AILERON(x)  ((!x) ? _tr("AIL")     : _tr("AIL"))
#define INPNAME_ELEVATOR(x) ((!x) ? _tr("ELE")     : _tr("ELE"))
#define INPNAME_THROTTLE(x) ((!x) ? _tr("THR")     : _tr("THR"))
#define INPNAME_RUDDER(x)   ((!x) ? _tr("RUD")     : _tr("RUD"))
#define INPNAME_AUX2(x)     ((!x) ? _tr("AUX2")    : _tr("AUX2"))
#define INPNAME_AUX3(x)     ((!x) ? _tr("AUX3")    : _tr("AUX3"))
#define INPNAME_AUX4(x)     ((!x) ? _tr("AUX4")    : _tr("AUX4"))
#define INPNAME_AUX5(x)     ((!x) ? _tr("AUX5")    : _tr("AUX5"))
#define INPNAME_AUX6(x)     ((!x) ? _tr("AUX6")    : _tr("AUX6"))
#define INPNAME_AUX7(x)     ((!x) ? _tr("AUX7")    : _tr("AUX7"))
#define INPNAME_DR0(x)      ((!x) ? _tr("DR0")     : _tr("DR"))
#define INPNAME_DR1(x)      ((!x) ? _tr("DR1")     : _tr("DR"))
#define INPNAME_RUD_DR0(x)  ((!x) ? _tr("RUD DR0") : _tr("RUD DR"))
#define INPNAME_RUD_DR1(x)  ((!x) ? _tr("RUD DR1") : _tr("RUD DR"))
#define INPNAME_RUD_DR2(x)  ((!x) ? _tr("RUD DR2") : _tr("RUD DR"))
#define INPNAME_ELE_DR0(x)  ((!x) ? _tr("ELE DR0") : _tr("ELE DR"))
#define INPNAME_ELE_DR1(x)  ((!x) ? _tr("ELE DR1") : _tr("ELE DR"))
#define INPNAME_ELE_DR2(x)  ((!x) ? _tr("ELE DR2") : _tr("ELE DR"))
#define INPNAME_AIL_DR0(x)  ((!x) ? _tr("AIL DR0") : _tr("AIL DR"))
#define INPNAME_AIL_DR1(x)  ((!x) ? _tr("AIL DR1") : _tr("AIL DR"))
#define INPNAME_AIL_DR2(x)  ((!x) ? _tr("AIL DR2") : _tr("AIL DR"))
#define INPNAME_GEAR0(x)    ((!x) ? _tr("GEAR0")   : _tr("GEAR"))
#define INPNAME_GEAR1(x)    ((!x) ? _tr("GEAR1")   : _tr("GEAR"))
#define INPNAME_MIX0(x)     ((!x) ? _tr("MIX0")    : _tr("MIX"))
#define INPNAME_MIX1(x)     ((!x) ? _tr("MIX1")    : _tr("MIX"))
#define INPNAME_MIX2(x)     ((!x) ? _tr("MIX2")    : _tr("MIX"))
#define INPNAME_FMOD0(x)    ((!x) ? _tr("FMODE0")  : _tr("FMODE"))
#define INPNAME_FMOD1(x)    ((!x) ? _tr("FMODE1")  : _tr("FMODE"))
#define INPNAME_FMOD2(x)    ((!x) ? _tr("FMODE2")  : _tr("FMODE"))
#define INPNAME_HOLD0(x)    ((!x) ? _tr("HOLD0")   : _tr("HOLD"))
#define INPNAME_HOLD1(x)    ((!x) ? _tr("HOLD1")   : _tr("HOLD"))
#define INPNAME_TRN0(x)     ((!x) ? _tr("TRN0")    : _tr("TRN"))
#define INPNAME_TRN1(x)     ((!x) ? _tr("TRN1")    : _tr("TRN"))

#define SWITCH_NAME_GEAR0 _tr("GEAR")
#define SWITCH_NAME_GEAR1 _tr("GEAR")
#define SWITCH_NAME_GEAR0 _tr("GEAR")
#define BUTNAME_TRIM_LV_NEG _tr("TRIMLV-")
#define BUTNAME_TRIM_LV_POS _tr("TRIMLV+")
#define BUTNAME_TRIM_RV_NEG _tr("TRIMRV-")
#define BUTNAME_TRIM_RV_POS _tr("TRIMRV+")
#define BUTNAME_TRIM_LH_NEG _tr("TRIMLH-")
#define BUTNAME_TRIM_LH_POS _tr("TRIMLH+")
#define BUTNAME_TRIM_RH_NEG _tr("TRIMRH-")
#define BUTNAME_TRIM_RH_POS _tr("TRIMRH+")
#define BUTNAME_TRIM_L_NEG  _tr("TRIM_L-")
#define BUTNAME_TRIM_L_POS  _tr("TRIM_L+")
#define BUTNAME_TRIM_R_NEG  _tr("TRIM_R-")
#define BUTNAME_TRIM_R_POS  _tr("TRIM_R+")
#define BUTNAME_LEFT        _tr("Left")
#define BUTNAME_RIGHT       _tr("Right")
#define BUTNAME_DOWN        _tr("Down")
#define BUTNAME_UP          _tr("Up")
#define BUTNAME_ENTER       _tr("Enter")
#define BUTNAME_EXIT        _tr("Exit")

const char *tx_stick_names[4] = {
    _tr_noop("RIGHT_H"),
    _tr_noop("LEFT_V"),
    _tr_noop("RIGHT_V"),
    _tr_noop("LEFT_H"),
};

static const char *_get_source_name(char *str, u8 src, int switchname)
{
    u8 is_neg = MIXER_SRC_IS_INV(src);
    src = MIXER_SRC(src);

    if(! src) {
        strcpy(str, _tr("None"));
    } else if(src <= NUM_TX_INPUTS) {
        const char *ptr = "";
        #define CHANDEF(x) case INP_##x : ptr = INPNAME_##x(switchname); break;
        switch(src) {
            #include "capabilities.h"
        };
        #undef CHANDEF
        sprintf(str, "%s%s", is_neg ? "!" : "", ptr);
    } else if(src <= NUM_INPUTS + NUM_OUT_CHANNELS) {
        sprintf(str, "%s%s%d", is_neg ? "!" : "", _tr("Ch"), src - NUM_INPUTS);
    } else {
        sprintf(str, "%s%s%d", is_neg ? "!" : "", _tr("Virt"), src - NUM_INPUTS - NUM_OUT_CHANNELS);
    }
    return str;
}
const char *INPUT_SourceName(char *str, u8 src)
{
    return _get_source_name(str, src, 0);
}
const char *INPUT_SourceNameAbbrevSwitch(char *str, u8 src)
{
    _get_source_name(str, src, 1);
    printf("%d: %s\n", src, str);
    return str;
}

const char *INPUT_MapSourceName(u8 idx, u8 *val)
{
    int i = 0;
    #define CHANMAP(oldname, new) if(idx == i++) { *val = INP_##new; return oldname;}
    #include "capabilities.h"
    #undef CHANMAP
    return NULL;
}


const char *INPUT_ButtonName(u8 button)
{
    if (! button) {
        return _tr("None");
    }
    #define BUTTONDEF(x) case BUT_##x : return BUTNAME_##x;
    switch(button) {
        #include "capabilities.h"
    };
    #undef BUTTONDEF
    return "";
}
