--- START OF FILE Tasks_List.md ---

# Qt6 Migration Task Management (AI Assistant Workflow)

This folder contains all configuration and instruction files for guiding the AI assistant in the migration of the project from the `wxwidgets` codebase to a Qt6-based application (`project_qt`).

## Workflow Overview (with AI Assistant):

1.  **Initialization & Contextual Understanding:**
    *   The AI Assistant **MUST** first parse **`master_prompt.txt`**. This file is its **sole authoritative operational guide**, detailing the precise workflow, interaction model, file modification procedures, communication protocols, and how to interpret other configuration files.
    *   The AI Assistant then parses **`migration_tasks_config.xml`** to obtain:
        *   The **static, ordered list** of all "Main Task" definitions (ID, title, path to detailed `.md` specification).
        *   General project context (e.g., source/target paths, Git repository details - though the Assistant won't use Git directly).
        *   Core coding mandates (e.g., file size management rules defined in the XML).

2.  **Main Task Execution Cycle by AI Assistant (Iterative):**
    *   **Approval Mode Selection:** At the very beginning, the AI Assistant will ask the user to choose between 'MANUAL' (user explicit approval needed) or 'AUTONOMOUS' (AI will attempt to self-approve and proceed if no explicit rework is given) mode.
    *   **Task Identification:** The AI Assistant identifies the next "Main Task" from `migration_tasks_config.xml` that has a `status` of `PENDING` or `REWORK_REQUESTED` (based on its internal tracking, which relies on the XML status itself).
    *   **Preliminary Analysis & Refactoring:**
        *   The AI Assistant analyzes the original `wxwidgets` code and any relevant existing code in `Project_QT/src` for the current task.
        *   **Crucially, per `master_prompt.txt` directives, the AI Assistant assesses target C++ source/header files for size based on criteria in Mandate M6 of `migration_tasks_config.xml` (e.g., exceeding ~250 lines or substantial changes to already large files). If criteria are met, the AI Assistant **MUST** refactor it into smaller, logically coherent modules (`.cpp`/`.h` pairs) to ensure it can operate within its processing limits and maintain code quality. All functionality must be preserved. The AI Assistant **MUST** report any new files created using the `NEW_FILES_REPORTED:` tag in its activity feed and final task summary.**
        *   If pre-refactoring is deemed too complex/risky for the current task, the AI Assistant will flag it (using the `DEFERRED_REFACTORING_WARNING:` format) and proceed with caution.
    *   **Core Task Implementation:** The AI Assistant implements the main logic of the task, directly modifying files and referring to the detailed instructions in the corresponding **`tasks_definitions/[TaskID].md`** file.
    *   **Activity Feed & Iterative Feedback:** Throughout analysis, refactoring, and implementation, the AI Assistant provides a real-time activity feed (using structured messages like `DESIGN_CHOICE:`, `CODE_CHANGE_SUMMARY:`, etc.). The user can provide iterative feedback, corrections, and requests via chat, which the AI Assistant incorporates after acknowledgement (e.g., `USER_FEEDBACK_ACKNOWLEDGED:`).
    *   **Task Status Update & Sign-off:** Upon believing a Main Task is fully complete (including addressing iterative feedback and ensuring 100% feature parity for the task's scope), the AI Assistant directly updates the `status` attribute of the current task within `tasks/migration_tasks_config.xml` to `COMPLETE_FOR_REVIEW`. It then provides a **structured final task summary** (as defined in `master_prompt.txt`, including TASK_ID, FILES_MODIFIED_DIRECTLY, NEW_FILES_REPORTED, etc.).

3.  **User Review & Formal Approval or Rework Request (and Autonomous Progression):**
    *   The AI Assistant signals Main Task completion by updating the XML status and providing its summary.
    *   **In 'MANUAL' mode:** The user (Auditor/Reviewer) reviews the modified files on their local machine and explicitly responds via chat (`APPROVED:[TaskID]` or `REWORK:[TaskID] [feedback]`).
    *   **In 'AUTONOMOUS' mode:** The AI Assistant will conceptually "wait" for a brief moment for explicit `REWORK` feedback from the user.
        *   If `REWORK:[TaskID]` is provided by the user, the AI Assistant proceeds to rework the task.
        *   If no `REWORK` command is given, the AI Assistant will automatically update the task's `status` to `APPROVED` in `migration_tasks_config.xml` and immediately proceed to identify and start the next pending Main Task.
    *   The user is responsible for committing these changes to version control (e.g., Git) as they see fit.

4.  **Cycle Repetition & Project Completion:**
    *   This cycle repeats until all Main Tasks defined in `migration_tasks_config.xml` are completed and formally approved (i.e., their `status` is `APPROVED`).
    *   The AI Assistant then provides a final project completion message (e.g., `PROJECT_STATUS: ALL_TASKS_APPROVED`).

## Key Files & Their Roles:

*   **`master_prompt.txt`**:
    *   **The sole and strictly authoritative operational instruction set for the AI assistant.** It dictates:
        *   The precise, step-by-step workflow for task execution, including the **initial approval mode selection**.
        *   The AI's interaction model with the user (Auditor/Reviewer), specifying different behaviors for `MANUAL` vs. `AUTONOMOUS` mode.
        *   Mandatory file handling strategies (especially file size management, refactoring logic, and reporting of new files using `NEW_FILES_REPORTED:`).
        *   **Direct file modification procedures (the AI edits files on the user's local machine).**
        *   **How the AI updates the task `status` directly within `migration_tasks_config.xml` based on the selected `APPROVAL_MODE`.**
        *   Communication protocols (structured messages for activity feed, task completion summaries, warnings like `DEFERRED_REFACTORING_WARNING:`, and acknowledgements like `USER_FEEDBACK_ACKNOWLEDGED:`).
        *   How the AI **MUST** interpret and utilize `migration_tasks_config.xml` (for static definitions and dynamic status) and `tasks_definitions/[TaskID].md` files (for task-specific details).

*   **`migration_tasks_config.xml`**:
    *   Serves as the **static data definition file** for the migration project, and **also as the dynamic state tracking mechanism for task progression.** The AI assistant uses this to retrieve:
        *   The **complete, ordered list of all "Main Tasks"** including their `id`, `title`, and the path to their `<detailed_spec_file>` (e.g., `tasks_definitions/TaskID.md`).
        *   General project context (e.g., `<project_paths>`, `<git_repository_details>`).
        *   **Definitive `<core_coding_mandates>`, such as Mandate M6 (File Size and Modularity) and Mandate M7 (Task Status Management), which details conditions and actions for file changes and status updates.**
        *   The current `status` (PENDING, COMPLETE_FOR_REVIEW, REWORK_REQUESTED, APPROVED) for each task. The AI **will directly update this `status` attribute** in the XML file as tasks progress.

*   **`tasks_definitions/` directory**:
    *   This directory contains individual Markdown files (e.g., `Task1.md`, `Task2.md`), one for each task ID defined in `migration_tasks_config.xml`.
        *   Each `[TaskID].md` file is the **authoritative source for the detailed technical and functional specifications** for that specific migration task.
        *   The AI Assistant **MUST** consult the relevant `[TaskID].md` file when implementing the "main task logic" (Step 4 of the workflow in `master_prompt.txt`) to understand *what* specific functionalities to migrate or implement, and any task-specific constraints or guidelines.

*   **DEPRECATED CONTEXT FILE (If Present):** `repomix-output-tasks.zip.md`.
    *   This file, if it exists in the repository, was likely used for initial, broad context loading for an LLM.
    *   It is **NOT to be used for current task definitions or workflow instructions.**
    *   Authoritative task definitions are SOLELY in `tasks_definitions/[TaskID].md` files, and the authoritative workflow is SOLELY in `master_prompt.txt`.