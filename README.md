# CMPUT 415 Testing Utility
This tool is meant to be a generic utility for testing solutions to CMPUT 415's
assignments. It can test multiple solutions with multiple toolchains against
a multitude of tests.

 1. [Usage](#usage)
    1. [Running](#running)
    1. [The TestFile](#testfile)
    1. [Configuration](#configuration)
       1. [Preparing Tests](#preparing-tests)
       1. [Preparing a Configuration File](#preparing-a-configuration-file)
    1. [Building](#building)
       1. [Linux](#linux)
       1. [MacOS](#macos)

## Usage
### Running
```
tester [options] <json_config>
```

Several useful options below you may find provide great assistance to your development.
#### Flags
  * `-q`, `--quiet`: Quiet mode, don't print diffs, only shows failures/ successes
  * `-v`, `--verbose`: Verbose mode, print out diffs and
  * `-h`, `--help`: List options and flags
  * `-t`, `--timings`: Print the time in seconds an exectable elapses beside its pass or failure message.

#### Options
  * `--debug-path <path>`: Over-rides the test directory specified in the config file for quick
  debugging on a subset of the test-suite.
  * `--summary <file_path>`: Writes the final summary to the file rather than
    stdout
  * `--file <path>` Override the test paths in the configuration and run one
  test (or directory) specifically. Great for debugging. Perserves intermediate toolchain files.

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
        "arguments": [
          "arg1",
          "arg2"
        ],
        "output": "<file_name_for_intermediate_output>",
        "usesRuntime": true,
        "usesInStr": true
      }
    ]
  }
}
```

#### Properties

* `testDir`: Path to the module contains packages of testfiles.

* `testedExecutablePaths`: A list of executable paths to be tested. Ensure ccid_or_groupid matches your test package name.
* `runtimes`: A list of shared libraries to be loaded before a command is executed (optional).

* `toolchains`: A list of toolchains defining steps to transform input files to expected output files.
  * `stepName`: Name of the step (e.g., `generator` or `arm-gcc`).
  * `executablePath`: Path to the executable for this step. Use `$EXE` for the tested executable or $INPUT for the output of the previous step.
  arguments: List of arguments for the executable. `$INPUT` and `$OUTPUT` resolve to input and output files.
  * `output`: Name of the output produced by this step. Use `"output": "-"` to capture stdout and save it to a temporary file.
  * `usesRuntime`: Boolean to preload the runtime library (optional).
  * `usesInStr`: Boolean to replace stdin with the file stream from the `testfile`.

  For the first step, $INPUT is the testfile. For the final step, $OUTPUT is compared to the expected output to determine success.

An example setup for running the SCalc mips toolchain with my solution:
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
        "arguments": [
          "mips",
          "$INPUT",
          "$OUTPUT"
          ],
        "output": "mipsOut.s"
      },
      {
        "stepName": "spim",
        "executablePath": "/usr/bin/spim",
        "arguments": [
          "-file",
          "$INPUT"
        ],
        "output": "-"
      },
      {
        "stepName": "tail",
        "executablePath": "/usr/bin/tail",
        "arguments": [
          "-n +6",
          "$INPUT"
        ],
        "output": "-"
      }
    ]
  }
}
```


### Testfile
In a testfile, an input stream and expected output may be supplied within the file inside comments.
There are three directives available. All directives are sensitive to whitespace and don't insert newlines between themselves by default. For example, `INPUT: a a a\n` contains three whitespace characters and a newline.  

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

All `.test` files are organized in a hierarchy of a single *Module* composed of
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
The tool makes use of the
[filesystem](https://en.cppreference.com/w/cpp/experimental/fs) library that
was merged into C++17 but has been experimental since C++11. Since the tool is
built using the C++11 standard it is necessary to have the experimental headers
available. On Linux they are readily available but MacOS does not provide them
by with the default library headers. While it is [possible to build
libc++](https://libcxx.llvm.org/docs/BuildingLibcxx.html) and link it manually,
building on MacOS can be achieved much more easily by installing GCC through
brew and using that to compile.

#### Linux
```bash
cd $HOME
git clone https://github.com/cmput415/Tester.git
cd Tester
mkdir build
cd build
cmake ..
make
```
Now, to have the tool available to your command line, add the following lines
to the end of `~/.bashrc`.
```bash
# C415 Testing Utility
export PATH="$HOME/Tester/bin/:$PATH"
```

#### MacOS
At the time of writing this, GCC 8.1.0 was installed by `brew install gcc`, so
`gcc-8` and `g++-8` were available.
```bash
brew install gcc
cd $HOME
git clone https://github.com/cmput415/Tester.git
cd Tester
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER="g++-8" -DCMAKE_C_COMPILER="gcc-8"
make
```
Now, to have the tool available to your command line, add the following lines
to the end of `~/.bash_profile`.
```bash
# C415 Testing Utility
export PATH="$HOME/Tester/bin/:$PATH"
```
