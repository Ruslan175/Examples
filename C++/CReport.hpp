#ifndef _CREPORT
#define _CREPORT

#include <iostream>
#include <string>
#include <vector>

#define INVALID_DEBIT 		(-1)


using namespace std;


class CReport
{
public:
	CReport();
	int create(string path, string beg_dt, string end_dt);
	
private:
	struct tRecord
	{
		string id;
		string date; // like "01.01.2020"
		time_t utc;
		int payment;
	};
	
	enum eRecIdx
	{
		Rec_Id = 0,
		Rec_Date,
		Rec_Payment,
		Rec_Len
	};
	
	typedef vector<tRecord> tRecVec;
	typedef vector<string> tRecRaw;
	
	const string mDateFormat;
	const string mDelimRecord;
	const int mMinValidYear;
	const int mMaxValidYear;
	const int mInvalidDate;
	string mPath;
	string mBegStr;
	string mEndStr;
	time_t mBeg;
	time_t mEnd;
	tRecVec mRecords;

	bool checkTime(const string &dt, time_t &utc);
	void reset();
	bool loadData();
	int applyRawRecord(tRecRaw &bf);
	int runMainTask();
};

#endif