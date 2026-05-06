# SRT Editor desktop software

## Project Overview

This project is a professional SRT subtitle editor application.

- Application name: SubtitleEdit Pro
- Domain: Editing `.srt` subtitle files
- Main stack: C++20, Qt6, CMake
- UI framework: Qt Widgets
- Build system: CMake
- Goal: native-feeling, maintainable, testable, safe desktop software

Core features may include:
- Open/save `.srt` files
- Parse subtitle entries
- Display subtitle rows in a table
- Edit start time, end time, duration, and subtitle text
- Validate subtitle timing
- Keep UI state and subtitle model synchronized
- Export valid `.srt` content

## Code Quality Goals

Write professional code that is:

- Clear
- Maintainable
- Testable
- Safe
- Consistent with existing project style
- Suitable for a Qt6 desktop application

Prefer clarity over clever tricks or premature optimization.

## General Coding Standards

- Follow the existing project conventions first.
- Keep formatting consistent.
- Avoid magic numbers and magic strings; define named constants.
- Avoid duplicated logic; extract helper functions or modules.
- Keep code readable before making it shorter.
- Do not introduce unnecessary abstractions.

## C++ / Qt6 Requirements

- Use strong types where useful.
- Use `const` correctness consistently.
- Prefer references over raw pointers when ownership is not transferred.
- Clearly document ownership when using pointers.
- Prefer RAII over manual resource management.
- Avoid raw `new` / `delete` unless required by Qt ownership rules.
- Use Qt parent-child ownership correctly for widgets and QObject-based classes.
- Use `QString`, `QVector`, `QList`, `QDateTime`, `QTime`, etc. consistently with the project style.
- Avoid blocking the UI thread for heavy file parsing or long-running work.
- Validate user input before applying it to the subtitle model.
- Keep UI code separate from subtitle parsing/business logic where practical.

## CMake Requirements

- Keep `CMakeLists.txt` simple and explicit.
- Do not hard-code machine-specific paths.
- Prefer target-based CMake:
  - `target_sources`
  - `target_include_directories`
  - `target_link_libraries`
  - `target_compile_features`
- Do not add global compiler flags unless necessary.
- Respect existing build structure.

## Design Rules

- Each function/class/module should have one clear responsibility.
- Keep functions short and focused.
- Avoid excessive parameters; introduce a struct when needed.
- Separate pure logic from I/O:
  - SRT parsing/formatting should be testable without UI.
  - File reading/writing should be isolated.
  - UI update logic should not contain parsing rules.
- Avoid hidden side effects.
- Inputs and outputs must be explicit.

## Naming

Use clear, meaningful names.

For C++:
- Follow the existing project naming convention.
- If no convention exists:
  - Classes: `PascalCase`
  - Functions/methods: `camelCase` or `snake_case`, but stay consistent
  - Variables: clear descriptive names
  - Constants: `kDescriptiveName` or project convention
- Do not use unclear abbreviations.

Examples:
- Good: `parseSrtTimestamp`, `subtitleEntry`, `validateTimeRange`
- Bad: `doIt`, `tmp`, `x1`, `handleStuff`

## Types and Comments

When writing functions:

- Always specify parameter types and return types.
- Use `const` where applicable.
- Use explicit structs/enums when they improve readability.
- Public APIs must describe:
  - Input
  - Output
  - Possible errors
  - Important assumptions

Comments should explain **why**, not repeat what the code already says.

Do not write noisy comments like:

```cpp
// increment i
++i;
