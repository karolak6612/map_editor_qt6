
---

# Qt6 Migration Task Management (Jules Workflow)

This folder contains all configuration and instruction files for guiding the AI assistant, Jules, in the migration of the project from the `wxwidgets` codebase to a Qt6-based application (`project_qt`).

## Workflow Overview (with Jules):

1.  **Initialization & Contextual Understanding:**
    *   Jules begins by parsing **`master_prompt.txt`**. This file is its primary and authoritative operational guide, detailing the precise workflow, interaction model, Git procedures, communication protocols, and how to interpret other configuration files.
    *   Jules then parses **`migration_tasks_config.xml`** to get the complete list of "Main Task" IDs, their brief titles (which point to detailed `.md` files), overall project context (paths, Git details), and core coding mandates.

2.  **Main Task Execution Cycle by Jules (Iterative):**
    *   **Task Identification:** Jules identifies the next "Main Task" from `migration_tasks_config.xml` that has not yet been formally approved by the user (based on its internal tracking of approved tasks).
    *   **Preliminary Analysis & Refactoring:**
        *   Jules analyzes the original `wxwidgets` code and any relevant existing code in `Project_QT/src` for the current task.
        *   **Crucially, per `master_prompt.txt` directives, Jules assesses target C++ source/header files for size. If a file is (or will become) too large (e.g., >200-300 lines), Jules MUST first attempt to refactor it into smaller, logically coherent modules (`.cpp`/`.h` pairs) to ensure it can operate within its processing limits and maintain code quality. All functionality must be preserved. Jules will report any new files created for build system integration.**
        *   If pre-refactoring is deemed too complex/risky for the current task, Jules will flag it and proceed with caution.
    *   **Core Task Implementation:** Jules implements the main logic of the task, referring to the detailed instructions in the corresponding **`tasks_definitions/[TaskID].md`** file.
    *   **Activity Feed & Iterative Feedback:** Throughout analysis, refactoring, and implementation, Jules provides a real-time activity feed. The user can provide iterative feedback, corrections, and requests via chat, which Jules incorporates.
    *   **Local Commit:** Upon believing a Main Task is fully complete (including addressing iterative feedback and ensuring 100% feature parity for the task's scope), Jules commits all changes (including any new files from refactoring) to a new local feature branch on its VM. Jules does *not* push to GitHub.

3.  **User-Facilitated Push to GitHub & Formal Review:**
    *   Jules signals that the Main Task is complete locally and provides a comprehensive task summary (changed files, runtime, lines added/changed, local branch name, commit message, new files from refactoring).
    *   The user (Auditor/Reviewer) then uses the **Jules interface's "Create branch" functionality**. This action pushes Jules's locally committed feature branch to the main GitHub repository.
    *   The user creates a Pull Request (PR) on GitHub from this newly pushed branch.
    *   The user reviews the code in the PR.

4.  **Formal Approval or Rework Request by User:**
    *   The user provides formal feedback for the completed Main Task via chat (e.g., `APPROVED:[TaskID]` or `REWORK:[TaskID] [specific feedback]`).
    *   **If `APPROVED`:** Jules internally marks the task complete and proceeds to identify the next pending Main Task.
    *   **If `REWORK`:** Jules re-works the same Main Task on its local feature branch, incorporating the formal feedback (still allowing for iterative chat during this rework). It then commits the changes locally and signals readiness for another user-triggered push and re-review (Step 3).

5.  **Cycle Repetition & Project Completion:**
    *   This cycle (Step 2-4) repeats until all Main Tasks defined in `migration_tasks_config.xml` are completed and formally approved.
    *   Jules then provides a final project completion message.

## Key Files & Their Roles:

*   **`master_prompt.txt`**:
    *   **The primary and authoritative operational instruction set for the AI assistant (Jules).** It details the precise workflow, interaction model with the user (Auditor/Reviewer), file handling strategies (including mandatory refactoring of large files), Git procedures on Jules's local VM, communication protocols for task completion and feedback, and how Jules should interpret and utilize the other configuration files.

*   **`migration_tasks_config.xml`**:
    *   Provides the **list of all migration tasks ("Main Tasks") with their IDs and brief descriptive titles.** Each title also serves as a **pointer to a more detailed `.md` specification file.**
    *   Contains general project context (paths to `wxwidgets` and `Project_QT/src`, Git repository details) and lists `<core_coding_mandates>` which include the critical directive on file size management and refactoring.
    *   *Note for Jules Workflow:* Fields like `current_task_id` and task `status` attributes within this XML are *not dynamically updated by Jules or the user during the process*. Jules manages task progression internally. These XML fields serve as an initial setup and can be used for external human tracking. The XML version reflects optimizations for the current Jules workflow.

*   **`tasks_definitions/` directory**:
    *   Contains individual Markdown files, one for each task (e.g., `Task1.md`, `Task2.md`, etc.). **These `[TaskID].md` files provide the comprehensive, detailed specifications, context, technical directives, and full instructions for implementing each specific migration task.** Jules is directed by `master_prompt.txt` to use these as the authoritative source for *what* to do for each task.

*   **`repomix-output-tasks.zip.md` (if present/used initially)**:
    *   Likely an initial input file that contained a concatenated representation of the repository, including original (now superseded) task definitions. Its primary role would have been to provide a large initial context to the LLM. The refined task definitions now reside in the `tasks_definitions/[TaskID].md` files.

---
