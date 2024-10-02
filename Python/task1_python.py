import os
import time
from datetime import datetime

file_path = r"../db_payment.csv"
split_char = ";"
date_format = "%d.%m.%Y"
valid_Year_min = 2000 # min valid year
valid_Year_max = 2100 # max valid year
invalid_UTC = -1
record_len = 3 # client_id, date, payment
INVALID_DEBIT = -1


# Used by internal container
class Record:
    # C-tor
    def __init__(self, id, date, payment):
        self.id = id
        self.date = date
        try:
            self.payment = int(payment)
        except ValueError:
            self.payment = 0
            
    # To print object
    def show(self):
        print(self.id + ": " + self.date + "  " + str(self.payment))
        
    # To convert the object date to UTC time
    def getUtc(self):
        ret = 0
        try:
            dt = datetime.strptime(self.date, date_format)
            utc = int(time.mktime(dt.timetuple()))
            ret = utc
        except ValueError:
            ret = invalid_UTC
        return ret


# isDateValid() checks if date is in the expected format / year limits
# dt - date string
def isDateValid(dt):
    ret = False
    try:
        date = datetime.strptime(dt, date_format)
        ret = (date.year >= valid_Year_min) and (date.year <= valid_Year_max)
    except ValueError:
        ret = False
    return ret


# runMainJob() is called to create the report only after passing all validations
# data - internal container filled by data base
# beg_utc, beg_utc - time period in UTC format
def runMainJob(data, beg_utc, end_utc):
    # Execute the main task
    sum = 0
    report = {}
    for i in data:
        utc = i.getUtc()
        if utc >= beg_utc and utc <= end_utc:
            sum += i.payment
            if report.get(i.id):
                report[i.id] += i.payment
            else:
                report[i.id] = i.payment
     
    # Print the result
    print("\n\t R E P O R T")
    for i in report:
        print("client_id = " + i + ": payment is", report[i])
    print("Total profit is", sum, "\n")
    return sum


# createReport() can create the requested report by using data base and time period
# file_loc - relative path to .csv data base
# beg_dt - start of time period
# end_dt - end of time period
def createReport(file_loc, beg_dt, end_dt):
    # Check if the file exist
    if not os.path.exists(file_loc):
        print("File " + file_loc + " does not exist")
        return INVALID_DEBIT
        
    # Check time period
    beg_utc = Record("0", beg_dt, "0").getUtc()
    if False == isDateValid(beg_dt) or invalid_UTC == beg_utc:
        print("Date is invalid", beg_dt)
        return INVALID_DEBIT
    end_utc = Record("0", end_dt, "0").getUtc()
    if False == isDateValid(end_dt) or invalid_UTC == end_utc:
        print("Date is invalid", end_dt)
        return INVALID_DEBIT
    if end_utc < beg_utc:
        print("Time period is inverted: beg " + beg_dt + ", end " + end_dt)
        return INVALID_DEBIT
        
    # Read all file data
    print("Reading data ...")
    file = open(file_loc, 'r')
    Lines = file.readlines()
    file.close()

    # Fill up internal container: [client_id, date, payment]
    data = []
    for i in Lines:
        parts = i[:-1].split(split_char)  # skip "\n"
        if record_len == len(parts):
            if True == isDateValid(parts[1]): 
                data.append(Record(parts[0], parts[1], parts[2])) # use valid records only
                continue
        print("An invalid record '" + i[:-1] + "'")

    # Check record container
    if 0 == len(data): 
        print("No valid records are in the file " + file_loc)
        return INVALID_DEBIT

    # If all is right ...
    print("It has been loaded " + str(len(data)) + " records")
    print("Time period is ", beg_dt, " - ", end_dt) 
    return runMainJob(data, beg_utc, end_utc)



################# Show some use cases
print(">>>>>>>> Case_1 - normal work")
if createReport(file_path, "1.05.2020", "10.5.2020") > 0:
    print(">>> Case 1.1 - OK")
else:
    print(">>> Case 1.1 - NOK")    
if 0 == createReport(file_path, "01.07.2020", "10.07.2020"):
    print(">>> Case 1.2 - OK")
else:
    print(">>> Case 1.2 - NOK")
r1 = createReport(file_path, "1.05.2020", "10.5.2020")
r2 = createReport(file_path, "11.05.2020", "20.5.2020")
r3 = createReport(file_path, "1.05.2020", "20.5.2020")
if  r3 == (r1 + r2):
    print(">>> Case 1.3 - OK")
else:
    print(">>> Case 1.3 - NOK")
r1 = createReport(file_path, "1.05.2020", "15.05.2020")
r2 = createReport(file_path, "7.05.2020", "13.05.2020")
if  r1 >= r2:
    print(">>> Case 1.4 - OK")
else:
    print(">>> Case 1.4 - NOK")     


print("\n>>>>>>>> Case_2 - invalid time period")
if INVALID_DEBIT == createReport(file_path, "1.13.2020", "10.05.2020"):
    print(">>> Case 2.1 - OK")
else:
    print(">>> Case 2.1 - NOK")
if INVALID_DEBIT == createReport(file_path, "01.05.2020", "32.05.2020"):
    print(">>> Case 2.2 - OK")
else:
    print(">>> Case 2.2 - NOK")    
if INVALID_DEBIT == createReport(file_path, "1.05.1999", "12.05.1999"):
    print(">>> Case 2.3 - OK")
else:
    print(">>> Case 2.3 - NOK")    
    
if INVALID_DEBIT == createReport(file_path, "1.05.2020", "30.05.2102"):
    print(">>> Case 2.4 - OK")
else:
    print(">>> Case 2.4 - NOK")
if INVALID_DEBIT == createReport(file_path, "11.03.2020", "11.02.2020"):
    print(">>> Case 2.5 - OK")
else:
    print(">>> Case 2.5 - NOK")    


print("\n>>>>>>>> Case_3 - file does not exist")
if INVALID_DEBIT == createReport("not_exist.csv", "1.05.2020", "10.05.2020"):
    print(">>> Case 3.1 - OK")
else:
    print(">>> Case 3.1 - NOK")     


print("\n>>>>>>>> Case_4 - file has no valid records")
if INVALID_DEBIT == createReport(r"..\empty.txt", "1.05.2020", "10.05.2020"):
    print(">>> Case 4.1 - OK")
else:
    print(">>> Case 4.1 - NOK")     
