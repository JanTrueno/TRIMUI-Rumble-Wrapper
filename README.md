# TRIMUI Rumble Wrapper
SDL2 rumble wrapper that redirects haptic feedback to a GPIO pin.
## What it does
Intercepts SDL2 rumble calls and triggers a GPIO pin instead of controller rumble. Should work with RetroArch, and any other SDL2 input application.

## How it works
- Hooks SDL2 functions: `SDL_JoystickRumble`, `SDL_JoystickHasRumble`, haptic API
- Maps rumble strength to simple ON/OFF 
- Uses threading for non-blocking timed rumble

## Build
Using the Makefile:
```bash
make
```

Or manually:
```bash
gcc -shared -fPIC -o libgpio_rumble.so libgpio_rumble.c -lpthread
```

## Usage Examples
```bash
# RetroArch  
LD_PRELOAD=./libgpio_rumble.so retroarch

# Any SDL2 game
LD_PRELOAD=./libgpio_rumble.so your_sdl2_game
```

## Requirements
- Linux with GPIO sysfs support
- GPIO pin has to be accessible
- Root permissions

## Configuration
Change `GPIO_PIN` in source code to use different pin. (227 by default for Trimui)
