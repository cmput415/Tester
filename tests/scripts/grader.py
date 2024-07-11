"""
Script which operates on the output json file recieved from running the tester
in grade mode.
"""

import argparse
import json
import pandas as pd
from fractions import Fraction
import numpy as np
# output file
# OUTPUT_CSV="output.csv"
MIN_COLUMNS=6

# score awarded to a team for passing all an attackers tests
DEFENSE_POINT_SCORE=2
COHERENCE_POINT_SCORE=1

#  
TA_PACKAGE="gazprea-solution"

# the test packages to use for timings
TIMED_PACKAGE="timed_tests"

# the list of toolchains for which to record timings
TIMED_TOOLCHAIN="gazprea-opt"

# the executable for which other executables should be compared too
TIMED_EXE_REFERENCE="TA"

TIME_MAX_SECONDS=10

# weights
TA_TEST_WEIGHT = 0.5
COMPETATIVE_WEGIHT = 0.2
TIMING_WEIGHT = 0.1

global data             # the parsed JSON data
global OUTPUT_CSV       # the filename of the output CSV
global n_attackers      # how many test packages are in the tournament (gte n_defenders)
global n_defenders      # how many exectuables are in the tournament (lte n_attackers)

from typing import List

class Attack:
    """
    Represents a test package attacking a single executable.
    """
    def __init__(self, attack_result):
        self.attacker = attack_result["attacker"]
        self.test_count = attack_result["testCount"]
        self.pass_count = attack_result["passCount"]
        self.timings = attack_result["timings"]

    def get_pass_rate(self) -> Fraction:
        return Fraction(self.pass_count, self.test_count)
    
    def __str__(self):
        return f"<Attack attacker={self.attacker}\>"

class Defense:
    """
    Lowers into a single row in the output CSV.
    """
    def __init__(self, defense_result):
        self.defender = defense_result["defender"]
        self.attacks = [ Attack(attack_result) for attack_result in defense_result["defenderResults"] ]
    
    def get_competative_df(self) -> pd.DataFrame:
        df = pd.DataFrame(None, index=range(0), columns=range(len(self.attacks)))
        df.at[0, 0] = self.defender 
        for i, attack in enumerate(self.attacks):
            df.at[0, i+1] = attack.get_pass_rate()
        return df

    def get_timed_tests(self) -> List[str]:
        timed_tests=[] 
        for i, attack in enumerate(self.attacks):
            if attack.attacker == TIMED_PACKAGE:
                for timing in attack.timings:
                    timed_tests.append(timing) 
        return timed_tests

    def get_timings(self):
        timings = []
        for attack in self.attacks:
            if attack.attacker == TIMED_PACKAGE:
                for timing in attack.timings:
                    if timing["time"] == 0:
                        timings.append(TIME_MAX_SECONDS)
                    else:
                        timings.append(timing["time"])
        return timings
    def __str__(self):
        return f"<Defense exe:{self.defender} attackers:{len(self.attacks)} />"

def insert_blank_row():
    """
    Like adding a newline to string, but for CSV.
    """
    df = pd.DataFrame(None, index=range(1), columns=range(MIN_COLUMNS))
    df.to_csv(OUTPUT_CSV, index=False, header=False, mode="a") 

def get_attack_header() -> pd.DataFrame:
    packages = get_attacking_package_names()
    df = pd.DataFrame(None, index=range(0), columns=range(len(packages))) 
    df.at[0, 0] = "D\A"
    for i, pkg in enumerate(packages):
        df.at[0, i+1] = pkg
    return df

def get_timing_tests(toolchain) -> List[str]:
    timed_tests=[]
    for defense_obj in toolchain["toolchainResults"]:
        for attack_obj in defense_obj["defenderResults"]:
            if attack_obj["attacker"] == TIMED_PACKAGE:
                for timing in attack_obj["timings"]:
                    timed_tests.append(timing['test']) 
                return timed_tests
    return timed_tests

