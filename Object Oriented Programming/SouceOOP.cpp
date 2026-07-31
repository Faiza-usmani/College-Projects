#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>
#include <sstream>

using namespace std;

class Card {
private:
    string Colour;
    int Num;
    string Type; // Added to retain special card identity without breaking structure
public:
    Card(string colour, int num, string type = "") : Colour(colour), Num(num), Type(type) {}
    
    string getColour() const { return Colour; }
    int getNum() const { return Num; }
    string getType() const { return Type; }
    
    void setColour(const string& a) { Colour = a; }
    void setNum(int a) { Num = a; }
    void setType(const string& a) { Type = a; }
    
    string toString() const {
        if (Num == -1) {
            return Colour + " " + Type;
        }
        return Colour + " " + to_string(Num);
    }
};

class SpecialCard {
private:
    string SColour;
    string Type;
public:
    SpecialCard(string sColour, string type) : SColour(sColour), Type(type) {}
    string GetType() const { return Type; }
    string GetSColour() const { return SColour; }
    void setType(const string& a) { Type = a; }
    void setSColour(const string& a) { SColour = a; }
    string toString() const { return SColour + " " + Type; }
};

class Colour_Deck {
private:
    string DColour;
    vector<Card> Numbers;
    vector<SpecialCard> SC;
public:
    Colour_Deck(string D) : DColour(D) {
        for (int i = 0; i <= 9; i++) {
            Numbers.push_back(Card(D, i));
            if (i > 0) {
                Numbers.push_back(Card(D, i));
            }
        }
        for (int i = 0; i < 2; i++) {
            SC.push_back(SpecialCard(D, "Reverse"));
            SC.push_back(SpecialCard(D, "Skip"));
            SC.push_back(SpecialCard(D, "Draw Two"));
        }
    }
    vector<Card> getNumberCards() const { return Numbers; }
    vector<SpecialCard> getSpecialCards() const { return SC; }
};

class Special_Deck {
private:
    string SDColour;
    vector<SpecialCard> SD;
public:
    Special_Deck() : SDColour("Black") {
        for (int i = 0; i < 4; i++) {
            SD.push_back(SpecialCard("Black", "Wild"));
            SD.push_back(SpecialCard("Black", "Draw Four"));
            SD.push_back(SpecialCard("Black", "Draw Eight"));
            SD.push_back(SpecialCard("Black", "Ban Colour"));
        }
    }
    vector<SpecialCard> getSpecialCards() const { return SD; }
};

class Game_Deck {
private:
    vector<Card> deck;
public:
    Game_Deck() {
        Colour_Deck Red("Red");
        Colour_Deck Blue("Blue");
        Colour_Deck Green("Green");
        Colour_Deck Yellow("Yellow");
        Special_Deck Black;

        auto addCards = [this](const vector<Card>& cards) {
            deck.insert(deck.end(), cards.begin(), cards.end());
        };
        auto addSpecials = [this](const vector<SpecialCard>& specials) {
            for (const auto& sc : specials) {
                deck.push_back(Card(sc.GetSColour(), -1, sc.GetType()));
            }
        };

        addCards(Red.getNumberCards());
        addCards(Blue.getNumberCards());
        addCards(Green.getNumberCards());
        addCards(Yellow.getNumberCards());

        addSpecials(Red.getSpecialCards());
        addSpecials(Blue.getSpecialCards());
        addSpecials(Green.getSpecialCards());
        addSpecials(Yellow.getSpecialCards());
        addSpecials(Black.getSpecialCards());

        shuffleDeck();
    }

    void shuffleDeck() {
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        shuffle(deck.begin(), deck.end(), default_random_engine(seed));
    }

    Card drawCard(vector<Card>& discardPile) {
        if (deck.empty()) {
            if (discardPile.size() <= 1) {
                return Card("None", -2); // Out of cards completely
            }
            cout << "Reshuffling discard pile into deck..." << endl;
            Card top = discardPile.back();
            discardPile.pop_back();
            deck = discardPile;
            discardPile.clear();
            discardPile.push_back(top);
            shuffleDeck();
        }
        Card topCard = deck.back();
        deck.pop_back();
        return topCard;
    }

    bool isEmpty() const { return deck.empty(); }
};

class Player {
private:
    string name;
    vector<Card> hand;
public:
    Player(string n) : name(n) {}
    void addCard(const Card& c) { hand.push_back(c); }
    
