/*
 * ============================================================
 *   Ethiopian Telecom Customer Service System
 *   Language  : C++17
 *   Structure : Country → Region → City → Branch → Customer
 * ============================================================
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <algorithm>
#include <vector>

using namespace std;

// ─────────────────────────────────────────────
//  Data Structures
// ─────────────────────────────────────────────

struct Customer {
    string  nameOrID;
    double  mobileBalance;   // ETB
    double  dataUsageGB;     // GB / month
    double  callMinutes;     // minutes / month
    int     smsCount;        // messages / month
};

struct Branch {
    string              name;
    vector<Customer>    customers;
};

struct City {
    string          name;
    vector<Branch>  branches;
};

struct Region {
    string      name;
    vector<City> cities;
};

// ─────────────────────────────────────────────
//  Input Validation Helpers
// ─────────────────────────────────────────────

// Read a positive integer (>= minVal) with an optional prompt.
int readPositiveInt(const string& prompt, int minVal = 1) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minVal) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  [!] Invalid input. Please enter an integer >= " << minVal << ".\n";
    }
}

// Read a non-negative double.
double readNonNegDouble(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= 0.0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  [!] Invalid input. Please enter a non-negative number.\n";
    }
}

// Read a non-negative integer (for SMS).
int readNonNegInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= 0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  [!] Invalid input. Please enter a non-negative integer.\n";
    }
}

// Read a non-empty string.
string readString(const string& prompt) {
    string s;
    while (true) {
        cout << prompt;
        getline(cin, s);
        if (!s.empty()) return s;
        cout << "  [!] Name/ID cannot be empty.\n";
    }
}

// ─────────────────────────────────────────────
//  Separator / Display Helpers
// ─────────────────────────────────────────────

void printLine(char ch = '=', int width = 70) {
    cout << string(width, ch) << "\n";
}

void printHeader(const string& title) {
    printLine();
    int pad = max(0, (70 - (int)title.size()) / 2);
    cout << string(pad, ' ') << title << "\n";
    printLine();
}

// ─────────────────────────────────────────────
//  Recursive Count Functions
// ─────────────────────────────────────────────

// Returns total customers in a branch (base case).
int countCustomersInBranch(const Branch& b) {
    return (int)b.customers.size();
}

// Recursively sums customers across branches in a city.
int countCustomersInCity(const vector<Branch>& branches, int idx = 0) {
    if (idx >= (int)branches.size()) return 0;
    return countCustomersInBranch(branches[idx])
         + countCustomersInCity(branches, idx + 1);
}

// Recursively sums customers across cities in a region.
int countCustomersInRegion(const vector<City>& cities, int idx = 0) {
    if (idx >= (int)cities.size()) return 0;
    return countCustomersInCity(cities[idx].branches)
         + countCustomersInRegion(cities, idx + 1);
}

// Recursively sums customers nationwide.
int countCustomersNationwide(const vector<Region>& regions, int idx = 0) {
    if (idx >= (int)regions.size()) return 0;
    return countCustomersInRegion(regions[idx].cities)
         + countCustomersNationwide(regions, idx + 1);
}

// ─────────────────────────────────────────────
//  Recursive Usage Aggregation
// ─────────────────────────────────────────────

struct UsageTotals {
    double dataGB    = 0;
    double callMins  = 0;
    long long sms    = 0;
};

UsageTotals branchUsage(const Branch& b) {
    UsageTotals t;
    for (auto& c : b.customers) {
        t.dataGB   += c.dataUsageGB;
        t.callMins += c.callMinutes;
        t.sms      += c.smsCount;
    }
    return t;
}

UsageTotals cityUsage(const vector<Branch>& branches, int idx = 0) {
    if (idx >= (int)branches.size()) return {};
    UsageTotals a = branchUsage(branches[idx]);
    UsageTotals b = cityUsage(branches, idx + 1);
    return { a.dataGB + b.dataGB, a.callMins + b.callMins, a.sms + b.sms };
}

UsageTotals regionUsage(const vector<City>& cities, int idx = 0) {
    if (idx >= (int)cities.size()) return {};
    UsageTotals a = cityUsage(cities[idx].branches);
    UsageTotals b = regionUsage(cities, idx + 1);
    return { a.dataGB + b.dataGB, a.callMins + b.callMins, a.sms + b.sms };
}

UsageTotals nationUsage(const vector<Region>& regions, int idx = 0) {
    if (idx >= (int)regions.size()) return {};
    UsageTotals a = regionUsage(regions[idx].cities);
    UsageTotals b = nationUsage(regions, idx + 1);
    return { a.dataGB + b.dataGB, a.callMins + b.callMins, a.sms + b.sms };
}

// ─────────────────────────────────────────────
//  Identifier Functions
// ─────────────────────────────────────────────

// Customer with highest mobile balance in a branch.
const Customer* topCustomerByBalance(const Branch& b) {
    if (b.customers.empty()) return nullptr;
    const Customer* top = &b.customers[0];
    for (auto& c : b.customers)
        if (c.mobileBalance > top->mobileBalance) top = &c;
    return top;
}

// Branch with highest customer count in a city.
int topBranchIdxInCity(const City& city) {
    int best = 0;
    for (int i = 1; i < (int)city.branches.size(); ++i)
        if (city.branches[i].customers.size() > city.branches[best].customers.size())
            best = i;
    return best;
}

// City with highest total service usage (data+call weight) in a region.
int topCityIdxInRegion(const Region& region) {
    int best = 0;
    UsageTotals bestU = cityUsage(region.cities[0].branches);
    double bestScore = bestU.dataGB + bestU.callMins / 60.0;
    for (int i = 1; i < (int)region.cities.size(); ++i) {
        UsageTotals u = cityUsage(region.cities[i].branches);
        double score = u.dataGB + u.callMins / 60.0;
        if (score > bestScore) { bestScore = score; best = i; }
    }
    return best;
}

// Region with highest customer count.
int topRegionIdx(const vector<Region>& regions) {
    int best = 0;
    int bestCount = countCustomersInRegion(regions[0].cities);
    for (int i = 1; i < (int)regions.size(); ++i) {
        int cnt = countCustomersInRegion(regions[i].cities);
        if (cnt > bestCount) { bestCount = cnt; best = i; }
    }
    return best;
}

// ─────────────────────────────────────────────
//  Data Entry
// ─────────────────────────────────────────────

Customer enterCustomer(int /*branchNum*/, int custNum) {
    cout << "\n      -- Customer " << custNum << " --\n";
    Customer c;
    c.nameOrID      = readString      ("      Name / ID          : ");
    c.mobileBalance = readNonNegDouble("      Mobile Balance (ETB): ");
    c.dataUsageGB   = readNonNegDouble("      Data Usage (GB)     : ");
    c.callMinutes   = readNonNegDouble("      Call Minutes        : ");
    c.smsCount      = readNonNegInt   ("      SMS Count           : ");
    return c;
}

