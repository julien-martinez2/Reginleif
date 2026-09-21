#ifndef DRV8908_HPP
#define DRV8908_HPP

#include "main.h"

class DRV8908 {
public:
    enum class MotorID { MOTOR_1 = 0, MOTOR_2, MOTOR_3, MOTOR_4 };
    enum class State { COAST, FORWARD, REVERSE, BRAKE };

    DRV8908(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin,
            GPIO_TypeDef* nfault_port, uint16_t nfault_pin);

    bool init();
    bool runPOLD();
    void setMotorState(MotorID motor, State state);
    void setMotorSpeed(MotorID motor, uint8_t duty_255);
    void updateOutputs();
    void clearFaults();
    bool isFaultActive();

private:
    SPI_HandleTypeDef* _hspi;
    GPIO_TypeDef* _cs_port;
    uint16_t _cs_pin;
    GPIO_TypeDef* _nfault_port;
    uint16_t _nfault_pin;

    uint8_t _op_ctrl_1;
    uint8_t _op_ctrl_2;

    uint16_t transfer16(uint16_t tx_frame);
    void writeRegister(uint8_t addr, uint8_t data);
    uint8_t readRegister(uint8_t addr);
};

#endif // DRV8908_HPP