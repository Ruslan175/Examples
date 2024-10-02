#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


#define MAX_REC_LEN   		32
#define MIN_REC_LEN			14
#define MAX_DATE_LEN		11
#define INVALID_DATE		(-1)
#define MIN_VALID_YEAR		2000
#define MAX_VALID_YEAR		2100
#define CODE_CHAR_0			0x30 // '0'
#define CODE_CHAR_9			0x39 // '9'
#define RECORDS_PER_LINE	3	// client, date, payment
#define INVALID_DEBIT 		(-1)


struct Record
{
	int id;
	char date[MAX_DATE_LEN]; // like "01.01.2020"
	int utc;
	int payment;
};
typedef struct Record tRecord;

struct Buffer
{
	char str[MAX_REC_LEN];
	int len;
};
typedef struct Buffer tBuffer;

const char *file_path = "../db_payment.csv";
const char Splitter = ';';
const char DT_Splitter = '.';

int createReport(const char *path, const char *beg_dt, const char *end_dt);
bool getFileDataParams(const char *path, int *rec_num);
bool loadFileData(const char *path, int rec_num, tRecord **rec, int *sz);
bool extractRecord(const char *buf, tBuffer *rec_bf);
int getDayShift(tBuffer *date);
bool checkTimePeriod(const char *str, int *days);
int runReportJob(const tRecord *rec, int sz, int beg, int end);



int main()
{
	printf(">>>>>>>> Case_1 - normal work\n");
	if (createReport(file_path, "1.05.2020", "10.5.2020") > 0) printf(">>> Case 1.1 - OK\n");
	else printf(">>> Case 1.1 - NOK\n");  
	if (0 == createReport(file_path, "01.07.2020", "10.07.2020")) printf(">>> Case 1.2 - OK\n");
	else printf(">>> Case 1.2 - NOK\n");
	int r1 = createReport(file_path, "1.05.2020", "10.5.2020");
	int r2 = createReport(file_path, "11.05.2020", "20.5.2020");
	int r3 = createReport(file_path, "1.05.2020", "20.5.2020");
	if (r3 == (r1 + r2)) printf(">>> Case 1.3 - OK\n");
	else printf(">>> Case 1.3 - NOK\n");
	r1 = createReport(file_path, "1.05.2020", "15.05.2020");
	r2 = createReport(file_path, "7.05.2020", "13.05.2020");
	if (r1 >= r2) printf(">>> Case 1.4 - OK\n");
	else printf(">>> Case 1.4 - NOK\n");

	printf("\n>>>>>>>> Case_2 - invalid time period\n");
	if (INVALID_DEBIT == createReport(file_path, "1.13.2020", "10.05.2020"))  printf(">>> Case 2.1 - OK\n");
	else printf(">>> Case 2.1 - NOK\n");
	if (INVALID_DEBIT == createReport(file_path, "01.05.2020", "32.05.2020")) printf(">>> Case 2.2 - OK\n");
	else printf(">>> Case 2.2 - NOK\n");
	if (INVALID_DEBIT == createReport(file_path, "1.05.1999", "12.05.1999")) printf(">>> Case 2.3 - OK\n");
	else printf(">>> Case 2.3 - NOK\n"); 
	if (INVALID_DEBIT == createReport(file_path, "1.05.2020", "30.05.2102")) printf(">>> Case 2.4 - OK\n");
	else printf(">>> Case 2.4 - NOK\n");
	if (INVALID_DEBIT == createReport(file_path, "11.03.2020", "11.02.2020")) printf(">>> Case 2.5 - OK\n");
	else printf(">>> Case 2.5 - NOK\n");

	printf("\n>>>>>>>> Case_3 - file does not exist\n");
	if (INVALID_DEBIT == createReport("not_exist.csv", "1.05.2020", "10.05.2020")) printf(">>> Case 3.1 - OK\n");
	else printf(">>> Case 3.1 - NOK\n"); 

	printf("\n>>>>>>>> Case_4 - file has no valid records\n");
	if (INVALID_DEBIT == createReport("..\\empty.txt", "1.05.2020", "10.05.2020")) printf(">>> Case 4.1 - OK\n");
	else printf(">>> Case 4.1 - NOK\n");
	
    return 0;
}

