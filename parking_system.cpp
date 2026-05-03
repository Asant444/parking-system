/*
 * ============================================================
 *  PARKING MANAGEMENT SYSTEM
 *  Author : Tinsae Ayalew (github.com/Asant444)
 *  Language: C++
 *  Description: A console-based parking management system
 *               that handles vehicle entry, exit, billing,
 *               and slot tracking.
 * ============================================================
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <limits>

using namespace std;

// ─────────────────────────────────────────────
//  CONSTANTS
// ─────────────────────────────────────────────
const int TOTAL_SLOTS    = 20;
const int SMALL_SLOTS    = 8;   // slots 1–8   (motorcycles/small)
const int MEDIUM_SLOTS   = 8;   // slots 9–16  (cars)
const int LARGE_SLOTS    = 4;   // slots 17–20 (trucks/buses)

const double RATE_SMALL  = 10.0;  // ETB per hour
const double RATE_MEDIUM = 20.0;
const double RATE_LARGE  = 35.0;

// ─────────────────────────────────────────────
//  ENUMS
// ─────────────────────────────────────────────
enum VehicleType { SMALL, MEDIUM, LARGE };

string vehicleTypeName(VehicleType t) {
    switch(t) {
        case SMALL:  return "Motorcycle / Small Car";
        case MEDIUM: return "Car / SUV";
        case LARGE:  return "Truck / Bus";
        default:     return "Unknown";
    }
}

// ─────────────────────────────────────────────
//  STRUCTS
// ─────────────────────────────────────────────
struct Vehicle {
    string plateNumber;
    string ownerName;
    VehicleType type;
    int slotNumber;
    time_t entryTime;
};

// ─────────────────────────────────────────────
//  GLOBALS
// ─────────────────────────────────────────────
vector<Vehicle> parkedVehicles;
bool slots[TOTAL_SLOTS + 1] = {false}; // index 1–20

// ─────────────────────────────────────────────
//  UTILITY FUNCTIONS
// ─────────────────────────────────────────────
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void drawLine(char ch = '─', int len = 60) {
    cout << string(len, ch) << "\n";
}

void printHeader() {
    cout << "\n";
    drawLine('═');
    cout << setw(38) << "  ██████  PARKING SYSTEM  ██████\n";
    cout << setw(38) << "  Tinsae Ayalew · github.com/Asant444\n";
    drawLine('═');
    cout << "\n";
}

string formatTime(time_t t) {
    char buf[32];
    struct tm* tm_info = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return string(buf);
}

double calcHours(time_t entry, time_t exit_t) {
    double secs = difftime(exit_t, entry);
    return max(secs / 3600.0, 0.5); // minimum 0.5 hour charge
}

double getRate(VehicleType t) {
    switch(t) {
        case SMALL:  return RATE_SMALL;
        case MEDIUM: return RATE_MEDIUM;
        case LARGE:  return RATE_LARGE;
        default:     return RATE_MEDIUM;
    }
}

// Find first available slot for given vehicle type
int findSlot(VehicleType type) {
    int start, end;
    switch(type) {
        case SMALL:  start=1;  end=SMALL_SLOTS;           break;
        case MEDIUM: start=9;  end=9+MEDIUM_SLOTS-1;      break;
        case LARGE:  start=17; end=17+LARGE_SLOTS-1;      break;
        default:     return -1;
    }
    for (int i = start; i <= end; i++) {
        if (!slots[i]) return i;
    }
    return -1; // no slot available
}

Vehicle* findVehicle(const string& plate) {
    for (auto& v : parkedVehicles)
        if (v.plateNumber == plate) return &v;
    return nullptr;
}

void toUpper(string& s) {
    transform(s.begin(), s.end(), s.begin(), ::toupper);
}

// ─────────────────────────────────────────────
//  SLOT DISPLAY
// ─────────────────────────────────────────────
void displaySlots() {
    cout << "\n  PARKING LOT STATUS\n";
    drawLine();
    cout << "  [■] = Occupied   [□] = Available\n\n";

    cout << "  SMALL (1–8) — ETB " << RATE_SMALL << "/hr\n  ";
    for (int i = 1; i <= SMALL_SLOTS; i++)
        cout << "[" << (slots[i] ? "■" : "□") << " " << setw(2) << i << "] ";
    cout << "\n\n";

    cout << "  MEDIUM (9–16) — ETB " << RATE_MEDIUM << "/hr\n  ";
    for (int i = 9; i <= 9+MEDIUM_SLOTS-1; i++)
        cout << "[" << (slots[i] ? "■" : "□") << setw(2) << i << "] ";
    cout << "\n\n";

    cout << "  LARGE (17–20) — ETB " << RATE_LARGE << "/hr\n  ";
    for (int i = 17; i <= 17+LARGE_SLOTS-1; i++)
        cout << "[" << (slots[i] ? "■" : "□") << setw(2) << i << "] ";
    cout << "\n";
    drawLine();

    int occupied = parkedVehicles.size();
    cout << "  Total: " << TOTAL_SLOTS << " slots | "
         << "Occupied: " << occupied << " | "
         << "Free: " << (TOTAL_SLOTS - occupied) << "\n\n";
}

// ─────────────────────────────────────────────
//  VEHICLE ENTRY
// ─────────────────────────────────────────────
void vehicleEntry() {
    cout << "\n  ── VEHICLE ENTRY ──────────────────────\n\n";

    string plate, owner;
    int typeChoice;

    cout << "  Plate Number  : ";
    cin >> plate; toUpper(plate);

    if (findVehicle(plate)) {
        cout << "\n  ⚠  Vehicle " << plate << " is already parked!\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "  Owner Name    : ";
    getline(cin, owner);

    cout << "\n  Vehicle Type:\n";
    cout << "    1. Motorcycle / Small Car  (ETB " << RATE_SMALL  << "/hr)\n";
    cout << "    2. Car / SUV               (ETB " << RATE_MEDIUM << "/hr)\n";
    cout << "    3. Truck / Bus             (ETB " << RATE_LARGE  << "/hr)\n";
    cout << "  Choice: ";
    cin >> typeChoice;

    if (typeChoice < 1 || typeChoice > 3) {
        cout << "  Invalid type.\n"; return;
    }
    VehicleType type = (VehicleType)(typeChoice - 1);

    int slot = findSlot(type);
    if (slot == -1) {
        cout << "\n  ⚠  No available slots for " << vehicleTypeName(type) << "!\n";
        return;
    }

    Vehicle v;
    v.plateNumber = plate;
    v.ownerName   = owner;
    v.type        = type;
    v.slotNumber  = slot;
    v.entryTime   = time(nullptr);

    slots[slot] = true;
    parkedVehicles.push_back(v);

    cout << "\n  ✔  Vehicle Parked Successfully!\n";
    drawLine('-', 40);
    cout << "  Plate  : " << plate       << "\n";
    cout << "  Owner  : " << owner       << "\n";
    cout << "  Type   : " << vehicleTypeName(type) << "\n";
    cout << "  Slot   : " << slot        << "\n";
    cout << "  Entry  : " << formatTime(v.entryTime) << "\n";
    drawLine('-', 40);
}

// ─────────────────────────────────────────────
//  VEHICLE EXIT
// ─────────────────────────────────────────────
void vehicleExit() {
    cout << "\n  ── VEHICLE EXIT ───────────────────────\n\n";

    string plate;
    cout << "  Enter Plate Number: ";
    cin >> plate; toUpper(plate);

    Vehicle* v = findVehicle(plate);
    if (!v) {
        cout << "\n  ⚠  Vehicle " << plate << " not found in parking lot.\n";
        return;
    }

    time_t exitTime = time(nullptr);
    double hours    = calcHours(v->entryTime, exitTime);
    double fee      = hours * getRate(v->type);

    cout << "\n  ✔  Exit Receipt\n";
    drawLine('─', 44);
    cout << "  Plate     : " << v->plateNumber << "\n";
    cout << "  Owner     : " << v->ownerName   << "\n";
    cout << "  Type      : " << vehicleTypeName(v->type) << "\n";
    cout << "  Slot      : " << v->slotNumber  << "\n";
    cout << "  Entry     : " << formatTime(v->entryTime) << "\n";
    cout << "  Exit      : " << formatTime(exitTime)     << "\n";
    cout << fixed << setprecision(2);
    cout << "  Duration  : " << hours << " hr(s)\n";
    cout << "  Rate      : ETB " << getRate(v->type) << "/hr\n";
    drawLine('─', 44);
    cout << "  TOTAL FEE : ETB " << fee << "\n";
    drawLine('─', 44);

    slots[v->slotNumber] = false;
    parkedVehicles.erase(
        remove_if(parkedVehicles.begin(), parkedVehicles.end(),
            [&](const Vehicle& x){ return x.plateNumber == plate; }),
        parkedVehicles.end());

    cout << "\n  Slot " << v->slotNumber << " is now free. Thank you!\n";
}

// ─────────────────────────────────────────────
//  LIST ALL PARKED VEHICLES
// ─────────────────────────────────────────────
void listVehicles() {
    cout << "\n  ── PARKED VEHICLES ────────────────────\n\n";
    if (parkedVehicles.empty()) {
        cout << "  No vehicles currently parked.\n\n";
        return;
    }
    cout << left
         << setw(12) << "Plate"
         << setw(18) << "Owner"
         << setw(10) << "Type"
         << setw(6)  << "Slot"
         << "Entry Time\n";
    drawLine('-', 60);
    for (const auto& v : parkedVehicles) {
        string typeShort = (v.type==SMALL?"Small":(v.type==MEDIUM?"Medium":"Large"));
        cout << left
             << setw(12) << v.plateNumber
             << setw(18) << v.ownerName.substr(0,16)
             << setw(10) << typeShort
             << setw(6)  << v.slotNumber
             << formatTime(v.entryTime) << "\n";
    }
    drawLine('-', 60);
    cout << "  " << parkedVehicles.size() << " vehicle(s) parked.\n\n";
}

// ─────────────────────────────────────────────
//  SEARCH
// ─────────────────────────────────────────────
void searchVehicle() {
    cout << "\n  ── SEARCH VEHICLE ─────────────────────\n\n";
    string plate;
    cout << "  Enter Plate Number: ";
    cin >> plate; toUpper(plate);

    Vehicle* v = findVehicle(plate);
    if (!v) { cout << "\n  ⚠  Vehicle not found.\n\n"; return; }

    time_t now = time(nullptr);
    double hours = calcHours(v->entryTime, now);
    double fee   = hours * getRate(v->type);

    cout << "\n  Vehicle Found:\n";
    drawLine('-', 44);
    cout << "  Plate    : " << v->plateNumber << "\n";
    cout << "  Owner    : " << v->ownerName   << "\n";
    cout << "  Type     : " << vehicleTypeName(v->type) << "\n";
    cout << "  Slot     : " << v->slotNumber  << "\n";
    cout << "  Entry    : " << formatTime(v->entryTime) << "\n";
    cout << fixed << setprecision(2);
    cout << "  So far   : " << hours << " hr(s) → ETB " << fee << "\n";
    drawLine('-', 44);
    cout << "\n";
}

// ─────────────────────────────────────────────
//  MAIN MENU
// ─────────────────────────────────────────────
int main() {
    int choice;
    do {
        clearScreen();
        printHeader();
        displaySlots();

        cout << "  MENU\n";
        drawLine('-', 40);
        cout << "  1. Vehicle Entry\n";
        cout << "  2. Vehicle Exit & Billing\n";
        cout << "  3. List All Parked Vehicles\n";
        cout << "  4. Search Vehicle\n";
        cout << "  0. Exit System\n";
        drawLine('-', 40);
        cout << "  Choice: ";
        cin >> choice;

        switch (choice) {
            case 1: vehicleEntry();   break;
            case 2: vehicleExit();    break;
            case 3: listVehicles();   break;
            case 4: searchVehicle();  break;
            case 0: cout << "\n  Goodbye! — Tinsae Ayalew\n\n"; break;
            default: cout << "\n  Invalid option.\n";
        }

        if (choice != 0) {
            cout << "\n  Press Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }

    } while (choice != 0);

    return 0;
}
