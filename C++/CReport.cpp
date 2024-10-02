#include "CReport.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <map>

CReport::CReport() :
	mDateFormat("%d.%m.%Y")
	, mDelimRecord(";")
	, mMinValidYear(2000 - 1900)
	, mMaxValidYear(2100 - 1900)
	, mInvalidDate(-1)
{};


void CReport::reset()
{
	mPath.clear();
	mBegStr.clear();
	mEndStr.clear();
	mBeg = mEnd = mInvalidDate;
	mRecords.clear();
};


int CReport::create(string path, string beg_dt, string end_dt)
{
	int sum = INVALID_DEBIT;
	reset();
	
	mPath = path;
	mBegStr = beg_dt;
	mEndStr = end_dt;
	do
	{
		if (false == checkTime(mBegStr, mBeg))
		{
			cout << "Invalid begin date: " << mBegStr <<"\n";
			break;
		}
		if (false == checkTime(mEndStr, mEnd))
		{
			cout << "Invalid end date: " << mEndStr <<"\n";
			break;
		}
		if (mEnd < mBeg)
		{
			cout << "Invalid time period: from " << mBegStr << " to " << mEndStr << "\n";
			break;
		}
		if (false == loadData())
		{
			cout << "File is NOK \n";
			break;
		}
		
		// Debug
		/*
		tRecVec::iterator it = mRecords.begin();
		for(; it != mRecords.end(); ++it)
		{
			cout << it->id << ", " << it->date << ", " << it->utc << ", " << it->payment << "\n";
		}
		*/
		sum = runMainTask();
	}
	while(false);
				
	return sum;
}


bool CReport::checkTime(const string &dt, time_t &utc)
{
	bool ret = false;
	utc = mInvalidDate;
	tm t = {0};
    istringstream ss(dt);
    ss >> get_time(&t, mDateFormat.c_str());
    if ((false == ss.fail()) && (t.tm_year >= mMinValidYear) && (t.tm_year <= mMaxValidYear))
	{
        ret = true;
		utc = mktime(&t);
	}
	return ret;
}

bool CReport::loadData()
{
	bool ret = false;
	ifstream file(mPath);
	if (true == file.is_open())
	{
		int cnt = 0;
		string line;
		tRecRaw buf;
		getline(file, line); // skip header
		while (true == getline(file, line).good())
		{// Read file lines
		  //cout << line << " => "; // debug only
		  size_t pos0 = 0;
		  size_t pos = 0;
		  buf.clear();
		  while (true)
		  {// Split line by mDelimRecord
			  pos = line.find(mDelimRecord, pos0);
			  if (string::npos == pos)
			  {
				  buf.push_back(line.substr(pos0));
				  break;
			  }
			  buf.push_back(line.substr(pos0, pos - pos0));
			  pos0 = pos + 1;
		  }
		  cnt += applyRawRecord(buf); // fill up internal container
		}
		file.close();
		ret = (cnt > 0);
		if (false == ret)
		{
			cout << "File " << mPath << " has no valid records\n";
		}
	}
	else
	{
		cout << "Unable to open " << mPath << "\n";
	}
	return ret;
}


int CReport::applyRawRecord(tRecRaw &bf)
{
	int ret = 0;
	do
	{
		if (eRecIdx::Rec_Len != bf.size()) break;
		tRecord rec;
		// Collect Id
		rec.id = bf[Rec_Id];
		// Collect payment
		try
		{
			rec.payment = stoi(bf[Rec_Payment]);
		}
		catch(...)
		{
			break;
		}
		// Collect date
		time_t t;
		if (false == checkTime(bf[Rec_Date], t)) break;
		rec.date = bf[Rec_Date];
		rec.utc = t;
		
		// Store record to container
		mRecords.push_back(rec);
		ret = 1;
	}
	while(false);

	return ret;
}


int CReport::runMainTask()
{
	typedef map<string, int> tRep;
	int sum = 0;
	tRep rp;
	tRecVec::iterator it = mRecords.begin();
	for(;it != mRecords.end(); ++it)
	{
		if((it->utc >= mBeg) && (it->utc <= mEnd))
		{
			sum += it->payment;
			if (0 < rp.count(it->id))
			{
				rp[it->id] += it->payment;
			}
			else
			{
				rp[it->id] = it->payment;
			}
		}
	}
	
	// Report output
	cout <<  "\n\t R E P O R T\n";
	for (tRep::iterator it=rp.begin(); it != rp.end(); ++it)
		cout << "client_id = " << it->first << ": paid " << it->second << "\n";
	cout << "Total debit is " << sum << "\n";
	return sum;
}