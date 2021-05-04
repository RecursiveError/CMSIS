#include <stm32f1xx.h>

void delay(uint32_t time);

int main(){
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL |= GPIO_CRL_MODE6_0 | GPIO_CRL_MODE6_1;
    GPIOA->CRL &= ~(GPIO_CRL_CNF6_0|GPIO_CRL_CNF6_1);
    GPIOA->ODR |= GPIO_ODR_ODR6;
    while(1){
        GPIOA->ODR ^= GPIO_ODR_ODR6;
        delay(100000);
    } 
}

void delay(uint32_t time ){
    while(time--){
        __NOP();
        __NOP();
        __NOP();
    }
}