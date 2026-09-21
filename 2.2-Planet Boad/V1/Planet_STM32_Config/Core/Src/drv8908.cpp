#include "drv8908.hpp"

// DRV8908 Registers[cite: 1]
#define REG_IC_STAT         0x00
#define REG_OLD_STAT_1      0x04
#define REG_OLD_STAT_2      0x05
#define REG_CONFIG_CTRL     0x07
#define REG_OP_CTRL_1       0x08
#define REG_OP_CTRL_2       0x09
#define REG_PWM_CTRL_1      0x0B
#define REG_PWM_CTRL_2      0x0C
#define REG_FW_CTRL_1       0x0D
#define REG_PWM_MAP_1       0x0F
#define REG_PWM_MAP_2       0x10
#define REG_PWM_MAP_3       0x11
#define REG_PWM_MAP_4       0x12
#define REG_PWM_FREQ_1      0x13
#define REG_PWM_DUTY_1      0x15
#define REG_SR_CTRL_1       0x1D
#define REG_OLD_CTRL_6      0x24

DRV8908::DRV8908(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin,
                 GPIO_TypeDef* nfault_port, uint16_t nfault_pin)
    : _hspi(hspi), _cs_port(cs_port), _cs_pin(cs_pin),
      _nfault_port(nfault_port), _nfault_pin(nfault_pin),
      _op_ctrl_1(0), _op_ctrl_2(0) {}

