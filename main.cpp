#include <SFML/Graphics.hpp>
#include <iostream>
#include <time.h>
#include <vector>
#include <random>
#include <chrono>
#include <thread>

#define RUNS 100
#define SCALE 0.5f

struct Card {
	int color;
	int value;
	sf::Sprite sprite;
	sf::Sprite back;
};

void interact(sf::RenderWindow& window) {
	sf::Event e;
	while (window.pollEvent(e)) {
		if (e.type == sf::Event::Closed) {
			window.close();
		}
	}
}

void draw(sf::RenderWindow& window, std::vector<Card> deck, std::vector<std::vector<Card>> piles, std::vector<Card> discard) {
	window.clear(sf::Color(0, 100, 0));
	float offsetX = 160 / 7;

	for (size_t i = 0; i < 4; i++) {
		if (piles[i].size() > 10) {
			for (size_t j = 0; j < 10; j++) {
				piles[i].at(j).sprite.setPosition((400.f + 2 * offsetX + (400.f + offsetX) * i) * SCALE, (100.f + 1.5f * j) * SCALE);
				window.draw(piles[i].at(j).sprite);
			}
			for (size_t j = 10; j < piles[i].size(); j++) {
				piles[i].at(j).sprite.setPosition((400.f + 2 * offsetX + (400.f + offsetX) * i) * SCALE, (115.f + 80.f * (j - 10)) * SCALE);
				window.draw(piles[i].at(j).sprite);
			}
		}
		else {
			for (size_t j = 0; j < piles[i].size(); j++) {
				piles[i].at(j).sprite.setPosition((400.f + 2 * offsetX + (400.f + offsetX) * i) * SCALE, (100 + 80.f * j) * SCALE);
				window.draw(piles[i].at(j).sprite);
			}
		}
	}

	for (size_t i = 0; i < deck.size(); i++) {
		deck[i].back.setPosition(offsetX * SCALE, (100.f + 1.5f * i) * SCALE);
		window.draw(deck[i].back);
	}

	if (discard.size() > 10) {
		size_t stack = (discard.size() / 10) * 10;
		for (size_t i = 0; i < stack; i++) {
			discard[i].sprite.setPosition((2000.f + 6 * offsetX) * SCALE, (100.f + 1.5f * i) * SCALE);
			window.draw(discard[i].sprite);
		}
		for (size_t i = stack; i < discard.size(); i++) {
			discard[i].sprite.setPosition((2000.f + 6 * offsetX) * SCALE, (100.f + 1.5f * stack + 80.f * float(i - stack)) * SCALE);
			window.draw(discard[i].sprite);
		}
	}
	else {
		for (size_t i = 0; i < discard.size(); i++) {
			discard[i].sprite.setPosition((2000.f + 6 * offsetX) * SCALE, (100 + 80.f * i) * SCALE);
			window.draw(discard[i].sprite);
		}
	}

	//std::this_thread::sleep_for(std::chrono::milliseconds(100));

	window.display();
}

void shuffle(std::vector<Card>& deck) {
	std::vector<Card> newDeck;
	std::random_device rd;
	std::mt19937 rng(rd());
	while (!deck.empty()) {
		std::uniform_int_distribution<int> uid(0, deck.size() - 1);
		int pos = uid(rng);
		newDeck.push_back(deck[pos]);
		deck.erase(deck.begin() + pos);
	}
	deck = newDeck;
}

std::vector<std::vector<std::pair<int, int>>> findMatches(std::vector<std::vector<Card>> piles) {
	std::vector<std::vector<std::pair<int, int>>> colors = { {}, {}, {}, {} }; // pile, value
	for (size_t i = 0; i < colors.size(); i++) {
		if (!piles[i].empty()) {
			Card currentCard = piles[i].at(piles[i].size() - 1);
			colors.at(currentCard.color).push_back(std::pair<int, int>(i, currentCard.value));
		}
	}
	return colors;
}

