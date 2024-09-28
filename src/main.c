#include "lbe_device.h"
#include "lbe_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(void) {
	printf("Usage: lbe-142x [OPTIONS]\n");
	printf("Options:\n");
	printf("  --f1 <freq> Set frequency for output 1 (1-1400000000 Hz) and save to flash\n");
	printf("  --f2 <freq> Set frequency for output 2 (1-1400000000 Hz) and save to flash (LBE-1421 dual output only)\n");
	printf("  --out <0|1> Enable or disable outputs\n");
	printf("  --blink     Blink output LED(s) for 3 seconds\n");
	printf("  --status    Display current device status\n");
}

int main(int argc, char *argv[]) {
	struct lbe_device *dev;
	struct lbe_status status;
	enum lbe_model model;
	int changed = 0;

	printf("Leo Bodnar LBE-142x GPS locked clock source config\n");

	if (argc == 1) {
		print_usage();
		return 1;
	}

	dev = lbe_open_device();
	if (!dev) {
		fprintf(stderr, "Failed to open LBE-142x device\n");
		return 1;
	}

	model = lbe_get_model(dev);
	printf("Connected to LBE-%s\n", model == LBE_1420 ? "1420" : "1421 dual output");

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--f1") == 0 || strcmp(argv[i], "--f2") == 0) {
			if (i + 1 < argc) {
				int out_no = (argv[i][3] == '1') ? 1 : 2;
				if (out_no == 2 && model == LBE_1420) {
					fprintf(stderr, "LBE-1420 does not support output 2\n");
					continue;
				}
				uint32_t new_freq = atoi(argv[++i]);
				if (new_freq >= 1 && new_freq <= 1400000000) {
					if (lbe_set_frequency(dev, out_no, new_freq) == 0) {
						printf("  Setting OUT%d Frequency: %u Hz\n", out_no, new_freq);
						changed = 1;
					}
				} else {
					fprintf(stderr, "Invalid frequency: %u\n", new_freq);
				}
			}
		} else if (strcmp(argv[i], "--out") == 0) {
			if (i + 1 < argc) {
				int enable = atoi(argv[++i]);
				if (enable == 0 || enable == 1) {
					if (lbe_set_outputs_enable(dev, enable) == 0) {
						printf("  Set output(s) to %d\n", enable);
						changed = 1;
					}
				} else {
					fprintf(stderr, "Invalid output state: %d\n", enable);
				}
			}
		} else if (strcmp(argv[i], "--blink") == 0) {
			if (lbe_blink_leds(dev) == 0) {
				printf("  Blink LED(s)\n");
				changed = 1;
			}
		} else if (strcmp(argv[i], "--status") == 0) {
			if (lbe_get_device_status(dev, &status) == 0) {
				printf("Device Status(0x%02X):\n", status.raw_status);
				printf("  GPS Lock: %s\n", (status.raw_status & LBE_GPS_LOCK_BIT) ? "Yes" : "No");
				printf("  Antenna: %s\n", (status.raw_status & LBE_ANT_OK_BIT) ? "OK" : "Short Circuit");
				printf("  Output(s) Enabled: %s\n", status.outputs_enabled ? "Yes" : "No");
				printf("  Frequency 1: %u Hz\n", status.frequency1);
				if (model == LBE_1421_DUALOUT) {
					printf("  Frequency 2: %u Hz\n", status.frequency2);
					printf("  FLL Enabled: %s\n", status.fll_enabled ? "Yes" : "No");
				}
			}
		} else {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			print_usage();
		}
	}

	if (!changed) {
		printf("No changes made\n");
	}

	lbe_close_device(dev);
	return 0;
}
