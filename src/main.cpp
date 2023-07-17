#include "rng.h"
#include "bootsel_button.h"

#include "pico/stdlib.h"
#include "hardware/timer.h"

#include <cstdint>
#include <limits>

constexpr uint32_t TIMER_INTERVAL_MS = 1;
constexpr uint32_t MIN_PRESS_TIME_MS = 10;
constexpr uint32_t MIN_RELEASE_TIME_MS = 10;
constexpr float PRESS_PROBABILITY = 0.01f;
constexpr float RELEASE_PROBABILITY = 0.005f;

static KissRNG rng;

class Button
{
public:
    void init(uint pin)
    {
        gpio_init(pin);
        gpio_put(pin, false);
        gpio_set_dir(pin, GPIO_IN);
        this->pin = pin;
    }

    void reset()
    {
        gpio_set_dir(pin, GPIO_IN);
        state = State::WaitForPress;
    }

    void tick()
    {
        constexpr uint32_t PRESS_PROBABILITY_UINT32 = static_cast<uint32_t>(PRESS_PROBABILITY * static_cast<float>(std::numeric_limits<uint32_t>::max()));
        constexpr uint32_t RELEASE_PROBABILITY_UINT32 = static_cast<uint32_t>(RELEASE_PROBABILITY * static_cast<float>(std::numeric_limits<uint32_t>::max()));

        switch (state)
        {
            case State::WaitForPress:
            {
                if (rng.next() < PRESS_PROBABILITY_UINT32)
                {
                    gpio_set_dir(pin, GPIO_OUT);
                    waitTicks = MIN_PRESS_TIME_MS;
                    state = State::MinPressTicks;
                }
            }
            break;

            case State::MinPressTicks:
            {
                if (--waitTicks == 0)
                {
                    state = State::WaitForRelease;
                }
            }
            break;

            case State::WaitForRelease:
            {
                if (rng.next() < RELEASE_PROBABILITY_UINT32)
                {
                    gpio_set_dir(pin, GPIO_IN);
                    waitTicks = MIN_RELEASE_TIME_MS;
                    state = State::MinReleaseTicks;
                }
            }
            break;

            case State::MinReleaseTicks:
            {
                if (--waitTicks == 0)
                {
                    state = State::WaitForPress;
                }
            }
            break;
        }
    }

private:
    enum class State
    {
        WaitForPress,
        MinPressTicks,
        WaitForRelease,
        MinReleaseTicks,
    };

    State state = State::WaitForPress;
    uint pin = 0;
    uint32_t waitTicks = 0;
};

static Button buttons[NUM_BANK0_GPIOS];

enum class AppState
{
    Paused,
    Active,
};

static AppState appState = AppState::Paused;

bool timerCallback(repeating_timer*) {
    const ButtonEvent buttonEvent = tickBootselButton();

    switch (appState)
    {
        case AppState::Paused:
        {
            if (buttonEvent == ButtonEvent::Pressed)
            {
                for (uint pin = 0; pin < NUM_BANK0_GPIOS; ++pin)
                {
                    buttons[pin].init(pin);
                }
                appState = AppState::Active;
            }
        }
        break;

        case AppState::Active:
        {
            for (Button& button : buttons)
            {
                button.tick();
            }

            if (buttonEvent == ButtonEvent::Pressed)
            {
                for (Button& button : buttons)
                {
                    button.reset();
                }
                appState = AppState::Paused;
            }
        }
        break;
    }

    return true;
}

int main() {
    repeating_timer_t timer;
    add_repeating_timer_ms(-TIMER_INTERVAL_MS, timerCallback, nullptr, &timer);

    // We don't to anything here, all the logic happens in the timer callback
    for (;;) {}

    return 0;
}
