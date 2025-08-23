#include <iostream>
#include <iomanip>
#include <string>
#include <vector>    // For using std::vector
#include <memory>    // For using smart pointers (std::unique_ptr)
#include <limits>    // For robust input handling
#include <cmath>

// Use a namespace to keep the code organized
namespace Finance {

// A base class for all financial transactions
class Transaction {
protected:
    double amount;
    std::string description;

public:
    // Use explicit to prevent accidental type conversions
    explicit Transaction(double amt, std::string des) 
        : amount(amt), description(std::move(des)) {}

    // Virtual destructor is crucial for base classes with virtual functions
    virtual ~Transaction() = default;

    // A pure virtual function to get the type of transaction
    virtual const char* getType() const = 0;

    // A single display function, now const-correct
    void display() const {
        std::cout << std::left << std::setw(15) << getType()
                  << std::right << std::setw(10) << amount
                  << "    " << std::left << description << std::endl;
    }

    double getAmount() const { return amount; }
};

class Income : public Transaction {
public:
    explicit Income(double amt, const std::string& des) : Transaction(amt, des) {}
    const char* getType() const override { return "Income"; }
};

class Expenditure : public Transaction {
public:
    explicit Expenditure(double amt, const std::string& des) : Transaction(amt, des) {}
    const char* getType() const override { return "Expenditure"; }
};


// A base class for all investments
class Investment {
protected:
    double principal;
    int durationYears;

public:
    explicit Investment(double amt, int dur) 
        : principal(amt), durationYears(dur) {}
    
    virtual ~Investment() = default;

    virtual const char* getType() const = 0;
    virtual double getMaturityAmount() const = 0;

    virtual void display() const {
        std::cout << std::left << std::setw(15) << getType()
                  << std::right << std::setw(10) << principal
                  << std::setw(15) << durationYears << " yrs";
    }

    double getPrincipal() const { return principal; }
};

class SIP : public Investment {
private:
    double monthlyInvestment;
    // AFTER: Using a constant for the interest rate
    static constexpr double ANNUAL_RATE = 0.096; 

public:
    explicit SIP(double principalAmt, int dur, double monthlyAmt) 
        : Investment(principalAmt, dur), monthlyInvestment(monthlyAmt) {}

    const char* getType() const override { return "SIP"; }

    double getMaturityAmount() const override {
        double finalAmount = principal * pow(1 + (ANNUAL_RATE / 12), durationYears * 12);
        // A more standard formula for future value of a series
        double monthlyContributionFutureValue = monthlyInvestment * ((pow(1 + (ANNUAL_RATE / 12), durationYears * 12) - 1) / (ANNUAL_RATE / 12));
        return finalAmount + monthlyContributionFutureValue;
    }

    void display() const override {
        Investment::display();
        std::cout << std::setw(25) << " (Monthly: " << monthlyInvestment << ")" << std::endl;
    }
};

class FD : public Investment {
private:
    // AFTER: Using a constant makes the code cleaner and easier to change
    static constexpr double ANNUAL_RATE = 0.071;

public:
    explicit FD(double amt, int dur) : Investment(amt, dur) {}
    const char* getType() const override { return "Fixed Deposit"; }
    
    double getMaturityAmount() const override {
        return principal * pow((1 + ANNUAL_RATE), durationYears);
    }
    
    void display() const override {
        Investment::display();
        std::cout << std::endl;
    }
};

class FinanceManager {
private:
    // BEFORE: Transaction* transactions[100]; (Fixed size, raw pointers, unsafe)
    // AFTER: A dynamic vector of unique_ptrs (Safe, modern, flexible)
    std::vector<std::unique_ptr<Transaction>> transactions;
    std::vector<std::unique_ptr<Investment>> investments;

public:
    // No need for counters like tcount, vector.size() handles it
    FinanceManager() = default;

    // The vector now owns the Transaction pointer, no memory leaks!
    void addTransaction(std::unique_ptr<Transaction> t) {
        transactions.push_back(std::move(t));
    }

    void addInvestment(std::unique_ptr<Investment> i) {
        investments.push_back(std::move(i));
    }

    void displayTransactionHistory() const {
        std::cout << "\n--- Transaction History ---\n";
        std::cout << std::left << std::setw(15) << "Type"
                  << std::right << std::setw(10) << "Amount"
                  << "    " << std::left << "Description" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        for (const auto& t : transactions) {
            t->display();
        }
    }

    void displayInvestmentPortfolio() const {
        std::cout << "\n--- Investment Portfolio ---\n";
        std::cout << std::left << std::setw(15) << "Type"
                  << std::right << std::setw(10) << "Principal"
                  << std::setw(15) << "Duration"
                  << "  Details" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        for (const auto& i : investments) {
            i->display();
        }
    }