// Validate all input data and then create the report
int createReport(const char *path, const char *beg_dt, const char *end_dt)
{
	int sum = INVALID_DEBIT;
	tRecord *pRec = NULL;
	do
	{
		int beg = 0;
		if (false == checkTimePeriod(beg_dt, &beg))
		{
			printf("Invalid begin date %s\n", beg_dt);
			break;
		}
		
		int end = 0;
		if (false == checkTimePeriod(end_dt, &end))
		{
			printf("Invalid end date %s\n", end_dt);
			break;
		}
		
		if (end < beg)
		{
			printf("Invalid time period: beg= %s, end= %s\n", beg_dt, end_dt);
			break;
		}
		
		int rec_num = 0;
		if (false == getFileDataParams(path, &rec_num))
		{
			break;
		}
		
		int sz = 0;
		if (false == loadFileData(path, rec_num, &pRec, &sz))
		{
			break;
		}
		
		if (0 == sz)
		{
			printf("No valid records have been found !\n");
			break;
		}
		
		sum = runReportJob(pRec, rec_num, beg, end);
	}
	while(false);
	
	if (NULL != pRec) free(pRec);
	return sum;
}

// Check file and its data, count record number
bool getFileDataParams(const char *path, int *rec_num)
{
	bool ret = false;
	FILE *fptr = fopen(path, "r");
	if (NULL != fptr)
	{
		int sz = 0;
		int cnt = -1;
		bool err = false;
		// Read the content
		char buffer[MAX_REC_LEN];
		while(NULL != fgets(buffer, MAX_REC_LEN, fptr)) 
		{
		  ++cnt;
		  if (0 == cnt) continue; // skip header line
		  sz = strlen(buffer);
		  if (((MAX_REC_LEN - 1) == sz) || (MIN_REC_LEN > sz))
		  {
			printf("File %s has unexpected format\n", path);
			printf("%s\n", buffer);
			err = true;
			break;
		  }
		}
		fclose(fptr);
		if (false == err)
		{
			*rec_num = cnt;
			ret = true;
		}
	}
	else
	{
		printf("File does not exist: %s\n", path);
	}
	return ret;
}

// Create dynamic container and load record data
bool loadFileData(const char *path, int rec_num, tRecord **rec, int *sz)
{
	bool ret = false;
	tRecord *ptr = NULL;
	
	do
	{
		// Create internal container
		ptr = (tRecord*)malloc(rec_num * sizeof(tRecord));
		if (NULL == ptr) 
		{
			printf("Needed dymamic memory is not available\n");
			break;
		}
		
		// Load data from file
		FILE *fptr = fopen(path, "r");
		if (NULL == fptr)
		{
			printf("File %s cannot be read\n", path);
			break;
			
		}
		// Reading the content
		ret = true;
		int idx = 0;
		tBuffer rec_buf[RECORDS_PER_LINE]; // client, date, payment
		char buffer[MAX_REC_LEN];
		fgets(buffer, MAX_REC_LEN, fptr); // just skip header
		while(NULL != fgets(buffer, MAX_REC_LEN, fptr)) 
		{
		  if (true == extractRecord(buffer, rec_buf))
		  {
			 const int utc = getDayShift(&rec_buf[1]);
			 if (INVALID_DATE != utc)
			 {
				 ptr[idx].id = atoi(rec_buf[0].str); // client
				 ptr[idx].payment = atoi(rec_buf[2].str); // payment
				 strcpy(ptr[idx].date, rec_buf[1].str); // date
				 ptr[idx].utc = utc;	// date
				 //printf("%d, %s, %d, %d\n", ptr[idx].id, ptr[idx].date, ptr[idx].utc, ptr[idx].payment);
				 ++idx; 
			 }
			 else
			 {
				 printf("Record with invalid date was skipped: %s\n", buffer);
			 }
		  }
		  else
		  {
			printf("Invalid record was skipped: %s\n", buffer);  
		  }
		}
		fclose(fptr);
		*rec = ptr;
		*sz = idx;
	}
	while(false);
	return ret;
}

