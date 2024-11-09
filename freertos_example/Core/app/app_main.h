#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	int app_main();
#ifdef __cplusplus
}
#endif

#define DEBOUNCE_DELAY pdMS_TO_TICKS(20)
#define BUTTON_NOT_PRESSED 0
#define BUTTON_PRESSED 1
#define RED_LED GPIO_PIN_14
#define BLUE_LED GPIO_PIN_7

typedef enum {
	CURRENTLY_DEBOUNCING = 0,
	NOT_DEBOUNCING
} hey;