    void displayInvestmentProjections() const {
        std::cout << "\n--- Investment Maturity Projections ---\n";
        for (size_t i = 0; i < investments.size(); ++i) {
            const auto& inv = investments[i];
            std::cout << "Portfolio Item " << i + 1 << " (" << inv->getType() << "):\n";
            std::cout << "  Matures to: " << std::fixed << std::setprecision(2) << inv->getMaturityAmount() << " INR" << std::endl;
        }
    }
    
    // Getter methods for the User class to access transaction data
    const std::vector<std::unique_ptr<Transaction>>& getTransactions() const { return transactions; }
    const std::vector<std::unique_ptr<Investment>>& getInvestments() const { return investments; }
};

class User {
private:
    FinanceManager manager;
    double balance;
    
    // AFTER: Use a constant for the minimum balance
    static constexpr double MINIMUM_BALANCE = 1000.0;

    // Helper functions for user input
    void recordIncome();
    void recordExpenditure();
    void makeInvestment();

    // A robust function to get numeric input from the user
    template<typename T>
    T getNumericInput(const std::string& prompt) {
        T value;
        while (true) {
            std::cout << prompt;
            std::cin >> value;
            if (std::cin.good()) {
                // Clear the rest of the line to handle inputs like "50abc"
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            } else {
                std::cout << "Invalid input. Please enter a number.\n";
                std::cin.clear(); // Clear error flags
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard bad input
            }
        }
    }
    
    std::string getStringInput(const std::string& prompt) {
        std::string value;
        std::cout << prompt;
        std::getline(std::cin, value);
        return value;
    }

public:
    explicit User(double initialBalance) : balance(initialBalance) {}

    void run() {
        int choice = -1;
        while (choice != 0) {
            std::cout << "\n========= FINANCE MENU =========\n";
            std::cout << "Current Balance: " << std::fixed << std::setprecision(2) << balance << " INR\n";
            std::cout << "--------------------------------\n";
            std::cout << "1. Record Income\n";
            std::cout << "2. Record Expenditure\n";
            std::cout << "3. Make Investment\n";
            std::cout << "4. View Transaction History\n";
            std::cout << "5. View Investment Portfolio\n";
            std::cout << "6. View Investment Projections\n";
            std::cout << "0. Exit\n";
            
            choice = getNumericInput<int>("Enter choice: ");

            switch (choice) {
                case 1: recordIncome(); break;
                case 2: recordExpenditure(); break;
                case 3: makeInvestment(); break;
                case 4: manager.displayTransactionHistory(); break;
                case 5: manager.displayInvestmentPortfolio(); break;
                case 6: manager.displayInvestmentProjections(); break;
                case 0: std::cout << "Exiting. Goodbye!\n"; break;
                default: std::cout << "Invalid option. Please try again.\n"; break;
            }
        }
    }
};

// Implementation of User helper methods
void User::recordIncome() {
    double amt = getNumericInput<double>("Enter income amount: ");
    std::string desc = getStringInput("Enter description (e.g., Salary): ");
    
    balance += amt;
    manager.addTransaction(std::make_unique<Income>(amt, desc));
    std::cout << "Income recorded successfully.\n";
}

void User::recordExpenditure() {
    double amt = getNumericInput<double>("Enter expenditure amount: ");
    if (balance - amt < MINIMUM_BALANCE) {
        std::cout << "Error: Transaction declined. Balance cannot fall below " << MINIMUM_BALANCE << " INR.\n";
        return;
    }
    std::string desc = getStringInput("Enter description (e.g., Groceries): ");
    
    balance -= amt;
    manager.addTransaction(std::make_unique<Expenditure>(amt, desc));
    std::cout << "Expenditure recorded successfully.\n";
}

void User::makeInvestment() {
    std::cout << "\n--- New Investment ---\n";
    std::cout << "1. Systematic Investment Plan (SIP)\n";
    std::cout << "2. Fixed Deposit (FD)\n";
    std::cout << "0. Back to Main Menu\n";
    int choice = getNumericInput<int>("Choose investment type: ");

    if (choice == 0) return;

    double principal = getNumericInput<double>("Enter principal amount to invest: ");
    if (balance - principal < MINIMUM_BALANCE) {
        std::cout << "Error: Investment failed. Balance cannot fall below " << MINIMUM_BALANCE << " INR.\n";
        return;
    }
    int duration = getNumericInput<int>("Enter duration in years: ");

    switch (choice) {
        case 1: {
            double monthly = getNumericInput<double>("Enter monthly investment amount: ");
            manager.addInvestment(std::make_unique<SIP>(principal, duration, monthly));
            balance -= principal;
            std::cout << "SIP investment made successfully.\n";
            break;
        }
        case 2: {
            manager.addInvestment(std::make_unique<FD>(principal, duration));
            balance -= principal;
            std::cout << "FD investment made successfully.\n";
            break;
        }
        default: {
            std::cout << "Invalid investment type.\n";
            break;
        }
    }
}

} // end namespace Finance

int main() {
    std::cout << "--- Welcome to your Personal Finance Management System! ---\n";
    const double initialBalance = 5000.0;
    Finance::User user(initialBalance);
    user.run();

    return 0;
}
