
import sys
import subprocess
from dbwrapper import sqlite, migration
import time

db = sqlite.Connection("./stats.db").database(sqlite.MASTER_DB)

def init(_):
    with db.table("stats").builder() as builder:
        builder.column("run", db.dtypes.varchar)
        builder.column("total", db.dtypes.float)
        builder.column("ns", db.dtypes.float)

migration.migrate(db, [init])

db.close()

if len(sys.argv) != 2:
    print(f"usage: {sys.argv[0]} [category]")
    sys.exit(1)

def run():
    output = subprocess.check_output(["bin/scratchpad"], stderr=subprocess.STDOUT).decode()
    print(output)
    total, percall = output.splitlines()[-1].split()

    sqlite.Connection("./stats.db").database(sqlite.MASTER_DB).table("stats").insert_one({
        "run": sys.argv[1],
        "total": total,
        "ns": percall
    })

while True:
    try:
        run()
    except subprocess.CalledProcessError as e:
        print(e.output)
        time.sleep(5)