def create_toolchain_summary_table(toolchains) -> pd.DataFrame:
    """
    Create a summary of all the toolchains 
    """ 
    # init the summary table as a copy of the first tc
    tcs_table = toolchains[0]

    # add each toolchain on top of the other. This is the first half of the average
    # computation where all numerical value of same coordinates are added
    for tc in toolchains[1:]: 
        tcs_table.iloc[1:,1:] += tc.iloc[1:,1:]

    # normalize each value to compute the average 
    tcs_table.iloc[1:, 1:] = tcs_table.iloc[1:, 1:] / len(toolchains)
    tcs_table.to_csv(OUTPUT_CSV, index=False, header=False, mode="a")
    return tcs_table

def create_toolchain_results_table(name, results) -> pd.DataFrame: 
    
    print(f"Generate a table for toolchain: {name}") 
    assert len(results) > 0, "Need at least one defending executable." 

    df = get_attack_header()

    for defense_result in results:
        row = Defense(defense_result)
        row_df = row.get_competative_df()
        df = pd.concat([df, row_df], ignore_index=True)

    points_df = pd.DataFrame(None, index=range(4), columns=range(n_attackers))
    points_df.iloc[:4, 0] = ["defensive point", "offensive points",
                             "coherence", "total points"];
    
    # calculate each defensive score
    for j in range(1, n_attackers):
        points_df.at[0, j] = (df.iloc[j, 1:] == 1).sum() * DEFENSE_POINT_SCORE

    # calculate each offensive score
    for j in range(1, n_attackers):
        points_df.at[1, j] = (1-df.iloc[1:, j]).sum()

    # give a coherence score
    for j in range(1, n_attackers):
        points_df.at[2, j] = (1 if df.at[j, j] == 1 else 0) * COHERENCE_POINT_SCORE

    # sum total competative points 
    points_df.iloc[3, 1:] = points_df.iloc[:3, 1:].sum()
    df = pd.concat([df, points_df], ignore_index=True)    
    df.to_csv(OUTPUT_CSV, index=False, header=False, mode="a")

    return df

def create_timing_table(timed_toolchain):
    """
    Create a table with a column for each tested executable and a row for each test
    in the testpackage(s) for which timing is desired.
    """
    timed_tests = get_timing_tests(timed_toolchain) 
    timing_df = pd.DataFrame(
        None, index=range(0, len(timed_tests)+1), columns=range(n_attackers))

    timing_df.at[0,0] = "timed_test\D" 
    timing_df.iloc[1:,0] = timed_tests
    for idx, defense_result in enumerate(timed_toolchain["toolchainResults"]):
        defense = Defense(defense_result)
        timing_df.at[0, idx+1] = defense.defender
        timing_df.iloc[1:,idx+1] = defense.get_timings()
        timing_df.iloc[1:,1:] = timing_df.iloc[1:,1:].fillna(0).round(3)

    timing_df.to_csv(OUTPUT_CSV, index=False, header=False, mode='a')
    insert_blank_row()

    # compute the relative timings as normalized by the fastest executable
    rel_timing_df = timing_df 
    for i, test in enumerate(timed_tests):
        fastest_exe = timing_df.iloc[i+1,1:].min() #fastest
        rel_timing_df.iloc[i+1,1:] = (fastest_exe / timing_df.iloc[i+1,1:]) #fastest gets score 1
        rel_timing_df.iloc[i+1,1:] = rel_timing_df.iloc[i+1,1:].fillna(0).round(5)

    rel_total = pd.DataFrame(None, index=range(1), columns=range(n_attackers))
    rel_total.iloc[0,0] = "Timing Score (Average)"
    rel_total.iloc[0,1:] = (
        rel_timing_df.iloc[1:,1:].sum(axis=0) / len(timed_tests)).fillna(0).round(5)
    
    # append the total row to the relative timing row
    rel_timing_df = pd.concat([rel_timing_df, rel_total], ignore_index=True) 
    rel_timing_df.to_csv(OUTPUT_CSV, index=False, header=False, mode='a')
    return rel_timing_df 

def create_test_summary_table():
    """
    Create the inital sumamry of tested packag names and test counts.  
    """
    df = pd.DataFrame(None, index=range(3), columns=range(MIN_COLUMNS))
    df.iloc[:3,0] = ["Test Summary", "Team Name", "Test Count"]
    # record the count and name of each test package 
    pkgs = get_attacking_packages()
    for i, package in enumerate(pkgs):
        df.at[1, i+1] = package["name"]
        df.at[2, i+1] = package["count"]

    df.to_csv(OUTPUT_CSV, index=False, header=False, mode='a')

