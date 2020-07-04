/* Copyright 2018, Eric Pernia.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "sapi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "teclas.h"
#include "supporting_functions.h"		/

void fsmButtonError( tTecla* tTeclaObject );
void fsmButtonInit( tTecla* tTeclaObject  );
void fsmButtonUpdate( tTecla* tTeclaObject, QueueHandle_t* cola  );
void buttonPressed(  tTecla* tTeclaObject );
void buttonReleased(  tTecla* tTeclaObject, QueueHandle_t* cola  );
#define DEBOUNCE_TIME 40


/* Acción de evento de tecla pulsada */
void buttonPressed( tTecla* tTeclaObject )
{
	tTeclaObject->t_presionado = xTaskGetTickCount();							// Agregamos al tick presionado el valor actual del tick count
}

/* Acción de evento de tecla liberada */
void buttonReleased( tTecla* tTeclaObject, QueueHandle_t* cola  )
{
	tTeclaObject->t_medido = xTaskGetTickCount() - tTeclaObject->t_presionado;	// Restamos el valor actual de cuando se presiono el boton al tick count actual
	char buffer [50];															// Creamos el buffer
	if (tTeclaObject->t_medido > 0){											// Comparamos si estuvo presioando.
		sprintf (buffer, "%s%dms",tTeclaObject->texto, tTeclaObject->t_medido);	// Concatenamos el mensaje
		BaseType_t res_send = xQueueSend( cola , buffer , portMAX_DELAY );		// Enviamos el mensaje a la cola
		if (res_send == pdPASS){
			vPrintString("Guardamos valor en la cola.\r\n");
		}

	}
}

/* Evento de error en la máquina de estado */
void fsmButtonError( tTecla* tTeclaObject )
{
	tTeclaObject->fsmButtonState = STATE_BUTTON_UP;
}

/* Inicialización de la máquina de estado */
void fsmButtonInit( tTecla* tTeclaObject )
{
	tTeclaObject->contFalling = 0;
	tTeclaObject->contRising = 0;
	tTeclaObject->fsmButtonState = STATE_BUTTON_UP;
}


/* Función de actualización de estado del botón */
void fsmButtonUpdate( tTecla* tTeclaObject, QueueHandle_t* cola  )
{

    switch( tTeclaObject->fsmButtonState )
    {
        case STATE_BUTTON_UP:
            /* CHECK TRANSITION CONDITIONS */
            if( !gpioRead( tTeclaObject->g_tecla ))
            {
            	tTeclaObject->fsmButtonState = STATE_BUTTON_FALLING;
            }
            break;

        case STATE_BUTTON_FALLING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */
            if( tTeclaObject->contFalling >= DEBOUNCE_TIME )
            {
                if( !gpioRead( tTeclaObject->g_tecla ) )
                {
                	tTeclaObject->fsmButtonState = STATE_BUTTON_DOWN;
                    /* ACCION DEL EVENTO !*/
                    buttonPressed(tTeclaObject);
                }
                else
                {
                	tTeclaObject->fsmButtonState = STATE_BUTTON_UP;
                }

                tTeclaObject->contFalling = 0;
            }

            tTeclaObject->contFalling++;

            /* LEAVE */
            break;

        case STATE_BUTTON_DOWN:
			/* CHECK TRANSITION CONDITIONS */
			if( gpioRead( tTeclaObject->g_tecla ) )
			{
				tTeclaObject->fsmButtonState = STATE_BUTTON_RISING;
			}
			break;

        case STATE_BUTTON_RISING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */

            if( tTeclaObject->contRising >= DEBOUNCE_TIME )
            {
                if( gpioRead( tTeclaObject->g_tecla ) )
                {
                	tTeclaObject->fsmButtonState = STATE_BUTTON_UP;

                    /* ACCION DEL EVENTO ! */
                    buttonReleased(tTeclaObject, cola);
                }
                else
                {
                	tTeclaObject->fsmButtonState = STATE_BUTTON_DOWN;
                }
                tTeclaObject->contRising = 0;
            }
            tTeclaObject->contRising++;

            /* LEAVE */
            break;

        default:
            fsmButtonError(tTeclaObject);
            break;
    }
}
