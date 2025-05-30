---

# Qt6 Migration Task Management (Jules Workflow)

This folder contains all configuration and instruction files for guiding the AI assistant, Jules, in the migration of the project from the `wxwidgets` codebase to a Qt6-based application (`project_qt`).

## Workflow Overview (with Jules):

1.  **Initialization & Contextual Understanding:**
    *   Jules **MUST** first parse **`master_prompt.txt`**. This file is its **sole authoritative operational guide**, detailing the precise workflow, interaction model, Git procedures, communication protocols, and how to interpret other configuration files.
    *   Jules then parses **`migration_tasks_config.xml`** to obtain:
        *   The **static, ordered list** of all "Main Task" definitions (ID, title, path to detailed `.md` specification).
        *   General project context (e.g., source/target paths, Git repository details).
        *   Core coding mandates (e.g., file size management rules defined in the XML).

2.  **Main Task Execution Cycle by Jules (Iterative):**
    *   **Task Identification:** Jules identifies the next "Main Task" from `migration_tasks_config.xml` that has not yet been formally approved by the user (based on its internal tracking of approved tasks).
    *   **Preliminary Analysis & Refactoring:**
        *   Jules analyzes the original `wxwidgets` code and any relevant existing code in `Project_QT/src` for the current task.
        *   **Crucially, per `master_prompt.txt` directives, Jules assesses target C++ source/header files for size based on criteria in Mandate M6 of `migration_tasks_config.xml` (e.g., exceeding ~250 lines or substantial changes to already large files). If criteria are met, Jules **MUST** refactor it into smaller, logically coherent modules (`.cpp`/`.h` pairs) to ensure it can operate within its processing limits and maintain code quality. All functionality must be preserved. Jules **MUST** report any new files created using the `NEW_FILES_FOR_CMAKE:` tag in its activity feed and final task summary.**
        *   If pre-refactoring is deemed too complex/risky for the current task, Jules will flag it (using the `DEFERRED_REFACTORING_WARNING:` format) and proceed with caution.
    *   **Core Task Implementation:** Jules implements the main logic of the task, referring to the detailed instructions in the corresponding **`tasks_definitions/[TaskID].md`** file.
    *   **Activity Feed & Iterative Feedback:** Throughout analysis, refactoring, and implementation, Jules provides a real-time activity feed (using structured messages like `DESIGN_CHOICE:`, `CODE_CHANGE_SUMMARY:`, etc.). The user can provide iterative feedback, corrections, and requests via chat, which Jules incorporates after acknowledgement (e.g., `USER_FEEDBACK_ACKNOWLEDGED:`).
    *   **Local Commit:** Upon believing a Main Task is fully complete (including addressing iterative feedback and ensuring 100% feature parity for the task's scope), Jules commits all changes (including any new files from refactoring) to a new local feature branch on its VM using the format specified in `master_prompt.txt`. Jules does *not* push to GitHub.

3.  **User-Facilitated Push to GitHub & Formal Review:**
    *   Jules signals Main Task completion by providing a **structured final task summary** (as defined in `master_prompt.txt`, including TASK_ID, LOCAL_BRANCH, COMMIT_SHA_FULL, FILES_MODIFIED, NEW_FILES_FOR_CMAKE, etc.).
    *   The user (Auditor/Reviewer) then uses the **Jules interface's "Create branch" functionality**. This action pushes Jules's locally committed feature branch (using `LOCAL_BRANCH` and `COMMIT_SHA_FULL` from summary) to the main GitHub repository.
    *   The user creates a Pull Request (PR) on GitHub from this newly pushed branch.
    *   The user reviews the code in the PR.

4.  **Formal Approval or Rework Request by User:**
    *   The user provides formal feedback for the completed Main Task via chat (e.g., `APPROVED:[TaskID]` or `REWORK:[TaskID] [specific feedback]`).
    *   **If `APPROVED`:** Jules internally marks the task complete and proceeds to identify the next pending Main Task.
    *   **If `REWORK`:** Jules re-works the same Main Task on its local feature branch (amending the previous commit), incorporating the formal feedback (still allowing for iterative chat during this rework). It then commits the changes locally and signals readiness (using `TASK_REWORK_COMPLETE:` and an updated summary) for another user-triggered push and re-review (Step 3).

5.  **Cycle Repetition & Project Completion:**
    *   This cycle (Step 2-4) repeats until all Main Tasks defined in `migration_tasks_config.xml` are completed and formally approved.
    *   Jules then provides a final project completion message (e.g., `PROJECT_STATUS: ALL_TASKS_APPROVED`).

## Key Files & Their Roles:

*   **`master_prompt.txt`**:
    *   **The sole and strictly authoritative operational instruction set for the AI assistant (Jules).** It dictates:
        *   The precise, step-by-step workflow for task execution.
        *   The AI's interaction model with the user (Auditor/Reviewer).
        *   Mandatory file handling strategies (especially file size management, refactoring logic, and reporting of new files using `NEW_FILES_FOR_CMAKE:`).
        *   Git procedures on Jules's local VM (branching, committing with specified formats, amending for rework).
        *   Communication protocols (structured messages for activity feed, task completion summaries, warnings like `DEFERRED_REFACTORING_WARNING:`, and acknowledgements like `USER_FEEDBACK_ACKNOWLEDGED:`).
        *   How Jules **MUST** interpret and utilize `migration_tasks_config.xml` (for static definitions) and `tasks_definitions/[TaskID].md` files (for task-specific details).

*   **`migration_tasks_config.xml`**:
    *   Serves as the **static data definition file** for the migration project. Jules uses this to retrieve:
        *   The **complete, ordered list of all "Main Tasks"** including their `id`, `title`, and the path to their `<detailed_spec_file>` (e.g., `tasks_definitions/TaskID.md`).
        *   General project context (e.g., `<project_paths>`, `<git_repository_details>`).
        *   **Definitive `<core_coding_mandates>`, such as Mandate M6 (File Size and Modularity), which details conditions and actions for refactoring large files (now in a structured XML format within this file).**
        *   **Important Note for AI Processing:** The AI (Jules) **MUST NOT** use fields like `current_task_id` (if present) or task `status_initial` values from this XML for dynamic state tracking during its operational workflow. Task progression is managed internally by the AI based on explicit user commands (e.g., `APPROVED:[TaskID]`) as per `master_prompt.txt`.

*   **`tasks_definitions/` directory**:
    *   This directory contains individual Markdown files (e.g., `Task1.md`, `Task2.md`), one for each task ID defined in `migration_tasks_config.xml`.
        *   Each `[TaskID].md` file is the **authoritative source for the detailed technical and functional specifications** for that specific migration task.
        *   Jules **MUST** consult the relevant `[TaskID].md` file when implementing the "main task logic" (Step 3 of the workflow in `master_prompt.txt`) to understand *what* specific functionalities to migrate or implement, and any task-specific constraints or guidelines.

*   **DEPRECATED CONTEXT FILE (If Present):** `repomix-output-tasks.zip.md`.
    *   This file, if it exists in the repository, was likely used for initial, broad context loading for an LLM.
    *   It is **NOT to be used for current task definitions or workflow instructions.**
    *   Authoritative task definitions are SOLELY in `tasks_definitions/[TaskID].md` files, and the authoritative workflow is SOLELY in `master_prompt.txt`.

---