void gameDraw(std::vector<Card> deck, std::vector<std::vector<Card>> piles, std::vector<Card> discard, std::vector<bool>& outcomes, sf::RenderWindow& window, int& recLevel) {
	if (!outcomes.empty() || !window.isOpen())
		return;
	while (!deck.empty()) {
		for (std::vector<std::vector<std::pair<int, int>>> colors = findMatches(piles); !(colors[0].size() < 2 && colors[1].size() < 2 && colors[2].size() < 2 && colors[3].size() < 2); colors = findMatches(piles)) {
			for (std::vector<std::pair<int, int>> color : colors) {
				if (color.size() > 1) {
					int largestPile = 0;
					for (size_t j = 1; j < color.size(); j++) {
						interact(window);
						if (color[j].second > color[largestPile].second) {
							largestPile = j;
						}
					}
					for (size_t j = 0; j < color.size(); j++) {
						interact(window);
						if (j != largestPile) {
							discard.push_back(piles[color[j].first].at(piles[color[j].first].size() - 1));
							piles[color[j].first].erase(piles[color[j].first].end() - 1);
							draw(window, deck, piles, discard);
						}
					}
				}
			}
			for (int i = 0; i < 4; i++) {
				interact(window);
				if (piles[i].empty()) { // fill empty slot
					for (int j = 0; j < 4; j++) {
						if (piles[j].size() > 1) {
							std::vector<std::vector<Card>> pilesCopy(piles);
							Card toMove = pilesCopy[j].at(pilesCopy[j].size() - 1);
							pilesCopy[j].erase(pilesCopy[j].end() - 1);
							pilesCopy[i].push_back(toMove);
							draw(window, deck, pilesCopy, discard);
							gameDraw(deck, pilesCopy, discard, outcomes, window, ++recLevel);
						}
					}
				}
			}
		}
		for (int i = 0; i < 4; i++) {
			interact(window);
			piles[i].push_back(deck[0]);
			deck.erase(deck.begin());
			draw(window, deck, piles, discard);
		}
	}
	if (piles[0].size() == 1 && piles[1].size() == 1 && piles[2].size() == 1 && piles[3].size() == 1) { // can only be the 4 aces
		outcomes.push_back(true);
	}
}

void runGamesDraw() {
	sf::RenderWindow window(sf::VideoMode(int(2560 * SCALE), int(1440 * SCALE)), "Cards");
	sf::Texture cardsTexture;
	cardsTexture.loadFromFile("C:/Users/psusk/Documents/CardsOrig.png");
	sf::Sprite cards(cardsTexture);
	cards.setScale(0.8f * SCALE, 0.8f * SCALE);
	sf::Texture cardBackTexture;
	cardBackTexture.loadFromFile("C:/Users/psusk/Documents/CardBackBlueOutline.png");
	sf::Sprite cardBack(cardBackTexture);
	cardBack.setScale(0.8f * SCALE, 0.8f * SCALE);
	std::vector<Card> deck;
	for (int col = 0; col < 4; col++) {
		for (int val = 0; val < 13; val++) {
			cards.setTextureRect(sf::IntRect(500 * val, 726 * col, 500, 726));
			deck.push_back(Card{ col, val, cards, cardBack });
		}
	}

	int wins = 0;
	std::vector<bool> outcomes;
	std::chrono::steady_clock::time_point beginRun = std::chrono::steady_clock::now();
	for (int i = 0; i < RUNS && window.isOpen(); i++) {
		std::cout << "Starting game " << i + 1 << "... " << std::endl;
		shuffle(deck);
		outcomes.clear();
		int recLevel = 0;
		std::chrono::steady_clock::time_point beginGame = std::chrono::steady_clock::now();
		gameDraw(deck, { {}, {}, {}, {} }, {}, outcomes, window, recLevel);
		std::chrono::steady_clock::time_point endGame = std::chrono::steady_clock::now();
		wins += !outcomes.empty();
		if (!outcomes.empty()) {
			std::cout << "Win!" << std::endl;
		}
		std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::seconds>(endGame - beginGame).count() << " seconds | Recursion level: " << recLevel << std::endl;
	}
	std::chrono::steady_clock::time_point endRun = std::chrono::steady_clock::now();
	std::cout << "Average elapsed time: " << std::chrono::duration_cast<std::chrono::seconds>(endRun - beginRun).count() / RUNS << std::endl;
	std::cout << "Win %: " << wins / RUNS << std::endl;
}

