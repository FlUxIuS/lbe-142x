#ifdef _WIN32

#include "lbe_device.h"
#include "lbe_common.h"
#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define REPORT_SIZE 64

// Fallback definitions for constants that might be missing
#ifndef LIBUSB_REQUEST_GET_REPORT
#define LIBUSB_REQUEST_GET_REPORT 0x01
#endif

#ifndef LIBUSB_REQUEST_SET_REPORT
#define LIBUSB_REQUEST_SET_REPORT 0x09
#endif

#ifndef LIBUSB_REPORT_TYPE_FEATURE
#define LIBUSB_REPORT_TYPE_FEATURE 0x03
#endif

struct lbe_device {
	libusb_device_handle *handle;
	uint16_t product_id;
	enum lbe_model model;
};

struct lbe_device* lbe_open_device(void) {
	struct lbe_device* dev = malloc(sizeof(struct lbe_device));
	if (!dev) return NULL;

	libusb_device **devs;
	ssize_t cnt;
	int ret;

	ret = libusb_init(NULL);
	if (ret < 0) {
		fprintf(stderr, "Failed to initialize libusb: %s\n", libusb_error_name(ret));
		free(dev);
		return NULL;
	}

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0) {
		fprintf(stderr, "Failed to get device list: %s\n", libusb_error_name((int)cnt));
		free(dev);
		return NULL;
	}

	for (ssize_t i = 0; i < cnt; i++) {
		struct libusb_device_descriptor desc;
		libusb_device *device = devs[i];

		if (libusb_get_device_descriptor(device, &desc) < 0)
			continue;

		if (desc.idVendor == VID_LBE && (desc.idProduct == PID_LBE_1420 || desc.idProduct == PID_LBE_1421)) {
			ret = libusb_open(device, &dev->handle);
			if (ret < 0) {
				fprintf(stderr, "Failed to open device: %s\n", libusb_error_name(ret));
				continue;
			}
			dev->product_id = desc.idProduct;
			dev->model = (desc.idProduct == PID_LBE_1420) ? LBE_1420 : LBE_1421_DUALOUT;
			libusb_free_device_list(devs, 1);
			return dev;
		}
	}

	fprintf(stderr, "LBE-142x device not found\n");
	libusb_free_device_list(devs, 1);
	free(dev);
	return NULL;
}

void lbe_close_device(struct lbe_device* dev) {
	if (dev) {
		libusb_close(dev->handle);
		libusb_exit(NULL);
		free(dev);
	}
}

enum lbe_model lbe_get_model(struct lbe_device* dev) {
	return dev->model;
}

int lbe_get_device_status(struct lbe_device* dev, struct lbe_status* status) {
	uint8_t buf[REPORT_SIZE] = {0};
	int ret;

	buf[0] = 0x4B; // Report Number
	ret = libusb_control_transfer(dev->handle,
				LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
				LIBUSB_REQUEST_GET_REPORT,
				(LIBUSB_REPORT_TYPE_FEATURE << 8) | buf[0],
				0,
				buf,
				REPORT_SIZE,
				5000);

	if (ret < 0) {
		fprintf(stderr, "Failed to get feature report: %s\n", libusb_error_name(ret));
		return -1;
	}

	status->raw_status = buf[2];
	if (dev->model == LBE_1420) {
		status->frequency1 = buf[6] | (buf[7] << 8) | (buf[8] << 16) | (buf[9] << 24);
		status->frequency2 = 0;
	} else { // LBE_1421_DUALOUT
		status->frequency1 = buf[7] | (buf[8] << 8) | (buf[9] << 16) | (buf[10] << 24);
		status->frequency2 = buf[15] | (buf[16] << 8) | (buf[17] << 16) | (buf[18] << 24);
	}
	status->outputs_enabled = (status->raw_status & 0x7F) == 0x7F;
	status->fll_enabled = buf[19] != 0;
/*
	printf("Raw report dump:\n");
	for (int i = 0; i < REPORT_SIZE; i++) {
		printf("%02X ", buf[i]);
		if ((i + 1) % 16 == 0) printf("\n");
	}
	printf("\n");
*/
	return 0;
}

int lbe_set_frequency(struct lbe_device* dev, int output, uint32_t frequency) {
	uint8_t buf[REPORT_SIZE] = {0};
	int ret;

	if (dev->model == LBE_1420 && output != 1) {
		fprintf(stderr, "LBE-1420 only supports output 1\n");
		return -1;
	}

	buf[0] = 0x4B; // Report ID 0x4B
	if (dev->model == LBE_1420) {
		buf[1] = LBE_1420_SET_F1; // Command
		buf[2] = (frequency >>  0) & 0xff;
		buf[3] = (frequency >>  8) & 0xff;
		buf[4] = (frequency >> 16) & 0xff;
		buf[5] = (frequency >> 24) & 0xff;
	} else { // LBE_1421_DUALOUT
		if (output == 1) {
			buf[1] = LBE_1421_SET_F1; // Command
		} else if (output == 2) {
			buf[1] = LBE_1421_SET_F2; // Command
		} else {
			fprintf(stderr, "Invalid output selection\n");
			return -1;
		}
		buf[6] = (frequency >>  0) & 0xff;
		buf[7] = (frequency >>  8) & 0xff;
		buf[8] = (frequency >> 16) & 0xff;
		buf[9] = (frequency >> 24) & 0xff;
	}

	ret = libusb_control_transfer(dev->handle,
				LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
				LIBUSB_REQUEST_SET_REPORT,
				(LIBUSB_REPORT_TYPE_FEATURE << 8) | buf[0],
				0,
				buf,
				REPORT_SIZE,
				5000);

	if (ret < 0) {
		fprintf(stderr, "Failed to set frequency: %s\n", libusb_error_name(ret));
		return -1;
	}

	return 0;
}

int lbe_set_outputs_enable(struct lbe_device* dev, int enable) {
	uint8_t buf[REPORT_SIZE] = {0};
	int ret;

	buf[0] = 0x4B; // Report ID 0x4B
	buf[1] = LBE_142X_EN_OUT; // Command
	buf[2] = enable ? (dev->model == LBE_1421_DUALOUT ? 0x03 : 0x01) : 0x00;

	ret = libusb_control_transfer(dev->handle,
				LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
				LIBUSB_REQUEST_SET_REPORT,
				(LIBUSB_REPORT_TYPE_FEATURE << 8) | buf[0],
				0,
				buf,
				REPORT_SIZE,
				5000);

	if (ret < 0) {
		fprintf(stderr, "Failed to set output enable: %s\n", libusb_error_name(ret));
		return -1;
	}

	return 0;
}

int lbe_blink_leds(struct lbe_device* dev) {
	uint8_t buf[REPORT_SIZE] = {0};
	int ret;

	buf[0] = 0x4B; // Report ID 0x4B
	buf[1] = LBE_142X_BLINK_OUT; // Command 

	ret = libusb_control_transfer(dev->handle,
				LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
				LIBUSB_REQUEST_SET_REPORT,
				(LIBUSB_REPORT_TYPE_FEATURE << 8) | buf[0],
				0,
				buf,
				REPORT_SIZE,
				5000);
	if (ret >= 0) {
		return 0;
	} else if (ret == LIBUSB_ERROR_PIPE) {
		libusb_clear_halt(dev->handle, LIBUSB_ENDPOINT_OUT);
		fprintf(stderr, "Failed to blink LEDs: %s\n", libusb_error_name(ret));
		return -1;
	} else {
		fprintf(stderr, "Failed to blink LEDs: %s\n", libusb_error_name(ret));
		return -1;
	}
}

#endif // _WIN32