Branch enterBranch(int /*cityNum*/, int branchNum) {
    cout << "\n    -- Branch " << branchNum << " --\n";
    Branch b;
    b.name = readString("    Branch Name : ");
    int n  = readPositiveInt("    Number of Customers in this Branch: ");
    for (int i = 1; i <= n; ++i)
        b.customers.push_back(enterCustomer(branchNum, i));
    return b;
}

City enterCity(int /*regionNum*/, int cityNum) {
    cout << "\n  -- City " << cityNum << " --\n";
    City city;
    city.name = readString("  City Name : ");
    int n     = readPositiveInt("  Number of Branches in this City: ");
    for (int i = 1; i <= n; ++i)
        city.branches.push_back(enterBranch(cityNum, i));
    return city;
}

Region enterRegion(int regionNum) {
    cout << "\n-- Region " << regionNum << " --\n";
    Region r;
    r.name = readString("Region Name : ");
    int n  = readPositiveInt("Number of Cities in this Region: ");
    for (int i = 1; i <= n; ++i)
        r.cities.push_back(enterCity(regionNum, i));
    return r;
}

// ─────────────────────────────────────────────
//  Report Generation
// ─────────────────────────────────────────────

void printCustomerRow(const Customer& c, int idx, const string& indent) {
    cout << indent << "Customer " << idx << ":\n";
    cout << indent << "  Name/ID        : " << c.nameOrID       << "\n";
    cout << indent << "  Balance (ETB)  : " << fixed << setprecision(2) << c.mobileBalance << "\n";
    cout << indent << "  Data (GB)      : " << fixed << setprecision(3) << c.dataUsageGB   << "\n";
    cout << indent << "  Call (mins)    : " << fixed << setprecision(1) << c.callMinutes   << "\n";
    cout << indent << "  SMS            : " << c.smsCount                                   << "\n";
}

void printBranchReport(const Branch& b, const string& indent) {
    cout << indent << "Branch: " << b.name << "\n";
    cout << indent << "  Customers: " << countCustomersInBranch(b) << "\n";
    UsageTotals u = branchUsage(b);
    cout << indent << "  Data (GB) : " << fixed << setprecision(3) << u.dataGB   << "\n";
    cout << indent << "  Call (min): " << fixed << setprecision(1) << u.callMins << "\n";
    cout << indent << "  SMS       : " << u.sms                                  << "\n";
    const Customer* top = topCustomerByBalance(b);
    if (top) {
        cout << indent << "  Top Balance Customer: " << top->nameOrID
             << " (" << fixed << setprecision(2) << top->mobileBalance << " ETB)\n";
    }
    cout << indent << "  ---- Customer Details ----\n";
    for (int i = 0; i < (int)b.customers.size(); ++i)
        printCustomerRow(b.customers[i], i + 1, indent + "  ");
}

