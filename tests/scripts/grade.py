"""
Script which operates on the output json file recieved from running the tester
in grade mode.
"""

import argparse
import json
import pandas as pd
import numpy as np
from typing import Dict, List, Tuple, Any

OUTPUT_CSV="output.csv"

def create_competative_table(toolchain: str, toolchain_results):
    """
    Generate a table for the pass rate.

    TODO: finalize the Competative Testing score system.
    """ 
    n_teams = len(toolchain_results)
    table_dim = n_teams + 2
    df = pd.DataFrame(0, index=range(table_dim), columns=range(table_dim))
    df.at[0, 0] = "D\A"
    
    for i, defense_obj in enumerate(toolchain_results): 
        d_name = defense_obj["defender"]
        d_result = defense_obj["defenderResults"]
        
        i += 1
        df.at[i, 0] = d_name
        for j, attack_obj in enumerate(d_result): 
            j += 1  
            if i == 1:
                df.at[0, j] = attack_obj["attacker"]

            # assign the percentage of tests passed.
            df.at[i, j] = (attack_obj["passCount"] / attack_obj["testCount"])

    df.at[0, table_dim-1] = "defensive points"
    df.at[table_dim-1, 0] = "offensive points"

    # sum the defensive score
    for i in range(1, table_dim-1):
        defense_points = 0 
        for j in range(1, table_dim-1):
            if df.at[i, j] == 1:
                defense_points += 2
            
        df.at[i, table_dim-1] = defense_points

    # sum the offensive score
    for j in range(1, table_dim-1):
        df.at[table_dim-1, j] = (n_teams - df.iloc[1:table_dim-1, j].sum()/n_teams)

    return df

def create_test_summary_table(data) -> pd.DataFrame:
    """
    Create the inital sumamry of team names, testcase counts etc. 
    """
    # teams = [ x['defender'] for x in data[0][1] ] 
    summary = data["testSummary"]
    df = pd.DataFrame(None, index=range(3), columns=range(len(summary) + 1))

    df.at[0,0] = "Test Summary"
    df.at[1,0] = "Team Name"
    df.at[2, 0] = "Test Count"

    for i, team in enumerate(summary):
        df.at[1, i+1] = team["team"]
        df.at[2, i+1] = team["testCount"]

    return df

def create_toolchain_summary_table(datal) -> pd.DataFrame:
    """
    Create a summary of all the toolchains 
    """
    df = pd.DataFrame(None, index=range(3), columns=range(3))
    return df

def create_final_summary_table(data) -> pd.DataFrame:
    """
    Create a final summary table and return a dataframe 
    """
    # summary = data["testSummary"]
    df = pd.DataFrame(None, index=range(3), columns=range(3))
    return df

def generate_csv(data):

    ts_table = create_test_summary_table(data) 
    ts_table.to_csv(OUTPUT_CSV, index=False, header=False)

    toolchain_tables = []
    for result in data["results"]:
        toolchain_name = result["toolchain"]
        toolchain_results = result["toolchainResults"]
        
        tc_table = create_competative_table(toolchain_name, toolchain_results)
        print(tc_table) 
        toolchain_tables.append(tc_table) 

    for table in toolchain_tables:
        table.to_csv(OUTPUT_CSV, index=False, header=False, mode="a")

    # toolchain summay table
    tcs_table = create_toolchain_summary_table(data)
    tcs_table.to_csv(OUTPUT_CSV, index=False, header=False, mode="a")

    # final summary table
    fs_table = create_final_summary_table(tcs_table) 
    fs_table.to_csv(OUTPUT_CSV, index=False, header=False, mode="a") 

    return OUTPUT_CSV

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--file', type=str, required=True, help='Path to the JSON file') 
    args = parser.parse_args()
     
    with open(args.file, "r") as file:
        data = json.load(file)
        grade_csv = generate_csv(data)