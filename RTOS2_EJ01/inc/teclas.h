/*
 * teclas.h
 *
 *  Created on: Jul 3, 2020
 *      Author: jose
 */

#ifndef PROGRAMS_RTOS2_EJ01_INC_TECLAS_H_
#define PROGRAMS_RTOS2_EJ01_INC_TECLAS_H_

#include "FreeRTOS.h"					// Librería FreeRTOS
#include "task.h"						// Librería para creación de tareas FreeRTOS
#include "semphr.h"						// Librería para uso de semáforos FreeRTOS
#include "FreeRTOSConfig.h"				// Archivo de configuración para FreeRTOS
#include "sapi.h"

typedef enum
{
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
} fsmButtonState_t;

typedef struct {
	gpioMap_t g_tecla; 					// Indica la tecla seleccionada
	TickType_t t_medido; 				// Almacena el tiempo medido
	TickType_t t_presionado;
	TickType_t contFalling;
	TickType_t contRising;
	const char *texto;					// Texto informativo de la tecla
	fsmButtonState_t fsmButtonState;
}tTecla; // Creamos la estructura de Tecla Led

void fsmButtonUpdate(  tTecla* tTeclaObject, QueueHandle_t* cola );

#endif /* PROGRAMS_RTOS2_EJ01_INC_TECLAS_H_ */