// Extract record data from line
bool extractRecord(const char *buf,  tBuffer *rec_bf)
{
	bool ret = true;
	// Clear all vars
	int spl_cnt = 0;
	int cnt = 0;
	for (int i = 0; i < RECORDS_PER_LINE; ++i)
	{
		memset(&rec_bf[i], 0, sizeof(tBuffer));
	}
	// Fill up relevant buffers
	for (int i = 0; i < MAX_REC_LEN; ++i)
	{
		const char ch = buf[i];
		if ((Splitter == ch) || ('\n' == ch))
		{// Switch record item
			++spl_cnt;
			cnt = 0;
		}
		else if (spl_cnt < RECORDS_PER_LINE)
		{	// Record items can have mumbbers or '.' only
			if (((ch >= CODE_CHAR_0) && (ch <= CODE_CHAR_9)) || ((DT_Splitter == ch) && (1 == spl_cnt)))
			{// OK case
				rec_bf[spl_cnt].str[cnt] = ch;
				rec_bf[spl_cnt].len++;
				++cnt;
			}
			else
			{// Incorrect line format
				ret = false;
				break;
			}
		}
		else break; // end of line
	}
	return ret;
}

// Expected data format: [d]d.[m]m.yyyy
// where [x] means x is optional
int getDayShift(tBuffer *date)
{
	int utc = INVALID_DATE;
	const int expected_delimeters = 3;
	if (date->len <= MAX_DATE_LEN)
	{
		int day = 0;
		int month = 0;
		int year = 0;
		int idx = 0;
		int cnt = 0;
		char bf[MAX_DATE_LEN] = {0};
		// Extract values of dd, mm, yyyy 
		int *item[] = {&day, &month, &year};	
		for (int i = 0; i < date->len; ++i)
		{
			const char ch = date->str[i];
			if (DT_Splitter == ch)
			{
				*item[idx] = atoi(bf);
				cnt = 0;
				++idx;
				memset(bf, 0, sizeof(bf));
			}
			else if (idx < expected_delimeters)
			{
				bf[cnt] = ch;
				++cnt;
			}
		}
		*item[idx] = atoi(bf); // get year
		
		do
		{// Validating extracted values
			if ((month < 1) || (month > 12) || (day < 1)) break;
			const int daysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; 
			if (day > daysPerMonth[month - 1]) break; // cases of 31/30/28 days
			if ((year < MIN_VALID_YEAR) || (year > MAX_VALID_YEAR)) break;
			utc = (year - MIN_VALID_YEAR) * 365 + month * 31 + day;
		}
		while(false);
	}
	return utc;
}

bool checkTimePeriod(const char *str, int *days)
{
	int len = strlen(str);
	*days = INVALID_DATE;
	if (MAX_DATE_LEN >= len)
	{
		tBuffer dt;
		dt.len = len;
		strcpy(dt.str, str);
		*days = getDayShift(&dt);
	}
	return (INVALID_DATE != *days);
}

int runReportJob(const tRecord *rec, int sz, int beg, int end)
{
	int sum = 0;
	tRecord *ptr_report = (tRecord*)malloc(sz * sizeof(tRecord));
	if (NULL != ptr_report)
	{
		int len = 0;
		memset(ptr_report, 0, sz * sizeof(tRecord));
		for (int i = 0; i < sz; ++i)
		{
			tRecord r = rec[i];
			if ((r.utc >= beg) && (r.utc <= end))
			{
				sum += r.payment;
				// group by client_id, get total payments for each group
				int j = 0;
				for (; j < len; ++j)
				{
					if (r.id == ptr_report[j].id)
					{ // Update report item
						ptr_report[j].payment += r.payment;
						break;
					}
				}
				if (len == j)
				{ // Add new report item
					ptr_report[len].id = r.id;
					ptr_report[len].payment = r.payment;
					++len;
				}					
			}
		}
		// Output the report
		printf("\n\t R E P O R T\n");
		for (int i = 0; i < len; ++i)
		{
			printf("client_id = %d: payment is %d\n", ptr_report[i].id, ptr_report[i].payment);
		}
		printf("Total profit is %d\n", sum);
		free(ptr_report);
	}
	else
	{
		printf("Memory heap is over. The report canot be done\n");
		sum = INVALID_DEBIT;
	}
	return sum;
}