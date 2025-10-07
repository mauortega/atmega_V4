#ifndef VW_H
#define VW_H

#include "can.h"

#define CAN_VW_PACKET_ENGINE_SPEED          0xF004 // OK
#define CAN_VW_PACKET_PEDAL_POSITION			  0xF003 // Validar pois doc diz que vai de 0-100%
#define CAN_VW_PACKET_TACHOGRAPH				    0xFE6C // Validar qual VW tem esse pacote
#define CAN_VW_PACKET_GEAR						      0xF005 // Não são todos os veículos que enviam esse pacote.
#define CAN_VW_PACKET_VEHICLE_SPEED				  0xFEBF // Não existe esse pacote no FMS. Somente no J1939 e lá fala em byte 0 e 1 para o valor médio do eixo da frente. 1/256km/h por bit. Mas na prática o valor que aparece é direto.
#define CAN_VW_PACKET_FUEL_ECONOMY				  0xFEF2 // Funciona, mas está transmitindo metros/Litros. Bytes 5 e 6 mostram o consumo em km/L precisa dividir por 512
#define CAN_VW_PACKET_DOORS_CONTROL1			  0xFE4E // OK
#define CAN_VW_PACKET_OIL_PRESSURE				  0xFEEF // FMS e J1939 diz 4kPa/bit estamos usando 40
#define CAN_VW_PACKET_FUEL_PRESSURE				  0xFEEF // FMS e J1939 diz 4kPa/bit estamos usando 40
#define CAN_VW_PACKET_PARKING_BRAKE				  0xFEF1 // OK
//#define CAN_VW_PACKET_BREAK_SWITCH				  0xFEF1 // OK
//#define CAN_VW_PACKET_CLUTCH_SWITCH         0xFEF1 // OK
// FEF1 tbm tem o valor de velocidade, assim como o tacógrafo
#define CAN_VW_PACKET_AMBIENT_TEMPERATURE		0xFEF5 // OK -  VW 9160 Não fala a temperatura da cabine só ambiente - Agrale não mostra temperatura nenhuma, mas ambos mostram a pressão atmosférica em kPa
#define CAN_VW_PACKET_ALTERNATOR				    0xFEF7 // OK - Ambos Agrale e VW somente enviam valor da tensão da bateria. Não tem corrente do alternador.
#define CAN_VW_PACKET_ENGINE_HOURS				  0xFEE5 // OK - Os 4 bytes mais significativos mostram a quantidade de voltas que o motor já deu. 1000 rotações por bit
#define CAN_VW_PACKET_ENGINE_TEMPERATURE		0xFEEE // OK - VAlor lido menos 40 graus byte 0
#define CAN_VW_PACKET_AIR_SUPPLY_PRESSURE		0xFEAE // OK - VW 9160 tem Agrale não tem fator é 8kPa/bit estamos usando 80kPa/bit VW mostra 3 pressões 
#define CAN_VW_PACKET_FUEL_CONSUMPTION			0xFEE9 // OK - Tanto VW 9160 e AGRALE mostram o TOTAL de combustível usado no motor e a quantidade usado nessa viagem
#define CAN_VW_PACKET_HIGHRES_DISTANCE			0xFEC1 // OK - Agrale tem 2 fontes de informação 0x00 e 0xEE enquanto o VW 9160 tem ampenas a fonte 0xEE
//#define CAN_VW_PACKET_DATE_TIME				    0xFEE6 // Ambos os veículos transmitem a informação. Mas o paring está comentado
#define CAN_VW_PACKET_WATER_IN_FUEL				  0xFEFF // OK - Ambos possuem os dados 0 Sem água 1 com água
#define CAN_PACKET_HIGHRES_FUEL_CONSUMPTION 0xFD09
#define CAN_VW_PACKET_FUEL_LEVEL            0xFEFC // VW 9160 sem dados. Agrale transmite o nível de combustível no tanque

#define CAN_VW_PACKET_LIGHTS 				        0xFD7D // Somente no protocolo FMS não tem no J1939 Agrale não em os dados VW9160 tbm não tem os dados
#define CAN_VW_PACKET_AIR_CONDITIONING			0xFD7D

#define CAN_MIX_MCP_NORMAL            			0xFF99 //Mensagem da MIX para alterar a configuração da CAN de MCP_LISTENONLY ou MCP_NORMAL
#define CAN_VW_PACKET_ARLA_LEVEL       			0xFE56 //Aftertreatment 1 Diesel Exhaust Fluid VW e AGRALE possuem Level byte 0 e temperatura byte 1 



void CanVWDataParse(unsigned long canId, unsigned char * data, Stream * serial, struct CANValue &pointer);

unsigned long CanVWTotalPackets();
unsigned long CanVWTotalTime();

bool CanVWPacketReceived();
void CanVWResetPacketReceived();
void CanMIXMCPNormal(unsigned char *data);

#endif
