#ifndef LBE_DEVICE_H
#define LBE_DEVICE_H

#include <stdint.h>

struct lbe_device;

enum lbe_model {
	LBE_1420,
	LBE_1421_DUALOUT // dual output
};

struct lbe_status {
	uint8_t raw_status;
	uint32_t frequency1;
	uint32_t frequency2; // Only used for LBE-1421 dual output
	int outputs_enabled;
	int fll_enabled; // Only used for LBE-1421 dual output
};

// Function prototypes
struct lbe_device* lbe_open_device(void);
void lbe_close_device(struct lbe_device* dev);
enum lbe_model lbe_get_model(struct lbe_device* dev);
int lbe_get_device_status(struct lbe_device* dev, struct lbe_status* status);
int lbe_set_frequency(struct lbe_device* dev, int output, uint32_t frequency);
int lbe_set_outputs_enable(struct lbe_device* dev, int enable);
int lbe_blink_leds(struct lbe_device* dev);

#endif // LBE_DEVICE_H
