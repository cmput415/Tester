# CMPUT 415 Testing Utility
This tool is meant to be a generic utility for testing solutions to CMPUT 415's
assignments. It can test multiple solutions with multiple toolchains against
a multitude of tests.

 1. [Usage](#usage)
    1. [Running](#running)
    1. [Configuration](#configuration)
       1. [Preparing Tests](#preparing-tests)
       1. [Preparing a Configuration File](#preparing-a-configuration-file)
       1. [Config Properties](#config-properties)
       1. [Automatic Variables](#automatic-variables)
    1. [The TestFile](#testfile)
    1. [Building](#building)

## Usage
### Running
```
tester <json_config> [options]
```

Several useful options below you may find provide great assistance to your development.
#### Flags
  * `-v`,: Print diff plus extra info with increasing levels as specified by additional `v` characters.
  * `-t`, `--time`: Print the time in seconds elapsed while executing the final toolchain step.
  * `-h`, `--help`: List options and flags

#### Options
  * `--package <path>`: Over-rides the test directory specified in the config file for quick debugging of a single test package.
  * `--timeout`: Set the maximum time before a testcase is interrupted and killed.
  * `--log-failures`: Only applicable for grading. Create a log of test cases that fail the solution compiler.

### Configuration
The configuration file specifies the directory of test packages, main executables, and toolchains used to transform the initial test file into output for comparison.

#### Preparing a Configuration File
The configuration file is in JSON format:
```json
{
  "testDir": "<path_to_input>",
  "testedExecutablePaths": {
    "ccid_or_groupid": "<path_to_executable>"
  },
  "runtimes": {
    "ccid_or_groupid": "<path_to_SHARED_library>"
  },
  "toolchains": {
    "toolchain_name": [
      {
        "stepName": "step 1",
        "executablePath": "<path_to_executable>",
        "arguments": ["arg1", "arg2", ...],
        "output": "<output_file_name>",  // Optional: Override use of stdout as input for next command to use a file.
        "usesRuntime": true,             // Optional: Set the LD_PRELOAD and LD_LIBRARY_PATH in the env to runtime
        "usesInStr": true                // Optional: Use the input stream of the testfile -- if it exists.
      }
    ]
  }
}
```
#### Config Properties

* `testDir`: Path to the module contains packages of testfiles.
* `testedExecutablePaths`: A list of executable paths to be tested. Ensure ccid_or_groupid matches your test package name.
* `runtimes`: A list of shared libraries to be loaded before a command is executed. (OPTIONAL)
* `solutionExecutable`: A string indicating which executable among the tested exectuables in the reference solution. (OPTIONAL).
* `toolchains`: A list of toolchains defining steps to transform input files to expected output files.
  * `stepName`: Name of the step (e.g., `generator` or `arm-gcc`).
  * `executablePath`: Path to the executable for this step. Use `$EXE` for the tested executable or $INPUT for the output of the previous step.
  arguments: List of arguments for the executable. `$INPUT` and `$OUTPUT` resolve to input and output files.
  * `output`: Use a named file to feed into the next commands input. Overrides using the stdout produced by the command. Useful for commands for which the output to be further transformed is a file like `Clang` or any of the 415 assignments.
  * `usesRuntime`: Will set environment variables `LD_LIBRARY_PATH` to equal `$RT_PATH` and `LD_PRELOAD` equal to `runtime`. Useful for `llc` and `lli` toolchains respectively. (OPTIONAL)
  * `usesInStr`: Boolean to replace stdin with the file stream from the `testfile`. (OPTIONAL)

#### Automatic Variables
Automatic variables may be provided in the arguments of a toolchain step and are resolved by the tester.
* `$INPUT`: For the first step, `$INPUT` is the testfile. For any following step `$INPUT` is the file alised by previous steps `$OUTPUT`.
* `$OUTPUT`: Refers to the file a successor command will use as `$INPUT`. Defaults to an anonymous file in `/tmp` that is filled with the commands stdout.
            If the `output` property is defined then `$OUTPUT` resolves to the provided file. 
* `$RT_PATH`: Resolves the path of the current `runtime` shared object -- if one is provided. For example given the property: ```runtimes: { /path/lib/libfoo.so }```, `$RT_PATH` resolves to `/path/lib`. This
is useful for providing the dynamic library path at link time to clang when using an `llc` based toolchain for `LLVM`. 
* `$RT_LIB`: Similarly to `$RT_PATH` resolves to the library name of the provided runtime. Acoording to the previous example `$RT_LIB` resolves to `foo`. Also useful in the clang step of an `llc` toolchain. See the runtime tests for a clear example. 

An example setup for running the `SCalc` toolchain with my solution:
Note `mips` has since been depreciated for `riscv` as a backend.
```json
{
  "inDir": "/home/braedy/scalc/tests/input/",
  "outDir": "/home/braedy/scalc/tests/output/",
  "testedExecutablePaths": {
    "braedy": "/home/braedy/scalc/bin/scalc"
  },
  "toolchains": {
    "mips": [
      {
        "stepName": "scalc-MIPS",
        "executablePath": "$EXE",
        "arguments": ["mips", "$INPUT", "$OUTPUT"],
        "output": "mipsOut.s"
      },
      {
        "stepName": "spim",
        "executablePath": "/usr/bin/spim",
        "arguments": ["-file", "$INPUT"]
      },
      {
        "stepName": "tail",
        "executablePath": "/usr/bin/tail",
        "arguments": ["-n +6", "$INPUT"]
      }
    ]
  }
}
```


### Testfile
In a testfile, an input stream and expected output may be supplied within the file inside comments.
All directives are sensitive to whitespace and do not insert newlines between themselves by default. For example, `INPUT: a a a` is equivalent to a file with three whitespace characters, three `'a'` characters and no newline for a total of `6 bytes`.  

 * `INPUT:` Direct a single line of text to `stdin`. Not newline terminated.
 * `INPUT_FILE:` Supply a relative or absolute path to a `.ins` file. Useful if testing for escape characters or long, cumbersome inputs.

 * `CHECK:` Direct a single line of text to `stdout` that the program is expected to output. Not newline terminated.
 * `CHECK_FILE:` Supply a relative or absolute path to a `.out` file.  

Finally, an arbitrary number of `INPUT` adn `CHECK` directives may be supplied in a file, and an `INPUT` and
`INPUT_FILE` directive may not co-exist. 
```
// This is a commnent.
// INPUT:a

procedure main() returns integer {
  character c;
  c <- std_input;
  c -> std_output; 
}

// CHECK:a
```
If you find youreself confused about `INPUT` and `CHECK` semantics look into `/tests` where valid and invalid testfiles can be found. Otherwise, falling back onto `INPUT_FILE` and `CHECK_FILE` is perfectly fine.

#### Preparing Tests
```
└── tests 
  ├── Config.json
  └── testfiles
      ├── package1
      │   ├── io
      │   │   ├── stdin.test
      │   │   ├── stdin.ins
      │   │   └── stdout.test
      │   └── vectors
      │       └── vec.test
      └── package2
          ├── hard-tests
          │   ├── filter.test
          │   ├── matrix.test
          │   └── tuple.test
          └── top_level.test
```

All testfiles are organized in a hierarchy of a single *Module* composed of
one or more *Packages*, which are further composed of *SubPackages*. In the example
above, the `io` and `vectors` are subpackages of `package1` and `hard-tests` is a
subpackage of `package2`. The file `top_level.test` which is not nested in a subpackage
will have one impliciltly created for it.

Submissions typically require one *Package* per student or group, with the directory appropriately named to **student IDs or group IDs**. This is for marking purposes. For example, if my CCID is `braedy`, then my directory tree may look like this for an invididual assignment.
```
└── tests 
  ├── Config.json
  └── testfiles
      └── braedy
          ├── cool-tests
          │   ├── edgecase1.test
          │   └── edgecase2.test
          └── basic.test
```

<br>

### Building
The tester reqiures `C++17` since it uses the `std::filesystem` library extensively. 

#### Linux
```bash
cd $HOME
git clone https://github.com/cmput415/Tester.git
cd Tester
mkdir build && cd build
cmake ..
make
```
Now, to have the tool available to your command line, add the following lines
to the end of `~/.bashrc`.
```bash
# C415 Testing Utility
export PATH="$HOME/Tester/bin/:$PATH"
```