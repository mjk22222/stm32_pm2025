#include <stdint.h>
#include <stm32f10x.h>

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        GPIOC->ODR ^= (1U << 13U);
        TIM2->SR &= ~TIM_SR_UIF;
    }
}

int main(void) {
    // Инициализация портов
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;

    GPIOC->CRH &= ~GPIO_CRH_CNF13;
    GPIOC->CRH |= GPIO_CRH_MODE13_0;

    GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
    GPIOA->CRL |= GPIO_CRL_CNF0_1;
    GPIOA->ODR |= (1U << 0);

    GPIOA->CRL &= ~(GPIO_CRL_CNF1 | GPIO_CRL_MODE1);
    GPIOA->CRL |= GPIO_CRL_CNF1_1;
    GPIOA->ODR |= (1U << 1U);

    // Инициализация таймера
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;

    uint32_t psc_values[] = {
        65535,
        32767,
        16383,
        8191
    };

    uint8_t mode = 0;

    TIM2->PSC = psc_values[mode];
    TIM2->ARR = 1000;
    TIM2->CNT = 0;

    TIM2->DIER |= TIM_DIER_UIE;
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->CR1 |= TIM_CR1_CEN;

    while (1) {
        // Увелечение скорости
        if (!(GPIOA->IDR & (1U << 0))) {
            for (volatile uint32_t i = 0; i < 10000; i++);

            if (!(GPIOA->IDR & (1U << 0))) {
                if (mode < 3) {  // Если не максимальная скорость
                    mode++;  // Увеличиваем скорость

                    TIM2->PSC = TIM2->PSC >> 1;

                    TIM2->EGR |= TIM_EGR_UG;
                }

                while (!(GPIOA->IDR & (1U << 0)));
            }
        }

        // Уменьшение скорости
        if (!(GPIOA->IDR & (1U << 1))) {
            for (volatile uint32_t i = 0; i < 10000; i++);

            if (!(GPIOA->IDR & (1U << 1))) {
                if (mode > 0) {  // Если не минимальная скорость
                    mode--;  // Уменьшаем скорость

                    TIM2->PSC = TIM2->PSC << 1;  // ЗАМЕДЛЕНИЕ

                    TIM2->EGR |= TIM_EGR_UG;
                }

                while (!(GPIOA->IDR & (1U << 1)));
            }
        }

        __asm volatile ("nop");
    }

    return 0;
}
