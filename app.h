#ifndef APP_H
#define APP_H

#define NX7000_ATMEGA_VERSION F("20251006")

#define NX7000

// PRODUCTION
//- Todos os itens precisam estar habilitados para produção
#ifdef NX7000
    #define OPTION_SEND_RESUME
    #define OPTION_RASP_WATCH_DOG
    #define OPTION_SOFT_SERIAL
    #define OPTION_VALIDATOR
    #define SERIAL_BAUD 9600
    #define CAN_CRYSTAL MCP_16MHZ
#else
    ////// TESTS
    //#define OPTION_SEND_RESUME
    // USE ONLY ONE
    //#define OPTION_CAN_SIMULATOR
    //#define CAN_CRYSTAL MCP_8MHZ
    //#define OPTION_RASP_WATCH_DOG
    #define OPTION_DISABLE_CAN
    //#define OPTION_CAN_EMULATOR
    //#define VALIDATOR_SIMULATOR
    // Validador muda a serialRpi para 19200
    #define OPTION_VALIDATOR
    #define OPTION_SOFT_SERIAL
    //#define OPTION_TEST_PROTOCOL

    // #ifdef OPTION_CAN_SIMULATOR
    //    #define SERIAL_BAUD 19200
    ///  #else
        #define SERIAL_BAUD 9600
    //  #endif

//#define GPS_ENABLED   // so para testes - em prod ele usa script nodejs

#endif  // ifdef NX7000 // production

#endif
