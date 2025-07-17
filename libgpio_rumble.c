/*
 * Minimal SDL2 GPIO Rumble Wrapper
 * gcc -shared -fPIC -o rumble.so rumble.c -lpthread
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

#define GPIO_PIN 227

static int gpio_ready = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t timer_thread;
static int timer_active = 0;

static int write_file(const char *path, const char *value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    
    ssize_t len = write(fd, value, strlen(value));
    close(fd);
    return (len > 0) ? 0 : -1;
}

static int setup_gpio(void) {
    char path[64];
    char pin_str[8];
    
    snprintf(pin_str, sizeof(pin_str), "%d", GPIO_PIN);
    
    // Export pin
    if (write_file("/sys/class/gpio/export", pin_str) < 0 && errno != EBUSY)
        return -1;
    
    // Set direction
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", GPIO_PIN);
    if (write_file(path, "out") < 0)
        return -1;
    
    // Set initial value
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", GPIO_PIN);
    if (write_file(path, "0") < 0)
        return -1;
    
    return 0;
}

static int set_gpio(int value) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", GPIO_PIN);
    return write_file(path, value ? "1" : "0");
}

static void *timer_func(void *arg) {
    int duration = *(int*)arg;
    
    if (duration > 0 && duration < 10000) {
        usleep(duration * 1000);
        
        pthread_mutex_lock(&mutex);
        if (timer_active) {
            set_gpio(0);
            timer_active = 0;
        }
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

static void rumble(float strength, int duration_ms) {
    pthread_mutex_lock(&mutex);
    
    if (!gpio_ready) {
        if (setup_gpio() == 0) {
            gpio_ready = 1;
        } else {
            pthread_mutex_unlock(&mutex);
            return;
        }
    }
    
    // Stop existing timer
    if (timer_active) {
        timer_active = 0;
        pthread_cancel(timer_thread);
        pthread_join(timer_thread, NULL);
    }
    
    if (strength > 0.1f) {
        set_gpio(1);
        
        if (duration_ms > 0) {
            timer_active = 1;
            static int duration;
            duration = duration_ms;
            pthread_create(&timer_thread, NULL, timer_func, &duration);
        }
    } else {
        set_gpio(0);
    }
    
    pthread_mutex_unlock(&mutex);
}

// SDL2 rumble functions
int SDL_JoystickRumble(void* joystick, unsigned short low, unsigned short high, unsigned int duration) {
    float strength = ((low + high) / 2.0f) / 65535.0f;
    rumble(strength, duration);
    return 0;
}

int SDL_JoystickHasRumble(void* joystick) {
    return 1;
}

int SDL_JoystickRumbleStop(void* joystick) {
    rumble(0, 0);
    return 0;
}

int SDL_JoystickRumbleTriggers(void* joystick, unsigned short left, unsigned short right, unsigned int duration) {
    float strength = ((left + right) / 2.0f) / 65535.0f;
    rumble(strength, duration);
    return 0;
}

int SDL_JoystickHasRumbleTriggers(void* joystick) {
    return 1;
}

// Haptic functions for compatibility
int SDL_JoystickIsHaptic(void* joystick) {
    return 1;
}

void* SDL_HapticOpenFromJoystick(void* joystick) {
    return (void*)0x1;
}

unsigned int SDL_HapticQuery(void* haptic) {
    return 0x8000; // SDL_HAPTIC_LEFTRIGHT
}

void* SDL_HapticOpen(int device_index) {
    return (void*)0x1;
}

int SDL_NumHaptics(void) {
    return 1;
}

const char* SDL_HapticName(int device_index) {
    return "GPIO Rumble";
}

int SDL_HapticNewEffect(void* haptic, void* effect) {
    return 1;
}

int SDL_HapticRunEffect(void* haptic, int effect_id, unsigned int iterations) {
    rumble(0.8, 100);
    return 0;
}

int SDL_HapticStopEffect(void* haptic, int effect_id) {
    rumble(0, 0);
    return 0;
}

int SDL_HapticUpdateEffect(void* haptic, int effect_id, void* effect) {
    return 0;
}

int SDL_HapticDestroyEffect(void* haptic, int effect_id) {
    return 0;
}

int SDL_HapticGetEffectStatus(void* haptic, int effect_id) {
    return 1;
}

int SDL_HapticIndex(void* haptic) {
    return 0;
}

int SDL_HapticNumAxes(void* haptic) {
    return 2;
}

void SDL_HapticClose(void* haptic) {
    rumble(0, 0);
}

// Property system
typedef unsigned int SDL_PropertiesID;

SDL_PropertiesID SDL_GetJoystickProperties(void* joystick) {
    return 0x1;
}

int SDL_GetBooleanProperty(SDL_PropertiesID props, const char* name, int default_value) {
    if (name && strstr(name, "rumble")) {
        return 1;
    }
    return default_value;
}

__attribute__((constructor))
static void init(void) {
    // Nothing - keep startup fast
}

__attribute__((destructor))
static void cleanup(void) {
    rumble(0, 0);
    pthread_mutex_destroy(&mutex);
}
