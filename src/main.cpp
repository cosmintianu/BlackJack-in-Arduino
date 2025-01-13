#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>
#include <time.h>

#include "HD44780.hpp"
#include "libADC.hpp"
#include "uart_buffer.hpp"

#define HIT 1
#define NOACTION 0
#define STAND -1

typedef struct {
  int value;
  bool out;
  char suit[9];
  char rank[3];
}Card;

typedef struct{
	int size = 0;
	int value = 0;
	int aces = 0;
	Card cards[8] ={0};
}Hand;

void createDeck(Card* deck){
	char suits[4][9] = {{"Hearts"}, {"Diamonds"}, {"Clubs"}, {"Spades"}};
	char ranks[13][3] = {{"2"}, {"3"}, {"4"}, {"5"}, {"6"}, {"7"}, {"8"}, {"9"}, {"10"}, {"J"}, {"Q"}, {"K"}, {"A"}};

  	int index = 0;
  	for(int i = 0; i < 4; ++i){
    	for(int j = 0; j < 13; ++j){
      		strcpy(deck[index].suit, suits[i]);
			// sprintf(deck[index].rank, "%s\0", ranks[j]);
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

void showCardDetails(Card* card){
	char aux[48];
	sprintf(aux,"%s of %s has value %d and out: %d\n\0" ,card->rank ,card->suit, card->value, card->out);
	
	uart_send_array((uint8_t*) aux, strlen(aux));

	_delay_ms(10);
}

void showCards(Card hand[]){
	for(int i = 0; i < 8 && hand[i].out == 1; ++i){
		showCardDetails(&hand[i]);
	}
}

void updateScoreAndAces(Hand* hand, Card* card) {
    hand->value += card->value;
    if (card->rank[0] == 'A') {
        hand->aces++;
    }
    while (hand->value > 21 && hand->aces > 0) {
        hand->value -= 10;
        hand->aces--;
    }
}

int main(void){ 	
	LCD_Initalize();
	uart_init(9600,0);
	ADC_Init();
	sei();

	// LCD_WriteCommand(HD44780_CLEAR);
	// _delay_ms(10);

	int seed = generateRandom();
	srand(seed);

	Card deck[52];
	createDeck(deck);

	Hand player_hand;
	Hand dealer_hand;
	player_hand.size = 0;
	player_hand.value = 0;
	dealer_hand.size = 0;
	dealer_hand.value = 0;
	char buf[16];

	uint16_t raw, rawOld;
	bool updateScreen = 1;
	
	bool dealtCards = false;
	bool gameOver = false;
	int action = NOACTION;

	while (1){

		if(gameOver == true){
			if(player_hand.value > dealer_hand.value || dealer_hand.value > 21){
				//win
				strcpy(buf, "You won.     \0");
				uart_send_array((uint8_t*) buf, strlen(buf));
			
			}else if(player_hand.value == dealer_hand.value){
				//draw
				strcpy(buf, "It's a tie.    \0");
				uart_send_array((uint8_t*) buf, strlen(buf));
			}else{	
				//lose
				strcpy(buf, "You lost.     \0");
				uart_send_array((uint8_t*) buf, strlen(buf));
			}
			// gameOver = false;
			uart_send_byte('\n');
			updateScreen = 1;
		}

		if(player_hand.value >21){
			gameOver = true;
			strcpy(buf, "You got busted.\0");
			uart_send_array((uint8_t*) buf, strlen(buf));
			uart_send_byte('\n');

			player_hand.value = 0;
		}

		if(dealtCards == false){
			for(int i = 0; i < 2; ++i){
				Card card1 = dealCard(deck);
				Card card2 = dealCard(deck);
				
				player_hand.cards[i] = card1;
				dealer_hand.cards[i] = card2;

				updateScoreAndAces(&player_hand, &card1);
				updateScoreAndAces(&dealer_hand, &card2);
				}
			player_hand.size += 2;
			dealer_hand.size += 2;

			// showCards(player_hand.cards);
			uart_send_array((uint8_t*) "Dealer has :\n? of ? has value ? and out: ?\n\0", strlen("Dealer has :\n? of ? has value ? and out: ?\n\0"));
			char aux[48];
			sprintf(aux,"%s of %s has value %d and out: %d\n\0" , dealer_hand.cards[1].rank, dealer_hand.cards[1].suit, dealer_hand.cards[1].value, dealer_hand.cards[1].out);
				
			uart_send_array((uint8_t*) aux, strlen(aux));
			
				
			dealtCards = true;
		}

		if(action != NOACTION){
			if (action == HIT){

				Card card = dealCard(deck);
				player_hand.cards[player_hand.size++] = card;

				updateScoreAndAces(&player_hand, &card);

				updateScreen = 1;
				action = NOACTION;
				// showCards(player_hand.cards);

			}else if(action == STAND){

				while (dealer_hand.value < 17){
					Card card = dealCard(deck);
					dealer_hand.cards[dealer_hand.size++] = card;
					updateScoreAndAces(&dealer_hand, &card);

				}
					char aux[64];
					sprintf(aux ,"Dealers score: %d and size: %d\n\0", dealer_hand.value, dealer_hand.size);
					uart_send_array((uint8_t*) aux, strlen(aux));
					showCards(dealer_hand.cards);

					action = NOACTION;
					gameOver = true;
			}
		}	

		

		raw = ADC_conversion();

		// Check if ADC measurement have changed
		if((raw - rawOld) < 50){
			rawOld = raw;
		}
		else {
			if (raw < 100){
				// STAND on R
				action = STAND;
				updateScreen = 1;			
			}else if (raw < 500) {
				// HIT on L
				action = HIT;
				updateScreen = 1;
			}			
		}

		if (updateScreen == 1){

			// LCD_WriteCommand(HD44780_CLEAR);
			// _delay_ms(10);

			LCD_GoTo(0,0);
			if(!gameOver){
				sprintf(buf, ">\0");

				int len = strlen(buf);

				for (int i = 0; i < player_hand.size ; ++i) {
					if(player_hand.cards[i].rank[0] == '1')
						sprintf(buf + len , " 10\0");
					else
						sprintf(buf + len, " %c\0", player_hand.cards[i].rank[0]);

					len = strlen(buf); 
				}
				buf[16]= '\0';
				LCD_WriteText(buf);
				_delay_ms(100);

				LCD_GoTo(0,1);
				sprintf(buf, "Score: %d\0", player_hand.value);
				_delay_ms(100);
				LCD_WriteText(buf);

			}else{

				LCD_WriteText(buf);
				_delay_ms(100);

				gameOver = false;
			}
			updateScreen = 0;
		}
	}
}
