# Trivia Quiz Multiplayer

#### Computer Networks Project A.A. 24/25

Trivia Quiz Multiplayer is a multiplayer quiz game developed in C. The project allows multiple players to challenge each other by answering general knowledge questions, with customizable quizzes stored in the `quizzes` folder.

## Features

- **Client-Server Architecture:** Allows multiple user to connect to the same server and play Trivia Quiz
- **I/O Multiplexing:** Utilizes the `select` primitive to ensure maximum scalability of the service.
- **Client Rankings:** Server keeps track of connected users and rankings for each quiz theme.
- **Customizable Quizzes:** Add or modify questions in the `quizzes` folder.
- **Developed in C:** Well-organized source code compiled via a Makefile.
- **Documentation:** Generate technical documentation using Doxygen (configured via the `Doxyfile`).

## Requirements

- **C Compiler:** GCC or a compatible C compiler.
- **Make:** To simplify the compilation process.
- _(Optional)_ **Doxygen:** To generate technical documentation.

## Installation and Compilation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/lorenzo-moni/triva-quiz-multiplayer.git
   ```
2. **Navigate to the project directory:**
   ```bash
   cd triva-quiz-multiplayer
   ```
3. **Compile and start the application:**
   The following script builds the source codes by using make and creates a server instance and two client instances
   ```bash
   ./start.sh
   ```
   Follow the on-screen instructions on one of the client instance to begin a quiz game.

## Documentation

To generate the project's technical documentation:

1. **Ensure Doxygen is installed**
2. **Run the command:**

   ```bash
   doxygen Doxyfile
   ```

   The documentation will be created in the directory specified in the Doxyfile.

## Project Structure

- **src/**: C source code.
- **quizzes/**: Files containing the quizzes.
- **Makefile:** Script to compile the project.
- **start.sh:** Script to launch the game.
- **Doxyfile:** Configuration for generating documentation with Doxygen.
