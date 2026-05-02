#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <limits>
#include <sstream>
using namespace std;

// ---------- Global Functions ----------
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int getValidInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (!cin.fail()) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid input. Please enter a valid integer.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

float getValidFloat(const string& prompt) {
    float value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (!cin.fail() && value >= 0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid input. Please enter a valid positive number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

char getValidChar(const string& prompt, const string& validOptions) {
    char ch;
    while (true) {
        cout << prompt;
        cin >> ch;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (validOptions.find(tolower(ch)) != string::npos) {
            return tolower(ch);
        }
        cout << "Invalid input. Please enter one of [" << validOptions << "].\n";
    }
}

// ---------- STRUCTURES ----------
struct Account {
    string name;
    int accNo;
    float balance;
};

struct LoanRequest {
    int accNo;
    float amount;
    bool approved;
};

struct Complaint {
    int accNo;
    string message;
    bool resolved;
};

// ---------- LOAN CLASS ----------
class Loan {
public:
    string customerID;
    double amount;
    string purpose;
    double monthlyIncome;
    string status;

    void applyLoan();
    void processLoans();
    void addLoanToCustomerBalance(string customerID, double loanAmount);
};

void Loan::applyLoan() {
    cout << "\n----------- APPLY FOR LOAN -----------\n\n";
    cout << "Enter your Customer ID: ";
    getline(cin, customerID);
    amount = getValidFloat("Enter Loan Amount: ");
    cout << "Enter Loan Purpose: ";
    getline(cin, purpose);
    monthlyIncome = getValidFloat("Enter Monthly Income: ");
    status = "Pending";

    ofstream outFile("loan_requests.txt", ios::app);
    if (outFile.is_open()) {
        outFile << customerID << "\t" << amount << "\t\"" << purpose << "\"\t" << monthlyIncome << "\t" << status << "\n";
        outFile.close();
        cout << "\nLoan request submitted successfully!\n";
    } else {
        cout << "\nError saving loan request.\n";
    }
}

void Loan::processLoans() {
    ifstream inFile("loan_requests.txt");
    ofstream tempFile("temp.txt");
    if (!inFile || !tempFile) {
        cout << "\nError opening loan request file.\n";
        return;
    }

    string line;
    bool foundPending = false;

    cout << "\n--------- PROCESS LOAN REQUESTS ---------\n";

    while (getline(inFile, line)) {
        if (line.empty()) continue;

        string custID, purpose, status;
        double amount, income;

        istringstream iss(line);
        string amountStr, incomeStr;

        if (!getline(iss, custID, '\t')) continue;
        if (!getline(iss, amountStr, '\t')) continue;
        if (!getline(iss, purpose, '\t')) continue;
        if (!getline(iss, incomeStr, '\t')) continue;
        if (!getline(iss, status)) continue;

        try {
            amount = stod(amountStr);
            income = stod(incomeStr);
        } catch (...) {
            continue; // skip malformed line
        }

        if (!purpose.empty() && purpose.front() == '"' && purpose.back() == '"') {
            purpose = purpose.substr(1, purpose.size() - 2);
        }

        if (status == "Pending") {
            foundPending = true;
            cout << "\nCustomer ID: " << custID << endl;
            cout << "Loan Amount: " << amount << endl;
            cout << "Loan Purpose: " << purpose << endl;
            cout << "Monthly Income: " << income << endl;
            cout << "-------------------------------------\n";

            char decision = getValidChar("Approve or Reject this loan? (A/R): ", "ar");

            if (decision == 'a') {
                status = "Approved";
                addLoanToCustomerBalance(custID, amount);
                cout << "Loan approved and amount credited.\n";
            } else {
                status = "Rejected";
                cout << "Loan rejected.\n";
            }
            cout << "-------------------------------------\n";
        }

        tempFile << custID << "\t" << amount << "\t\"" << purpose << "\"\t" << income << "\t" << status << "\n";
    }

    inFile.close();
    tempFile.close();

    remove("loan_requests.txt");
    rename("temp.txt", "loan_requests.txt");

    if (!foundPending) {
        cout << "\nNo pending loan requests found.\n";
    }
}

void Loan::addLoanToCustomerBalance(string customerID, double loanAmount) {
    ifstream inFile("accounts.txt");
    ofstream tempFile("temp_accounts.txt");

    if (!inFile || !tempFile) {
        cout << "\nError processing accounts file.\n";
        return;
    }

    string line;
    bool found = false;

    while (getline(inFile, line)) {
        size_t nameStart = line.find("\"");
        size_t nameEnd = line.find("\"", nameStart + 1);
        if (nameStart == string::npos || nameEnd == string::npos) {
            tempFile << line << "\n";
            continue;
        }

        string name = line.substr(nameStart + 1, nameEnd - nameStart - 1);
        istringstream iss(line.substr(nameEnd + 1));
        string fileID;
        double balance;
        iss >> fileID >> balance;

        if (fileID == customerID) {
            balance += loanAmount;
            found = true;
            cout << "Loan amount of " << fixed << setprecision(2) << loanAmount << " added to customer " << name << "'s balance.\n";
        }

        tempFile << "\"" << name << "\" " << fileID << " " << fixed << setprecision(2) << balance << "\n";
    }

    inFile.close();
    tempFile.close();

    remove("accounts.txt");
    rename("temp_accounts.txt", "accounts.txt");

    if (!found) {
        cout << "Customer not found. Loan amount not credited.\n";
    }
}

// ---------- BASE CLASS ----------
class User {
public:
    virtual void login() = 0;
protected:
    void pause() {
        cout << "\nPress Enter to continue...";
        cin.get();
    }
};

// --------- Helper functions for credential handling ---------
bool readCredentials(const string& filename, string& username, string& password) {
    ifstream fin(filename);
    if (!fin) return false;
    getline(fin, username);
    getline(fin, password);
    fin.close();
    return !username.empty() && !password.empty();
}

void writeCredentials(const string& filename, const string& username, const string& password) {
    ofstream fout(filename);
    fout << username << "\n" << password << "\n";
    fout.close();
}

// ---------- ADMIN ----------
class Admin : public User {
public:
    void login() override;
private:
    void adminMenu();
    void createAccount();
    bool deleteAccount(int accNo);
    void viewAccounts();
    bool isNameExists(const string& nameToCheck);
    bool isAccountNumberExists(int accNoToCheck);
    bool verifyOrSetCredentials();
};

void Admin::login() {
    clearScreen();
    cout << "=====================================\n";
    cout << "          ADMIN LOGIN SCREEN          \n";
    cout << "=====================================\n\n";

    if (!verifyOrSetCredentials()) {
        pause();
        return;
    }

    cout << "\nLogin successful!\n";
    pause();
    adminMenu();
}

bool Admin::verifyOrSetCredentials() {
    string storedUser, storedPass;
    const string filename = "admin.txt";

    bool hasCreds = readCredentials(filename, storedUser, storedPass);

    if (!hasCreds) {
        // First time setup
        cout << "No admin credentials found. Set your admin username and password now.\n";
        cout << "Set Username: ";
        getline(cin, storedUser);
        cout << "Set Password: ";
        getline(cin, storedPass);
        writeCredentials(filename, storedUser, storedPass);
        cout << "\nAdmin credentials set successfully.\n";
        return true;
    }

    // Prompt user for login
    string inputUser, inputPass;
    cout << "Username: ";
    getline(cin, inputUser);
    cout << "Password: ";
    getline(cin, inputPass);

    if (inputUser == storedUser && inputPass == storedPass) {
        return true;
    } else {
        cout << "\nInvalid credentials!\n";
        return false;
    }
}

void Admin::adminMenu() {
    int choice;
    do {
        clearScreen();
        cout << "=====================================\n";
        cout << "            ADMIN MAIN MENU           \n";
        cout << "=====================================\n";
        cout << "1. Create Account\n";
        cout << "2. Delete Account\n";
        cout << "3. View All Accounts\n";
        cout << "4. Logout\n";
        cout << "=====================================\n";
        choice = getValidInt("Enter choice: ");

        switch (choice) {
            case 1:
                createAccount();
                break;
            case 2: {
                int accNo = getValidInt("Enter account number to delete: ");
                if (deleteAccount(accNo)) {
                    cout << "\nAccount deleted successfully.\n";
                } else {
                    cout << "\nAccount not found.\n";
                }
                pause();
                break;
            }
            case 3:
                viewAccounts();
                break;
            case 4:
                cout << "\nLogging out from Admin...\n";
                pause();
                break;
            default:
                cout << "Invalid choice!\n";
                pause();
                break;
        }
    } while (choice != 4);
}

bool Admin::isNameExists(const string& nameToCheck) {
    ifstream fin("accounts.txt");
    if (!fin) return false;

    string line;
    while (getline(fin, line)) {
        size_t nameStart = line.find("\"");
        size_t nameEnd = line.find("\"", nameStart + 1);
        if (nameStart == string::npos || nameEnd == string::npos) continue;

        string existingName = line.substr(nameStart + 1, nameEnd - nameStart - 1);

        if (existingName == nameToCheck) {
            fin.close();
            return true;
        }
    }
    fin.close();
    return false;
}

bool Admin::isAccountNumberExists(int accNoToCheck) {
    ifstream fin("accounts.txt");
    if (!fin) return false;

    string line;
    while (getline(fin, line)) {
        size_t nameStart = line.find("\"");
        size_t nameEnd = line.find("\"", nameStart + 1);
        if (nameStart == string::npos || nameEnd == string::npos) continue;

        istringstream iss(line.substr(nameEnd + 1));
        int existingAccNo;
        iss >> existingAccNo;

        if (existingAccNo == accNoToCheck) {
            fin.close();
            return true;
        }
    }
    fin.close();
    return false;
}

void Admin::createAccount() {
    Account acc;
    ofstream fout("accounts.txt", ios::app);
    clearScreen();

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "--------- CREATE NEW ACCOUNT ---------\n\n";

    while (true) {
        cout << "Enter Name: ";
        getline(cin, acc.name);

        if (isNameExists(acc.name)) {
            cout << "This name already exists. Please enter a different name.\n";
        } else {
            break;
        }
    }

    while (true) {
        acc.accNo = getValidInt("Enter Account Number: ");
        if (isAccountNumberExists(acc.accNo)) {
            cout << "This account number already exists. Please enter a different account number.\n";
        } else {
            break;
        }
    }

    acc.balance = getValidFloat("Enter Initial Balance: ");

    if (fout.is_open()) {
        fout << "\"" << acc.name << "\" " << acc.accNo << " " << fixed << setprecision(2) << acc.balance << endl;
        fout.close();
        cout << "\nAccount created successfully!\n";
    } else {
        cout << "\nFailed to open accounts file.\n";
    }
    pause();
}

bool Admin::deleteAccount(int accNo) {
    ifstream fin("accounts.txt");
    ofstream fout("temp.txt");
    string line;
    bool deleted = false;

    if (!fin || !fout) {
        cout << "\nError opening accounts file.\n";
        return false;
    }

    while (getline(fin, line)) {
        size_t nameStart = line.find("\"");
        size_t nameEnd = line.find("\"", nameStart + 1);
        if (nameStart == string::npos || nameEnd == string::npos) {
            fout << line << endl;
            continue;
        }

        string name = line.substr(nameStart + 1, nameEnd - nameStart - 1);
        istringstream iss(line.substr(nameEnd + 1));
        int fileAccNo;
        float balance;
        iss >> fileAccNo >> balance;

        if (fileAccNo == accNo) {
            deleted = true; // skip writing this line
        } else {
            fout << "\"" << name << "\" " << fileAccNo << " " << fixed << setprecision(2) << balance << endl;
        }
    }

    fin.close();
    fout.close();

    remove("accounts.txt");
    rename("temp.txt", "accounts.txt");

    return deleted;
}

void Admin::viewAccounts() {
    ifstream fin("accounts.txt");
    if (!fin) {
        cout << "\nError opening accounts file.\n";
        pause();
        return;
    }
    Account acc;
    clearScreen();
    cout << "---------------------------- ACCOUNTS LIST ----------------------------\n";
    cout << "| " << left << setw(20) << "Name"
         << "| " << setw(12) << "Acc No"
         << "| " << setw(12) << "Balance" << "|\n";
    cout << "------------------------------------------------------------------------\n";

    string line;
    while (getline(fin, line)) {
        size_t nameStart = line.find("\"");
        size_t nameEnd = line.find("\"", nameStart + 1);
        if (nameStart == string::npos || nameEnd == string::npos) continue;

        acc.name = line.substr(nameStart + 1, nameEnd - nameStart - 1);

        istringstream iss(line.substr(nameEnd + 1));
        iss >> acc.accNo >> acc.balance;

        cout << "| " << left << setw(20) << acc.name
             << "| " << setw(12) << acc.accNo
             << "| " << setw(12) << fixed << setprecision(2) << acc.balance << "|\n";
    }

    cout << "------------------------------------------------------------------------\n";
    fin.close();
    pause();
}

// ---------- CUSTOMER ----------
class Customer : public User {
public:
    void login() override;
private:
    int accNo;
    bool accountExists(int accNo);
    void customerMenu();
    bool updateBalance(int accNo, float newBalance);
    void depositMoney();
    void withdrawMoney();
    void checkBalance();
    void submitComplaint();
    float getBalance();
    void pause();
};

void Customer::login() {
    clearScreen();
    cout << "=====================================\n";
    cout << "           CUSTOMER LOGIN SCREEN       \n";
    cout << "=====================================\n\n";

    accNo = getValidInt("Enter your Account Number to login: ");

    if (accountExists(accNo)) {
        cout << "\nLogin successful!\n";
        pause();
        customerMenu();
    } else {
        cout << "\nAccount not found!\n";
        pause();
    }
}

bool Customer::accountExists(int accNo) {
    ifstream fin("accounts.txt");
    string line;
    while (getline(fin, line)) {
        size_t nameStart = line.find("\"");
        size_t nameEnd = line.find("\"", nameStart + 1);
        if (nameStart == string::npos || nameEnd == string::npos) continue;

        istringstream iss(line.substr(nameEnd + 1));
        int fileAccNo;
        float balance;
        iss >> fileAccNo >> balance;

        if (fileAccNo == accNo) {
            fin.close();
            return true;
        }
    }
    fin.close();
    return false;
}

void Customer::customerMenu() {
    int choice;
    do {
        clearScreen();
        cout << "=====================================\n";
        cout << "           CUSTOMER MAIN MENU          \n";
        cout << "=====================================\n";
        cout << "1. Deposit Money\n";
        cout << "2. Withdraw Money\n";
        cout << "3. Check Balance\n";
        cout << "4. Apply for Loan\n";
        cout << "5. Submit Complaint\n";
        cout << "6. Logout\n";
        cout << "=====================================\n";
        choice = getValidInt("Enter choice: ");

        switch (choice) {
            case 1: depositMoney(); break;
            case 2: withdrawMoney(); break;
            case 3: checkBalance(); break;
            case 4: {
                Loan loan;
                loan.applyLoan();
                pause();
                break;
            }
            case 5: submitComplaint(); break;
            case 6:
                cout << "\nLogging out from Customer...\n";
                pause();
                break;
            default:
                cout << "Invalid choice!\n";
                pause();
                break;
        }
    } while (choice != 6);
}

bool Customer::updateBalance(int accNo, float newBalance) {
    ifstream fin("accounts.txt");
    ofstream fout("temp.txt");
    string line;
    bool updated = false;

    if (!fin || !fout) {
        cout << "\nError opening accounts file.\n";
        return false;
    }

    while (getline(fin, line)) {
        size_t nameStart = line.find("\"");
        size_t nameEnd = line.find("\"", nameStart + 1);
        if (nameStart == string::npos || nameEnd == string::npos) {
            fout << line << endl; // Copy malformed lines as-is
            continue;
        }

        string name = line.substr(nameStart + 1, nameEnd - nameStart - 1);
        istringstream iss(line.substr(nameEnd + 1));
        int fileAccNo;
        float balance;
        iss >> fileAccNo >> balance;

        if (fileAccNo == accNo) {
            fout << "\"" << name << "\" " << fileAccNo << " " << fixed << setprecision(2) << newBalance << endl;
            updated = true;
        } else {
            fout << "\"" << name << "\" " << fileAccNo << " " << fixed << setprecision(2) << balance << endl;
        }
    }

    fin.close();
    fout.close();

    remove("accounts.txt");
    rename("temp.txt", "accounts.txt");

    return updated;
}

void Customer::depositMoney() {
    float amount = getValidFloat("\nEnter amount to deposit: ");
    float currentBalance = getBalance();
    float newBalance = currentBalance + amount;
    if (updateBalance(accNo, newBalance)) {
        cout << "\nAmount deposited successfully. New balance: " << fixed << setprecision(2) << newBalance << endl;
    } else {
        cout << "\nFailed to update balance.\n";
    }
    pause();
}

void Customer::withdrawMoney() {
    float amount = getValidFloat("\nEnter amount to withdraw: ");
    float currentBalance = getBalance();
    if (amount > currentBalance) {
        cout << "\nInsufficient balance.\n";
    } else {
        float newBalance = currentBalance - amount;
        if (updateBalance(accNo, newBalance)) {
            cout << "\nAmount withdrawn successfully. New balance: " << fixed << setprecision(2) << newBalance << endl;
        } else {
            cout << "\nFailed to update balance.\n";
        }
    }
    pause();
}

void Customer::checkBalance() {
    ifstream fin("accounts.txt");
    string line;
    bool found = false;
    while (getline(fin, line)) {
        size_t nameStart = line.find("\"");
        size_t nameEnd = line.find("\"", nameStart + 1);
        if (nameStart == string::npos || nameEnd == string::npos) continue;

        istringstream iss(line.substr(nameEnd + 1));
        int fileAccNo;
        float balance;
        iss >> fileAccNo >> balance;

        if (fileAccNo == accNo) {
            cout << "\nYour current balance is: " << fixed << setprecision(2) << balance << endl;
            found = true;
            break;
        }
    }
    fin.close();
    if (!found) {
        cout << "\nAccount balance not found.\n";
    }
    pause();
}

void Customer::submitComplaint() {
    cout << "\nEnter your complaint: ";
    string complaint;
    getline(cin, complaint);
    ofstream fout("complaints.txt", ios::app);
    if (!fout) {
        cout << "\nError opening complaints file.\n";
        pause();
        return;
    }
    fout << accNo << "\t" << complaint << "\t0\n"; // 0 for unresolved
    fout.close();
    cout << "\nComplaint submitted.\n";
    pause();
}

float Customer::getBalance() {
    ifstream fin("accounts.txt");
    string line;
    while (getline(fin, line)) {
        size_t nameStart = line.find("\"");
        size_t nameEnd = line.find("\"", nameStart + 1);
        if (nameStart == string::npos || nameEnd == string::npos) continue;

        istringstream iss(line.substr(nameEnd + 1));
        int fileAccNo;
        float balance;
        iss >> fileAccNo >> balance;

        if (fileAccNo == accNo) {
            return balance;
        }
    }
    return 0.0f;
}

void Customer::pause() {
    cout << "\nPress Enter to continue...";
    cin.get();
}

// ---------- EMPLOYEE ----------
class Employee : public User {
public:
    void login() override;
private:
    void menu();
    void viewComplaints();
    void resolveComplaints();
    void pause();
    bool verifyOrSetCredentials();
};


void Employee::login() {
    clearScreen();
    cout << "=====================================\n";
    cout << "         EMPLOYEE LOGIN SCREEN        \n";
    cout << "=====================================\n\n";

    if (!verifyOrSetCredentials()) {
        pause();
        return;
    }

    cout << "\nLogin successful!\n";
    pause();
    menu();
}

bool Employee::verifyOrSetCredentials() {
    string storedUser, storedPass;
    const string filename = "employee.txt";

    bool hasCreds = readCredentials(filename, storedUser, storedPass);

    if (!hasCreds) {
        cout << "No employee credentials found. Set your employee username and password now.\n";
        cout << "Set Username: ";
        getline(cin, storedUser);
        cout << "Set Password: ";
        getline(cin, storedPass);
        writeCredentials(filename, storedUser, storedPass);
        cout << "\nEmployee credentials set successfully.\n";
        return true;
    }

    string inputUser, inputPass;
    cout << "Username: ";
    getline(cin, inputUser);
    cout << "Password: ";
    getline(cin, inputPass);

    if (inputUser == storedUser && inputPass == storedPass) {
        return true;
    } else {
        cout << "\nInvalid credentials!\n";
        return false;
    }
}

void Employee::menu() {
    int choice;
    do {
        clearScreen();
        cout << "=====================================\n";
        cout << "          EMPLOYEE MAIN MENU          \n";
        cout << "=====================================\n";
        cout << "1. View Complaints\n";
        cout << "2. Resolve Complaints\n";
        cout << "3. Process Loan Requests\n";
        cout << "4. Logout\n";
        cout << "=====================================\n";
        choice = getValidInt("Enter choice: ");

        switch (choice) {
            case 1:
                viewComplaints();
                break;
            case 2:
                resolveComplaints();
                break;
            case 3: {
                Loan loan;
                loan.processLoans();
                pause();
                break;
            }
            case 4:
                cout << "\nLogging out from Employee...\n";
                pause();
                break;
            default:
                cout << "\nInvalid choice.\n";
                pause();
                break;
        }
    } while (choice != 4);
}

void Employee::viewComplaints() {
    ifstream fin("complaints.txt");
    if (!fin) {
        cout << "\nError opening complaints file.\n";
        pause();
        return;
    }

    string line;
    clearScreen();
    cout << "=====================================\n";
    cout << "             COMPLAINTS LIST          \n";
    cout << "=====================================\n";

    while (getline(fin, line)) {
        istringstream iss(line);
        string accNoStr, complaintMsg, resolvedStr;

        if (!getline(iss, accNoStr, '\t')) continue;
        if (!getline(iss, complaintMsg, '\t')) continue;
        if (!getline(iss, resolvedStr)) continue;

        int accNo = stoi(accNoStr);
        int resolved = stoi(resolvedStr);

        cout << "Account: " << accNo << " | Complaint: " << complaintMsg
             << (resolved ? " [Resolved]" : " [Pending]") << endl;
    }
    fin.close();
    pause();
}

void Employee::resolveComplaints() {
    ifstream fin("complaints.txt");
    ofstream fout("temp.txt");

    if (!fin || !fout) {
        cout << "\nError opening complaints file.\n";
        pause();
        return;
    }

    string line;
    clearScreen();
    cout << "=====================================\n";
    cout << "           RESOLVING COMPLAINTS       \n";
    cout << "=====================================\n";

    while (getline(fin, line)) {
        istringstream iss(line);
        string accNoStr, complaintMsg, resolvedStr;

        if (!getline(iss, accNoStr, '\t')) continue;
        if (!getline(iss, complaintMsg, '\t')) continue;
        if (!getline(iss, resolvedStr)) continue;

        int accNo = stoi(accNoStr);
        int resolved = stoi(resolvedStr);

        if (!resolved) {
            cout << "Resolving complaint for account: " << accNo << endl;
            resolved = 1;
        }

        fout << accNo << "\t" << complaintMsg << "\t" << resolved << "\n";
    }

    fin.close();
    fout.close();

    remove("complaints.txt");
    rename("temp.txt", "complaints.txt");
    pause();
}

void Employee::pause() {
    cout << "\nPress Enter to continue...";
    cin.get();
}

// ---------- MAIN ----------
int main() {
    int choice;
    Admin admin;
    Customer customer;
    Employee employee;

    clearScreen();
    cout << "=====================================\n";
    cout << "         WELCOME TO BANK SYSTEM       \n";
    cout << "=====================================\n";
    cout << "Press Enter to start...";
    cin.get();

    do {
        clearScreen();
        cout << "=====================================\n";
        cout << "         BANK MANAGEMENT SYSTEM       \n";
        cout << "=====================================\n";
        cout << "1. Admin Login\n";
        cout << "2. Customer Login\n";
        cout << "3. Employee Login\n";
        cout << "4. Exit\n";
        cout << "=====================================\n";
        choice = getValidInt("Enter your choice: ");

        switch (choice) {
            case 1:
                admin.login();
                break;
            case 2:
                customer.login();
                break;
            case 3:
                employee.login();
                break;
            case 4:
                cout << "\nThank you for using the Bank Management System.\n";
                cout << "Goodbye!\n\n";
                break;
            default:
                cout << "\nInvalid option!\n";
                cout << "Press Enter to continue...";
                cin.get();
                break;
        }
    } while (choice != 4);

    return 0;
}
