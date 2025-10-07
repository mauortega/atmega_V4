#ifndef FMS_H
#define FMS_H

#define FMS_ENABLED

#define CAN_PACKET_FUEL_CONSUMPTION 		    0x00FEE9
#define CAN_PACKET_FUEL_LEVEL 				      0x00FEFC
//#define CAN_PACKET_TACHOGRAPH 				      0x00FE6C
//#define CAN_PACKET_HIGHRES_DISTANCE 		    0x00FEC1
#define CAN_PACKET_ENGINE_TEMPERATURE 		  0x00FEEE
#define CAN_PACKET_AIR_SUPPLY_PRESSURE 		  0x00FEAE
#define CAN_PACKET_FUEL_ECONOMY 			      0x00FEF2
#define CAN_PACKET_HIGHRES_FUEL_CONSUMPTION 0x00FD09
//#define CAN_PACKET_VEHICLE_SPEED 			      0x00FEF1
#define CAN_PACKET_DATE_TIME 				        0x00FEE6
#define CAN_PACKET_ALTERNATOR 				      0x00FED5
#define CAN_PACKET_GEAR 				            0x00F005
#define CAN_PACKET_AIR_SUSPENSION 			    0x00FE58
#define CAN_PACKET_VEHICLE_WEIGHT 			    0x00FEEA

//ToDo: Check for documentation
//#define CAN_PACKET_LIGHTS 				          0x00FD7D
//#define CAN_PACKET_AIR_CONDITIONING			    0x00FD7D

#define FMS_AIR_HUMIDITY                    0x00FDE0 // Byte 3 e 4 - Não temos a documentação 
#define CAN_PACKET_WINDSHIELD_WIPER			    0x00FDCD

#define CAN_PACKET_BREAK				            0x00F001
#define FMS_PARTICULATE_MATERIAL			      0x00FD7B

#define CAN_PACKET_IGNITION                 0x00FF01 // Byte 01 Bit 01  
#define FMS_TOXIC_GASES                     0x00FF01 // Byte 02 e 03 - 0 - 65535 ppm CO 
#define FMS_NOISE_LEVEL                     0x00FF01 // Byte 04 - db 

#define CAN_PACKET_TIRE_PRESSURE            0x00FEF4

//Embreagem 

void CanFMSDataParse(unsigned long canId, unsigned char * data, Stream * serial, struct CANValue &pointer);
//void FmsCreatePacket(struct canfd_frame *cf, unsigned long packetType, unsigned long value);
//void FmsCreatePacketString(char * buffer, unsigned long packetType, unsigned long value);
//void FmsGet(FMSValue * fmsValuePtr);
//unsigned char * CanGetDataPointer();
bool CanFMSPacketReceived();
void CanFMSResetPacketReceived();

#endif
