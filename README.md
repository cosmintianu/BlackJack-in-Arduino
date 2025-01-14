# Blackjack Using Arduino

## Overview
This project demonstrates the implementation of the card game Blackjack using an Arduino Uno board and a D1 Robot LCD Keypad Shield. It combines hardware and software to provide an interactive game experience, adhering to standard Blackjack rules. The game uses custom libraries, ADC for user input, and an LCD display for visual feedback.

---

## Features
- **Deck Creation**: A 52-card deck is initialized, mimicking a standard real-life deck with suits and ranks.
- **Random Card Dealing**: Utilizes ADC noise for pseudo-random card dealing.
- **Game Logic**: Implements Blackjack rules, allowing the player to hit or stand and calculates the outcome.
- **Hand Value Calculation**: Supports Ace's dual value (1 or 11) and adjusts dynamically to avoid busting.
- **Player Input**: Buttons on the LCD keypad shield are read through an ADC, enabling user interaction.
- **Output**: Displays game status on a 16x2 LCD and through the UART protocol.

---

## Hardware Requirements
- **Arduino Uno**
- **D1 Robot LCD Keypad Shield**
  - 16x2 HD44780 LCD display
  - Analog signal-based buttons (connected through a resistor ladder)
- **Integrated ADC** for analog-to-digital signal conversion

---

## Project Structure

### System Architecture
- Deck creation
- Random card dealing
- Game logic and player interaction
- Display results on the LCD and UART

### Code Breakdown
- Deck initialization and shuffling
- Handling player actions (hit/stand)
- Calculating scores and determining outcomes

---

## How to Use

### Setup
1. Connect the **D1 Robot LCD Keypad Shield** to the Arduino Uno.
2. Upload the source code from the GitHub repository.

### Gameplay
- Use the **LEFT** button to **hit** (draw a card).
- Use the **RIGHT** button to **stand** (end your turn).
- Press **RESET** after each game to play again.

### Winning Conditions
- **Win**: Player wins if their hand is under 21 and greater than the dealer's hand.
- **Tie**: Both have the same score under 21.
- **Loss**: 
  - Player loses if they bust (score exceeds 21).
  - Player loses if their hand is lower than the dealer's hand.

---

Feel free to copy this into your `README.md` file! Let me know if you'd like any further refinements or additions.
