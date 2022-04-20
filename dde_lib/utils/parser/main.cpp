#include "DDE_TYPES.h"
#include "db_sqlib.h"

#include "csv_parser.h"

const char* string_stream = "DCDC010203040506,27.03.2022;\
0100,M1_ADC;\
0101,W1_mode,2,1,Wt;\
0102,W3_Udc1_gain,3,1,V;\
0103,W2_Udc1_offset,3,1,V;\
0104,W5_Udc2_gain,3,1,V;\
0105,W4_Udc2_offset,3,1,V;\
0106,W7_UdcBB_gain,3,1,V;\
0107,W6_UdcBB_offset,3,1,V;\
0108,W9_UAB_gain,3,1,V;\
0109,W8_UAB_offset,3,1,V;\
010a,W11_UBC_gain,3,1,V;\
010b,W10_UBC_offset,3,1,V;\
010c,W13_UCA_gain,3,1,V;\
010d,W12_UCA_offset,3,1,V;\
010e,W15_UCN_gain,3,1,V;\
010f,W14_UCN_offset,3,1,V;\
0110,W17_IA_gain,3,1,A;\
0111,W16_IA_offset,3,1,A;\
0112,W19_IB_gain,3,1,A;\
0113,W18_IB_offset,3,1,A;\
0114,W21_IC_offset,3,1,A;\
0115,W20_IC_gain,3,1,A;\
011c,R1_Udc1_adc_reg16,3,2,V,1.3;\
011e,R3_UdcBB_adc_reg16,3,2,V,1.3;\
011f,R4_IdcBB_adc_reg16,3,2,V,1.3;\
0120,R5_Idc2_adc_reg16,3,2,V,1.3;\
0121,R6_IA_adc_reg16,3,2,V,1.3;\
0122,R7_IB_adc_reg16,3,2,V,1.3;\
0123,R8_IC_adc_reg16,3,2,V,1.3;\
0124,R9_UAB_adc_reg16,3,2,V,1.3;\
0125,R10_UBC_adc_reg16,3,2,V,1.3;\
0126,R11_UCN_adc_reg16,3,2,V,1.3;\
0127,R60_counter,2,1,,1.3;\
0200,M2_CMD_LOGIC;\
0228,W1_cmd,1,2,,0;\
0229,W2_param2,1,2,,0;\
022a,W3_param3,1,2,,0;\
0300,M3_MEAS;\
032b,W1_Tfilter,1,1,,0;\
032c,W2_param2,1,1,,0;\
032d,W3_param3,1,1,,0;\
0400,M4_PROTECT;\
042e,W1_cmd,1,2,,0;\
042f,W2_Imax1,1,2,,0;\
0430,W3_Imax2,1,2,,0;\
0431,W4_Umax1,1,2,,0;\
0432,W5_Umax2,1,2,,0;\
0433,W6_UmaxBB,1,2,,0;";

int main()
{
    UAVCANcsvParser* obj = new UAVCANcsvParser();
    obj->parse(string_stream);

    ParamDescr par_desc;
    par_desc.init(obj->tbl_name, "NONE", db_type::usual);
    for (int i = 0; i < obj->result.size(); i++)
        par_desc.set(&obj->result[i], db_type::usual);
    par_desc.close();
}