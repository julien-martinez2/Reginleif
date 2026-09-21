#ifndef MAGALPHA_HPP
#define MAGALPHA_HPP

#include "main.h"
#include <cstdint>

class MagAlpha {
public:
    /**
     * @brief MagAlpha MA730 Sensor Driver (14-bit SPI Angle Sensor)
     * @param hspi Pointer to STM32 SPI hardware handle (e.g., &hspi1)
     * @param cs_port GPIO port for Chip Select (e.g., GPIOC)
     * @param cs_pin GPIO pin mask for Chip Select (e.g., GPIO_PIN_6)
     */
    MagAlpha(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin);

    // Initialization and bus verification
    void init();

    // Fast Angle Acquisitions
    uint16_t readRaw14();        // Returns 0 to 16383 (14-bit raw angle)
    uint16_t readRaw16();        // Returns 0 to 65535 (16-bit raw register)
    float readAngleDegrees();    // Returns 0.0f to 360.0f degrees
    float readAngleRadians();    // Returns 0.0f to 2*PI radians

    // Register Access (MA730 Internal Registers 0 to 31)
    uint8_t readRegister(uint8_t address);
    void writeRegister(uint8_t address, uint8_t value);

    // Utility
    void setZeroOffset(float offset_deg);

private:
    SPI_HandleTypeDef* _hspi;
    GPIO_TypeDef* _cs_port;
    uint16_t _cs_pin;
    float _zero_offset_deg;

    // Hardware SPI helper (handles MSB-first byte ordering)
    uint16_t transfer16(uint16_t tx_data);

    inline void select() {
        HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
    }

    inline void deselect() {
        HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
    }
};

#endif // MAGALPHA_HPP
