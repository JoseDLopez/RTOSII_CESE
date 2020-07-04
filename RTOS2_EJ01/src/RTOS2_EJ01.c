/*=============================================================================
 * Copyright (c) 2020, José Daniel López <josedlopez11@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2020/07/03
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

/* Librerias de FreeRTOS. */
#include "FreeRTOS.h"					// Librería FreeRTOS
#include "task.h"						// Librería para creación de tareas FreeRTOS
#include "semphr.h"						// Librería para uso de semáforos FreeRTOS
#include "FreeRTOSConfig.h"				// Archivo de configuración para FreeRTOS
#include "supporting_functions.h"		// Librerias de soporte que vienen con FreeRTOS
#include "sapi.h"						// Librería SAPI
#include "teclas.h"						// Librería SAPI


#define TECLALEDCANT	2				// Indicamos cuantas teclas se van a trabajar
tTecla TECLAS_LED_CONFIG[TECLALEDCANT];	// Declaramos array de tipo Tecla Led del tamaño de dos teclas.
QueueHandle_t cola_1;					// Definimos el nombre de la cola requerida
DEBUG_PRINT_ENABLE;						// Habilitamos el DEBUG PRINT


/* Creación de tarea para el RTOS */
void vTareaA (void *pvParameters );
void vTareaB (void *pvParameters );
void vTareaC (void *pvParameters );

/*------------------ MAIN PRINCIPAL --------------------------*/
int main( void ){
	boardConfig(); 									// Iniciamos la configuración de la tarjeta
	debugPrintConfigUart( UART_USB, 115200 );		// Inicializamos el UART para imprimir el DEBUG
	gpioWrite( LED3 , ON ); 						// Encendemos el led para indicar que inicia el programa.
	debugPrintlnString( "Inicio del programa" );

	/* Creamos las teclas */
	TECLAS_LED_CONFIG[0].g_tecla = TEC1;
	TECLAS_LED_CONFIG[0].texto = "TEC1 T";

	TECLAS_LED_CONFIG[1].g_tecla = TEC2;
	TECLAS_LED_CONFIG[1].texto = "TEC2 T";

	/* Creamos TASK para la tarea A */
	BaseType_t res_tarea_a = xTaskCreate(vTareaA,
										(const char *)"Encender LED Periodicamente",
										configMINIMAL_STACK_SIZE*2,
										NULL, tskIDLE_PRIORITY+1,
										NULL);
	/* Creamos TASK para la tarea B */
	BaseType_t res_tarea_b = xTaskCreate(vTareaB,
										(const char *)"Lectura de teclas",
										configMINIMAL_STACK_SIZE*2,
										NULL, tskIDLE_PRIORITY+1,
										NULL);
	/* Creamos TASK para la tarea C */
	BaseType_t res_tarea_c = xTaskCreate(vTareaC,
										(const char *)"Obtener mensajes y enviar por la UART",
										configMINIMAL_STACK_SIZE*2,
										NULL, tskIDLE_PRIORITY+1,
										NULL);


	/* Validamos que la creación de las tareas sea correcta además de que se haya podido crear la cola */
	if (res_tarea_a != pdFAIL && res_tarea_b != pdFAIL && res_tarea_c != pdFAIL){
		debugPrintlnString("Tareas creadas OK.");
		cola_1  = xQueueCreate(10,sizeof(char[50]));							// Tratamos de crear la cola
		if( cola_1 == NULL){													// Validamos la creación
			debugPrintlnString( "No fue posible crear la cola." );
		}else{
			vTaskStartScheduler(); 												// Iniciamos el scheduler
		}
	}else{
		debugPrintlnString("No fue posible crear todas las tareas.");
		for(;;);
	}




	for (;;);
	return 0;
}
/*------------------ FIN MAIN PRINCIPAL --------- -----------*/

void vTareaA(void *pvParameters){
	const TickType_t xLedDelay = pdMS_TO_TICKS( 2000 ); 						// Retardo de 2s que es cuanto va a durar el led encendido y apagado
	for(;;){
		gpioWrite(LED1, ON);
		debugPrintlnString("Encendemos LED1 periodicamente.");
		BaseType_t res_send = xQueueSend( cola_1 , "LED ON" , portMAX_DELAY );	// Enviamos el valor a la cola y si no es posible esperamos acá
		debugPrintlnString("Guardamos mensaje en la cola.");
		vTaskDelay( xLedDelay );
		gpioWrite( LED1, OFF );
		debugPrintlnString("Apagamos LED1 periodicamente.");
		vTaskDelay( xLedDelay );
	}
}

void vTareaB(void *pvParameters){
	uint8_t i;
	const TickType_t xDelayms = pdMS_TO_TICKS( 1 );
	for(;;){
		for (i=0; i<TECLALEDCANT; i ++){
			fsmButtonUpdate( &TECLAS_LED_CONFIG[i], cola_1);				 	// Leemos las teclas y hacemos el proceso de la FMS
			vTaskDelay( xDelayms );
		}
	}
}

void vTareaC(void *pvParameters){
	char mensaje[50];
	const TickType_t xReadDelay = pdMS_TO_TICKS( 50 );

	for(;;){
		BaseType_t res_receive = xQueueReceive( cola_1 , &mensaje ,portMAX_DELAY );	// Leemos de la cola, si no es posible leer nos quedamos acá
		debugPrintlnString( mensaje );												// Imprimimos el mensaje recibido
		vTaskDelay( xReadDelay );
	}
}


