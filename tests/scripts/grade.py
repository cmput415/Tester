"""
Script which operates on the output json file recieved from running the tester
in grade mode.
"""

import argparse
import json
import pandas as pd
from fractions import Fraction

# output file
OUTPUT_CSV="output.csv"

# width of the CSV for initialization
MIN_COLUMNS=6

# score awarded to a team for passing all an attackers tests
DEFENSE_POINT_SCORE=2

# the test packages to use for timings
TIMED_PACKAGES=[
    "timed_tests"
]

# the list of toolchains for which to record timings
TIMED_TOOLCHAINS=[
    "LLVM-opt"
]

# the executable for which other executables should be compared too
TIMED_EXE_REFERENCE="TA"

# weights
TA_TEST_WEIGHT = 0.5
COMPETATIVE_WEGIHT = 0.2
TIMING_WEIGHT = 0.1

def create_competative_table(toolchain_results, n_exe, n_tests): 
    """
    Generate a table for the pass rate.
    """
    n_cols = len(toolchain_results)
    n_rows = n_cols
    df = pd.DataFrame(None, index=range(n_exe), columns=range(n_tests))
    df.at[0, 0] = "D\A"
    
    for i, defense_obj in enumerate(toolchain_results): 
        df.at[i+1, 0] = defense_obj["defender"]
        for j, attack_obj in enumerate(defense_obj["defenderResults"]): 
            if i == 0:
                df.at[0, j+1] = attack_obj["attacker"]
            # assign the percentage of tests passed.
            passed=attack_obj["passCount"]
            count=attack_obj["testCount"]
            df.at[i+1, j+1] = f"{passed}/{count}"

    # set up offensive an defensive point rows
    df.at[n_rows+1, 0] = "defensive points"
    df.at[n_rows+2, 0] = "offensive points"
    df.at[n_rows+3, 0] = "coherence"

    # calculate each defensive score
    for i in range(1, n_exe+1):
        defense_points = 0
        for j in range(1, n_tests+1):
            if Fraction(df.at[i, j]) == 1:
                defense_points += DEFENSE_POINT_SCORE
        df.at[n_rows+1, i] = defense_points

    # calculate each offensive score
    for j in range(1, n_tests+1):
        offensive_points = 0
        if j > n_exe:
            # dont calculate points for tests which don't have an exe
            continue 
        for i in range(1, n_exe+1):
            offensive_points += 1 - Fraction(df.at[i, j])    
        df.at[n_rows+2, j] = offensive_points

    # calculate coherence score
    for i in range(1, n_exe+1):
        if Fraction(df.at[i, i]) == 1:
            df.at[n_rows+3, i] = 1
        else:
            df.at[n_rows+3, i] = 0

    return df

def create_test_summary_table(data) -> pd.DataFrame:
    """
    Create the inital sumamry of team names, testcase counts etc. 
    """
    # teams = [ x['defender'] for x in data[0][1] ] 
    summary = data["testSummary"]
    df = pd.DataFrame(None, index=range(1), columns=range(MIN_COLUMNS))

    df.at[0,0] = "Test Summary"
    df.at[1,0] = "Team Name"
    df.at[2, 0] = "Test Count"

    for i, package in enumerate(summary["packages"]):
        df.at[1, i+1] = package["name"]
        df.at[2, i+1] = package["count"]

    return df

