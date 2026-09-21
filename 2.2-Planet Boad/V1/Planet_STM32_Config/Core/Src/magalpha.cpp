#include "magalpha.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// MA730 3-bit Command Opcodes
static constexpr uint16_t MA_OP_READ_REG  = 0x2000; // 0b010 << 13
static constexpr uint16_t MA_OP_WRITE_REG = 0x8000; // 0b100 << 13

MagAlpha::MagAlpha(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin)
    : _hspi(hspi), _cs_port(cs_port), _cs_pin(cs_pin), _zero_offset_deg(0.0f) {}

void MagAlpha::init() {
    deselect();
}

uint16_t MagAlpha::transfer16(uint16_t tx_data) {
    uint8_t tx_buf[2] = {
        static_cast<uint8_t>((tx_data >> 8) & 0xFF),
        static_cast<uint8_t>(tx_data & 0xFF)
    };
    uint8_t rx_buf[2] = {0, 0};

    select();
    HAL_SPI_TransmitReceive(_hspi, tx_buf, rx_buf, 2, 5); // 5ms timeout
    deselect();

    return (static_cast<uint16_t>(rx_buf[0]) << 8) | static_cast<uint16_t>(rx_buf[1]);
}

uint16_t MagAlpha::readRaw16() {
    // Sending 0x0000 clocks out the 16-bit angle frame (MSB aligned)
    return transfer16(0x0000);
}

uint16_t MagAlpha::readRaw14() {
    // MA730 outputs 14-bit angle data in bits [15:2]
    return (readRaw16() >> 2) & 0x3FFF;
}

float MagAlpha::readAngleDegrees() {
    // 14-bit resolution: 16384 counts = 360 degrees
    float raw_deg = (static_cast<float>(readRaw14()) * 360.0f) / 16384.0f;

    // Apply calibration offset and wrap to [0, 360)
    float angle = raw_deg - _zero_offset_deg;
    while (angle < 0.0f)   angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;

    return angle;
}

float MagAlpha::readAngleRadians() {
    return readAngleDegrees() * (static_cast<float>(M_PI) / 180.0f);
}

void MagAlpha::setZeroOffset(float offset_deg) {
    _zero_offset_deg = offset_deg;
}

uint8_t MagAlpha::readRegister(uint8_t address) {
    // Frame 1: Send Read Command + 5-bit Address
    uint16_t command = MA_OP_READ_REG | (static_cast<uint16_t>(address & 0x1F) << 8);
    transfer16(command);

    // Frame 2: Clock out response (Data returned in bits [15:8])
    uint16_t response = transfer16(0x0000);
    return static_cast<uint8_t>((response >> 8) & 0xFF);
}

void MagAlpha::writeRegister(uint8_t address, uint8_t value) {
    // Single 16-bit frame: Opcode (0b100) + 5-bit Address + 8-bit Data
    uint16_t command = MA_OP_WRITE_REG |
                       (static_cast<uint16_t>(address & 0x1F) << 8) |
                       static_cast<uint16_t>(value & 0xFF);
    transfer16(command);

    // Safety delay for non-volatile register update/settling
    HAL_Delay(20);
}