void game(std::vector<Card> deck, std::vector<std::vector<Card>> piles, std::vector<bool>& outcomes, int& recLevel) {
	if (!outcomes.empty())
		return;
	while (!deck.empty()) {
		for (std::vector<std::vector<std::pair<int, int>>> colors = findMatches(piles); !(colors[0].size() < 2 && colors[1].size() < 2 && colors[2].size() < 2 && colors[3].size() < 2); colors = findMatches(piles)) {
			for (std::vector<std::pair<int, int>> color : colors) {
				if (color.size() > 1) {
					int largestPile = 0;
					for (size_t j = 1; j < color.size(); j++) {
						if (color[j].second > color[largestPile].second) {
							largestPile = j;
						}
					}
					for (size_t j = 0; j < color.size(); j++) {
						if (j != largestPile) {
							piles[color[j].first].erase(piles[color[j].first].end() - 1);
						}
					}
				}
			}
			for (int i = 0; i < 4; i++) {
				if (piles[i].empty()) { // fill empty slot
					for (int j = 0; j < 4; j++) {
						if (piles[j].size() > 1) {
							std::vector<std::vector<Card>> pilesCopy(piles);
							Card toMove = pilesCopy[j].at(pilesCopy[j].size() - 1);
							pilesCopy[j].erase(pilesCopy[j].end() - 1);
							pilesCopy[i].push_back(toMove);
							game(deck, pilesCopy, outcomes, ++recLevel);
						}
					}
				}
			}
		}
		for (int i = 0; i < 4; i++) {
			piles[i].push_back(deck[0]);
			deck.erase(deck.begin());
		}
	}
	if (piles[0].size() == 1 && piles[1].size() == 1 && piles[2].size() == 1 && piles[3].size() == 1) { // can only be the 4 aces
		outcomes.push_back(true);
	}
}

void runGames() {
	std::vector<Card> deck;
	for (int col = 0; col < 4; col++) {
		for (int val = 0; val < 13; val++) {
			deck.push_back(Card{ col, val });
		}
	}

	int wins = 0;
	std::vector<bool> outcomes;
	std::vector<long long> durations;
	std::vector<int> recursionDepths;
	std::chrono::steady_clock::time_point beginRun = std::chrono::steady_clock::now();
	for (int i = 0; i < RUNS; i++) {
		std::cout << "Starting game " << i + 1 << "... " << std::endl;
		shuffle(deck);
		outcomes.clear();
		int recLevel = 0;
		std::chrono::steady_clock::time_point beginGame = std::chrono::steady_clock::now();
		game(deck, { {}, {}, {}, {} }, outcomes, recLevel);
		std::chrono::steady_clock::time_point endGame = std::chrono::steady_clock::now();
		wins += !outcomes.empty();
		if (!outcomes.empty()) {
			std::cout << "Win!" << std::endl;
		}
		std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::seconds>(endGame - beginGame).count() << " seconds | Recursion level: " << recLevel << std::endl;
	}
	std::chrono::steady_clock::time_point endRun = std::chrono::steady_clock::now();
	std::cout << "Average elapsed time: " << std::chrono::duration_cast<std::chrono::seconds>(endRun - beginRun).count() / RUNS << std::endl;
	std::cout << "Win %: " << wins / RUNS << std::endl;
}

int main() {
#ifdef _DEBUG
	runGamesDraw();
#else
	runGames();
	system("PAUSE");
#endif
	return 0;
}