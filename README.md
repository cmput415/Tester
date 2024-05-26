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
Much of the setup is done in the [configuration file](#configuration), so the
command line is quite simple. All that's required to run is the configuration
file:
```
tester [options] <path_to_config_file>
```
Currently there are a few options available.
 
  * `-q`, `--quiet`: Quiet mode, don't print diffs, only shows failures/
    successes
  
  * `--summary <path_to_file>`: Writes the final summary to the file rather than
    stdout

  * `--file <path_to_test_file_or_dir>` Override the test paths in the configuration and run one
  test (or directory) specifically. Great for debugging. Perserves intermediate toolchain files.

If you forget this, you can always use `-h` or `--help` to see info.

### Testfile
In a testfile, an input stream and expected output may be supplied within the file inside comments.
There are three directives available. All directives are sensitive to whitespace and don't insert newlines between themselves by default. For example, `INPUT: a a a\n` contains three whitespace characters and a newline.  

 * `INPUT:` Define a single line of stdinput. Not newline terminated.

 * `INPUT_FILE:` Supply a relative or absolute filepath to a `.ins` file if using `INPUT` directives
 becomes too cumbersome.

 * `CHECK:` Define a single line of stdouput that the program is expected to output. Not newline terminated.

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
### Configuration
The configuration file drives the testing process by specifying which
executables to test, how to run a test, and what files to test with. Currently,
the testing process consists of three steps. These steps will be explained in
greater detail shortly:
 * Select an input file from the list.
 * Pass the input file through the toolchain to produce an output file.
 * Diff the output file with the expected output file that was paired with the
   input file.

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

#### Preparing a Configuration File
The configuration file is a JSON file with a specific format.
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
        "outputName": "<file_name_for_intermediate_output>",
        "usesRuntime": true,
        "usesInStr": true
      }
    ]
  }
}
```
The above format can look a bit intimidating, but once broken down it makes
much more sense.

The three top level properties `inDir`, `outDir`, and `inStrDir` point to the
input, output, and input stream directories containing your test files. If none
of your tests use stdin then `inStrDir` is not required.

The `testedExecutablePaths` property is a named list of executable paths that
should be tested using the toolchains and tests. If you're testing your own
solution there should only be one entry in this list: your own solution. Make
sure `ccid_or_groupid` matches your test package name: this is used to match
your solution with your tests. You need to be able to pass all of your own
tests!

The `runtimes` property is a named list of _shared_ libraries that can be
loaded before a command is executed. This is useful to load a runtime library
into lli. This property is _not required_.

The `toolchains` property is a named list of toolchains. A toolchain defines how
to take an input file and turn it into the expected output file, assuming the
solution works. Some assignments need only one toolchain (e.g. an interpreter)
while others need more (e.g. interpreter, arm, mips, x86).

A toolchain is a list of steps where the output of the previous step is used
to fuel the current step.
  * `stepName` is the name of the step. Give it something useful like
    `generator` or `arm-gcc`.
  * `executablePath` is the path to the executable to run for this step. The
    magic variable `$EXE` can be used here to signify that you want the tested
    executable or `$INPUT` can be used to try to run the output of the previous
    step as an executable. More commonly, using the absolute path (e.g.
    `/usr/bin/tail`) to a program's executable should be here.
  * `arguments` is the list of arguments to pass to the specified executable.
    Any text argument can be used here and it will be passed as-is when the
    command is run. Additionally, the magic variables `$INPUT` and `$OUTPUT`
    will be automatically resolved to the input and output files of this step
  * `outputName` is the name of the output produced by this step. This will be
    used to replace `$OUTPUT` in arguments or to find the output file if you
    are unable to name output from a step. If `"outputName": "-"` is used then
    stdout will be captured as output and saved into a temporary file to be
    passed to the next step.
  * `usesRuntime` is a boolean that, if `true`, tells the tester to preload the
    runtime library associated with the current executable name. If this
    option is missing or `false` then the runtime will not be loaded.
  * `usesInStr` is a boolean that, if `true`, tells the tester to replace stdin
    with the file stream associated with the matching file in the directory
    at `inStrDir`. If the option is missing or false then stdin will not be
    replaced in this step.

For the first step, `$INPUT` will be resolved to the `.in` file. For the final
step `$OUTPUT` will be compared against the expected output to determine
success.

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
