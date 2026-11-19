# IFJ-compiler

## Build and run tests

The Makefile builds the compiler and runs the tests. The unit tests are built with CMake, which
uses an installed GoogleTest package or downloads GoogleTest on the first configure. A C23/C++20
compiler (e.g. gcc 14), GNU Make, and CMake 3.21 or newer are required on a POSIX-compatible system
(Linux or macOS).

```sh
make            # Build the compiler (./compiler)
make unit-tests # Configure and build all unit test executables
make test       # Build and run all unit tests
make valgrind   # Run all unit tests under Valgrind
make clean      # Remove the compiler, object files and the build directory
```

### End-to-end tests

```sh
make e2e      # Run the end-to-end tests against ./compiler
```

`tests/e2e/cases` holds complete IFJ26 programs whose expected results come only from the
specification (lexical, syntax, semantic and runtime errors, the base language and our extensions).
The runner checks the compiler's exit code; with the interpreter (`IC26INT=/path/to/ic26int`) it
also runs the generated code and compares the output. Rebuild with `make` after changing the code.

## Developer Setup

### Code formatting

This project uses `clang-format` to enforce a consistent C source code style.
The rules will soon be defined in the `.clang-format` file.

**For VS Code users:**

Install the C/C++ extension in VS code and configure `settings.json`
in the `.vscode` folder like this:

```json
{
    "editor.formatOnSave": true
}
```

**For CLI and other editors:**

1. Install the tool using your system's package manager (e.g., on Fedora):

    ```bash
    sudo dnf install clang-tools-extra
    ```


2. Format the desired file by running:

    ```bash
    clang-format -i src/your_file.c
    ```

### Commit linting

This repository uses Commitlint to enforce Conventional Commits formatting.

#### Installation

After cloning the repository, you must activate the local git hooks:

1. Ensure you have Node.js and npm installed (e.g., `sudo dnf install nodejs npm`).
2. Run the following command in the project root:

    ```bash
    npm install
    ```

All the commits must be in the Conventional commits format.
For more details see [Conventional Commits](https://www.conventionalcommits.org/).

#### Commit message format

Every commit message must follow this strict format:

```text
type(scope): subject
```

The **type** must be a standard Conventional Commits type, the **scope** must be from the
`scope-enum` in `commitlint.config.js` (add new ones if needed), and the **subject** is your
mandatory custom description of the change.