    const vector<Card>& getHand() const { return hand; }
    string getName() const { return name; }
    int getHandSize() const { return hand.size(); }
    
    void displayHand() const {
        cout << name << "'s hand:\n";
        for (size_t i = 0; i < hand.size(); ++i) {
            cout << "[" << i << "] " << hand[i].toString() << "   ";
            if ((i + 1) % 4 == 0) cout << "\n";
        }
        cout << endl;
    }

    bool removeCardByIndex(int index, Card& removedCard) {
        if (index < 0 || index >= static_cast<int>(hand.size())) {
            return false;
        }
        removedCard = hand[index];
        hand.erase(hand.begin() + index);
        return true;
    }
};

class Game {
private:
    vector<Player> players;
    Game_Deck deck;
    vector<Card> discardPile;
    int currentPlayerIndex;
    string currentDirection;
    vector<string> bannedColours;

    bool isBanned(const string& colour) const {
        return find(bannedColours.begin(), bannedColours.end(), colour) != bannedColours.end();
    }

public:
    Game(int numPlayers) : currentPlayerIndex(0), currentDirection("clockwise") {
        if (numPlayers < 2 || numPlayers > 10) {
            cerr << "Number of players must be between 2 and 10." << endl;
            exit(1);
        }
        for (int i = 0; i < numPlayers; i++) {
            players.push_back(Player("Player " + to_string(i + 1)));
        }
    }

    void dealInitialCards(int numCards = 7) {
        for (int i = 0; i < numCards; i++) {
            for (auto& player : players) {
                player.addCard(deck.drawCard(discardPile));
            }
        }
        Card firstCard = deck.drawCard(discardPile);
        while (firstCard.getColour() == "Black" || firstCard.getNum() == -1) {
            discardPile.push_back(firstCard);
            firstCard = deck.drawCard(discardPile);
        }
        discardPile.push_back(firstCard);
        cout << "First card on discard pile: " << firstCard.toString() << endl;
    }

    void startGame() {
        dealInitialCards();
        while (!gameOver()) {
            playTurn();
        }
        announceWinner();
    }

    bool gameOver() const {
        for (const auto& player : players) {
            if (player.getHandSize() == 0) return true;
        }
        return false;
    }

    void announceWinner() const {
        for (const auto& player : players) {
            if (player.getHandSize() == 0) {
                cout << "\n*** " << player.getName() << " wins the game! ***" << endl;
                return;
            }
        }
    }

    int getNextPlayerIndex(int offset = 1) const {
        int n = players.size();
        if (currentDirection == "clockwise") {
            return (currentPlayerIndex + offset) % n;
        } else {
            return (currentPlayerIndex - offset + n * 10) % n;
        }
    }

    void moveToNextPlayer(int steps = 1) {
        currentPlayerIndex = getNextPlayerIndex(steps);
    }

    bool isValidPlay(const Card& card, const Card& topDiscard) const {
        if (isBanned(card.getColour())) {
            cout << "Cannot play " << card.getColour() << "; that colour is currently banned!" << endl;
            return false;
        }
        if (card.getColour() == "Black") return true;
        if (card.getColour() == topDiscard.getColour()) return true;
        if (card.getNum() != -1 && card.getNum() == topDiscard.getNum()) return true;
        if (card.getNum() == -1 && topDiscard.getNum() == -1 && card.getType() == topDiscard.getType()) return true;
        return false;
    }

