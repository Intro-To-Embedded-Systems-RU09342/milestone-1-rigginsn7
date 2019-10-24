/***********************************
* msp430G2553 milestone 1
*
* Richard Dell and Nicholas Riggins
* Created: 10/14/19
* Last Updated: 10/15/19
*
***********************************/
#include <msp430.h>
#include <stdint.h>

volatile int length = 0; //length of message
volatile int i = 1; //location of current byte
volatile int RED_LED = 0; //red LED duty cycle
volatile int GREEN_LED = 0; //green LED duty cycle
volatile int BLUE_LED = 0; //blue LED duty cycle


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  TA0CTL |= TASSEL_2 + MC_1;     //set smclk, up mode
  TA0CCTL1 |= OUTMOD_7;   //set/reset output
  TA0CCR0 = 255;          //100% duty cycle
  TA0CCR1 = 0;            //0% red pwm


  TA1CTL |= TASSEL_2 + MC_1;     //set smclk, up mode
  TA1CCTL1 |= OUTMOD_7;   //set/reset output
  TA1CCTL2 |= OUTMOD_7;   //set/reset output
  TA1CCR0 = 255;          //100% duty cycle
  TA1CCR1 = 0;            //0% green pwm
  TA1CCR2 = 0;            //0% blue pwm

  P1DIR |= BIT6;          //set P1.6 to output
  P1SEL |= BIT6;          //enable pwm for P1.6
  P2DIR |= (BIT1 + BIT5); //set P2.1 and P2.5 to output
  P2SEL |= (BIT1 + BIT5); //enable pwm for P2.1 and P2.5

  P1SEL |= BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
  P1SEL2 |= BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 104;                            // 1MHz 9600
  UCA0BR1 = 0;                              // 1MHz 9600
  UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
}

void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
{
    while (!(IFG2&UCA0TXIFG));        //Wait until a byte is ready, is USCI_A0 TX buffer ready?
      if(i==1)                       //first signal
      {
          UCA0TXBUF=UCA0RXBUF-3;        //length sent is input signal minus 3
          length=UCA0RXBUF-3;           //set length to input length - 3
      }
      else if(i<5)
      {
       switch(i)
        {
         case 2:
             RED_LED = UCA0RXBUF;         //set red LED duty cycle
             break;
         case 3:
             GREEN_LED = UCA0RXBUF;       //set green LED duty cycle
             break;
         case 4:
             BLUE_LED = UCA0RXBUF;        //set blue LED duty cycle
             break;
         default:
             break;
          }
      }
       else
       {
           UCA0TXBUF = UCA0RXBUF; //send byte
       }
      if(i==length+3)             //if there are more bytes to be received, reset the counter
        {
            i=0;                    //reset counter back to zero
            TA0CCR1=RED_LED;              //set red duty cycle to its timer
            TA1CCR2=GREEN_LED;            //set green duty cycle to its timer
            TA1CCR1=BLUE_LED;             //set blue duty cycle to its timer
        }

      i++;
}