def create_toolchain_summary_table(toolchains, n_teams) -> pd.DataFrame:
    """
    Create a summary of all the toolchains 
    """
    tcs_table = toolchains[0] # copy from the first toolchain to retain labels

    for d in range(0, n_teams):
        for a in range(0, n_teams):
            avg_score = 0 
            for toolchain in toolchains:
                avg_score += Fraction(toolchain.at[d+1, a+1])
            avg_score = avg_score / len(toolchains) 
            tcs_table.at[d+1, a+1] = avg_score 

    tcs_table.at[n_teams+4, 0] = "total competative points" 
    for team_idx in range(0, n_teams):
        offensive_sum = 0
        defensive_sum = 0 
        coherence_sum = 0 
        for toolchain in toolchains:
            offensive_sum += Fraction(toolchain.at[n_teams+1, team_idx+1])
            defensive_sum += Fraction(toolchain.at[n_teams+2, team_idx+1])
            coherence_sum += Fraction(toolchain.at[n_teams+3, team_idx+1]) 

        offensive_avg = offensive_sum/ len(toolchains)
        defensive_avg = defensive_sum/ len(toolchains)
        coherence_avg = coherence_sum/ len(toolchains)
        
        tcs_table.at[n_teams+1, team_idx+1] = offensive_avg 
        tcs_table.at[n_teams+2, team_idx+1] = defensive_avg
        tcs_table.at[n_teams+3, team_idx+1] = coherence_avg

        # sum all the averages
        tcs_table.at[n_teams+4, team_idx+1] = offensive_avg + defensive_avg + coherence_avg

    # calculate the competative score as a ratio over the max
    tcs_table.at[n_teams+5, 0] = "competative score" 
    max_competative = tcs_table.iloc[n_teams+4, 1:n_teams+1].max()
    tcs_table.iloc[n_teams+5, 1:n_teams+1] \
        = (tcs_table.iloc[n_teams+4, 1:n_teams+1] / max_competative)

    # calcaulte pass rate for TA tests 
    tcs_table.at[n_teams+6, 0] = "TA test score"
    tcs_table.iloc[n_teams+6, 1:n_teams+1] = tcs_table.iloc[1:n_teams+1, 1].T

    return tcs_table

def create_execution_timing_table(results, n_exe, n_timed_tests):
    """
    Create a table with a column for each tested executable and a row for each test
    in the testpackage(s) for which timing is desired.
    """
    df = pd.DataFrame(None, index=range(n_timed_tests + 2), columns=range(n_exe))
    
    for j, defense_obj in enumerate(results):
        df.at[0, j+1] = defense_obj["defender"]
        for attack_obj in defense_obj["defenderResults"]: 
            if attack_obj["attacker"] in TIMED_PACKAGES:
                for i, test in enumerate(attack_obj["timings"]):
                    df.at[i+1, j+1] = round(test[1], 5) # time of test execution (s)
                    df.at[i+1, 0] = test[0] # name of test

    df.at[n_timed_tests+1, 0] = "Speedup Points"
    
    # Calculate speedup points
    for j in range(0, n_exe):
        speedup_points = 0.0
        for i in range(n_timed_tests):
            ta_time = df.at[i+1, 1]  # TA execution time
            team_time = df.at[i+1, j+1]  # Team execution time

            if pd.notna(ta_time) and pd.notna(team_time) and ta_time != 0:
                speedup_points += ta_time / team_time

        df.at[n_timed_tests+1, j+1] = round(speedup_points, 5)

    # calculate the competative score as a ratio over the max
    df.at[n_timed_tests+2, 0] = "relative timing score" 
    max_timing_score = df.iloc[n_timed_tests+1, 1:n_exe+1].max()

    df.iloc[n_timed_tests + 2, 1:n_exe + 1] = (
        df.iloc[n_timed_tests + 1, 1:n_exe + 1] / max_timing_score
    ).fillna(0).round(4)

    return df

def create_final_summary_table(toolchain_summary, timing_summary, n_exe) -> pd.DataFrame:
    """
    Create a final summary table and return a dataframe 
    """
    df = pd.DataFrame(None, index=range(3), columns=range(n_exe+1))

    df.at[0, 0] = "TA Testing (50%)"
    df.at[1, 0] = "Competative Testing (20%)"
    df.at[2, 0] = "Timing Testing (10%)"
    df.at[3, 0] = "Grammar (10%)"
    df.at[4, 0] = "Code Style (10%)"
    df.at[5, 0] = "Final Grade (100%)"

    df.iloc[0, 1:n_exe+1] = (
        toolchain_summary.iloc[10, 1:n_exe+1] * TA_TEST_WEIGHT
    ).fillna(0).round(4)
    
    df.iloc[1, 1:n_exe+1] = (
        toolchain_summary.iloc[9, 1:n_exe+1] * COMPETATIVE_WEGIHT
    ).fillna(0).round(4)
    
    df.iloc[2, 1:n_exe+1] = (
        timing_summary.iloc[5, 1:n_exe+1] * TIMING_WEIGHT
    ).fillna(0).round(4)

    df.iloc[3, 1:n_exe + 1] = 0
    df.iloc[4, 1:n_exe + 1] = 0 

    df.iloc[5, 1:n_exe + 1] = df.iloc[0:5, 1:n_exe + 1].sum(axis=0) 
    return df

