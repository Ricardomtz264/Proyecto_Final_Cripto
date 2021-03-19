/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"

/* AES */
#include "aes.h"
#include "aes_layer.c"
#include "aes_layer.h"
#include "aes_layer_config.h"

/* ChaChaXX */
#include "chacha.h"

/* RSA */
#include "hydrogen.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 1)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hello_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    if (xTaskCreate(hello_task, "Hello_task", configMINIMAL_STACK_SIZE + 100, NULL, hello_task_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
static void hello_task(void *pvParameters)
{

	#define CONTEXT "Example"
	//#define MESSAGE "Test"
	//#define MESSAGE_LEN 4

	// Message
	uint8_t MESSAGE[] = {"This message that will be sing through RSA then encrypt through ChachaXX finally encrypt through AES128"};

    int i;

	uint8_t key[] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13,
        0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b,
        0x1c, 0x1d, 0x1e, 0x1f
    };

	uint8_t iv[]  = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};

    uint8_t nonce[] = {
        0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x4a,
		0x00, 0x00, 0x00, 0x00
    };

/*
    uint8_t input[114] = {
        0x4c, 0x61, 0x64, 0x69, 0x65, 0x73, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x47, 0x65, 0x6e, 0x74, 0x6c,
        0x65, 0x6d, 0x65, 0x6e, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x61, 0x73,
        0x73, 0x20, 0x6f, 0x66, 0x20, 0x27, 0x39, 0x39, 0x3a, 0x20, 0x49, 0x66, 0x20, 0x49, 0x20, 0x63,
        0x6f, 0x75, 0x6c, 0x64, 0x20, 0x6f, 0x66, 0x66, 0x65, 0x72, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x6f,
        0x6e, 0x6c, 0x79, 0x20, 0x6f, 0x6e, 0x65, 0x20, 0x74, 0x69, 0x70, 0x20, 0x66, 0x6f, 0x72, 0x20,
        0x74, 0x68, 0x65, 0x20, 0x66, 0x75, 0x74, 0x75, 0x72, 0x65, 0x2c, 0x20, 0x73, 0x75, 0x6e, 0x73,
        0x63, 0x72, 0x65, 0x65, 0x6e, 0x20, 0x77, 0x6f, 0x75, 0x6c, 0x64, 0x20, 0x62, 0x65, 0x20, 0x69,
        0x74, 0x2e
    };
*/
	struct AES_ctx ctx;
	size_t MESSAGE_LEN, padded_len;
	MESSAGE_LEN = strlen(MESSAGE);

	uint8_t padded_msg[512] = {0};

    uint8_t encrypt[114];
    uint8_t decrypt[114];

    PRINTF("\n********************* KEY ********************* \r\n\n");
    for (i = 0; i < 32; i++) {
        if (!(i % 16)) {
        	PRINTF("\r\n");
        }
        PRINTF("0x%02x ", key[i]);
    }
	PRINTF("\r\n********************* nonce ********************* \r\n\n");
	for (i = 0; i < 12; i++) {
		PRINTF("0x%02x ", nonce[i]);
	}

    PRINTF("\r\n********************* Sing by RSA ********************* \r\n\n");


    hydro_sign_keygen(&key_pair);
    uint8_t signature[hydro_sign_BYTES];

    hydro_sign_create(signature, MESSAGE, MESSAGE_LEN, CONTEXT, key_pair.sk);
/*
    for (i = 0; i < 114; i++) {
        if (!(i % 16)) {
        	PRINTF("\r\n");
        }
        PRINTF("0x%02x ", input[i]);
    }
*/
	PRINTF("\n********************* Encryption by AES128 ********************* \r\n\n");
	PRINTF("Message: %s \r\n\n", MESSAGE);

	// Init the AES context structure
	AES_init_ctx_iv(&ctx, key, iv);

	// To encrypt an array its lenght must be a multiple of 16 so we add zeros
	padded_len = MESSAGE_LEN + (16 - (MESSAGE_LEN%16) );
	memcpy(padded_msg, MESSAGE, MESSAGE_LEN);

	AES_CBC_encrypt_buffer(&ctx, padded_msg, padded_len);

    for (i = 0; i < padded_len; i++) {
        if (!(i % 16)) {
        	PRINTF("\r\n");
        }
        PRINTF("0x%02x ", padded_msg[i]);
    }

    PRINTF("\r\n********************* Encrypted by ChachaXX ********************* \r\n\n");

    ChaCha20XOR(key, 1, nonce, padded_msg, encrypt, 114);

    for (i = 0; i < 114; i++) {
        if (!(i % 16)) {
        	PRINTF("\r\n");
        }
        PRINTF("0x%02x ", encrypt[i]);
    }

	PRINTF("\r\n********************* Decrypted by ChachaXX ********************* \r\n\n");

    ChaCha20XOR(key, 1, nonce, encrypt, decrypt, 114);

	for (i = 0; i < 114; i++) {
		if (!(i % 16)) {
			PRINTF("\r\n");
		}
		PRINTF("0x%02x ", decrypt[i]);
	}

	PRINTF("\n********************* Descryption by AES128 ********************* \r\n\n");

	//AES_CBC_decrypt_buffer(&ctx, padded_msg, padded_len);
	AES_ECB_decrypt(&ctx, decrypt);

    for (i = 0; i < strlen(decrypt); i++) {
        if (!(i % 16)) {
        	PRINTF("\r\n");
        }
        PRINTF("0x%02x ", decrypt[i]);
    }
    PRINTF("\r\n\n");

	PRINTF("\n********************* Verify sing by RSA ********************* \r\n\n");
    /* Verify the signature using the public key */
    if (hydro_sign_verify(signature, MESSAGE, MESSAGE_LEN, CONTEXT, key_pair.pk) != 0) {
        /* forged */
    }

    PRINTF("Got Message: %s \r\n\n", decrypt);

}
