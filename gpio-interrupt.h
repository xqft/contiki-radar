/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx
 * @{
 *
 * \defgroup cc26xx-gpio-interrupts CC13xx/CC26xx GPIO interrupt handling
 *
 * The CC13xx/CC26xx GPIO interrupt handler and an API which can be used by
 * other parts of the code when they wish to be notified of a GPIO interrupt
 *
 * @{
 *
 * \file
 * Header file for the CC13xx/CC26xx GPIO interrupt management
 */
/*---------------------------------------------------------------------------*/
#ifndef GPIO_INTERRUPT_H_
#define GPIO_INTERRUPT_H_
/*---------------------------------------------------------------------------*/
#include <stdint.h>
/*---------------------------------------------------------------------------*/
typedef void (*gpio_interrupt_handler_t)(uint8_t ioid);
/*---------------------------------------------------------------------------*/
/** \brief Initialise the GPIO interrupt handling module */
void gpio_interrupt_init(void);

/**
 * \brief Register a GPIO interrupt handler
 * \param f Pointer to a handler to be called when an interrupt is raised on
 * ioid
 * \param ioid Associate \a f with this ioid. \e ioid must be specified with
 *        its numeric representation (0, 1, .. 31). Defines for these
 *        numeric representations are IOID_x
 */
void gpio_interrupt_register_handler(uint8_t ioid, gpio_interrupt_handler_t f);

#endif /* GPIO_INTERRUPT_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */