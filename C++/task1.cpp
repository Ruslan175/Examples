#include "CReport.hpp"

using namespace std;

string file_path = "../db_payment.csv";


int main() 
{
	CReport report;
	// Normal case
	cout << ">>>>>>>> Case_1 - normal work\n";
	if (report.create(file_path, "1.05.2020", "10.5.2020") > 0) cout << ">>> Case 1.1 - OK\n";
	else cout << ">>> Case 1.1 - NOK\n";   
	if (0 == report.create(file_path, "01.07.2020", "10.07.2020")) cout << ">>> Case 1.2 - OK\n";
	else cout << ">>> Case 1.2 - NOK\n";
  
	int r1 = report.create(file_path, "1.05.2020", "10.5.2020");
	int r2 = report.create(file_path, "11.05.2020", "20.5.2020");
	int r3 = report.create(file_path, "1.05.2020", "20.5.2020");
	if (r3 == (r1 + r2)) cout << ">>> Case 1.3 - OK\n";
	else cout << ">>> Case 1.3 - NOK\n";
	
	r1 = report.create(file_path, "1.05.2020", "15.05.2020");
	r2 = report.create(file_path, "7.05.2020", "13.05.2020");
	if (r1 >= r2) cout << ">>> Case 1.4 - OK\n";
	else cout << ">>> Case 1.4 - NOK\n";
  
	// Invalid time period
	cout << "\n>>>>>>>> Case_2 - invalid time period\n";
	if (INVALID_DEBIT == report.create(file_path, "1.13.2020", "10.05.2020"))  cout << ">>> Case 2.1 - OK\n";
	else cout << ">>> Case 2.1 - NOK\n";
	if (INVALID_DEBIT == report.create(file_path, "01.05.2020", "32.05.2020")) cout << ">>> Case 2.2 - OK\n";
	else cout << ">>> Case 2.2 - NOK\n";
	if (INVALID_DEBIT == report.create(file_path, "1.05.1999", "12.05.1999")) cout << ">>> Case 2.3 - OK\n";
	else cout << ">>> Case 2.3 - NOK\n";
	if (INVALID_DEBIT == report.create(file_path, "1.05.2020", "30.05.2102")) cout << ">>> Case 2.4 - OK\n";
	else cout << ">>> Case 2.4 - NOK\n";
	if (INVALID_DEBIT == report.create(file_path, "11.03.2020", "11.02.2020")) cout << ">>> Case 2.5 - OK\n";
	else cout << ">>> Case 2.5 - NOK\n";
  
    // File does not exist
	cout << "\n>>>>>>>> Case_3 - file does not exist\n";
	if (INVALID_DEBIT == report.create("not_exist.csv", "1.05.2020", "10.05.2020"))  cout << ">>> Case 3.1 - OK\n";
	else cout << ">>> Case 3.1 - NOK\n";  

	// No valid records
	cout << "\n>>>>>>>> Case_4 - file has no valid records\n";
	if (INVALID_DEBIT == report.create("..\empty.txt", "1.05.2020", "10.05.2020"))  cout << ">>> Case 4.1 - OK\n";
	else cout << ">>> Case 4.1 - NOK\n";

	return 0;
} 


