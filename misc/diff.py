
import sys
from dbwrapper import sqlite

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} [category 1] [category 2]")
    sys.exit(1)

db = sqlite.Connection("./stats.db").database(sqlite.MASTER_DB)

x1 = db.table("stats").select("ns").where(db.table("stats").column("run") == sys.argv[1]).execute()
x2 = db.table("stats").select("ns").where(db.table("stats").column("run") == sys.argv[2]).execute()

from scipy import stats
tStat, pValue = stats.ttest_ind(x1, x2, equal_var = False)
print("P-Value:{0} T-Statistic:{1}".format(pValue,tStat))
