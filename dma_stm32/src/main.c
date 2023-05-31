#include <stm32f1xx.h>
#include <stdint.h>


//====================== USER FUNCTIONS ========================

volatile uint8_t running = 0;
void serial_async_send(uint8_t *data, uint32_t len);

volatile uint16_t millis = 0;
void delay(uint16_t time);

//==============================================================


//= ============================ HARDWARE SETUP ==================================

void DMA_setup(){

    //Enbale DMA1 clock
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel4->CCR |= DMA_CCR_MINC | DMA_CCR_TCIE | DMA_CCR_DIR | DMA_CCR_CIRC;
    DMA1_Channel4->CPAR = (uint32_t) &USART1->DR;
}

void USART_setup(){

    //enable USART and GPIOA clocks
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;

    GPIOA->CRH |= GPIO_CRH_MODE9 | GPIO_CRH_CNF9_1;
    GPIOA->CRH &= ~(GPIO_CRH_CNF9_0);

    //CLOCK = 8Mhz
    //BRR = 115200
    //8000000/(16*115200);
    //H_BRR = 4 = 0x004
    //L_BRR = 5 = 0x5

    USART1->BRR = 0x0045;
    USART1->CR1 |= USART_CR1_RXNEIE;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_UE | USART_CR1_RE;
    USART1->CR3 |= USART_CR3_DMAT;

    NVIC_EnableIRQ(USART1_IRQn);
}

void TIM2_setup(){

    //enable TIM2 Clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->CNT = 0;
    TIM2->ARR = 1000;
    TIM2->PSC = 15;
    TIM2->DIER = TIM_DIER_UIE;
    TIM2->CR1 = TIM_CR1_CEN;
    NVIC_SetPriority(TIM2_IRQn, 1);
}

//==========================================================================


uint8_t text[] = "\n Hello, Async data\n";
uint32_t len = sizeof(text) - 1;

int main(){
    USART_setup();
    DMA_setup();
    TIM2_setup();


    while(1){
        serial_async_send(text, len);
        delay(1000);
    }
}

void serial_async_send(uint8_t *data, uint32_t len){
    if(!running){
        running = 1;
        NVIC_DisableIRQ(DMA1_Channel4_IRQn);
        DMA1_Channel4->CCR &= ~DMA_CCR_EN;

        DMA1_Channel4->CNDTR = len;
        DMA1_Channel4->CMAR = (uint32_t) data;

        NVIC_EnableIRQ(DMA1_Channel4_IRQn);
        DMA1_Channel4->CCR |= DMA_CCR_EN;
    }
}

void delay(uint16_t time){
    NVIC_DisableIRQ(TIM2_IRQn);
    TIM2->CR1 &= ~(TIM_CR1_CEN);

    millis = time;

    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= TIM_CR1_CEN;
    while(millis != 0);
}

//===================== IRQ HANDLERS =======================
void USART1_IRQHandler(void){
    if(running) return;
    if(USART1->SR & USART_SR_RXNE){
        uint8_t temp = USART1->DR;
        USART1->DR = temp;
        while (!(USART1->SR & USART_SR_TC));
    }
}

void DMA1_Channel4_IRQHandler(void){
    DMA1->IFCR = DMA_IFCR_CTCIF4;
    DMA1_Channel4->CCR &= ~DMA_CCR_EN;
    running = 0;
}

void TIM2_IRQHandler(void){
    if(millis != 0){
        millis--;
    }else{
        NVIC_DisableIRQ(TIM2_IRQn);
    }
    TIM2->SR &= ~(TIM_SR_UIF);
}
//==========================================================