def create_final_summary_table(toolchain_summary, timing_summary) -> pd.DataFrame:
    """
    Create a final summary table and return a dataframe 
    """
    fst = pd.DataFrame(None, index=range(6), columns=range(n_attackers))
    fst.iloc[:6, 0] = [ "TA Testing (50%)", "Competative Testing (20%)",
                        "Timing Testing (10%)", "Grammar (10%)",
                        "Code Style (10%)", "Final Grade (100%)" ]

    # Get Pass Rate on TA tests.
    first_row = toolchain_summary.iloc[0] # get first row
    index = fst.loc[first_row.str.contains(TA_PACKAGE, na=False)].index 
    ta_pass_rate_col = toolchain_summary.iloc[1:n_attackers, index]    
    fst.iloc[0,1:n_attackers] = (ta_pass_rate_col.T * TA_TEST_WEIGHT).fillna(0).round(5)

    # Get competiative testing scores
    comp_row = toolchain_summary.iloc[n_defenders+4, 1:(1+n_defenders)]
    max_comp_score = comp_row.max() 
    normalized_comp_row = comp_row / max_comp_score 
    fst.iloc[1,1:n_attackers] = (normalized_comp_row * COMPETATIVE_WEGIHT).fillna(0).round(5)

    # Get timing scores
    timing_scores = (timing_summary.iloc[-1,1:] * TIMING_WEIGHT).fillna(0).round(5)
    fst.iloc[2,1:n_attackers] = timing_scores 
    print(timing_scores)

    # Write to CSV 
    fst.to_csv(OUTPUT_CSV, index=False, header=False, mode='a')

    return fst

def fill_csv():
    
    ## STEP 1: initial summary
    create_test_summary_table() 
    insert_blank_row()

    ## STEP 2: toolchain results
    toolchains : List[pd.DataFrame] =[]
    for result in data["results"]:
        toolchain_name = result["toolchain"]
        toolchain_results = result["toolchainResults"]
        tc_df = create_toolchain_results_table(toolchain_name, toolchain_results)
        toolchains.append(tc_df)
        insert_blank_row()

    ## STEP 3: toolchain summary
    tcs = create_toolchain_summary_table(toolchains)
    insert_blank_row()
    
    ## STEP 4: timing results
    timed_toolchain = [tc for tc in data["results"] if tc["toolchain"] == "gazprea"]
    assert len(timed_toolchain), f"Could not find the toolchain supposed to be timed: {TIMED_TOOLCHAIN}"
    rel_timing_table = create_timing_table(timed_toolchain[0])
    insert_blank_row()
    
    ## STEP 5: final summary and grades
    create_final_summary_table(tcs, rel_timing_table)

def get_attacking_packages():
    """
    Returns a list of all attacking packages, sorted by those packages for which
    a corresponding executable exists first. Sorting maintains the symmetry between
    columns and rows in the sheet.
    """
    priority_list = get_defending_executables()
    return sorted(
        data["testSummary"]["packages"],
        key=lambda exe: (exe["name"] not in priority_list)
    )

def get_attacking_package_names():
    return [ pkg["name"] for pkg in get_attacking_packages() ]

def get_defending_executables():
    return sorted(data["testSummary"]["executables"])

def get_student_packages():
    student_exes = get_defending_executables()
    all_packages = get_attacking_package_names()
    student_packages = [ pkg for pkg in all_packages if pkg in student_exes]
    return student_packages

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--file', type=str, required=True, help='Path to the JSON file') 
    args = parser.parse_args()

    # init globals
    with open(args.file, "r") as file:
        data = json.load(file)
    OUTPUT_CSV = "output.csv" 
    n_attackers = len(get_attacking_packages())
    n_defenders = len(get_defending_executables())

    with open(OUTPUT_CSV, "w") as csv:
        fill_csv()  
        df = pd.read_csv(OUTPUT_CSV)
        print(df.to_string())