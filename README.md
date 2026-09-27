# IFJ-compiler

## Developer Setup

### Code formatting

This project uses `clang-format` to enforce a consistent C source code style. The rules will soon be defined in the `.clang-format` file.

**For VS Code users:**

Install the C/C++ extension in VS code and configure settings.json in the .vscode folder like this:

    ```json
    {
    "editor.formatOnSave": true,
    }
    ```

**For CLI and other editors**

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

After cloning the repository, you must activate the local git hooks:

1. Ensure you have Node.js and npm installed (e.g., `sudo dnf install nodejs npm`).
2. Run the following command in the project root:

    ```bash
    npm install
    ```
