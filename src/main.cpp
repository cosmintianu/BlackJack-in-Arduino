#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>
#include <time.h>

#include "HD44780.hpp"
#include "libADC.hpp"
#include "uart_buffer.hpp"

typedef struct {
  int value;
  bool out;
  char suit[10];
  char rank[10];
}Card;

void createDeck(Card deck[]){
	char suits[4][10] = {{"Hearts"}, {"Diamonds"}, {"Clubs"}, {"Spades"}};
	char ranks[13][10] = {{"2"}, {"3"}, {"4"}, {"5"}, {"6"}, {"7"}, {"8"}, {"9"}, {"10"}, {"Jack"}, {"Queen"}, {"King"}, {"Ace"}};

  	int index = 0;
  	for(int i = 0; i < 4; ++i){
    	for(int j = 0; j < 13; ++j){
      		strcpy(deck[index].suit, suits[i]);
      		strcpy(deck[index].rank, ranks[j]);
			deck[index].out = false;

			if(j == 12){
				deck[index].value = 11;
			}else if (j >= 9){
				deck[index].value = 10;
			}else{
				deck[index].value = j + 2;
			}
			++index;
  		}
	}
}

uint16_t generateRandom() {
    uint16_t seed = 0;

    for (uint8_t i = 0; i < 16; i++) {
        // XOR successive ADC readings to add entropy
        seed = (seed << 1) | (ADC_read_noise() & 0x01);  // Use only the LSB of the ADC value
    }
    return seed;
}

Card dealCard(Card deck[]){
	while (1) {
        int pos = generateRandom() % 52;
        if (!deck[pos].out) {
            deck[pos].out = true;
            return deck[pos];
        }
	}
}

void showCardDetails(Card card){
	char aux[64];
	sprintf(aux,"%s of %s has value %d and out: %d\n",card.rank ,card.suit, card.value, card.out);
	
	uart_send_array((uint8_t*) aux, strlen(aux));
	
	// uart_send_array((uint8_t*) card.rank, strlen(card.rank));
	// uart_send_array((uint8_t*) card.suit, strlen(card.suit));
	// sprintf(aux,"%d_%d", card.out, card.value);
	// uart_send_byte('\n');

	_delay_ms(10);
}

void showPlayerHand(Card hand[]);

int main(void){ 	
	LCD_Initalize();
	uart_init(9600,0);
	ADC_Init();
	sei();

	int seed = generateRandom();
	srand(seed);

	Card deck[52];
	createDeck(deck);

	// Card card = dealCard(deck);
	// uart_send_array((uint8_t*)"Please enter a seed to start the game: ",40);
	
	char buf[16];

	// variables
	uint16_t raw, rawOld;
	bool updateScreen;
	
	char state;
	uint8_t data = 0;
	int level = 1;
	Card player_hand[10];
	Card dealer_hand[10];

	bool dealtCards = false;

	while (1){
		if(dealtCards == false){
			for(int i = 0; i < 2; ++i){
				player_hand[i] = dealCard(deck);
				dealer_hand[i] = dealCard(deck);
				// showCardDetails(hand[i]);
				
			}
				showPlayerHand(player_hand);
				// showCardDetails(card);
				// showCardDetails(card2);

			dealtCards = true;
		}	

		
		
		// -----------
		// PART I - read sign from UART
		// -----------
	// 	if(uart_read_count()>0){
	// 		data = uart_read();
	// 		uart_send_byte(data);

	// 		if (data=='R'||data=='L'||data=='U'||data=='D'){
	// 			uart_send_string((uint8_t*)" < COMMAND\r\n");
	// 			state = data;
	// 			updateScreen = 1;
	// 		}
    //   else if(data == 'F'){
    //       // flushSerial();
    //   }
	// 	  else {
	// 			uart_send_string((uint8_t*)" < unknown sign\r\n");
	// 		}
	// 	}



		// -----------
		// PART II - MEASURE ADC
		// -----------
		raw = ADC_conversion();

		
		// check if ADC measurement have changed
		if((raw - rawOld) < 50){
			rawOld = raw;
		}
		else {
			/*
				RIGHT		0
				UP 			131
				DOWN		306
				LEFT		480
				SELECT		721
				released 	1023
			*/
			// if(setSeed == false && raw < 1020){

				
			// seed = raw;
			// setSeed = true;
			// srand(seed);
			// }
			// RIGHT
			if (raw < 100){
				state = 'R';
				updateScreen = 1;			
			}
			// UP
			else if (raw < 250) {
				state = 'U';
				updateScreen = 1;			
			}
			// DOWN
			else if (raw < 350) {
				state = 'D';
				updateScreen = 1;
			}
			// LEFT
			else if (raw < 500) {
				state = 'L';
				updateScreen = 1;
			}			
		}

		if (updateScreen == 1){
			LCD_WriteCommand(HD44780_CLEAR);
			_delay_ms(10);
			LCD_GoTo(0,0);
			
			sprintf(buf, "Hand:");
			int score = 0;
			int len = strlen(buf); // Track the current length of the string in buf

			for (int i = 0; i < 1 && player_hand[i].value != 0; ++i) {
				// score += player_hand[i].value;
				score += 1;
				// Append the rank's first character to buf without overwriting
				sprintf(buf + len, " %c\0", player_hand[i].rank[0]);
				len = strlen(buf); // Update the length after appending
			}
			sprintf(buf + len, " %c", 'A');
			LCD_WriteText(buf);
			_delay_ms(100);

			LCD_GoTo(0,1);
			sprintf(buf, "Score: %d\0", score);
			_delay_ms(100);
			LCD_WriteText(buf);

			updateScreen = 0;
		}
    
	}
}

void showPlayerHand(Card hand[]){
	for(int i = 0; i < 10 && hand[i].value != 0; ++i){
		showCardDetails(hand[i]);
	}
}