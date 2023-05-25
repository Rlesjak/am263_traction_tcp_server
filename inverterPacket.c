#include "inverterPacket.h"

#define STRINGIFY_FIELD(TYPEBUFF, DATABUFFPTR, STRINGLEN, CHUNKLENBUFF, STRINGBUFFPTR, TEMPLATE) \
    memcpy(&TYPEBUFF, DATABUFFPTR, sizeof(TYPEBUFF)); \
    DATABUFFPTR = DATABUFFPTR + sizeof(TYPEBUFF); \
    CHUNKLENBUFF = sprintf(STRINGBUFFPTR, TEMPLATE, TYPEBUFF); \
    STRINGBUFFPTR = STRINGBUFFPTR + CHUNKLENBUFF; \
    STRINGLEN = STRINGLEN + CHUNKLENBUFF;


int stringifyInverterPacket(char packetBuffer[], char stringBuffer[])
{
    int stringLen = 0;
    int stringChunkLen = 0;
    char* packetPosPtr = packetBuffer;
    uint32_t uintBuff;
    float32_t floatBuff;
    float32_t floatBuf2;
    unsigned short ushortBuff;

    STRINGIFY_FIELD(uintBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "data: %u,"); // IsrTick

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // Ia
    STRINGIFY_FIELD(floatBuf2, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // Ib
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // Ic

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // DcBus

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // Id
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // Iq

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // theta_e
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // omega_e

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // Out_Vd
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // Out_Vq

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegSpeed_Fback
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegSpeed_Output

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegId_Fback
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegId_Output

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegIq_Fback
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegIq_Output

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // EncoderTheta
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // EncoderOmega

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // SpeedRef

    STRINGIFY_FIELD(ushortBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%u,"); // MotorRunStop

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegSpeed_RefVal
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegSpeed_Kp
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegSpeed_Ki

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegId_RefVal
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegId_Kp
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegId_Ki

    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegIq_RefVal
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f,"); // RegIq_Kp
    STRINGIFY_FIELD(floatBuff, packetPosPtr, stringLen, stringChunkLen, stringBuffer, "%0.6f\n\n"); // RegIq_Ki

    // stringLen = stringLen + sprintf(stringBuffer, "\n\n");
    return stringLen;
}