def generate_csv(data):

    ts_table = create_test_summary_table(data) 
    ts_table.to_csv(OUTPUT_CSV, index=False, header=False)
    insert_blank_row()

    # how many tests are timed 
    n_timed_tests = next(
        (pkg for pkg in data["testSummary"]["packages"] if pkg['name'] in TIMED_PACKAGES), None
    )['count']

    toolchain_tables = []
    time_tables = []
    for result in data["results"]:
        toolchain_name = result["toolchain"]
        toolchain_results = result["toolchainResults"]
        
        n_exe = len(toolchain_results)
        n_tests = len(toolchain_results[0]["defenderResults"])

        tc_table = create_competative_table(toolchain_results, n_exe, n_tests) 
        toolchain_tables.append(tc_table) 

        # write the toolchain table to the CSV
        insert_label_row(toolchain_name)
        tc_table.to_csv(OUTPUT_CSV, index=False, header=False, mode="a")
        insert_blank_row()
        
        if toolchain_name in TIMED_TOOLCHAINS: 
            time_table = create_execution_timing_table(toolchain_results, n_exe, n_timed_tests)
            time_tables.append(time_table)

            # write the timing table to the CSV
            insert_label_row(f"{toolchain_name} Timings")
            time_table.to_csv(OUTPUT_CSV, index=False, header=False, mode="a")
            insert_blank_row()
         
    # toolchain summay table
    insert_label_row("Toolchain Summary")
    tcs_table = create_toolchain_summary_table(toolchain_tables, n_exe)
    tcs_table.to_csv(OUTPUT_CSV, index=False, header=False, mode="a")
    insert_blank_row()

    # final summary table

    insert_label_row("Final Grade Summary")
    fs_table = create_final_summary_table(tcs_table, time_tables[0], n_exe) 
    fs_table.to_csv(OUTPUT_CSV, index=False, header=False, mode="a") 

    return OUTPUT_CSV

def insert_label_row(label: str):
    """
    A label row is a dataframe that has one cell at [0,0] that contains a string. 
    """
    df = pd.DataFrame(None, index=range(1), columns=range(1))
    df.at[0,0] = label
    df.to_csv(OUTPUT_CSV, index=False, header=False, mode="a") 

def insert_blank_row():
    """
    Like adding a newline to string, but for CSV.
    """
    df = pd.DataFrame(None, index=range(1), columns=range(MIN_COLUMNS))
    df.to_csv(OUTPUT_CSV, index=False, header=False, mode="a") 

def sort_results(data):
    """
    It is necessary for the order of attacking test packages to "line up" with the executables.
    Specifically, the packages for which a corresponding executable exists must be ordered
    first. 
    """
    summary = data["testSummary"]
    executables = summary["executables"] 
    executable_set = set(executables)
    executable_index = {name: idx for idx, name in enumerate(executables)}

    # sort the packages in the summary to be executables first 
    summary["packages"] = sorted(
        summary["packages"],
        key=lambda pkg: (
            pkg["name"] not in executable_set,
            executable_index.get(pkg["name"], float('inf'))
        )
    )

    # sort the attack order to be packages with corresponding executables first.
    for toolchain_result in data["results"]:
        for defender_result in toolchain_result["toolchainResults"]:
            defender_result["defenderResults"] = sorted(
                defender_result["defenderResults"],
                key=lambda res: (res["attacker"] not in executable_set, executable_index.get(res["attacker"], float('inf')))
            )
 
    return data

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--file', type=str, required=True, help='Path to the JSON file') 
    args = parser.parse_args()

    with open(OUTPUT_CSV, "w") as csv:
        # clear the previous file.
        pass

    with open(args.file, "r") as file:
        data = json.load(file)
        grade_csv = generate_csv(sort_results(data))

        # reload the csv for printing
        df = pd.read_csv(grade_csv)
        print(df.to_string())