void printCityReport(const City& city, const string& indent) {
    int totalCust   = countCustomersInCity(city.branches);
    UsageTotals u   = cityUsage(city.branches);
    int topBranch   = topBranchIdxInCity(city);

    cout << indent << "City: " << city.name << "\n";
    cout << indent << "  Total Customers : " << totalCust                              << "\n";
    cout << indent << "  Total Data (GB) : " << fixed << setprecision(3) << u.dataGB  << "\n";
    cout << indent << "  Total Call (min): " << fixed << setprecision(1) << u.callMins<< "\n";
    cout << indent << "  Total SMS       : " << u.sms                                 << "\n";
    cout << indent << "  Top Branch      : " << city.branches[topBranch].name
         << " (" << countCustomersInBranch(city.branches[topBranch]) << " customers)\n";
    printLine('-', 60);
    for (auto& br : city.branches)
        printBranchReport(br, indent + "  ");
}

void printRegionReport(const Region& r, const string& indent) {
    int totalCust  = countCustomersInRegion(r.cities);
    UsageTotals u  = regionUsage(r.cities);
    int topCity    = topCityIdxInRegion(r);

    cout << indent << "Region: " << r.name << "\n";
    cout << indent << "  Total Customers : " << totalCust                              << "\n";
    cout << indent << "  Total Data (GB) : " << fixed << setprecision(3) << u.dataGB  << "\n";
    cout << indent << "  Total Call (min): " << fixed << setprecision(1) << u.callMins<< "\n";
    cout << indent << "  Total SMS       : " << u.sms                                 << "\n";
    cout << indent << "  Top City (usage): " << r.cities[topCity].name               << "\n";
    printLine('=', 60);
    for (auto& city : r.cities)
        printCityReport(city, indent + "  ");
}

void generateReport(const vector<Region>& regions) {
    printHeader("ETHIOPIAN TELECOM — FULL HIERARCHICAL REPORT");

    int    totalCustomers = countCustomersNationwide(regions);
    UsageTotals natUsage  = nationUsage(regions);
    int    topReg         = topRegionIdx(regions);

    // Count total branches nationwide
    int totalBranches = 0;
    for (auto& r : regions)
        for (auto& c : r.cities)
            totalBranches += (int)c.branches.size();

    // ── National Statistics ──────────────────────────────
    cout << "\n";
    printLine('-');
    cout << "  NATIONAL STATISTICS\n";
    printLine('-');
    cout << "  Total Regions         : " << regions.size()    << "\n";
    cout << "  Total Branches        : " << totalBranches      << "\n";
    cout << "  Total Customers       : " << totalCustomers     << "\n";
    cout << "  Total Data Usage (GB) : " << fixed << setprecision(3) << natUsage.dataGB  << "\n";
    cout << "  Total Call Usage (min): " << fixed << setprecision(1) << natUsage.callMins<< "\n";
    cout << "  Total SMS             : " << natUsage.sms       << "\n";
    cout << "  Top Region (customers): " << regions[topReg].name
         << " (" << countCustomersInRegion(regions[topReg].cities) << " customers)\n";

    // ── Averages (guarded against division by zero) ──────
    if (totalCustomers > 0) {
        // Collect totals for balance averages
        double totalBalance = 0;
        for (auto& r : regions)
            for (auto& c : r.cities)
                for (auto& b : c.branches)
                    for (auto& cu : b.customers)
                        totalBalance += cu.mobileBalance;

        cout << "\n";
        printLine('-');
        cout << "  NATIONAL AVERAGES (per customer)\n";
        printLine('-');
        cout << "  Avg Mobile Balance: " << fixed << setprecision(2)
             << (totalBalance        / totalCustomers) << " ETB\n";
        cout << "  Avg Data Usage    : " << fixed << setprecision(3)
             << (natUsage.dataGB     / totalCustomers) << " GB\n";
        cout << "  Avg Call Usage    : " << fixed << setprecision(1)
             << (natUsage.callMins   / totalCustomers) << " minutes\n";
        cout << "  Avg SMS           : " << fixed << setprecision(1)
             << ((double)natUsage.sms / totalCustomers) << " messages\n";
    }

    // ── Per-Region Detail ────────────────────────────────
    cout << "\n";
    printLine();
    cout << "  REGIONAL DETAIL\n";
    printLine();
    for (auto& r : regions) {
        printRegionReport(r, "  ");
        cout << "\n";
    }

    printHeader("END OF REPORT");
}

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────

int main() {
    printHeader("ETHIOPIAN TELECOM CUSTOMER SERVICE SYSTEM");
    cout << "\nWelcome! Please enter the system configuration.\n\n";

    int numRegions = readPositiveInt("Enter number of Regions: ");

    vector<Region> regions;
    regions.reserve(numRegions);
    for (int i = 1; i <= numRegions; ++i)
        regions.push_back(enterRegion(i));

    cout << "\n\nData entry complete. Generating report...\n\n";
    generateReport(regions);

    return 0;
}