uint16_t DRV8908::transfer16(uint16_t tx_frame) {
    uint16_t rx_frame = 0;
    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(_hspi, (uint8_t*)&tx_frame, (uint8_t*)&rx_frame, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
    return rx_frame;
}

void DRV8908::writeRegister(uint8_t addr, uint8_t data) {
    uint16_t frame = ((uint16_t)(addr & 0x3F) << 8) | data; // W=0 (Bit 14)[cite: 1]
    transfer16(frame);
}

uint8_t DRV8908::readRegister(uint8_t addr) {
    uint16_t frame = (1 << 14) | ((uint16_t)(addr & 0x3F) << 8); // R=1 (Bit 14)[cite: 1]
    return (uint8_t)(transfer16(frame) & 0xFF);
}

bool DRV8908::init() {
    HAL_Delay(2); // Wait t_WAKE

    clearFaults();

    // 1. PWM & Slew Rate setup
    writeRegister(REG_PWM_CTRL_1, 0xFF);   // All half-bridges in PWM mode[cite: 1]
    writeRegister(REG_PWM_CTRL_2, 0xF0);   // Enable PWM Generators 1-4[cite: 1]
    writeRegister(REG_FW_CTRL_1, 0x00);    // Passive / Diode Freewheeling[cite: 1]
    writeRegister(REG_SR_CTRL_1, 0x00);    // 0.6 V/us slow slew rate[cite: 1]

    // 2. Map channels[cite: 1]
    writeRegister(REG_PWM_MAP_1, 0x03);    // M1(HB2->CH1), M4(HB1->CH4)[cite: 1]
    writeRegister(REG_PWM_MAP_2, 0x09);    // M3(HB4->CH3), M2(HB3->CH2)[cite: 1]
    writeRegister(REG_PWM_MAP_3, 0x0B);    // M3(HB6->CH3), M4(HB5->CH4)[cite: 1]
    writeRegister(REG_PWM_MAP_4, 0x01);    // M1(HB8->CH1), M2(HB7->CH2)[cite: 1]

    // 3. Frequency setup (2 kHz on CH1-CH4)[cite: 1]
    writeRegister(REG_PWM_FREQ_1, 0xFF);   // 2 kHz on CH1-4[cite: 1]

    // 4. Set duty cycles to 0[cite: 1]
    for (uint8_t i = 0; i < 4; i++) {
        writeRegister(REG_PWM_DUTY_1 + i, 0x00);
    }

    updateOutputs();
    return true;
}

bool DRV8908::runPOLD() {
    writeRegister(REG_OLD_CTRL_6, 0x00);   // Full-bridge floating mode[cite: 1]

    // Forward current routing[cite: 1]
    writeRegister(REG_OP_CTRL_1, 0xAA);    // HB1-4 High-Side[cite: 1]
    writeRegister(REG_OP_CTRL_2, 0x55);    // HB5-8 Low-Side[cite: 1]

    writeRegister(REG_CONFIG_CTRL, 0xA0);  // Enable POLD[cite: 1]
    HAL_Delay(3);                          // Wait t_POLD[cite: 1]
    writeRegister(REG_CONFIG_CTRL, 0x20);  // Disable POLD[cite: 1]

    uint8_t stat1 = readRegister(REG_OLD_STAT_1); // 0x04[cite: 1]
    uint8_t stat2 = readRegister(REG_OLD_STAT_2); // 0x05[cite: 1]

    // Reset outputs to High-Z[cite: 1]
    writeRegister(REG_OP_CTRL_1, 0x00);
    writeRegister(REG_OP_CTRL_2, 0x00);
    clearFaults();

    // Evaluate open-load conditions[cite: 1]
    bool m1_open = ((stat1 & (1 << 3)) != 0) || ((stat2 & (1 << 6)) != 0); // HB2/HB8[cite: 1]
    bool m2_open = ((stat1 & (1 << 5)) != 0) || ((stat2 & (1 << 4)) != 0); // HB3/HB7[cite: 1]
    bool m3_open = ((stat1 & (1 << 7)) != 0) || ((stat2 & (1 << 2)) != 0); // HB4/HB6[cite: 1]
    bool m4_open = ((stat1 & (1 << 1)) != 0) || ((stat2 & (1 << 0)) != 0); // HB1/HB5[cite: 1]

    return !(m1_open || m2_open || m3_open || m4_open);
}

void DRV8908::setMotorState(MotorID motor, State state) {
    uint8_t mask1 = 0, mask2 = 0;
    switch (motor) {
        case MotorID::MOTOR_1: // HB2 (OP1) & HB8 (OP2)[cite: 1]
            _op_ctrl_1 &= ~0x0C; _op_ctrl_2 &= ~0xC0;
            if (state == State::FORWARD) { mask1 = 0x08; mask2 = 0x40; } //[cite: 1]
            else if (state == State::REVERSE) { mask1 = 0x04; mask2 = 0x80; } //[cite: 1]
            else if (state == State::BRAKE)   { mask1 = 0x04; mask2 = 0x40; } //[cite: 1]
            break;
        case MotorID::MOTOR_2: // HB3 (OP1) & HB7 (OP2)[cite: 1]
            _op_ctrl_1 &= ~0x30; _op_ctrl_2 &= ~0x30;
            if (state == State::FORWARD) { mask1 = 0x20; mask2 = 0x10; } //[cite: 1]
            else if (state == State::REVERSE) { mask1 = 0x10; mask2 = 0x20; } //[cite: 1]
            else if (state == State::BRAKE)   { mask1 = 0x10; mask2 = 0x10; } //[cite: 1]
            break;
        case MotorID::MOTOR_3: // HB4 (OP1) & HB6 (OP2)[cite: 1]
            _op_ctrl_1 &= ~0xC0; _op_ctrl_2 &= ~0x0C;
            if (state == State::FORWARD) { mask1 = 0x80; mask2 = 0x04; } //[cite: 1]
            else if (state == State::REVERSE) { mask1 = 0x40; mask2 = 0x08; } //[cite: 1]
            else if (state == State::BRAKE)   { mask1 = 0x40; mask2 = 0x04; } //[cite: 1]
            break;
        case MotorID::MOTOR_4: // HB1 (OP1) & HB5 (OP2)[cite: 1]
            _op_ctrl_1 &= ~0x03; _op_ctrl_2 &= ~0x03;
            if (state == State::FORWARD) { mask1 = 0x02; mask2 = 0x01; } //[cite: 1]
            else if (state == State::REVERSE) { mask1 = 0x01; mask2 = 0x02; } //[cite: 1]
            else if (state == State::BRAKE)   { mask1 = 0x01; mask2 = 0x01; } //[cite: 1]
            break;
    }
    _op_ctrl_1 |= mask1;
    _op_ctrl_2 |= mask2;
}

void DRV8908::setMotorSpeed(MotorID motor, uint8_t duty_255) {
    writeRegister(REG_PWM_DUTY_1 + static_cast<uint8_t>(motor), duty_255); //[cite: 1]
}

void DRV8908::updateOutputs() {
    writeRegister(REG_OP_CTRL_1, _op_ctrl_1);
    writeRegister(REG_OP_CTRL_2, _op_ctrl_2);
}

void DRV8908::clearFaults() {
    writeRegister(REG_CONFIG_CTRL, 0x21); // Set CLR_FLT bit[cite: 1]
}

bool DRV8908::isFaultActive() {
    return (HAL_GPIO_ReadPin(_nfault_port, _nfault_pin) == GPIO_PIN_RESET);
}