    void playTurn() {
        Player& currentPlayer = players[currentPlayerIndex];
        cout << "\n--------------------------------------------------" << endl;
        cout << "It's " << currentPlayer.getName() << "'s turn. [Direction: " << currentDirection << "]" << endl;
        if (!bannedColours.empty()) {
            cout << "Banned Colours: ";
            for (const auto& c : bannedColours) cout << c << " ";
            cout << endl;
        }
        cout << "Top card: [" << discardPile.back().toString() << "]" << endl;
        
        currentPlayer.displayHand();

        bool hasValid = false;
        for (const auto& c : currentPlayer.getHand()) {
            if (isValidPlay(c, discardPile.back())) {
                hasValid = true;
                break;
            }
        }

        if (!hasValid) {
            cout << "No playable cards. Drawing a card..." << endl;
            Card drawn = deck.drawCard(discardPile);
            cout << "Drawn: " << drawn.toString() << endl;
            if (isValidPlay(drawn, discardPile.back())) {
                cout << "Playing drawn card immediately!" << endl;
                discardPile.push_back(drawn);
                applyCardEffect(drawn);
            } else {
                currentPlayer.addCard(drawn);
                moveToNextPlayer();
            }
            return;
        }

        int choice = -1;
        while (true) {
            cout << "Enter card index to play: ";
            string input;
            if (!getline(cin >> ws, input)) return;
            stringstream ss(input);
            if (ss >> choice && choice >= 0 && choice < currentPlayer.getHandSize()) {
                const Card& selected = currentPlayer.getHand()[choice];
                if (isValidPlay(selected, discardPile.back())) {
                    Card playedCard("", -2);
                    currentPlayer.removeCardByIndex(choice, playedCard);
                    discardPile.push_back(playedCard);
                    cout << currentPlayer.getName() << " played " << playedCard.toString() << endl;
                    applyCardEffect(playedCard);
                    break;
                } else {
                    cout << "Invalid move. Card does not match colour/number/type." << endl;
                }
            } else {
                cout << "Invalid index. Please select a valid number from your hand." << endl;
            }
        }
    }

    void applyCardEffect(const Card& playedCard) {
        int nextIdx = getNextPlayerIndex(1);

        if (playedCard.getColour() == "Black") {
            string newColour;
            while (true) {
                cout << "Choose new colour (Red, Blue, Green, Yellow): ";
                if (!getline(cin >> ws, newColour)) return;
                transform(newColour.begin(), newColour.end(), newColour.begin(), ::tolower);
                if (newColour == "red") { discardPile.back().setColour("Red"); break; }
                if (newColour == "blue") { discardPile.back().setColour("Blue"); break; }
                if (newColour == "green") { discardPile.back().setColour("Green"); break; }
                if (newColour == "yellow") { discardPile.back().setColour("Yellow"); break; }
                cout << "Invalid color name." << endl;
            }

            string type = playedCard.getType();
            if (type == "Draw Four") {
                cout << players[nextIdx].getName() << " draws 4 cards and loses their turn!" << endl;
                for (int i = 0; i < 4; ++i) players[nextIdx].addCard(deck.drawCard(discardPile));
                moveToNextPlayer(2);
            } else if (type == "Draw Eight") {
                cout << players[nextIdx].getName() << " draws 8 cards and loses their turn!" << endl;
                for (int i = 0; i < 8; ++i) players[nextIdx].addCard(deck.drawCard(discardPile));
                moveToNextPlayer(2);
            } else if (type == "Ban Colour") {
                cout << "Enter a colour to ban (Red, Blue, Green, Yellow): ";
                string banColour;
                getline(cin >> ws, banColour);
                banColour[0] = toupper(banColour[0]);
                bannedColours.push_back(banColour);
                cout << banColour << " is now banned!" << endl;
                moveToNextPlayer(1);
            } else {
                moveToNextPlayer(1);
            }
        } else if (playedCard.getNum() == -1) {
            string type = playedCard.getType();
            if (type == "Reverse") {
                currentDirection = (currentDirection == "clockwise") ? "counter-clockwise" : "clockwise";
                cout << "Play direction reversed!" << endl;
                if (players.size() == 2) {
                    moveToNextPlayer(2); // In 2-player UNO, Reverse acts as Skip
                } else {
                    moveToNextPlayer(1);
                }
            } else if (type == "Skip") {
                cout << players[nextIdx].getName() << " is skipped!" << endl;
                moveToNextPlayer(2);
            } else if (type == "Draw Two") {
                cout << players[nextIdx].getName() << " draws 2 cards and is skipped!" << endl;
                for (int i = 0; i < 2; ++i) players[nextIdx].addCard(deck.drawCard(discardPile));
                moveToNextPlayer(2);
            }
        } else {
            moveToNextPlayer(1);
        }
    }
};

int main() {
    int numPlayers = 0;
    while (true) {
        cout << "Enter number of players (2-10): ";
        string input;
        if (!getline(cin >> ws, input)) return 0;
        stringstream ss(input);
        if (ss >> numPlayers && numPlayers >= 2 && numPlayers <= 10) {
            break;
        }
        cout << "Invalid input. Please enter a number between 2 and 10." << endl;
    }

    Game game(numPlayers);
    game.startGame();

    return 0;
}