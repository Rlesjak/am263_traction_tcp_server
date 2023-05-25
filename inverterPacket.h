/*
 * inverterPacket.h
 *
 *  Created on: 18. svi 2023.
 *      Author: Robi
 */

#include "device.h"

#ifndef INVERTERPACKET_H_
#define INVERTERPACKET_H_

typedef struct __InverterStreamPacket {

    // Trenutak vremena u kojemu je uzet snapshot
    uint32_t IsrTick;

    // Mjerene struje
    float32_t Ia;
    float32_t Ib;
    float32_t Ic;

    // Napon napajanja
    float32_t DcBus;

    // D i Q komponente mjerene struje
    float32_t Id;
    float32_t Iq;

    // Mjeren kut i brzina rotora
    float32_t theta_e;
    float32_t omega_e;

    // Izlazni napon u dq sustavu
    float32_t Out_Vd;
    float32_t Out_Vq;

    // Regulator brzine
    float32_t RegSpeed_Fback;
    float32_t RegSpeed_Output;

    // Regulator struje Id
    float32_t RegId_Fback;
    float32_t RegId_Output;

    // Regulator struje Iq
    float32_t RegIq_Fback;
    float32_t RegIq_Output;

    // Mjerenja s enkodera
    float32_t EncoderTheta;
    float32_t EncoderOmega;

    /* Sporo promijenjivi podatci, ako bude problem velicine paketa
     * odvojiti u poseban paket koji ce se slati rijde.
     */

    // Referentna brzina normalizirana na sinkronu brzinu
    float32_t SpeedRef;

    // Status motora
    unsigned short MotorRunStop;

    // Regulator brzine
    float32_t RegSpeed_RefVal;
    float32_t RegSpeed_Kp;
    float32_t RegSpeed_Ki;

    // Regulator struje Id
    float32_t RegId_RefVal;
    float32_t RegId_Kp;
    float32_t RegId_Ki;

    // Regulator struje Iq
    float32_t RegIq_RefVal;
    float32_t RegIq_Kp;
    float32_t RegIq_Ki;

} InverterStreamPacket_t;

int stringifyInverterPacket(char packetBuffer[], char stringBuffer[]);

#endif /* INVERTERPACKET_H_ */
