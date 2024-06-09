"""
Script which operates on the output json file recieved from running the tester
in grade mode.
"""

import argparse
import json
import pandas as pd
import numpy as np
from fractions import Fraction
from typing import Dict, List, Tuple, Any

OUTPUT_CSV="output.csv"
MIN_COLUMNS=7
DEFENSE_POINT_SCORE=2

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

    for i, team in enumerate(summary):
        df.at[1, i+1] = team["team"]
        df.at[2, i+1] = team["testCount"]

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

    for team_idx in range(0, n_teams):
        offensive_avg = 0
        defensive_avg = 0 
        coherence_avg = 0 
        for toolchain in toolchains:
            offensive_avg += Fraction(toolchain.at[n_teams+1, team_idx+1])
            defensive_avg += Fraction(toolchain.at[n_teams+2, team_idx+1])
            coherence_avg += Fraction(toolchain.at[n_teams+3, team_idx+1]) 

        tcs_table.at[n_teams+1, team_idx+1] = offensive_avg / len(toolchains)
        tcs_table.at[n_teams+2, team_idx+1] = defensive_avg / len(toolchains)
        tcs_table.at[n_teams+3, team_idx+1] = coherence_avg / len(toolchains)

    return tcs_table

def create_final_summary_table(data) -> pd.DataFrame:
    """
    Create a final summary table and return a dataframe 
    """
    # summary = data["testSummary"]
    df = pd.DataFrame(None, index=range(3), columns=range(3))
    return df

def create_execution_timing_table(results, toolchain, testpackage, n_exe):
    # df = pd.DataFrame(None, index=range(len(testsuite)+1), columns=range(n_teams))
    print("toolchain: ", toolchain)
    print("testpackage: ", testpackage)
    # print(results)

    for defense_obj in results:
        print(defense_obj["defender"])
        for attack_obj in defense_obj["defenderResults"]: 
            if attack_obj["attacker"] == testpackage:
                # print(attack_obj)
                for test in attack_obj["timings"]:
                    print(test)

def generate_csv(data):

    ts_table = create_test_summary_table(data) 
    ts_table.to_csv(OUTPUT_CSV, index=False, header=False)
    insert_blank_row()
    
    toolchain_tables = []
    time_tables = []
    for result in data["results"]:
        toolchain_name = result["toolchain"]
        toolchain_results = result["toolchainResults"]
        
        n_exe = len(toolchain_results)
        n_tests = len(toolchain_results[0]["defenderResults"])

        tc_table = create_competative_table(toolchain_results, n_exe, n_tests)
        time_table = create_execution_timing_table(toolchain_results, toolchain_name, "timed_tests", n_exe)

        toolchain_tables.append(tc_table) 
        time_tables.append(time_table)

        insert_label_row(toolchain_name)
        tc_table.to_csv(OUTPUT_CSV, index=False, header=False, mode="a")
        
        insert_blank_row()
 
    # toolchain summay table
    insert_label_row("Toolchain Summary")
    tcs_table = create_toolchain_summary_table(toolchain_tables, n_exe)
    tcs_table.to_csv(OUTPUT_CSV, index=False, header=False, mode="a")
    insert_blank_row()

    # final summary table
    fs_table = create_final_summary_table(tcs_table) 
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

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--file', type=str, required=True, help='Path to the JSON file') 
    args = parser.parse_args()

    with open(OUTPUT_CSV, "w") as csv:
        # clear the previous file.
        pass

    with open(args.file, "r") as file:
        data = json.load(file)
        grade_csv = generate_csv(data)

        # reload the csv for printing
        df = pd.read_csv(grade_csv)
        print(df.to_string())