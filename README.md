# mysh
### Prerequisites
- gcc (or equivalent C compiler)
- flex
- bison
- (optionally) GNU make

### Build
`make` builds the shell into `./mysh`. `make debug` builds the shell with verbose debugging output enabled.

### Run
`./mysh <file>` runs the shell with `file` as input. `./mysh` run without an argument uses stdin as input. `./mysh -c <command>` executes `<command>`.

### Test
`make test` runs all tests in the project. `make test1` runs only phase 1 tests.
