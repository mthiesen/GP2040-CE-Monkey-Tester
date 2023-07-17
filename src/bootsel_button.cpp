#include "bootsel_button.h"

#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

#include <cstdint>

static bool __no_inline_not_in_flash_func(getBootselButton)()
{
    const uint CS_PIN_INDEX = 1;

    // Must disable interrupts, as interrupt handlers may be in flash, and we
    // are about to temporarily disable flash access!
    uint32_t flags = save_and_disable_interrupts();

    // Set chip select to Hi-Z
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // Note we can't call into any sleep functions in flash right now
    for (volatile int i = 0; i < 1000; ++i)
        ;

    // The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
    // Note the button pulls the pin *low* when pressed.
    bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));

    // Need to restore the state of chip select, else we are going to have a
    // bad time when we return to code in flash!
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);

    return button_state;
}

const uint32_t DEBOUNCE_TICKS = 50;

static bool externalState = false;
static uint32_t debounceCounter = 0;

ButtonEvent tickBootselButton()
{
    const bool internalState = getBootselButton();
    if (externalState != internalState)
    {
        if (++debounceCounter >= DEBOUNCE_TICKS)
        {
            debounceCounter = 0;
            externalState = internalState;
            return externalState ? ButtonEvent::Pressed : ButtonEvent::Released;
        }
    }
    else
    {
        debounceCounter = 0;
    }

    return ButtonEvent::None;
}
