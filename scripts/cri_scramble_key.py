import sys

# Sources
#
# https://github.com/hozuki/libcgss/issues/4#issuecomment-429415659
# https://blog.mottomo.moe/categories/Tech/RE/en/2018-10-12-New-HCA-Encryption/
# https://github.com/vgmstream/vgmstream/blob/776c4d8cf53a73336e1314660ae2a04961b288e3/src/coding/hca_decoder.c#L314
# https://github.com/ActualMandM/ScrambleCriKey/blob/main/ScrambleCriKey/ScrambleCriKey.cpp

def cri_scramble_key(keycode, awbkey):
    keycode_64 = keycode&0xFFFFFFFFFFFFFFFF
    awbkey_16 = awbkey&0xFFFF
    shifted_awbkey_64 = (awbkey_16<<16)&0xFFFFFFFFFFFFFFFF
    inverted_awbkey = (~awbkey_16+2)&0xFFFF
    acbkey = keycode_64 * (shifted_awbkey_64 | inverted_awbkey)
    acbkey_final = acbkey&0xFFFFFFFFFFFFFFFF
    return acbkey_final

argc = len(sys.argv)

if argc <= 2:
    print("Computes a key to be used in HCA/ADX encryption.")
    print("Both encryption and awb keys should be in decimal.")
    print("Usage:\n    ", sys.argv[0], "<encryption key> <awb key>")
    sys.exit()
    
print("Key:", cri_scramble_key(int(sys.argv[1]), int(sys.argv[2])))