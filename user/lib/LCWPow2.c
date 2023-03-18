/*
Copyright 2023 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "LCWPow2.h"

static SQ7_24 pow2table[256+1] = {
    0x01000000, 0x0100B1AF, 0x010163DA, 0x01021681,
    0x0102C9A3, 0x01037D42, 0x0104315E, 0x0104E5F7,
    0x01059B0D, 0x010650A0, 0x010706B2, 0x0107BD42,
    0x01087451, 0x01092BDF, 0x0109E3EC, 0x010A9C79,
    0x010B5586, 0x010C0F14, 0x010CC922, 0x010D83B2,
    0x010E3EC3, 0x010EFA55, 0x010FB66A, 0x01107302,
    0x0111301D, 0x0111EDBA, 0x0112ABDC, 0x01136A81,
    0x011429AA, 0x0114E959, 0x0115A98C, 0x01166A45,
    0x01172B83, 0x0117ED48, 0x0118AF93, 0x01197265,
    0x011A35BE, 0x011AF99F, 0x011BBE08, 0x011C82F9,
    0x011D4873, 0x011E0E75, 0x011ED502, 0x011F9C18,
    0x012063B8, 0x01212BE3, 0x0121F499, 0x0122BDDA,
    0x012387A6, 0x012451FF, 0x01251CE4, 0x0125E857,
    0x0126B456, 0x012780E3, 0x01284DFE, 0x01291BA7,
    0x0129E9DF, 0x012AB8A6, 0x012B87FD, 0x012C57E3,
    0x012D285A, 0x012DF961, 0x012ECAFA, 0x012F9D24,
    0x01306FE0, 0x0131432E, 0x0132170F, 0x0132EB83,
    0x0133C08B, 0x01349626, 0x01356C55, 0x0136431A,
    0x01371A73, 0x0137F262, 0x0138CAE6, 0x0139A401,
    0x013A7DB3, 0x013B57FB, 0x013C32DC, 0x013D0E54,
    0x013DEA64, 0x013EC70D, 0x013FA450, 0x0140822C,
    0x014160A2, 0x01423FB2, 0x01431F5D, 0x0143FFA3,
    0x0144E086, 0x0145C204, 0x0146A41E, 0x014786D6,
    0x01486A2B, 0x01494E1E, 0x014A32AF, 0x014B17DE,
    0x014BFDAD, 0x014CE41B, 0x014DCB29, 0x014EB2D8,
    0x014F9B27, 0x01508417, 0x01516DAA, 0x015257DE,
    0x015342B5, 0x01542E2F, 0x01551A4C, 0x0156070D,
    0x0156F473, 0x0157E27D, 0x0158D12D, 0x0159C082,
    0x015AB07D, 0x015BA11F, 0x015C9268, 0x015D8459,
    0x015E76F1, 0x015F6A32, 0x01605E1B, 0x016152AE,
    0x016247EB, 0x01633DD1, 0x01643463, 0x01652B9F,
    0x01662388, 0x01671C1C, 0x0168155D, 0x01690F4B,
    0x016A09E6, 0x016B052F, 0x016C0127, 0x016CFDCD,
    0x016DFB23, 0x016EF929, 0x016FF7DF, 0x0170F746,
    0x0171F75E, 0x0172F828, 0x0173F9A4, 0x0174FBD3,
    0x0175FEB5, 0x0177024B, 0x01780694, 0x01790B93,
    0x017A1147, 0x017B17B0, 0x017C1ED0, 0x017D26A6,
    0x017E2F33, 0x017F3878, 0x01804275, 0x01814D2A,
    0x01825899, 0x018364C1, 0x018471A4, 0x01857F41,
    0x01868D99, 0x01879CAD, 0x0188AC7D, 0x0189BD0A,
    0x018ACE54, 0x018BE05B, 0x018CF321, 0x018E06A5,
    0x018F1AE9, 0x01902FED, 0x019145B0, 0x01925C35,
    0x0193737B, 0x01948B82, 0x0195A44C, 0x0196BDD9,
    0x0197D829, 0x0198F33E, 0x019A0F17, 0x019B2BB4,
    0x019C4918, 0x019D6741, 0x019E8631, 0x019FA5E8,
    0x01A0C667, 0x01A1E7AE, 0x01A309BE, 0x01A42C98,
    0x01A5503B, 0x01A674A8, 0x01A799E1, 0x01A8BFE5,
    0x01A9E6B5, 0x01AB0E52, 0x01AC36BB, 0x01AD5FF3,
    0x01AE89F9, 0x01AFB4CE, 0x01B0E072, 0x01B20CE6,
    0x01B33A2B, 0x01B46841, 0x01B59728, 0x01B6C6E2,
    0x01B7F76F, 0x01B928CF, 0x01BA5B03, 0x01BB8E0B,
    0x01BCC1E9, 0x01BDF69C, 0x01BF2C25, 0x01C06286,
    0x01C199BD, 0x01C2D1CD, 0x01C40AB5, 0x01C54477,
    0x01C67F12, 0x01C7BA88, 0x01C8F6D9, 0x01CA3405,
    0x01CB720D, 0x01CCB0F2, 0x01CDF0B5, 0x01CF3155,
    0x01D072D4, 0x01D1B532, 0x01D2F870, 0x01D43C8E,
    0x01D5818D, 0x01D6C76E, 0x01D80E31, 0x01D955D7,
    0x01DA9E60, 0x01DBE7CD, 0x01DD321F, 0x01DE7D56,
    0x01DFC973, 0x01E11676, 0x01E26461, 0x01E3B333,
    0x01E502EE, 0x01E65392, 0x01E7A51F, 0x01E8F797,
    0x01EA4AFA, 0x01EB9F48, 0x01ECF482, 0x01EE4AAA,
    0x01EFA1BE, 0x01F0F9C1, 0x01F252B3, 0x01F3AC94,
    0x01F50765, 0x01F66327, 0x01F7BFDA, 0x01F91D80,
    0x01FA7C18, 0x01FBDBA3, 0x01FD3C22, 0x01FE9D96,
    0x02000000
};

SQ7_24 LCWPow2(SQ7_24 val)
{
    // 端数16bitのうち8bit使用した線形補間 Ver.
    const int32_t i = val >> 24;
    const uint32_t frac = (uint32_t)val & 0xFFFFFF;
    const uint32_t j = frac >> 16;
    const uint32_t ratio = (frac >> 8) & 0xFF;

    const int32_t val1 = pow2table[j];
    const int32_t val2 = pow2table[j+1];
    const int32_t delta = ((val2 - val1) * (int32_t)ratio) >> 8;
    const int32_t ret = val1 + delta;

    if ( i < 0 ) {
        return ret >> -i;
    }
    else {
        return ret << i;
    }
}